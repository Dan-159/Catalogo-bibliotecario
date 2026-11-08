#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "abb.h"

/* Muestra los datos de un libro. */
static void imprimirLibro(Libro *libro) {
    if (libro == NULL) {
        return;
    }

    printf("ISBN: %s | Titulo: %s | Autor: %s | Anio: %d\n",
           libro->isbn, libro->titulo, libro->autor, libro->anio);
}

/* Inserta un nuevo nodo. Devuelve 1 si tiene exito y 0 si falla. */
int insertarABB(NodoABB **raiz, Libro *libro) {
    NodoABB **actual;
    NodoABB *nuevo;
    int comparacion;

    if (raiz == NULL || libro == NULL) {
        return 0;
    }

    actual = raiz;

    while (*actual != NULL) {
        comparacion = strcmp(libro->isbn, (*actual)->libro->isbn);

        if (comparacion == 0) {
            return 0; /* ISBN duplicado */
        } else if (comparacion < 0) {
            actual = &((*actual)->izq);
        } else {
            actual = &((*actual)->der);
        }
    }

    nuevo = (NodoABB *)malloc(sizeof(NodoABB));
    if (nuevo == NULL) {
        return 0;
    }

    nuevo->libro = libro;
    nuevo->izq = NULL;
    nuevo->der = NULL;

    *actual = nuevo;
    return 1;
}

/* Busca un libro por ISBN. Devuelve NULL si no existe. */
Libro *buscarABB(NodoABB *raiz, const char *isbn) {
    int comparacion;

    while (raiz != NULL) {
        comparacion = strcmp(isbn, raiz->libro->isbn);

        if (comparacion == 0) {
            return raiz->libro;
        } else if (comparacion < 0) {
            raiz = raiz->izq;
        } else {
            raiz = raiz->der;
        }
    }

    return NULL;
}

/* Extrae el nodo con el menor ISBN de un subarbol. */
static NodoABB *extraerMinimo(NodoABB *raiz, Libro **libroMinimo) {
    NodoABB *nuevoSubarbol;

    if (raiz->izq == NULL) {
        *libroMinimo = raiz->libro;
        nuevoSubarbol = raiz->der;
        free(raiz);
        return nuevoSubarbol;
    }

    raiz->izq = extraerMinimo(raiz->izq, libroMinimo);
    return raiz;
}

/* Eliminacion recursiva. Libera el nodo, pero no el libro. */
static NodoABB *eliminarRec(
    NodoABB *raiz,
    const char *isbn,
    int *eliminado,
    Libro **libroEliminado
) {
    int comparacion;
    NodoABB *temporal;
    Libro *libroReemplazo;

    if (raiz == NULL) {
        return NULL;
    }

    comparacion = strcmp(isbn, raiz->libro->isbn);

    if (comparacion < 0) {
        raiz->izq = eliminarRec(raiz->izq, isbn, eliminado, libroEliminado);
    } else if (comparacion > 0) {
        raiz->der = eliminarRec(raiz->der, isbn, eliminado, libroEliminado);
    } else {
        *eliminado = 1;
        *libroEliminado = raiz->libro;

        /* Caso 1: el nodo no tiene hijo izquierdo. */
        if (raiz->izq == NULL) {
            temporal = raiz->der;
            free(raiz);
            return temporal;
        }

        /* Caso 2: el nodo no tiene hijo derecho. */
        if (raiz->der == NULL) {
            temporal = raiz->izq;
            free(raiz);
            return temporal;
        }

        /* Caso 3: tiene dos hijos. Se usa el sucesor inorden. */
        raiz->der = extraerMinimo(raiz->der, &libroReemplazo);
        raiz->libro = libroReemplazo;
    }

    return raiz;
}

/* Elimina un libro y devuelve su puntero para que el llamador lo libere. */
int eliminarABB(NodoABB **raiz, const char *isbn, Libro **libroEliminado) {
    int eliminado;
    Libro *descartado;

    if (raiz == NULL || isbn == NULL) {
        return 0;
    }

    eliminado = 0;
    descartado = NULL;

    if (libroEliminado == NULL) {
        libroEliminado = &descartado;
    }
    *libroEliminado = NULL;

    *raiz = eliminarRec(*raiz, isbn, &eliminado, libroEliminado);
    return eliminado;
}

/* Recorrido inorden: izquierda, raiz, derecha. */
void inordenABB(NodoABB *raiz) {
    if (raiz == NULL) {
        return;
    }

    inordenABB(raiz->izq);
    imprimirLibro(raiz->libro);
    inordenABB(raiz->der);
}

/* Recorrido preorden: raiz, izquierda, derecha. */
void preordenABB(NodoABB *raiz) {
    if (raiz == NULL) {
        return;
    }

    imprimirLibro(raiz->libro);
    preordenABB(raiz->izq);
    preordenABB(raiz->der);
}

/* Recorrido postorden: izquierda, derecha, raiz. */
void postordenABB(NodoABB *raiz) {
    if (raiz == NULL) {
        return;
    }

    postordenABB(raiz->izq);
    postordenABB(raiz->der);
    imprimirLibro(raiz->libro);
}

/* Altura medida en niveles: arbol vacio = 0 y hoja = 1. */
int alturaABB(NodoABB *raiz) {
    int alturaIzquierda;
    int alturaDerecha;

    if (raiz == NULL) {
        return 0;
    }

    alturaIzquierda = alturaABB(raiz->izq);
    alturaDerecha = alturaABB(raiz->der);

    if (alturaIzquierda > alturaDerecha) {
        return alturaIzquierda + 1;
    }

    return alturaDerecha + 1;
}

int contarNodosABB(NodoABB *raiz) {
    if (raiz == NULL) {
        return 0;
    }

    return 1 + contarNodosABB(raiz->izq) + contarNodosABB(raiz->der);
}

/* Busca por autor recorriendo todo el ABB. */
int mostrarPorAutorABB(NodoABB *raiz, const char *autor) {
    int encontrados;

    if (raiz == NULL) {
        return 0;
    }

    encontrados = mostrarPorAutorABB(raiz->izq, autor);

    if (strcmp(raiz->libro->autor, autor) == 0) {
        imprimirLibro(raiz->libro);
        encontrados++;
    }

    encontrados += mostrarPorAutorABB(raiz->der, autor);
    return encontrados;
}

/* Muestra el arbol girado: derecha arriba e izquierda abajo. */
void mostrarEstructuraABB(NodoABB *raiz, int nivel) {
    int i;

    if (raiz == NULL) {
        return;
    }

    mostrarEstructuraABB(raiz->der, nivel + 1);

    for (i = 0; i < nivel; i++) {
        printf("    ");
    }
    printf("%s\n", raiz->libro->isbn);

    mostrarEstructuraABB(raiz->izq, nivel + 1);
}

/* Libera solamente los nodos del arbol, no los libros. */
void liberarNodosABB(NodoABB *raiz) {
    if (raiz == NULL) {
        return;
    }

    liberarNodosABB(raiz->izq);
    liberarNodosABB(raiz->der);
    free(raiz);
}

/* Libera los libros dinamicos y los nodos del catalogo principal. */
void liberarCatalogoABB(NodoABB *raiz) {
    if (raiz == NULL) {
        return;
    }

    liberarCatalogoABB(raiz->izq);
    liberarCatalogoABB(raiz->der);
    free(raiz->libro);
    free(raiz);
}
