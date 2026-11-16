// Productor-Consumidor - versión mejorada (reemplaza productores00..02)
// Compilar: g++ -std=c++17 -O2 -Wall -Wextra -pthread productor_consumidor_mejorado.cpp -o prodcons
// Uso:      ./prodcons [productores=2] [consumidores=2] [capacidad=5] [items_por_productor=20] [verbose=1]
//
// Problemas de los originales: había productores pero NINGÚN consumidor ni búfer; bucles
// infinitos (nunca terminan); rand() compartido y no thread-safe; printf entrelazado;
// productores02 hacía new sin delete y lanzaba el hilo dentro del constructor.
// Solución: búfer acotado = MONITOR (mutex + 2 variables de condición):
//   - productor espera si el búfer está lleno (no_lleno)
//   - consumidor espera si está vacío (no_vacio)
//   - cerrar() avisa que no habrá más datos -> los consumidores terminan sin interbloqueo
#include <array>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <queue>
#include <random>
#include <thread>
#include <vector>
using namespace std;

template <class T> class BufferAcotado {
    queue<T> q_;
    size_t cap_;
    bool cerrado_ = false;
    mutex m_;
    condition_variable no_lleno_, no_vacio_;
public:
    explicit BufferAcotado(size_t cap) : cap_(cap) {}
    void poner(T x) {
        unique_lock<mutex> l(m_);
        no_lleno_.wait(l, [&] { return q_.size() < cap_; });   // predicado: evita despertares espurios
        q_.push(x);
        no_vacio_.notify_one();
    }
    bool sacar(T& x) {                                          // false => cerrado y vacío
        unique_lock<mutex> l(m_);
        no_vacio_.wait(l, [&] { return !q_.empty() || cerrado_; });
        if (q_.empty()) return false;
        x = q_.front(); q_.pop();
        no_lleno_.notify_one();
        return true;
    }
    void cerrar() { { lock_guard<mutex> l(m_); cerrado_ = true; } no_vacio_.notify_all(); }
};

mutex m_out;
int main(int argc, char** argv) {
    int P = argc > 1 ? atoi(argv[1]) : 2, C = argc > 2 ? atoi(argv[2]) : 2;
    int cap = argc > 3 ? atoi(argv[3]) : 5, items = argc > 4 ? atoi(argv[4]) : 20;
    bool verbose = argc > 5 ? atoi(argv[5]) != 0 : true;
    if (P < 1 || C < 1 || cap < 1 || items < 1) { cerr << "Parametros deben ser > 0\n"; return 1; }

    BufferAcotado<char> buf(cap);
    vector<array<int, 26>> prod(P), cons(C);
    for (auto& a : prod) a.fill(0);
    for (auto& a : cons) a.fill(0);
    vector<thread> hp, hc;

    for (int c = 0; c < C; ++c)
        hc.emplace_back([&, c] {
            char x; int n = 0;
            while (buf.sacar(x)) {
                ++cons[c][x - 'a']; ++n;
                if (verbose) { lock_guard<mutex> l(m_out); cout << "   Consumidor-" << c << " consume: " << x << '\n'; }
            }
            lock_guard<mutex> l(m_out); cout << "Consumidor-" << c << " termino (" << n << " items)\n";
        });
    for (int p = 0; p < P; ++p)
        hp.emplace_back([&, p] {
            mt19937 rng(random_device{}() + p);
            uniform_int_distribution<int> d(0, 25);
            for (int i = 1; i <= items; ++i) {
                char x = 'a' + d(rng);
                buf.poner(x); ++prod[p][x - 'a'];
                if (verbose) { lock_guard<mutex> l(m_out); cout << "(" << i << ") Productor-" << p << " produce: " << x << '\n'; }
            }
        });

    for (auto& t : hp) t.join();
    buf.cerrar();                                   // ya no habrá más producción
    for (auto& t : hc) t.join();

    long tp = 0, tc = 0; bool igual = true;
    for (int k = 0; k < 26; ++k) {
        int a = 0, b = 0;
        for (auto& x : prod) a += x[k];
        for (auto& x : cons) b += x[k];
        tp += a; tc += b; igual &= (a == b);
    }
    cout << "Producidos = " << tp << ", consumidos = " << tc << ", distribucion por letra "
         << (igual ? "IDENTICA" : "DISTINTA") << " -> " << (tp == (long)P * items && igual ? "OK" : "FALLO") << '\n';
    return (tp == (long)P * items && igual) ? 0 : 2;
}
