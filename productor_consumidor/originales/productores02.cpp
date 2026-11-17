// productores02.cpp - ORIGINAL entregado, comentado línea por línea.
// Encapsula el productor en una clase, pero conserva los mismos errores:
// no hay búfer ni consumidor, el bucle es infinito y se usa new sin delete.
// Compilar: g++ productores02.cpp -lpthread -o productores02

#include <stdio.h>   // printf.
#include <thread>    // std::thread.
#include <stdlib.h>  // rand.

using namespace std; // Permite usar thread sin std::.

// g++ productores02.cpp -lpthread

#define NUM_PRODUCTORES 2 // Cantidad de productores.

const char letras[26] = {'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z'}; // Letras posibles.

//void productor (int k);

class Productor { // Clase que intenta encapsular un productor.
	private:
		thread t; // Hilo interno del productor.
		int k;    // Identificador del productor.
		void run_thread() { // Función que ejecuta el hilo.
			int aleatorio; // Índice aleatorio de la letra.
			for (int i = 1; true; i++) { // ERROR: bucle infinito; el productor nunca termina.
				aleatorio = rand() % 26; // Elige una letra; ERROR: rand() compartido entre hilos.
				printf("(%d) Productor-%d estoy produciendo : %c \n", i, k, letras[aleatorio]); // Imprime; ERROR: sin búfer ni consumidor y con salida entrelazada.
			}
		}
	public:
		Productor(int id) { // Constructor de la clase.
			k = id; // Guarda el identificador.
			t = thread(&Productor::run_thread, this); // ERROR: lanza el hilo dentro del constructor, antes de que el objeto esté completamente construido.
			//t.join();
		}
		void join_thread() { // Método para esperar al hilo.
			t.join(); // ERROR: si el hilo no termina, join se bloquea para siempre.
		}
};

int main(void) // Punto de entrada.
{
	Productor* productores[NUM_PRODUCTORES]; // Arreglo de punteros a Productor.
	int identificadores[NUM_PRODUCTORES];    // ERROR: se declara pero no se usa (advertencia).

	int i; // Variable de control.

	for (i = 0; i < NUM_PRODUCTORES; i++) { // Recorre los productores.
		productores[i] = new Productor(i); // Crea el productor; ERROR: se usa new pero nunca se hace delete (fuga de memoria).
	}

	for (i = 0; i < NUM_PRODUCTORES; i++) { // Recorre los productores creados.
		productores[i]->join_thread(); // Espera al hilo; ERROR: nunca retorna porque el productor es infinito.
	}

	return 0; // Inalcanzable.
}
