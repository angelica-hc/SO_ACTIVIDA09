// productores00.cpp - ORIGINAL entregado, comentado línea por línea.
// Este programa SOLO tiene productores: no existe consumidor ni búfer compartido.
// Por eso no resuelve el problema productor-consumidor y además nunca termina.
// Compilar: gcc productores00.cpp -pthread -o productores00

#include <stdio.h>    // printf: salida por pantalla.
#include <pthread.h>  // pthread_create y pthread_join: hilos de POSIX.
#include <stdlib.h>   // rand: generación de números pseudoaleatorios.

// gcc productores00.cpp -pthread

#define NUM_PRODUCTORES 2 // Cantidad fija de productores que se crearán.

const char letras[26] = {'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z'}; // Alfabeto del que se elige cada letra.

void *productor(void *arg); // Declaración de la función que ejecutará cada hilo productor.

int main(void) // Punto de entrada del programa.
{
	pthread_t productores[NUM_PRODUCTORES]; // Arreglo con los identificadores de los hilos.
	int identificadores[NUM_PRODUCTORES];   // Arreglo con el número que recibirá cada productor.

	int i; // Variable de control de los bucles.

	for (i = 0; i < NUM_PRODUCTORES; i++) { // Recorre cada productor que se va a crear.
		identificadores[i] = i; // Asigna al productor i el identificador i.
		pthread_create(&productores[i], NULL, &productor, &identificadores[i]); // Crea el hilo; ERROR: no se comprueba si pthread_create falla.
	}

	for (i = 0; i < NUM_PRODUCTORES; i++) { // Recorre todos los hilos creados.
		pthread_join(productores[i], NULL); // Espera a que el hilo termine; ERROR: el hilo nunca termina, así que este join se bloquea para siempre.
	}

	return 0; // Nunca se alcanza porque los productores son infinitos.
}

void *productor(void *arg) // Función que ejecuta cada hilo productor.
{
	int arg2 = *((int *)arg); // Convierte el argumento void* al identificador entero.
	int aleatorio;            // Guarda el índice aleatorio de la letra.

	for (int i = 1; true; i++) { // ERROR: condición siempre verdadera; el bucle nunca termina.
		aleatorio = rand() % 26; // Elige una letra al azar; ERROR: rand() se comparte entre hilos y no es seguro entre hilos.
		printf("(%d) Productor-%d estoy produciendo : %c \n", i, arg2, letras[aleatorio]); // Imprime el ítem; ERROR: no hay consumidor ni búfer, y printf puede entrelazarse.
	}

	return NULL; // Inalcanzable: el bucle infinito nunca deja salir de la función.
}
