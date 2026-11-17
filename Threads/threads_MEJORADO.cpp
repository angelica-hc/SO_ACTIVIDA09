// Threads - versión mejorada y comentada línea por línea.
// Mejoras frente a pthread-example.cpp:
//   - std::async/std::future en lugar de pthread (RAII y resultado del hilo).
//   - Un generador mt19937 por hilo en lugar de rand() compartido.
//   - Salida serializada con mutex para que los mensajes no se mezclen.
//   - Manejo de errores con std::system_error.
// Compilar: g++ -std=c++17 -O2 -Wall -Wextra -pthread threads_MEJORADO.cpp -o threads

#include <chrono>       // sleep_for y medición de tiempos.
#include <future>       // async y future para lanzar tareas y recuperar su resultado.
#include <iostream>     // cout y cerr para la salida.
#include <mutex>        // mutex y lock_guard para serializar la salida.
#include <random>       // mt19937, random_device y uniform_int_distribution.
#include <system_error> // system_error para capturar errores al crear hilos.
#include <thread>       // this_thread::sleep_for.
#include <vector>       // vector para guardar los future.
using namespace std;    // Permite omitir std:: en los tipos usados.

mutex m_out; // Mutex que protege la salida estándar compartida.

// log: imprime todos sus argumentos como una sola línea, bajo el mutex de salida.
template <class... A> void log(A&&... a) { // Plantilla variádica: acepta cualquier cantidad y tipo de argumentos.
    lock_guard<mutex> l(m_out);             // Bloquea la salida durante toda la impresión.
    (cout << ... << a) << '\n';             // Escribe todos los argumentos y agrega un salto de línea.
}

// trabajo: simula una tarea que duerme un tiempo aleatorio y devuelve ese tiempo.
int trabajo(int id) {                                          // Recibe el identificador del hilo.
    mt19937 rng(random_device{}() + id);                       // Generador propio de este hilo, sembrado con entropía e id.
    int dormir_ms = 100 * uniform_int_distribution<int>(1, 5)(rng); // Elige 1–5 y lo multiplica por 100 ms.
    log("THREAD ", id, ": inicia, dormira ", dormir_ms, " ms"); // Informa el inicio de forma atómica.
    this_thread::sleep_for(chrono::milliseconds(dormir_ms));   // Simula el trabajo durmiendo ese tiempo.
    log("THREAD ", id, ": termina");                           // Informa el final de la tarea.
    return dormir_ms;                                          // Devuelve el tiempo dormido al hilo principal.
}

int main(int argc, char** argv) { // Punto de entrada; recibe opcionalmente el número de hilos.
    int n = argc > 1 ? atoi(argv[1]) : 5; // Número de hilos; por defecto 5.
    if (n < 1) {                          // Valida que haya al menos un hilo.
        cerr << "Uso: " << argv[0] << " [hilos>0]\n"; // Muestra el uso correcto.
        return 1;                         // Termina con error si n no es válido.
    }
    vector<future<int>> resultados;       // Guarda un future por cada tarea lanzada.
    auto t0 = chrono::steady_clock::now();// Marca el inicio para comparar espera total vs tiempo real.

    try {                                 // Captura errores del sistema al crear hilos.
        for (int i = 0; i < n; ++i) {     // Lanza n tareas.
            log("MAIN: creando hilo ", i); // Informa la creación del hilo i.
            resultados.push_back(async(launch::async, trabajo, i)); // Ejecuta trabajo(i) en un hilo nuevo.
        }
    } catch (const system_error& e) {     // Si el sistema no puede crear un hilo...
        cerr << "No se pudo crear hilo: " << e.what() << '\n'; // ...muestra el error.
    }

    log("MAIN: hilos creados");           // Informa que terminó la creación de tareas.
    int suma = 0;                         // Acumulador de los tiempos devueltos por cada hilo.

    for (size_t i = 0; i < resultados.size(); ++i) { // Recorre todos los future creados.
        suma += resultados[i].get();                 // Espera el resultado de la tarea i y lo suma.
        log("MAIN: hilo ", i, " finalizo");          // Informa que la tarea i terminó.
    }

    double ms = chrono::duration<double, milli>(chrono::steady_clock::now() - t0).count(); // Tiempo real total en ms.
    log("MAIN: suma de esperas = ", suma, " ms; tiempo real = ", ms, " ms (ejecucion concurrente)"); // Compara suma de esperas con tiempo real.
}
