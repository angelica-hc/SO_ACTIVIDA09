// Filósofos comensales - versión unificada, mejorada y comentada línea por línea.
// Reúne lo mejor de filosofos00..04, Completo_1 y Completo_2:
//   - Un hilo por filósofo, identificado por su id.
//   - Un tenedor (mutex) por filósofo y un plato de fideos compartido.
//   - Estadísticas y verificación final de que no se pierden fideos.
// Compilar: g++ -std=c++17 -O2 -Wall -Wextra -pthread filosofos.cpp -o filosofos

#include <iostream>  // std::cout, std::cin y std::cerr.
#include <thread>    // std::thread y this_thread::sleep_for.
#include <mutex>     // std::mutex, std::lock_guard y std::lock.
#include <chrono>    // std::chrono::seconds y milliseconds.
#include <random>    // mt19937, random_device y uniform_int_distribution.
#include <vector>    // std::vector para filósofos, tenedores e hilos.
#include <algorithm> // std::min y std::swap.
#include <cstdlib>   // std::exit.

using namespace std; // Permite escribir cout, mutex, thread, etc. sin std::.

// Limita y valida la cantidad de fideos; recibe long long para detectar valores grandes.
int LimitFideMax(long long fide) { // fide es la cantidad ingresada por el usuario.
    const int maxFi = 2147483647;  // Valor máximo que cabe en un int.
    if (fide > maxFi) {            // Si el usuario pide más que el máximo...
        cout << "\n-------------------------------------------" << endl; // Separa el mensaje de error.
        cout << "¡Se produjo un error!, el número de fideos excede la cantidad máxima" << endl; // Explica el problema.
        exit(1);                   // Termina el programa con error.
    }
    if (fide <= 0) {               // Si la cantidad no es positiva...
        cout << "\n¡Se produjo un error!, debe haber al menos 1 fideo" << endl; // Explica el problema.
        exit(1);                   // Termina el programa con error.
    }
    return (int)min(fide, (long long)maxFi); // Devuelve un int seguro dentro del rango permitido.
}

// Clase que representa a un filósofo y su comportamiento concurrente.
class Filo {
public:
    // Constructor: guarda id, referencias a fideos y a su mutex, e inicializa contadores.
    Filo(int id, int& fide, mutex& mutexFide)
        : id(id), fide(fide), mutexFide(mutexFide), estmag(0), timPensa(0), timCome(0) {} // Inicializa todos los datos del filósofo.

    // Sobrecarga del operador de llamada: permite usar el objeto como función de hilo.
    void operator()() {
        // La condición se revisa con mutex y al inicio de cada ciclo.
        while (quedanFideos()) {   // Mientras haya fideos en el plato...
            pensa();               // ...el filósofo piensa un tiempo aleatorio.
            tomarTenedores();      // ...toma el tenedor izquierdo y el derecho sin interbloqueo.
            come();                // ...toma una porción de fideos y come.
            dejarTenedores();      // ...libera ambos tenedores para que otros puedan comer.
        }
    }

    // Getters de solo lectura para las estadísticas finales.
    int getEstmag() const { return estmag; }     // Devuelve los fideos que comió el filósofo.
    int getTimPensa() const { return timPensa; } // Devuelve el tiempo total pensando.
    int getTimCome() const { return timCome; }   // Devuelve el tiempo/porciones comidas.

public:
    // Lee si quedan fideos protegiendo el acceso con el mutex del plato.
    bool quedanFideos() {
        lock_guard<mutex> lock(mutexFide); // Bloquea el mutex del plato durante la lectura.
        return fide > 0;                   // Devuelve true si todavía hay fideos.
    }

    // El filósofo piensa durante un tiempo aleatorio.
    void pensa() {
        int timp = getTimAlea();   // Obtiene un tiempo aleatorio entre 1 y 5.
        imprimir("Filosofo N°" + to_string(id) + " está pensando (" + to_string(timp) + "s)"); // Informa que está pensando.
        this_thread::sleep_for(chrono::seconds(timp)); // Simula el tiempo pensando.
        timPensa += timp;          // Acumula el tiempo pensado para las estadísticas.
    }

