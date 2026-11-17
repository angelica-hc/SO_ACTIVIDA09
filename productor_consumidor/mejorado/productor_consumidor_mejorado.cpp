// Productor-consumidor - versión mejorada y comentada línea por línea.
// Reemplaza productores00..02, que solo tenían productores y bucles infinitos.
// Solución: búfer acotado implementado como monitor (mutex + 2 variables de condición).
// Compilar: g++ -std=c++17 -O2 -Wall -Wextra -pthread productor_consumidor_mejorado.cpp -o prodcons
// Uso:      ./prodcons [productores=2] [consumidores=2] [capacidad=5] [items_por_productor=20] [verbose=1]

#include <array>             // std::array para contar las letras por productor/consumidor.
#include <condition_variable> // Variables de condición para esperar sin espera activa.
#include <iostream>          // std::cout y std::cerr.
#include <mutex>             // std::mutex, unique_lock y lock_guard.
#include <queue>             // std::queue como almacenamiento del búfer.
#include <random>            // mt19937, random_device y uniform_int_distribution.
#include <thread>            // std::thread para productores y consumidores.
#include <vector>            // std::vector para hilos y contadores.
using namespace std;         // Permite escribir cout, mutex, thread, etc. sin std::.

// BufferAcotado: monitor que encapsula la cola, la capacidad y la sincronización.
template <class T> class BufferAcotado {
    queue<T> q_;                      // Cola interna con los datos producidos y aún no consumidos.
    size_t cap_;                      // Capacidad máxima del búfer.
    bool cerrado_ = false;            // Indica que ya no se producirán más datos.
    mutex m_;                         // Mutex que protege todo el estado del búfer.
    condition_variable no_lleno_, no_vacio_; // Condiciones para esperar por espacio o por datos.

public:
    explicit BufferAcotado(size_t cap) : cap_(cap) {} // Guarda la capacidad máxima indicada.

    // poner: inserta un dato; si el búfer está lleno, espera hasta que haya espacio.
    void poner(T x) {
        unique_lock<mutex> l(m_);     // Bloquea el mutex del búfer con un lock flexible.
        no_lleno_.wait(l, [&] { return q_.size() < cap_; }); // Espera mientras esté lleno; el predicado evita despertares espurios.
        q_.push(x);                   // Inserta el dato en la cola.
        no_vacio_.notify_one();       // Despierta a un consumidor que esté esperando datos.
    }

    // sacar: extrae un dato; devuelve false si el búfer está cerrado y vacío.
    bool sacar(T& x) {
        unique_lock<mutex> l(m_);     // Bloquea el mutex del búfer.
        no_vacio_.wait(l, [&] { return !q_.empty() || cerrado_; }); // Espera hasta que haya datos o se cierre el búfer.
        if (q_.empty()) return false; // Si está vacío y cerrado, no hay nada que consumir.
        x = q_.front();               // Copia el primer dato a la variable de salida.
        q_.pop();                     // Elimina el dato de la cola.
        no_lleno_.notify_one();       // Despierta a un productor que esté esperando espacio.
        return true;                  // Indica que se extrajo un dato correctamente.
    }

    // cerrar: marca el fin de la producción y despierta a todos los consumidores.
    void cerrar() {
        { lock_guard<mutex> l(m_); cerrado_ = true; } // Cambia el estado bajo el mutex.
        no_vacio_.notify_all();                        // Despierta a todos para que vacíen y terminen.
    }
};

mutex m_out; // Mutex que serializa la salida por pantalla cuando verbose está activo.

