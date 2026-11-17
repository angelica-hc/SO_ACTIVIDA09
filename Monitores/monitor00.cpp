// Monitor 00 - versión corregida y comentada línea por línea.
// El monitor encapsula el contador y su mutex; todos los hilos comparten
// el mismo objeto Monitor. Se usa std::async para lanzar las tareas.
// Compilar: g++ -std=c++17 -O2 -Wall -Wextra -pthread monitor00.cpp -o monitor00

#include <array>     // std::array para guardar las tareas.
#include <cstddef>   // std::size_t.
#include <exception> // std::exception para capturar errores.
#include <future>    // std::async y std::future.
#include <iostream>  // std::cout y std::cerr.
#include <mutex>     // std::mutex y std::lock_guard.

constexpr std::size_t numHilos = 10;      // Cantidad de tareas/hilos que se ejecutarán.
constexpr int incrementoPorHilo = 10;     // Cuánto suma cada tarea al contador.

class Monitor {                            // Monitor: datos privados + operaciones sincronizadas.
public:
    void incrementarEImprimir() {          // Única operación pública del monitor.
        const std::lock_guard<std::mutex> bloqueo(mutex_); // Bloquea el mutex al entrar y lo libera al salir (RAII).
        contador_ += incrementoPorHilo;    // Modifica el contador dentro de la sección crítica.
        std::cout << contador_ << '\n';    // Imprime el valor también dentro de la sección crítica.
    }

private:
    int contador_ = 0;                     // Dato compartido, privado: solo se toca bajo el mutex.
    std::mutex mutex_;                     // Mutex interno que protege contador_ y la impresión.
};

int main() {                               // Punto de entrada del programa.
    Monitor monitor;                       // Un único monitor compartido por todas las tareas.

    try {                                  // Captura errores al lanzar o esperar tareas.
        std::array<std::future<void>, numHilos> tareas; // Un future por cada tarea asíncrona.

        // launch::async obliga a ejecutar cada tarea en un hilo propio.
        for (auto& tarea : tareas) {       // Recorre el arreglo de future.
            tarea = std::async(std::launch::async, [&monitor] { // Lanza una tarea que captura el monitor por referencia.
                monitor.incrementarEImprimir(); // Ejecuta la operación sincronizada del monitor.
            });
        }

        for (auto& tarea : tareas) {       // Recorre nuevamente los future.
            tarea.get();                   // Espera a que la tarea termine y propaga sus excepciones.
        }
    } catch (const std::exception& error) { // Si alguna tarea falla...
        std::cerr << "No se pudieron completar las tareas: " << error.what() << '\n'; // ...muestra el error.
        return 1;                          // Termina con código de error.
    }
}
