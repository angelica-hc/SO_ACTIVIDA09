// Monitor 04 - versión corregida y comentada línea por línea.
// Separa la creación del hilo en una clase Hilo y garantiza que el
// monitor exista antes de lanzar los hilos y hasta que todos terminen.
// Compilar: g++ -std=c++17 -O2 -Wall -Wextra -pthread monitor04.cpp -o monitor04

#include <array>     // std::array para los hilos.
#include <cstddef>   // std::size_t.
#include <exception> // std::exception.
#include <iostream>  // std::cout y std::cerr.
#include <mutex>     // std::mutex y std::lock_guard.
#include <thread>    // std::thread.

constexpr std::size_t numHilos = 10;  // Cantidad de hilos.
constexpr int incrementoPorHilo = 10; // Incremento por hilo.

class Monitor {                       // Monitor encapsulado.
public:
    void incrementarEImprimir() {     // Suma e imprime bajo el mutex.
        const std::lock_guard<std::mutex> bloqueo(mutex_); // Bloquea el mutex al entrar.
        contador_ += incrementoPorHilo; // Suma el incremento al contador.
        std::cout << contador_ << '\n'; // Imprime el valor de forma segura.
    }

private:
    int contador_ = 0;                // Contador compartido.
    std::mutex mutex_;                // Mutex que protege el contador.
};

class Hilo {                          // Objeto que se comporta como tarea de hilo.
public:
    explicit Hilo(Monitor& monitor) : monitor_(monitor) {} // Guarda una referencia al monitor compartido.

    void operator()() const {         // Operador que ejecuta el hilo.
        monitor_.incrementarEImprimir(); // Llama a la operación sincronizada del monitor.
    }

private:
    Monitor& monitor_;                // Referencia al monitor; el monitor vive en main.
};

int main() {                          // Punto de entrada.
    // Este objeto existe antes de crear los hilos y hasta que terminan.
    Monitor monitor;                  // Monitor compartido con vida suficiente.
    std::array<std::thread, numHilos> hilos; // Arreglo de hilos.
    int codigoSalida = 0;             // Código de salida del programa.

    try {                             // Captura errores de creación.
        for (auto& hilo : hilos) {    // Recorre cada posición.
            hilo = std::thread(Hilo{monitor}); // Crea un hilo usando el funtor Hilo.
        }
    } catch (const std::exception& error) { // Si no se puede crear un hilo...
        std::cerr << "No se pudo iniciar un hilo: " << error.what() << '\n'; // ...muestra el error.
        codigoSalida = 1;             // Marca el fallo.
    }

    for (auto& hilo : hilos) {        // Recorre todos los hilos.
        if (hilo.joinable()) {        // Comprueba si el hilo existe.
            hilo.join();              // Espera a que termine.
        }
    }

    return codigoSalida;              // Devuelve 0 si todo fue correcto.
}