    // El filósofo toma los dos tenedores que necesita para comer.
    void tomarTenedores() {
        int tenedorIzq = id - 1;                          // Calcula el índice del tenedor izquierdo.
        int tenedorDer = id % (int)tenedores.size();      // Calcula el tenedor derecho según el número de filósofos.

        // Ordena los índices para que todos los filósofos adquieran los mutex en el mismo orden.
        if (tenedorIzq > tenedorDer) { // Si el izquierdo es mayor que el derecho...
            swap(tenedorIzq, tenedorDer); // ...intercambia los índices.
        }

        // std::lock bloquea ambos mutex sin interbloqueo y sin liberarlos al salir de la función.
        lock(tenedores[tenedorIzq], tenedores[tenedorDer]); // Adquiere los dos tenedores de forma segura.

        imprimir("Filosofo N°" + to_string(id) + " ha tomado los tenedores " + // Informa el primer tenedor...
                 to_string(tenedorIzq + 1) + " y " + to_string(tenedorDer + 1)); // ...y el segundo.
    }

    // El filósofo come una porción de fideos mientras sostiene los tenedores.
    void come() {
        int porci = 0; // Cantidad de fideos que comerá; 0 si el plato está vacío.
        int antes = 0; // Valor del estómago antes de comer, para mostrar la suma.

        { // Inicio de la sección crítica del plato.
            lock_guard<mutex> lock(mutexFide); // Bloquea el mutex del plato.

            // Solo come si todavía hay fideos.
            if (fide > 0) {            // Comprueba que queden fideos.
                porci = getTimAlea();  // Elige una porción aleatoria entre 1 y 5.

                // Ajusta la porción para no pedir más fideos de los disponibles.
                porci = min(porci, fide); // Toma como máximo lo que queda.
                fide -= porci;         // Descuenta la porción del plato compartido.
                antes = estmag;        // Guarda el valor anterior del estómago.
                estmag += porci;       // Suma la porción al estómago del filósofo.
                timCome += porci;      // Acumula lo comido para las estadísticas.
            }
        } // Fin de la sección crítica: aquí se libera el mutex del plato.

        if (porci > 0) { // Si realmente comió algo...
            imprimir("Filosofo N° " + to_string(id) + " está comiendo (" + to_string(porci) + ")\n" + // ...informa la porción...
                     "Filosofo N° " + to_string(id) + " incrementó el valor del estómago (" +      // ...y muestra el cálculo...
                     to_string(antes) + " + " + to_string(porci) + " = " + to_string(estmag) + ")"); // ...del estómago.
            // Comer tarda un tiempo proporcional a la porción y se hace con los tenedores en la mano.
            this_thread::sleep_for(chrono::milliseconds(200 * porci)); // Simula el tiempo de comida.
        }
    }

    // El filósofo deja los tenedores que tomó antes de comer.
    void dejarTenedores() {
        int tenedorIzq = id - 1;                     // Recalcula el índice del tenedor izquierdo.
        int tenedorDer = id % (int)tenedores.size(); // Recalcula el índice del tenedor derecho.

        // Vuelve a ordenar los índices igual que en tomarTenedores().
        if (tenedorIzq > tenedorDer) { // Si el izquierdo es mayor...
            swap(tenedorIzq, tenedorDer); // ...los intercambia para liberar en el mismo orden.
        }

        // Este hilo todavía posee ambos mutex, por lo que puede liberarlos.
        tenedores[tenedorIzq].unlock(); // Libera el primer tenedor.
        tenedores[tenedorDer].unlock(); // Libera el segundo tenedor.

        imprimir("Filosofo N°" + to_string(id) + " ha dejado los tenedores " + // Informa el primer tenedor...
                 to_string(tenedorIzq + 1) + " y " + to_string(tenedorDer + 1)); // ...y el segundo.
    }