int main(int argc, char** argv) { // Punto de entrada; recibe parámetros opcionales.
    int P = argc > 1 ? atoi(argv[1]) : 2;     // Número de productores; por defecto 2.
    int C = argc > 2 ? atoi(argv[2]) : 2;     // Número de consumidores; por defecto 2.
    int cap = argc > 3 ? atoi(argv[3]) : 5;   // Capacidad del búfer; por defecto 5.
    int items = argc > 4 ? atoi(argv[4]) : 20; // Ítems por productor; por defecto 20.
    bool verbose = argc > 5 ? atoi(argv[5]) != 0 : true; // Si es true, imprime cada ítem.
    if (P < 1 || C < 1 || cap < 1 || items < 1) { // Valida que todos los parámetros sean positivos.
        cerr << "Parametros deben ser > 0\n";     // Explica el error.
        return 1;                                  // Termina con error.
    }

    BufferAcotado<char> buf(cap);            // Crea el búfer compartido con la capacidad indicada.
    vector<array<int, 26>> prod(P), cons(C); // Contadores por letra: uno por productor y otro por consumidor.
    for (auto& a : prod) a.fill(0);          // Inicializa en cero los contadores de productores.
    for (auto& a : cons) a.fill(0);          // Inicializa en cero los contadores de consumidores.
    vector<thread> hp, hc;                   // Vectores para los hilos productores y consumidores.

    // Lanza los consumidores.
    for (int c = 0; c < C; ++c) {            // Crea C consumidores.
        hc.emplace_back([&, c] {             // Cada consumidor captura el búfer y su índice c.
            char x; int n = 0;               // x recibe el dato; n cuenta los ítems consumidos.
            while (buf.sacar(x)) {           // Mientras haya datos o el búfer no esté cerrado...
                ++cons[c][x - 'a']; ++n;     // ...cuenta la letra consumida y el total.
                if (verbose) {               // Si la salida detallada está activa...
                    lock_guard<mutex> l(m_out); // ...bloquea la salida.
                    cout << "   Consumidor-" << c << " consume: " << x << '\n'; // ...imprime el ítem.
                }
            }
            lock_guard<mutex> l(m_out);      // Bloquea la salida para el mensaje final.
            cout << "Consumidor-" << c << " termino (" << n << " items)\n"; // Informa cuántos consumió.
        });
    }

    // Lanza los productores.
    for (int p = 0; p < P; ++p) {            // Crea P productores.
        hp.emplace_back([&, p] {             // Cada productor captura el búfer y su índice p.
            mt19937 rng(random_device{}() + p); // Generador aleatorio propio de este productor.
            uniform_int_distribution<int> d(0, 25); // Distribución para elegir una letra de la 'a' a la 'z'.
            for (int i = 1; i <= items; ++i) { // Genera exactamente "items" datos.
                char x = 'a' + d(rng);       // Elige una letra aleatoria.
                buf.poner(x);                // Deposita la letra en el búfer (puede esperar si está lleno).
                ++prod[p][x - 'a'];          // Cuenta la letra producida.
                if (verbose) {               // Si la salida detallada está activa...
                    lock_guard<mutex> l(m_out); // ...bloquea la salida.
                    cout << "(" << i << ") Productor-" << p << " produce: " << x << '\n'; // ...imprime el ítem.
                }
            }
        });
    }

    for (auto& t : hp) t.join(); // Espera a que todos los productores terminen.
    buf.cerrar();                // Indica que ya no habrá más producción.
    for (auto& t : hc) t.join(); // Espera a que los consumidores vacíen el búfer y terminen.

    long tp = 0, tc = 0;         // Totales producidos y consumidos.
    bool igual = true;           // Indica si la distribución por letra coincide.
    for (int k = 0; k < 26; ++k) { // Recorre las 26 letras.
        int a = 0, b = 0;          // Acumuladores para la letra k.
        for (auto& x : prod) a += x[k]; // Suma la letra k de todos los productores.
        for (auto& x : cons) b += x[k]; // Suma la letra k de todos los consumidores.
        tp += a; tc += b;               // Acumula los totales generales.
        igual &= (a == b);              // La distribución sigue siendo igual solo si coinciden.
    }
    cout << "Producidos = " << tp << ", consumidos = " << tc << ", distribucion por letra " // Muestra los totales...
         << (igual ? "IDENTICA" : "DISTINTA") << " -> "                                      // ...si la distribución coincide...
         << (tp == (long)P * items && igual ? "OK" : "FALLO") << '\n';                      // ...y el veredicto final.
    return (tp == (long)P * items && igual) ? 0 : 2; // Devuelve 0 si todo es correcto; 2 si falló.
}
