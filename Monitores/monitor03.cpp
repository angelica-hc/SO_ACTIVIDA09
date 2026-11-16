#include <array>
#include <cstddef>
#include <exception>
#include <iostream>
#include <mutex>
#include <thread>

constexpr std::size_t numHilos = 10;
constexpr int incrementoPorHilo = 10;

class Monitor {
public:
    void incrementarEImprimir() {
        // El monitor protege toda la operacion, incluida la impresion.
        const std::lock_guard<std::mutex> bloqueo(mutex_);
        contador_ += incrementoPorHilo;
        std::cout << contador_ << '\n';
    }

private:
    int contador_ = 0;
    std::mutex mutex_;
};

int main() {
    Monitor monitor;
    std::array<std::thread, numHilos> hilos;
    int codigoSalida = 0;

    try {
        for (auto& hilo : hilos) {
            hilo = std::thread([&monitor] { monitor.incrementarEImprimir(); });
        }
    } catch (const std::exception& error) {
        std::cerr << "No se pudo iniciar un hilo: " << error.what() << '\n';
        codigoSalida = 1;
    }

    for (auto& hilo : hilos) {
        if (hilo.joinable()) {
            hilo.join();
        }
    }

    return codigoSalida;
}
