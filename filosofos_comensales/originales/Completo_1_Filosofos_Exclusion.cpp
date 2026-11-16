#include <iostream>     // Librería estándar de entrada/salida
#include <thread>       // Librería para trabajar con hilos de ejecución
#include <chrono>       // Librería para manejar tiempos
#include <vector>       // Librería para trabajar con vectores
#include <random>       // Librería para generar números aleatorios
#include <mutex>        // Librería para proporcionar mecanismos de sincronización

unsigned int fideos = 256;      // Cantidad inicial de fideos en el plato
std::mutex mutex;               // Objeto mutex para sincronizar el acceso a la variable fideos

class Filosofo {
private:
    int id;                     // Identificador del filósofo
    int estomago;               // Cantidad de fideos en el estómago del filósofo
    double tiempoPensando;      // Tiempo total que el filósofo ha pasado pensando
    double tiempoComiendo;      // Tiempo total que el filósofo ha pasado comiendo

public:
    Filosofo(int id) : id(id), estomago(0), tiempoPensando(0.0), tiempoComiendo(0.0) {}

    void pensar() {
        std::random_device rd;                      // Genera una semilla aleatoria
        std::mt19937 gen(rd());                      // Generador de números aleatorios
        std::uniform_real_distribution<> dis(1.0, 10.0);  // Distribución uniforme entre 1 y 10
        double tiempo = dis(gen);                   // Genera un tiempo aleatorio de pensamiento
        tiempoPensando += tiempo;                    // Actualiza el tiempo total de pensamiento del filósofo
        std::cout << "Filosofo" << id << " está pensando (" << tiempo << "s)\n"; // Imprime el mensaje
        std::this_thread::sleep_for(std::chrono::duration<double>(tiempo));     // Pausa la ejecución durante el tiempo de pensamiento
    }

    void comer() {
        std::random_device rd;                      // Genera una semilla aleatoria
        std::mt19937 gen(rd());                      // Generador de números aleatorios
        std::uniform_int_distribution<> dis(1, 50);   // Distribución uniforme entre 1 y 50
        int cantidad = dis(gen);                    // Genera una cantidad aleatoria de fideos a comer

        mutex.lock();                               // Bloquea el acceso a la variable fideos utilizando el mutex
        if (fideos < cantidad) {
            cantidad = fideos;                       // Si la cantidad de fideos disponibles es menor a la cantidad aleatoria generada, ajusta la cantidad al número de fideos disponibles
        }
        fideos -= cantidad;                         // Resta la cantidad de fideos comidos a la variable fideos
        mutex.unlock();                             // Desbloquea el acceso a la variable fideos

        estomago += cantidad;                       // Incrementa la cantidad de fideos en el estómago del filósofo
        tiempoComiendo += cantidad;                  // Actualiza el tiempo total de comida del filósofo

        std::cout << "Filosofo" << id << " está comiendo (" << cantidad << ")\n"; // Imprime el mensaje
        std::cout << "Filosofo" << id << " incrementó el valor de estómago (" << estomago - cantidad << " + " << cantidad << " = " << estomago << ")\n"; // Imprime el mensaje
    }

    int getEstomago() const {
        return estomago;                            // Devuelve la cantidad de fideos en el estómago del filósofo
    }

    int getId() const {
        return id;                                  // Devuelve el identificador del filósofo
    }

    double getTiempoPensando() const {
        return tiempoPensando;                      // Devuelve el tiempo total de pensamiento del filósofo
    }

    double getTiempoComiendo() const {
        return tiempoComiendo;                      // Devuelve el tiempo total de comida del filósofo
    }
};

