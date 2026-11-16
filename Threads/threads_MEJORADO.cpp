
#include <chrono>
#include <future>
#include <iostream>
#include <mutex>
#include <random>
#include <system_error>
#include <thread>
#include <vector>
using namespace std;

mutex m_out;
template <class... A> void log(A&&... a) { lock_guard<mutex> l(m_out); (cout << ... << a) << '\n'; }

int trabajo(int id) {
    mt19937 rng(random_device{}() + id);
    int dormir_ms = 100 * uniform_int_distribution<int>(1, 5)(rng);
    log("THREAD ", id, ": inicia, dormira ", dormir_ms, " ms");
    this_thread::sleep_for(chrono::milliseconds(dormir_ms));
    log("THREAD ", id, ": termina");
    return dormir_ms;                               // resultado devuelto al hilo principal
}

int main(int argc, char** argv) {
    int n = argc > 1 ? atoi(argv[1]) : 5;
    if (n < 1) { cerr << "Uso: " << argv[0] << " [hilos>0]\n"; return 1; }
    vector<future<int>> resultados;
    auto t0 = chrono::steady_clock::now();
    try {
        for (int i = 0; i < n; ++i) {
            log("MAIN: creando hilo ", i);
            resultados.push_back(async(launch::async, trabajo, i));
        }
    } catch (const system_error& e) { cerr << "No se pudo crear hilo: " << e.what() << '\n'; }
    log("MAIN: hilos creados");
    int suma = 0;
    for (size_t i = 0; i < resultados.size(); ++i) { suma += resultados[i].get(); log("MAIN: hilo ", i, " finalizo"); }
    double ms = chrono::duration<double, milli>(chrono::steady_clock::now() - t0).count();
    log("MAIN: suma de esperas = ", suma, " ms; tiempo real = ", ms, " ms (ejecucion concurrente)");
}
