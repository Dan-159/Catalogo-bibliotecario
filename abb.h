#ifndef ABB_H
#define ABB_H

#include "libro.h"

typedef struct NodoABB {
    Libro *libro;
    struct NodoABB *izq;
    struct NodoABB *der;
} NodoABB;

int insertarABB(NodoABB **raiz, Libro *libro);
Libro *buscarABB(NodoABB *raiz, const char *isbn);
int eliminarABB(NodoABB **raiz, const char *isbn, Libro **libroEliminado);

void inordenABB(NodoABB *raiz);
void preordenABB(NodoABB *raiz);
void postordenABB(NodoABB *raiz);

int alturaABB(NodoABB *raiz);
int contarNodosABB(NodoABB *raiz);
int mostrarPorAutorABB(NodoABB *raiz, const char *autor);
void mostrarEstructuraABB(NodoABB *raiz, int nivel);

void liberarNodosABB(NodoABB *raiz);
void liberarCatalogoABB(NodoABB *raiz);

#endif
