// Monitor 03 - versión corregida y comentada línea por línea.
// El monitor protege el contador y la impresión dentro de la misma
// sección crítica, de modo que la salida siempre es coherente.
// Compilar: g++ -std=c++17 -O2 -Wall -Wextra -pthread monitor03.cpp -o monitor03

#include <array>     // std::array para los hilos.
#include <cstddef>   // std::size_t.
#include <exception> // std::exception.
#include <iostream>  // std::cout y std::cerr.
#include <mutex>     // std::mutex y std::lock_guard.
#include <thread>    // std::thread.

constexpr std::size_t numHilos = 10;  // Cantidad de hilos.
constexpr int incrementoPorHilo = 10; // Incremento que aplica cada hilo.

class Monitor {                       // Monitor con datos y mutex privados.
public:
    void incrementarEImprimir() {     // Operación que suma e imprime de forma atómica.
        // El monitor protege toda la operación, incluida la impresión.
        const std::lock_guard<std::mutex> bloqueo(mutex_); // Bloquea el mutex al entrar.
        contador_ += incrementoPorHilo; // Suma el incremento.
        std::cout << contador_ << '\n'; // Imprime el nuevo valor sin que otro hilo se intercale.
    }

private:
    int contador_ = 0;                // Contador compartido.
    std::mutex mutex_;                // Mutex interno del monitor.
};

int main() {                          // Punto de entrada.
    Monitor monitor;                  // Monitor compartido por todos los hilos.
    std::array<std::thread, numHilos> hilos; // Arreglo de hilos.
    int codigoSalida = 0;             // Código que se devolverá al sistema.

    try {                             // Captura errores al crear hilos.
        for (auto& hilo : hilos) {    // Recorre cada posición del arreglo.
            hilo = std::thread([&monitor] { monitor.incrementarEImprimir(); }); // Crea un hilo que ejecuta el monitor.
        }
    } catch (const std::exception& error) { // Si falla la creación...
        std::cerr << "No se pudo iniciar un hilo: " << error.what() << '\n'; // ...informa el error.
        codigoSalida = 1;             // Marca el código de salida como error.
    }

    for (auto& hilo : hilos) {        // Recorre todos los hilos.
        if (hilo.joinable()) {        // Comprueba que el hilo se haya creado.
            hilo.join();              // Espera a que termine.
        }
    }

    return codigoSalida;              // Devuelve 0 si todo fue bien; 1 si hubo error.
}
