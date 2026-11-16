
#include <atomic>
#include <chrono>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>
using namespace std;

volatile int sin_proteccion = 0;     // volatile fuerza lectura/escritura real sigue siendo una carrera
int con_mutex = 0;
mutex candado;
atomic<int> con_atomic{0};

template <class F> double medir(int hilos, F f) {
    auto t0 = chrono::steady_clock::now();
    vector<thread> v;
    for (int i = 0; i < hilos; ++i) v.emplace_back(f);
    for (auto& t : v) t.join();
    return chrono::duration<double, milli>(chrono::steady_clock::now() - t0).count();
}

int main(int argc, char** argv) {
    int hilos = argc > 1 ? atoi(argv[1]) : 5;
    int tope = argc > 2 ? atoi(argv[2]) : 1000000;
    if (hilos < 1 || tope < 1) { cerr << "Uso: " << argv[0] << " [hilos>0] [tope>0]\n"; return 1; }
    bool demo = argc > 3 ? atoi(argv[3]) != 0 : true;
    long esperado = static_cast<long>(hilos) * tope;

    double t1 = demo ? medir(hilos, [&] { for (int i = 0; i < tope; ++i) sin_proteccion = sin_proteccion + 1; }) : 0;
    double t2 = medir(hilos, [&] { for (int i = 0; i < tope; ++i) { lock_guard<mutex> l(candado); ++con_mutex; } });
    double t3 = medir(hilos, [&] { for (int i = 0; i < tope; ++i) con_atomic.fetch_add(1, memory_order_relaxed); });

    cout << "Esperado: " << esperado << "\n";
    if (demo) cout << "1) sin proteccion : " << sin_proteccion << (sin_proteccion == esperado ? " (no se noto la carrera)" : "  <-- incrementos perdidos") << "  [" << t1 << " ms]\n";
    cout
         << "2) mutex          : " << con_mutex << (con_mutex == esperado ? "  OK" : "  FALLO") << "  [" << t2 << " ms]\n"
         << "3) atomic         : " << con_atomic.load() << (con_atomic == esperado ? "  OK" : "  FALLO") << "  [" << t3 << " ms]\n";
    return (con_mutex == esperado && con_atomic == esperado) ? 0 : 2;
}
