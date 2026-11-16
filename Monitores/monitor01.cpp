#include <array>
#include <atomic>
#include <cstddef>
#include <exception>
#include <iostream>
#include <thread>

constexpr std::size_t numHilos = 10;
constexpr int incrementoPorHilo = 10;

class Monitor {
public:
    int incrementar() {
        // fetch_add devuelve el valor anterior y suma en una sola operacion.
        return contador_.fetch_add(incrementoPorHilo) + incrementoPorHilo;
    }

private:
    std::atomic<int> contador_{0};
};

int main() {
    Monitor monitor;
    std::array<int, numHilos> resultados{};
    std::array<std::thread, numHilos> hilos;
    bool inicioCompleto = true;

    try {
        for (std::size_t i = 0; i < hilos.size(); ++i) {
            hilos[i] = std::thread([&monitor, &resultados, i] {
                resultados[i] = monitor.incrementar();
            });
        }
    } catch (const std::exception& error) {
        std::cerr << "No se pudo iniciar un hilo: " << error.what() << '\n';
        inicioCompleto = false;
    }

    for (auto& hilo : hilos) {
        if (hilo.joinable()) {
            hilo.join();
        }
    }

    if (!inicioCompleto) {
        return 1;
    }

    for (int resultado : resultados) {
        std::cout << resultado << '\n';
    }
}
