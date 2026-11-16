#include <iostream> // Librería estándar para entrada y salida de datos
#include <thread>   // Librería para trabajar con hilos de ejecución
#include <mutex>    // Librería para trabajar con mutex (exclusión mutua)
#include <chrono>   // Librería para trabajar con tiempo y duraciones
#include <random>   // Librería para generar números aleatorios
#include <vector>   // Librería para trabajar con vectores
#include <algorithm> // min y swap
#include <cstdlib>   // exit

// Compilar: g++ Filosofos_Comensales_Mejorado.cpp -pthread -o filosofos
//
// PROGRAMA UNIFICADO. Reúne lo mejor de filosofos00..04, Completo_1 y Completo_2:
//  - Un hilo por filósofo, identificado por su id (filosofos03 / filosofos04)
//  - Un tenedor (mutex) por filósofo + plato de fideos compartido (Completo_1 / 2)
//  - Estadísticas completas (Completo_1) y datos finales (Completo_2)

using namespace std;

// Función para limitar el número máximo de fideos
// MEJORA: recibe long long; antes recibía int y "fide > maxFi" nunca era verdadero.
int LimitFideMax(long long fide) {
    const int maxFi = 2147483647;
    if (fide > maxFi) {
        cout << "\n-------------------------------------------" << endl;
        cout << "¡Se produjo un error!, el número de fideos excede la cantidad máxima" << endl;
        exit(1);
    }
    if (fide <= 0) {
        cout << "\n¡Se produjo un error!, debe haber al menos 1 fideo" << endl;
        exit(1);
    }
    return (int)min(fide, (long long)maxFi);
}

// Clase que representa a un filósofo
class Filo {
public:
    Filo(int id, int& fide, mutex& mutexFide)
        : id(id), fide(fide), mutexFide(mutexFide), estmag(0), timPensa(0), timCome(0) {}

    // Sobrecarga del operador de llamada para utilizar el objeto como función (hilo)
    void operator()() {
        // MEJORA: la condición se revisa con mutex y AL INICIO del ciclo
        while (quedanFideos()) {
            pensa();
            tomarTenedores();
            come();
            dejarTenedores();
        }
    }

    // Métodos para obtener los datos del filósofo
    int getEstmag() const { return estmag; }
    int getTimPensa() const { return timPensa; }
    int getTimCome() const { return timCome; }

public:
    // MEJORA: lectura de fide protegida con mutex (antes "fide <= 0" se leía sin protección)
    bool quedanFideos() {
        lock_guard<mutex> lock(mutexFide);
        return fide > 0;
    }

    // El filósofo está pensando
    void pensa() {
        int timp = getTimAlea();
        imprimir("Filosofo N°" + to_string(id) + " está pensando (" + to_string(timp) + "s)");
        this_thread::sleep_for(chrono::seconds(timp));
        timPensa += timp;
    }

    // El filósofo toma los tenedores
    void tomarTenedores() {
        int tenedorIzq = id - 1;
        int tenedorDer = id % (int)tenedores.size();   // MEJORA: antes "id % 5" fijo

        // Si el número del tenedor izquierdo es mayor al del derecho, se intercambian
        if (tenedorIzq > tenedorDer) {
            swap(tenedorIzq, tenedorDer);
        }

        // MEJORA: antes los unique_lock locales soltaban los tenedores al salir de
        // esta función. Ahora se bloquean los mutex directamente (std::lock evita
        // el interbloqueo) y siguen tomados hasta dejarTenedores().
        lock(tenedores[tenedorIzq], tenedores[tenedorDer]);

        imprimir("Filosofo N°" + to_string(id) + " ha tomado los tenedores " +
                 to_string(tenedorIzq + 1) + " y " + to_string(tenedorDer + 1));
    }

    // El filósofo está comiendo
    void come() {
        int porci = 0;
        int antes = 0;
        {
            lock_guard<mutex> lock(mutexFide);

            // Si aún hay fideos disponibles
            if (fide > 0) {
                porci = getTimAlea();

                // Limitamos la cantidad de fideos a la cantidad disponible
                porci = min(porci, fide);
                fide -= porci;
                antes = estmag;
                estmag += porci;
                timCome += porci;
            }
        }

        if (porci > 0) {
            imprimir("Filosofo N° " + to_string(id) + " está comiendo (" + to_string(porci) + ")\n" +
                     "Filosofo N° " + to_string(id) + " incrementó el valor del estómago (" +
                     to_string(antes) + " + " + to_string(porci) + " = " + to_string(estmag) + ")");
            // Comer toma tiempo y se hace con los tenedores en la mano
            this_thread::sleep_for(chrono::milliseconds(200 * porci));
        }
    }

    // El filósofo deja los tenedores
    void dejarTenedores() {
        int tenedorIzq = id - 1;
        int tenedorDer = id % (int)tenedores.size();

        if (tenedorIzq > tenedorDer) {
            swap(tenedorIzq, tenedorDer);
        }

        // Ahora sí es válido: este hilo todavía tiene bloqueados los mutex
        tenedores[tenedorIzq].unlock();
        tenedores[tenedorDer].unlock();

        imprimir("Filosofo N°" + to_string(id) + " ha dejado los tenedores " +
                 to_string(tenedorIzq + 1) + " y " + to_string(tenedorDer + 1));
    }

