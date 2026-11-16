#include <array>
#include <mutex>
#include <cstddef>
#include <exception>
#include <iostream>
#include <thread>

constexpr std::size_t numHilos = 10;
constexpr int incrementoPorHilo = 10;

class Monitor {
public:
    int incrementar() {
        const std::lock_guard<std::mutex> bloqueo(mutex_);
        contador_ += incrementoPorHilo;
        // El resultado se copia mientras el mutex sigue bloqueado.
        return contador_;
    }

private:
    int contador_ = 0;
    std::mutex mutex_;
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
