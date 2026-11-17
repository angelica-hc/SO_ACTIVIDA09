// productores01.cpp - ORIGINAL entregado, comentado línea por línea.
// Igual que productores00, pero usando std::thread de C++.
// Sigue sin existir consumidor ni búfer, y los productores son infinitos.
// Compilar: g++ productores01.cpp -lpthread -o productores01

#include <stdio.h>   // printf: salida por pantalla.
#include <thread>    // std::thread: hilos de C++.
#include <stdlib.h>  // rand: generación pseudoaleatoria.

using namespace std; // Permite escribir thread sin std::.

// g++ productores01.cpp -lpthread

#define NUM_PRODUCTORES 2 // Cantidad fija de productores.

const char letras[26] = {'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z'}; // Letras posibles.

void productor(int k); // Declaración de la función que ejecutará cada hilo.

int main(void) // Punto de entrada.
{
	thread productores[NUM_PRODUCTORES]; // Arreglo de objetos std::thread.
	int identificadores[NUM_PRODUCTORES]; // Arreglo de identificadores; ERROR: se declara pero nunca se usa (genera advertencia).

	int i; // Variable de control.

	for (i = 0; i < NUM_PRODUCTORES; i++) { // Recorre los productores.
		productores[i] = thread(productor, i); // Crea el hilo i y le pasa el índice; ERROR: si falla, lanza una excepción no controlada.
	}

	for (i = 0; i < NUM_PRODUCTORES; i++) { // Recorre los hilos creados.
		productores[i].join(); // Espera su finalización; ERROR: el productor es infinito, así que join nunca retorna.
	}

	return 0; // Inalcanzable por el bucle infinito.
}

void productor(int k) // Función ejecutada por cada hilo productor.
{
	int aleatorio; // Índice aleatorio de la letra.

	for (int i = 1; true; i++) { // ERROR: bucle infinito; el programa nunca termina.
		aleatorio = rand() % 26; // Elige una letra; ERROR: rand() compartido entre hilos, sin sincronización.
		printf("(%d) Productor-%d estoy produciendo : %c \n", i, k, letras[aleatorio]); // Imprime; ERROR: no hay búfer ni consumidor y la salida puede mezclarse.
	}
}