    // Método para generar un número aleatorio entre 1 y 5 (tiempo de pensar / porción)
    int getTimAlea() const {
        // MEJORA: el generador se crea una vez por hilo (antes en cada llamada)
        thread_local mt19937 gen{random_device{}()};
        uniform_int_distribution<int> dist(1, 5);
        return dist(gen);
    }

    // MEJORA: salida protegida para que los mensajes de varios hilos no se mezclen
    static void imprimir(const string& texto) {
        lock_guard<mutex> lock(mutexSalida);
        cout << texto << endl;
    }

    int id;
    int& fide;
    mutex& mutexFide;
    int estmag;
    int timPensa;
    int timCome;
    static vector<mutex> tenedores;
    static mutex mutexSalida;
};

vector<mutex> Filo::tenedores;   // MEJORA: se dimensiona en main según numFilos (antes fijo en 5)
mutex Filo::mutexSalida;

int main() {
    long long fideLeido;
    int numFilos;

    cout << "\n--------SIMULADOR DE FILOSOFOS COMENSALES--------" << endl;
    cout << "\nBienvenido Usuario, Escriba los datos pedidos:" << endl;
    cout << "\n(Recuerda que el máximo de fideos es 2,147,483,647)" << endl;
    cout << "\n- Cuantos Fideos Pondras: ";
    // MEJORA: validación de la entrada
    if (!(cin >> fideLeido)) {
        cout << "Error: ingrese un número entero." << endl;
        return 1;
    }
    int fide = LimitFideMax(fideLeido);
    const int fideInicial = fide;

    cout << "\n- Escriba la cantidad de filósofos que habrá: ";
    // MEJORA: con menos de 2 filósofos no existen dos tenedores distintos
    if (!(cin >> numFilos) || numFilos < 2) {
        cout << "Error: debe haber al menos 2 filósofos." << endl;
        return 1;
    }
    cout << "\n-------------------------------------------" << endl;
    cout << "\nEjecutando procedimiento......." << endl;
    cout << "\n-------------------------------------------" << endl;

    Filo::tenedores = vector<mutex>(numFilos);   // un tenedor por filósofo

    mutex mutexFide;

    vector<Filo> filos;
    filos.reserve(numFilos);   // evita realocaciones: los hilos guardan referencias
    vector<thread> hilos;

    // Creación de los filósofos y los hilos de ejecución
    for (int i = 0; i < numFilos; ++i) {
        filos.emplace_back(i + 1, fide, mutexFide);
        hilos.emplace_back(ref(filos[i]));
    }

    // Espera a que todos los hilos terminen su ejecución
    for (int i = 0; i < numFilos; ++i) {
        hilos[i].join();
    }

    cout << "\n-------------------------------------------" << endl;
    cout << "\n....La ejecución del programa ha terminado...." << endl;
    cout << "\nMostrando datos finales........" << endl;
    cout << "\n---------------Estadísticas:----------------\n" << endl;

    int totalEstmag = 0;
    int totalTimPensa = 0;
    int totalTimCome = 0;
    int maxEstmag = 0;
    int maxFilo = 0;

    for (int i = 0; i < numFilos; ++i) {
        totalEstmag += filos[i].getEstmag();
        totalTimPensa += filos[i].getTimPensa();
        totalTimCome += filos[i].getTimCome();
        if (filos[i].getEstmag() > maxEstmag) {
            maxEstmag = filos[i].getEstmag();
            maxFilo = filos[i].id;
        }
    }

    for (int i = 0; i < numFilos; ++i) {
        // MEJORA: porcentajes protegidos contra división entre cero
        double porcPensa = totalTimPensa > 0 ? 100.0 * filos[i].getTimPensa() / totalTimPensa : 0.0;
        double porcCome = totalTimCome > 0 ? 100.0 * filos[i].getTimCome() / totalTimCome : 0.0;
        cout << "+ Filosofos N°(" << filos[i].id << ") :" << endl;
        cout << "\t* Tiempo pensando: " << filos[i].getTimPensa() << "s (" << porcPensa << "%)" << endl;
        cout << "\t* Estómago: " << filos[i].getEstmag() << " (" << porcCome << "% de lo comido)" << endl;
        cout << endl;
    }

    cout << "\n---------------Datos finales---------------\n" << endl;
    cout << "1.- Total de comida consumida por los filósofos: " << totalEstmag << endl;
    cout << "2.- Fideos que restan: " << fide << endl;
    cout << "3.- Total de tiempo pensando: " << totalTimPensa << "s" << endl;
    cout << "4.- Filósofo con el estómago más grande: Filosofo N°" << maxFilo << " (" << maxEstmag << " fideos)" << endl;
    cout << "5.- Promedio de fideos por filósofo: " << static_cast<double>(totalEstmag) / numFilos << endl;
    // MEJORA: verificación de consistencia (no se crean ni se pierden fideos)
    cout << "6.- Verificación: " << totalEstmag << " + " << fide << " = " << totalEstmag + fide
         << (totalEstmag + fide == fideInicial ? " (correcto)" : " (ERROR)") << endl;
    cout << "\n-------------------------------------------" << endl;
    cout << "\t\t\n-----FIN DE LA EJECUCIÓN------" << endl;
    return 0;
}
