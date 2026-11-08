#ifndef AVL_H
#define AVL_H

#include "libro.h"

/*
 * Cada nodo AVL guarda una referencia al registro Libro.
 * El registro no se libera desde el AVL: el catalogo ABB es
 * responsable de liberar los libros al cerrar el programa.
 */
typedef struct NodoAVL {
    Libro *libro;
    int altura;
    struct NodoAVL *izq;
    struct NodoAVL *der;
} NodoAVL;

/*
 * Las rotaciones simples representan un caso LL o RR.
 * Las rotaciones dobles representan un caso LR o RL.
 * Una rotacion doble equivale a dos rotaciones primitivas.
 */
typedef struct {
    unsigned long casosLL;
    unsigned long casosRR;
    unsigned long casosLR;
    unsigned long casosRL;
    unsigned long rotacionesSimples;
    unsigned long rotacionesDobles;
} EstadisticasAVL;

void inicializarEstadisticasAVL(EstadisticasAVL *estadisticas);
unsigned long totalRotacionesPrimitivasAVL(const EstadisticasAVL *estadisticas);

int insertarAVL(NodoAVL **raiz, Libro *libro, EstadisticasAVL *estadisticas);
Libro *buscarAVL(NodoAVL *raiz, const char *isbn);
int eliminarAVL(NodoAVL **raiz, const char *isbn,
                Libro **libroEliminado, EstadisticasAVL *estadisticas);

void inordenAVL(NodoAVL *raiz);
void preordenAVL(NodoAVL *raiz);
void postordenAVL(NodoAVL *raiz);

int alturaAVL(NodoAVL *raiz);
int factorEquilibrioAVL(NodoAVL *raiz);
int contarNodosAVL(NodoAVL *raiz);
void mostrarEstructuraAVL(NodoAVL *raiz, int nivel);

/* Libera nodos, pero no los registros Libro compartidos. */
void liberarNodosAVL(NodoAVL *raiz);

#endif