    // Genera un número aleatorio entre 1 y 5 para pensar o para elegir la porción.
    int getTimAlea() const {
        // El generador se crea una sola vez por hilo y se conserva entre llamadas.
        thread_local mt19937 gen{random_device{}()}; // Generador local al hilo, sembrado con entropía.
        uniform_int_distribution<int> dist(1, 5);    // Distribución uniforme entre 1 y 5.
        return dist(gen);                            // Devuelve el número aleatorio generado.
    }

    // Imprime un mensaje de forma segura entre varios hilos.
    static void imprimir(const string& texto) {
        lock_guard<mutex> lock(mutexSalida); // Bloquea el mutex de salida.
        cout << texto << endl;               // Imprime el mensaje completo sin mezclarlo con otros.
    }

    int id;                          // Identificador del filósofo.
    int& fide;                       // Referencia a los fideos compartidos del plato.
    mutex& mutexFide;                // Referencia al mutex que protege el plato.
    int estmag;                      // Fideos que ha comido este filósofo.
    int timPensa;                    // Tiempo total que ha pasado pensando.
    int timCome;                     // Cantidad total que ha comido.
    static vector<mutex> tenedores;  // Un mutex por cada tenedor compartido.
    static mutex mutexSalida;        // Mutex que serializa la salida por pantalla.
};

vector<mutex> Filo::tenedores; // Definición del vector estático; se dimensiona en main.
mutex Filo::mutexSalida;       // Definición del mutex estático de salida.

