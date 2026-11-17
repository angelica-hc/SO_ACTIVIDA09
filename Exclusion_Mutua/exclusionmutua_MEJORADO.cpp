// Exclusión mutua - versión mejorada y comentada línea por línea.
// Compara tres formas de incrementar un contador compartido:
//   1) sin protección (incorrecta, solo para demostrar la carrera)
//   2) std::mutex + lock_guard (correcta)
//   3) std::atomic (correcta y más rápida para un contador)
// Compilar: g++ -std=c++17 -O2 -Wall -Wextra -pthread exclusionmutua_MEJORADO.cpp -o exclusion

#include <atomic>    // std::atomic: operaciones atómicas sin mutex.
#include <chrono>    // Relojes y duraciones para medir el tiempo de cada estrategia.
#include <iostream>  // cout y cerr para mostrar resultados y errores.
#include <mutex>     // std::mutex y std::lock_guard para exclusión mutua.
#include <thread>    // std::thread para crear los hilos de prueba.
#include <vector>    // std::vector para guardar los hilos.
using namespace std; // Permite escribir cout, mutex, thread, etc. sin std::.

volatile int sin_proteccion = 0; // Contador sin sincronización; volatile obliga a leer/escribir memoria, pero NO evita la carrera.
int con_mutex = 0;               // Contador protegido por el mutex.
mutex candado;                   // Mutex que protege la sección crítica de con_mutex.
atomic<int> con_atomic{0};       // Contador atómico; cada suma es indivisible.

// Función auxiliar: ejecuta f en "hilos" hilos y devuelve los milisegundos transcurridos.
template <class F> double medir(int hilos, F f) { // Plantilla que acepta cualquier función o lambda f.
    auto t0 = chrono::steady_clock::now();         // Guarda el instante inicial de la medición.
    vector<thread> v;                              // Vector que almacenará los hilos creados.
    for (int i = 0; i < hilos; ++i) v.emplace_back(f); // Crea un hilo por iteración; cada uno ejecuta f.
    for (auto& t : v) t.join();                    // Espera a que todos los hilos terminen antes de medir.
    return chrono::duration<double, milli>(chrono::steady_clock::now() - t0).count(); // Devuelve el tiempo en ms.
}

int main(int argc, char** argv) { // Punto de entrada; recibe parámetros opcionales.
    int hilos = argc > 1 ? atoi(argv[1]) : 5;         // Número de hilos; por defecto 5.
    int tope = argc > 2 ? atoi(argv[2]) : 1000000;    // Iteraciones por hilo; por defecto 1 000 000.
    if (hilos < 1 || tope < 1) {                      // Valida que ambos parámetros sean positivos.
        cerr << "Uso: " << argv[0] << " [hilos>0] [tope>0]\n"; // Muestra el uso correcto del programa.
        return 1;                                     // Termina con error si la entrada no es válida.
    }
    bool demo = argc > 3 ? atoi(argv[3]) != 0 : true; // Indica si se ejecuta la variante insegura (por defecto sí).
    long esperado = static_cast<long>(hilos) * tope;  // Resultado correcto esperado: hilos × iteraciones.

    // Estrategia 1: incremento sin protección. Es una carrera de datos intencional.
    double t1 = demo ? medir(hilos, [&] { for (int i = 0; i < tope; ++i) sin_proteccion = sin_proteccion + 1; }) : 0; // Lee, suma y escribe sin bloquear.

    // Estrategia 2: mutex. lock_guard bloquea al entrar y libera automáticamente al salir.
    double t2 = medir(hilos, [&] { for (int i = 0; i < tope; ++i) { lock_guard<mutex> l(candado); ++con_mutex; } }); // Sección crítica protegida.

    // Estrategia 3: atomic. fetch_add realiza la suma de forma indivisible.
    double t3 = medir(hilos, [&] { for (int i = 0; i < tope; ++i) con_atomic.fetch_add(1, memory_order_relaxed); }); // Suma atómica con orden relajado.

    cout << "Esperado: " << esperado << "\n"; // Muestra el total que debería obtenerse.

    if (demo) { // Solo imprime la estrategia insegura si se ejecutó.
        cout << "1) sin proteccion : " << sin_proteccion                       // Muestra el total obtenido sin protección.
             << (sin_proteccion == esperado ? " (no se noto la carrera)" : "  <-- incrementos perdidos") // Indica si se perdieron incrementos.
             << "  [" << t1 << " ms]\n";                                      // Muestra el tiempo empleado.
    }
    cout << "2) mutex          : " << con_mutex                            // Muestra el total con mutex.
         << (con_mutex == esperado ? "  OK" : "  FALLO")                   // Comprueba que sea el esperado.
         << "  [" << t2 << " ms]\n";                                      // Muestra el tiempo del mutex.
    cout << "3) atomic         : " << con_atomic.load()                    // Muestra el total atómico.
         << (con_atomic == esperado ? "  OK" : "  FALLO")                  // Comprueba que sea el esperado.
         << "  [" << t3 << " ms]\n";                                      // Muestra el tiempo del atomic.

    return (con_mutex == esperado && con_atomic == esperado) ? 0 : 2; // Éxito solo si mutex y atomic son correctos.
}
