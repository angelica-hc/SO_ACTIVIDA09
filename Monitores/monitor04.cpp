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
        const std::lock_guard<std::mutex> bloqueo(mutex_);
        contador_ += incrementoPorHilo;
        std::cout << contador_ << '\n';
    }

private:
    int contador_ = 0;
    std::mutex mutex_;
};

class Hilo {
public:
    explicit Hilo(Monitor& monitor) : monitor_(monitor) {}

    void operator()() const {
        monitor_.incrementarEImprimir();
    }

private:
    Monitor& monitor_;
};

int main() {
    // Este objeto existe antes de crear los hilos y hasta que terminan.
    Monitor monitor;
    std::array<std::thread, numHilos> hilos;
    int codigoSalida = 0;

    try {
        for (auto& hilo : hilos) {
            hilo = std::thread(Hilo{monitor});
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