int main() {
    int numFilosofos;
    std::cout << "Fideos = " << fideos << std::endl;                            // Imprime la cantidad inicial de fideos en el plato
    std::cout << "Ingrese cantidad de filósofos: ";
    std::cin >> numFilosofos;                                                    // Lee la cantidad de filósofos ingresada por el usuario

    std::vector<Filosofo> filosofos;                                             // Vector para almacenar los filósofos
    std::vector<std::thread> threads;                                            // Vector para almacenar los hilos de ejecución

    for (int i = 1; i <= numFilosofos; i++) {
        filosofos.emplace_back(i);                                               // Crea un nuevo objeto Filosofo y lo agrega al vector de filósofos
    }

    for (Filosofo& filosofo : filosofos) {
        threads.emplace_back([&filosofo]() {                                      // Crea un nuevo hilo de ejecución que ejecutará el código lambda pasado como argumento
            while (fideos > 0) {
                filosofo.pensar();                                                 // El filósofo piensa
                if (fideos > 0) {
                    filosofo.comer();                                              // El filósofo come si hay fideos disponibles
                }
            }
        });
    }

    for (std::thread& thread : threads) {
        thread.join();                                                           // Espera a que todos los hilos terminen su ejecución
    }

    std::cout << "Estadísticas:" << std::endl;
    double tiempoTotalPensando = 0.0;
    double tiempoTotalComiendo = 0.0;
    int maxEstomago = 0;
    int maxFilosofoEstomago = 0;
    int totalEstomagos = 0;

    for (const Filosofo& filosofo : filosofos) {
        std::cout << "Estómago filosofo" << filosofo.getId() << " = " << filosofo.getEstomago() << std::endl; // Imprime la cantidad de fideos en el estómago de cada filósofo

        if (filosofo.getEstomago() > maxEstomago) {
            maxEstomago = filosofo.getEstomago();                                   // Actualiza el valor máximo de fideos en el estómago y el identificador del filósofo correspondiente
            maxFilosofoEstomago = filosofo.getId();
        }

        tiempoTotalPensando += filosofo.getTiempoPensando();                         // Acumula el tiempo total de pensamiento de todos los filósofos
        tiempoTotalComiendo += filosofo.getTiempoComiendo();                         // Acumula el tiempo total de comida de todos los filósofos
        totalEstomagos += filosofo.getEstomago();                                    // Acumula la cantidad total de fideos en el estómago de todos los filósofos
    }

    std::cout << "Fideos = " << fideos << std::endl;                                // Imprime la cantidad restante de fideos en el plato
    std::cout << "Filósofo con el estómago más grande: Filosofo" << maxFilosofoEstomago << " (" << maxEstomago << " fideos)" << std::endl; // Imprime el filósofo con el estómago más grande
    std::cout << "Porcentaje de tiempo pensando por filósofo:" << std::endl;

    for (const Filosofo& filosofo : filosofos) {
        double porcentajeTiempoPensando = (filosofo.getTiempoPensando() / tiempoTotalPensando) * 100.0;  // Calcula el porcentaje de tiempo de pensamiento de cada filósofo respecto al total
        std::cout << "Filosofo" << filosofo.getId() << ": " << porcentajeTiempoPensando << "%" << std::endl; // Imprime el porcentaje de tiempo de pensamiento de cada filósofo
    }

    std::cout << "Porcentaje de tiempo comiendo por filósofo:" << std::endl;

    for (const Filosofo& filosofo : filosofos) {
        double porcentajeTiempoComiendo = (filosofo.getTiempoComiendo() / tiempoTotalComiendo) * 100.0;  // Calcula el porcentaje de tiempo de comida de cada filósofo respecto al total
        std::cout << "Filosofo" << filosofo.getId() << ": " << porcentajeTiempoComiendo << "%" << std::endl; // Imprime el porcentaje de tiempo de comida de cada filósofo
    }

    double promedioEstomagos = static_cast<double>(totalEstomagos) / numFilosofos;  // Calcula el promedio de fideos en el estómago de los filósofos
    std::cout << "Promedio de fideos en el estómago de los filósofos: " << promedioEstomagos << std::endl; // Imprime el promedio de fideos en el estómago de los filósofos

    return 0;
}
