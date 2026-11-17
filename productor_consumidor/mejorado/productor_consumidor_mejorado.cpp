
#include <iostream>            // Permite mostrar mensajes en pantalla.
#include <thread>              // Permite crear productores y consumidores.
#include <queue>               // Permite guardar letras en una cola.
#include <mutex>               // Protege la bandeja cuando varios hilos la utilizan.
#include <condition_variable>  // Permite esperar cuando no hay letras o espacio.
#include <random>              // Permite generar letras aleatorias.

using namespace std;           // Evita escribir std:: delante de cada nombre.

// CLASE BANDEJA: lugar compartido entre productores y consumidores.

class Bandeja {                 // Creamos una clase para controlar la bandeja.

private:                       // Datos que solo puede modificar esta clase.

    queue<char> letras;        // Guarda las letras que esperan ser consumidas.
    const size_t capacidad = 3; // La bandeja puede guardar como máximo 3 letras.
    mutex candado;             // Funciona como una llave para entrar a la bandeja.
    condition_variable hayEspacio; // Avisa cuando queda espacio para producir.
    condition_variable hayLetras;  // Avisa cuando existen letras para consumir.
    bool terminado = false;    // Indica si todos los productores terminaron.
    int producidas = 0;        // Cuenta cuántas letras fueron producidas.
    int consumidas = 0;        // Cuenta cuántas letras fueron consumidas.

public:                        // Funciones que pueden utilizar los hilos.

    // FUNCIÓN 1: EL PRODUCTOR COLOCA UNA LETRA.

    void poner(char letra, int numero) { // Recibe una letra y el número del productor.

        unique_lock<mutex> llave(candado); // Toma la llave para acceder a la bandeja.

        hayEspacio.wait(llave, [this] { return letras.size() < capacidad; }); // Si está llena, espera y libera temporalmente la llave.

        letras.push(letra);    // Guarda la nueva letra al final de la cola.
        producidas++;          // Aumenta en uno las letras producidas.

        cout << "Productor " << numero << " coloca " << letra << '\n'; // Muestra quién produjo la letra.

        llave.unlock();        // Libera la llave para que otro hilo pueda trabajar.
        hayLetras.notify_one(); // Avisa a un consumidor que hay una letra disponible.

    }                          // Finaliza la función poner.

    // FUNCIÓN 2: EL CONSUMIDOR RETIRA UNA LETRA.

    bool sacar(int numero) {    // Recibe el número del consumidor.

        unique_lock<mutex> llave(candado); // Toma la llave para revisar la bandeja.

        hayLetras.wait(llave, [this] { return !letras.empty() || terminado; }); // Espera hasta que haya letras o termine la producción.

        if (letras.empty()) {   // Comprueba si ya no quedan letras por consumir.
            return false;      // Indica que terminó el trabajo y libera la llave.
        }                      // Finaliza la comprobación.

        char letra = letras.front(); // Obtiene la primera letra de la cola.
        letras.pop();          // Retira esa letra de la bandeja.
        consumidas++;          // Aumenta en uno las letras consumidas.

        cout << "Consumidor " << numero << " retira " << letra << '\n'; // Muestra quién consumió la letra.

        llave.unlock();        // Libera la llave para otros hilos.
        hayEspacio.notify_one(); // Avisa a un productor que existe espacio libre.

        return true;           // Indica que se consumió una letra correctamente.

    }                          // Finaliza la función sacar.

    // FUNCIÓN 3: AVISAR QUE YA NO HABRÁ MÁS PRODUCCIÓN.

    void cerrar() {            // Se utiliza cuando terminan todos los productores.

        {                      // Inicia una sección protegida.
            lock_guard<mutex> llave(candado); // Toma la llave automáticamente.
            terminado = true;  // Indica que no se producirán más letras.
        }                      // Libera automáticamente la llave.

        hayLetras.notify_all(); // Despierta a todos los consumidores que esperan.

    }                          // Finaliza la función cerrar.

    // FUNCIÓN 4: MOSTRAR LOS RESULTADOS.

    void mostrarResumen() {    // Se ejecuta cuando todos los hilos terminaron.

        cout << "\nTotal producido: " << producidas << '\n'; // Muestra las letras creadas.
        cout << "Total consumido: " << consumidas << '\n'; // Muestra las letras retiradas.
        cout << (producidas == consumidas ? "CORRECTO" : "ERROR") << '\n'; // Comprueba que ambas cantidades coincidan.

    }                          // Finaliza la función mostrarResumen.

};                             // Finaliza la clase Bandeja.

// CREAMOS LA BANDEJA QUE COMPARTIRÁN TODOS LOS HILOS.

Bandeja bandeja;               // Todos los productores y consumidores usarán esta misma bandeja.

// FUNCIÓN QUE REALIZARÁ CADA PRODUCTOR.

void productor(int numero) {   // Recibe el número que identifica al productor.

    mt19937 generador(random_device{}()); // Cada productor obtiene su propio generador aleatorio.
    uniform_int_distribution<int> elegir(0, 25); // Permite elegir números entre 0 y 25.

    for (int i = 0; i < 5; i++) { // Cada productor realizará 5 producciones.

        char letra = 'a' + elegir(generador); // Genera una letra aleatoria entre a y z.
        bandeja.poner(letra, numero); // Coloca la letra en la bandeja compartida.

    }                          // Finaliza el ciclo de las 5 producciones.

}                              // Finaliza la función productor.

// FUNCIÓN QUE REALIZARÁ CADA CONSUMIDOR.

void consumidor(int numero) {  // Recibe el número que identifica al consumidor.

    while (true) {             // Repite el consumo mientras exista trabajo.

        if (!bandeja.sacar(numero)) { // Intenta retirar una letra; false indica que todo terminó.
            break;             // Sale del ciclo cuando no quedan letras ni habrá nuevas.
        }                      // Finaliza la comprobación.

    }                          // Finaliza el ciclo de consumo.

}                              // Finaliza la función consumidor.

// FUNCIÓN PRINCIPAL DEL PROGRAMA.

int main() {                   // Punto donde comienza la ejecución.

    thread consumidor1(consumidor, 1); // Crea e inicia el consumidor número 1.
    thread consumidor2(consumidor, 2); // Crea e inicia el consumidor número 2.

    thread productor1(productor, 1); // Crea e inicia el productor número 1.
    thread productor2(productor, 2); // Crea e inicia el productor número 2.

    productor1.join();         // Espera hasta que el productor 1 termine.
    productor2.join();         // Espera hasta que el productor 2 termine.

    bandeja.cerrar();          // Avisa que ya no habrá nuevas letras.

    consumidor1.join();        // Espera hasta que el consumidor 1 termine.
    consumidor2.join();        // Espera hasta que el consumidor 2 termine.

    bandeja.mostrarResumen();  // Muestra el resultado final de producción y consumo.

    return 0;                  // Finaliza el programa correctamente.

}                              // Fin del programa principal.