int main() {                       // Punto de entrada del programa.
    long long fideLeido;           // Cantidad de fideos leída como long long para validarla.
    int numFilos;                  // Cantidad de filósofos que se crearán.

    cout << "\n--------SIMULADOR DE FILOSOFOS COMENSALES--------" << endl; // Título del programa.
    cout << "\nBienvenido Usuario, Escriba los datos pedidos:" << endl;    // Instrucción al usuario.
    cout << "\n(Recuerda que el máximo de fideos es 2,147,483,647)" << endl; // Informa el límite.
    cout << "\n- Cuantos Fideos Pondras: ";                                // Pide la cantidad de fideos.

    if (!(cin >> fideLeido)) { // Intenta leer un entero; si falla...
        cout << "Error: ingrese un número entero." << endl; // ...muestra el error.
        return 1;              // ...y termina con código de error.
    }
    int fide = LimitFideMax(fideLeido); // Valida y convierte la cantidad al rango permitido.
    const int fideInicial = fide;       // Guarda el valor inicial para la verificación final.

    cout << "\n- Escriba la cantidad de filósofos que habrá: "; // Pide el número de filósofos.
    if (!(cin >> numFilos) || numFilos < 2) { // Si no se lee o hay menos de 2...
        cout << "Error: debe haber al menos 2 filósofos." << endl; // ...explica el error.
        return 1;                          // ...y termina con error.
    }

    cout << "\n-------------------------------------------" << endl; // Separa la entrada de la ejecución.
    cout << "\nEjecutando procedimiento......." << endl;              // Informa el inicio.
    cout << "\n-------------------------------------------" << endl; // Separa el aviso.

    Filo::tenedores = vector<mutex>(numFilos); // Crea exactamente un tenedor por filósofo.

    mutex mutexFide; // Mutex único que protege el plato de fideos compartido.

    vector<Filo> filos;              // Vector que contiene a todos los filósofos.
    filos.reserve(numFilos);         // Reserva espacio para evitar realocaciones que invalidarían referencias.
    vector<thread> hilos;            // Vector que contiene los hilos de los filósofos.

    // Crea cada filósofo y su hilo correspondiente.
    for (int i = 0; i < numFilos; ++i) {
        filos.emplace_back(i + 1, fide, mutexFide); // Construye el filósofo i+1 con referencias al plato.
        hilos.emplace_back(ref(filos[i]));          // Lanza un hilo que ejecuta el operador() del filósofo.
    }

    // Espera a que todos los filósofos terminen.
    for (int i = 0; i < numFilos; ++i) {
        hilos[i].join();              // Bloquea hasta que el hilo i termine.
    }

    cout << "\n-------------------------------------------" << endl; // Separa la ejecución de los resultados.
    cout << "\n....La ejecución del programa ha terminado...." << endl; // Informa el final.
    cout << "\nMostrando datos finales........" << endl;                // Avisa que mostrará estadísticas.
    cout << "\n---------------Estadísticas:----------------\n" << endl;  // Encabezado de estadísticas.

    int totalEstmag = 0;             // Suma de fideos comidos por todos.
    int totalTimPensa = 0;           // Suma de tiempos pensando.
    int totalTimCome = 0;            // Suma de porciones comidas.
    int maxEstmag = 0;               // Mayor estómago encontrado.
    int maxFilo = 0;                 // Id del filósofo con mayor estómago.

    // Recorre todos los filósofos para acumular totales y encontrar el máximo.
    for (int i = 0; i < numFilos; ++i) {
        totalEstmag += filos[i].getEstmag();     // Acumula lo comido por el filósofo i.
        totalTimPensa += filos[i].getTimPensa(); // Acumula su tiempo pensando.
        totalTimCome += filos[i].getTimCome();   // Acumula su cantidad comida.
        if (filos[i].getEstmag() > maxEstmag) {  // Si el filósofo i comió más que el máximo actual...
            maxEstmag = filos[i].getEstmag();    // ...actualiza el máximo.
            maxFilo = filos[i].id;               // ...y guarda su id.
        }
    }

    // Muestra las estadísticas individuales de cada filósofo.
    for (int i = 0; i < numFilos; ++i) {
        // Calcula porcentajes evitando la división entre cero.
        double porcPensa = totalTimPensa > 0 ? 100.0 * filos[i].getTimPensa() / totalTimPensa : 0.0; // Porcentaje pensado.
        double porcCome = totalTimCome > 0 ? 100.0 * filos[i].getTimCome() / totalTimCome : 0.0;    // Porcentaje comido.
        cout << "+ Filosofos N°(" << filos[i].id << ") :" << endl; // Encabezado del filósofo i.
        cout << "\t* Tiempo pensando: " << filos[i].getTimPensa() << "s (" << porcPensa << "%)" << endl; // Tiempo y porcentaje.
        cout << "\t* Estómago: " << filos[i].getEstmag() << " (" << porcCome << "% de lo comido)" << endl; // Fideos y porcentaje.
        cout << endl; // Línea en blanco entre filósofos.
    }

    cout << "\n---------------Datos finales---------------\n" << endl; // Encabezado de datos finales.
    cout << "1.- Total de comida consumida por los filósofos: " << totalEstmag << endl; // Total consumido.
    cout << "2.- Fideos que restan: " << fide << endl;                                  // Fideos restantes.
    cout << "3.- Total de tiempo pensando: " << totalTimPensa << "s" << endl;           // Tiempo total pensado.
    cout << "4.- Filósofo con el estómago más grande: Filosofo N°" << maxFilo << " (" << maxEstmag << " fideos)" << endl; // Mayor estómago.
    cout << "5.- Promedio de fideos por filósofo: " << static_cast<double>(totalEstmag) / numFilos << endl; // Promedio.
    // Verificación de consistencia: lo comido más lo restante debe ser lo inicial.
    cout << "6.- Verificación: " << totalEstmag << " + " << fide << " = " << totalEstmag + fide // Muestra la suma...
         << (totalEstmag + fide == fideInicial ? " (correcto)" : " (ERROR)") << endl;          // ...y si coincide con el inicio.
    cout << "\n-------------------------------------------" << endl; // Línea final.
    cout << "\t\t\n-----FIN DE LA EJECUCIÓN------" << endl;            // Mensaje de despedida.
    return 0;                                                          // Termina correctamente.
}
