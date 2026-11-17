// Monitor 02 - versión corregida y comentada línea por línea.
// El mutex vive dentro del monitor y protege tanto la suma como la
// lectura del valor que se devuelve.
// Compilar: g++ -std=c++17 -O2 -Wall -Wextra -pthread monitor02.cpp -o monitor02

#include <array>     // std::array para resultados e hilos.
#include <mutex>     // std::mutex y std::lock_guard.
#include <cstddef>   // std::size_t.
#include <exception> // std::exception.
#include <iostream>  // std::cout y std::cerr.
#include <thread>    // std::thread.

constexpr std::size_t numHilos = 10;  // Número de hilos.
constexpr int incrementoPorHilo = 10; // Incremento por hilo.

class Monitor {                       // Monitor con mutex interno.
public:
    int incrementar() {               // Suma y devuelve el nuevo valor.
        const std::lock_guard<std::mutex> bloqueo(mutex_); // Bloquea el mutex durante toda la operación.
        contador_ += incrementoPorHilo; // Suma el incremento al contador compartido.
        // El resultado se copia mientras el mutex sigue bloqueado.
        return contador_;             // Devuelve una copia segura del valor.
    }

private:
    int contador_ = 0;                // Contador compartido.
    std::mutex mutex_;                // Mutex que protege el contador.
};

int main() {                          // Punto de entrada.
    Monitor monitor;                  // Monitor compartido.
    std::array<int, numHilos> resultados{}; // Resultados por hilo.
    std::array<std::thread, numHilos> hilos; // Arreglo de hilos.
    bool inicioCompleto = true;       // Indica si todos los hilos se crearon.

    try {                             // Captura errores de creación.
        for (std::size_t i = 0; i < hilos.size(); ++i) { // Recorre los hilos.
            hilos[i] = std::thread([&monitor, &resultados, i] { // Crea el hilo i con capturas por referencia.
                resultados[i] = monitor.incrementar(); // Ejecuta la operación del monitor.
            });
        }
    } catch (const std::exception& error) { // Si falla la creación...
        std::cerr << "No se pudo iniciar un hilo: " << error.what() << '\n'; // ...muestra el error.
        inicioCompleto = false;       // Marca el fallo.
    }

    for (auto& hilo : hilos) {        // Recorre todos los hilos.
        if (hilo.joinable()) {        // Comprueba que el hilo exista.
            hilo.join();              // Espera su finalización.
        }
    }

    if (!inicioCompleto) {            // Si hubo un fallo de creación...
        return 1;                     // ...termina con error.
    }

    for (int resultado : resultados) { // Recorre los resultados.
        std::cout << resultado << '\n'; // Imprime un resultado por línea.
    }
}
