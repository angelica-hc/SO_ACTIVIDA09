#include <array>
#include <cstddef>
#include <exception>
#include <future>
#include <iostream>
#include <mutex>

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

int main() {
    Monitor monitor;

    try {
        std::array<std::future<void>, numHilos> tareas;

        // launch::async ejecuta cada tarea en un hilo propio.
        for (auto& tarea : tareas) {
            tarea = std::async(std::launch::async, [&monitor] {
                monitor.incrementarEImprimir();
            });
        }

        for (auto& tarea : tareas) {
            tarea.get();
        }
    } catch (const std::exception& error) {
        std::cerr << "No se pudieron completar las tareas: " << error.what() << '\n';
        return 1;
    }
}
