// Monitor 01 - versión corregida y comentada línea por línea.
// Usa un contador atómico: la suma es indivisible aunque varios hilos
// llamen a incrementar() al mismo tiempo.
// Compilar: g++ -std=c++17 -O2 -Wall -Wextra -pthread monitor01.cpp -o monitor01

#include <array>     // std::array para los resultados y los hilos.
#include <atomic>    // std::atomic para el contador.
#include <cstddef>   // std::size_t.
#include <exception> // std::exception para manejar errores.
#include <iostream>  // std::cout y std::cerr.
#include <thread>    // std::thread.

constexpr std::size_t numHilos = 10;  // Número de hilos a crear.
constexpr int incrementoPorHilo = 10; // Incremento que aplica cada hilo.

class Monitor {                       // Monitor que encapsula el contador atómico.
public:
    int incrementar() {               // Operación pública: suma y devuelve el nuevo valor.
        // fetch_add devuelve el valor anterior y suma en una sola operación atómica.
        return contador_.fetch_add(incrementoPorHilo) + incrementoPorHilo; // Suma atómica y calcula el nuevo valor.
    }

private:
    std::atomic<int> contador_{0};    // Contador atómico inicializado en cero.
};

int main() {                          // Punto de entrada.
    Monitor monitor;                  // Monitor compartido por todos los hilos.
    std::array<int, numHilos> resultados{}; // Cada hilo guarda aquí su resultado.
    std::array<std::thread, numHilos> hilos; // Arreglo de objetos thread.
    bool inicioCompleto = true;       // Bandera para saber si todos los hilos se crearon.

    try {                             // Captura errores al crear hilos.
        for (std::size_t i = 0; i < hilos.size(); ++i) { // Recorre las posiciones del arreglo.
            hilos[i] = std::thread([&monitor, &resultados, i] { // Crea el hilo i capturando monitor, resultados e i.
                resultados[i] = monitor.incrementar(); // Guarda el valor devuelto por el monitor.
            });
        }
    } catch (const std::exception& error) { // Si falla la creación de algún hilo...
        std::cerr << "No se pudo iniciar un hilo: " << error.what() << '\n'; // ...informa el error.
        inicioCompleto = false;       // Marca que la creación no fue completa.
    }

    for (auto& hilo : hilos) {        // Recorre todos los hilos.
        if (hilo.joinable()) {        // Solo espera hilos que realmente se crearon.
            hilo.join();              // Espera a que el hilo termine.
        }
    }

    if (!inicioCompleto) {            // Si algún hilo no se pudo crear...
        return 1;                     // ...termina con error.
    }

    for (int resultado : resultados) { // Recorre los resultados en orden.
        std::cout << resultado << '\n'; // Imprime el valor obtenido por cada hilo.
    }
}
