#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "avl.h"

/* Imprime un registro sin modificarlo. */
static void imprimirLibroAVL(Libro *libro) {
    if (libro == NULL) {
        return;
    }

    printf("ISBN: %s | Titulo: %s | Autor: %s | Anio: %d\n",
           libro->isbn, libro->titulo, libro->autor, libro->anio);
}

/* Devuelve la altura almacenada; un arbol vacio tiene altura 0. */
static int alturaNodo(NodoAVL *nodo) {
    if (nodo == NULL) {
        return 0;
    }

    return nodo->altura;
}

/* Actualiza la altura a partir de las alturas de los hijos. */
static void actualizarAltura(NodoAVL *nodo) {
    int izquierda;
    int derecha;

    if (nodo == NULL) {
        return;
    }

    izquierda = alturaNodo(nodo->izq);
    derecha = alturaNodo(nodo->der);

    if (izquierda > derecha) {
        nodo->altura = izquierda + 1;
    } else {
        nodo->altura = derecha + 1;
    }
}

/* Factor de equilibrio: altura izquierda menos altura derecha. */
static int factorNodo(NodoAVL *nodo) {
    if (nodo == NULL) {
        return 0;
    }

    return alturaNodo(nodo->izq) - alturaNodo(nodo->der);
}

/* Rotacion simple a la derecha. El conteo se realiza al clasificar el caso. */
static NodoAVL *rotacionDerecha(NodoAVL *y) {
    NodoAVL *x;
    NodoAVL *subArbol;

    x = y->izq;
    subArbol = x->der;

    x->der = y;
    y->izq = subArbol;

    actualizarAltura(y);
    actualizarAltura(x);

    return x;
}

/* Rotacion simple a la izquierda. */
static NodoAVL *rotacionIzquierda(NodoAVL *x) {
    NodoAVL *y;
    NodoAVL *subArbol;

    y = x->der;
    subArbol = y->izq;

    y->izq = x;
    x->der = subArbol;

    actualizarAltura(x);
    actualizarAltura(y);

    return y;
}

/* Inicializa los contadores acumulados de rotaciones. */
void inicializarEstadisticasAVL(EstadisticasAVL *estadisticas) {
    if (estadisticas == NULL) {
        return;
    }

    estadisticas->casosLL = 0;
    estadisticas->casosRR = 0;
    estadisticas->casosLR = 0;
    estadisticas->casosRL = 0;
    estadisticas->rotacionesSimples = 0;
    estadisticas->rotacionesDobles = 0;
}

/* Cuenta las rotaciones primitivas: simple = 1 y doble = 2. */
unsigned long totalRotacionesPrimitivasAVL(const EstadisticasAVL *estadisticas) {
    if (estadisticas == NULL) {
        return 0;
    }

    return estadisticas->rotacionesSimples +
           (2UL * estadisticas->rotacionesDobles);
}

/*
 * Reequilibra un nodo que ya tiene actualizadas las alturas.
 * LL y RR aplican una rotacion simple.
 * LR y RL aplican dos rotaciones simples consecutivas.
 */
static NodoAVL *reequilibrarAVL(
    NodoAVL *nodo,
    EstadisticasAVL *estadisticas
) {
    int factor;
    int factorHijo;

    if (nodo == NULL) {
        return NULL;
    }

    actualizarAltura(nodo);
    factor = factorNodo(nodo);

    /* Desbalance hacia la izquierda. */
    if (factor > 1) {
        factorHijo = factorNodo(nodo->izq);

        if (factorHijo >= 0) {
            /* Caso LL: una rotacion simple a la derecha. */
            if (estadisticas != NULL) {
                estadisticas->casosLL++;
                estadisticas->rotacionesSimples++;
            }
            return rotacionDerecha(nodo);
        }

        /* Caso LR: izquierda sobre el hijo y derecha sobre el nodo. */
        if (estadisticas != NULL) {
            estadisticas->casosLR++;
            estadisticas->rotacionesDobles++;
        }
        nodo->izq = rotacionIzquierda(nodo->izq);
        return rotacionDerecha(nodo);
    }

    /* Desbalance hacia la derecha. */
    if (factor < -1) {
        factorHijo = factorNodo(nodo->der);

        if (factorHijo <= 0) {
            /* Caso RR: una rotacion simple a la izquierda. */
            if (estadisticas != NULL) {
                estadisticas->casosRR++;
                estadisticas->rotacionesSimples++;
            }
            return rotacionIzquierda(nodo);
        }

        /* Caso RL: derecha sobre el hijo e izquierda sobre el nodo. */
        if (estadisticas != NULL) {
            estadisticas->casosRL++;
            estadisticas->rotacionesDobles++;
        }
        nodo->der = rotacionDerecha(nodo->der);
        return rotacionIzquierda(nodo);
    }

    return nodo;
}

/* Insercion recursiva. exito se pone en 0 si hay duplicado o falla malloc. */
static NodoAVL *insertarRecAVL(
    NodoAVL *raiz,
    Libro *libro,
    int *exito,
    EstadisticasAVL *estadisticas
) {
    NodoAVL *nuevo;
    int comparacion;

    if (raiz == NULL) {
        nuevo = (NodoAVL *)malloc(sizeof(NodoAVL));

        if (nuevo == NULL) {
            *exito = 0;
            return NULL;
        }

        nuevo->libro = libro;
        nuevo->altura = 1;
        nuevo->izq = NULL;
        nuevo->der = NULL;
        *exito = 1;

        return nuevo;
    }

    comparacion = strcmp(libro->isbn, raiz->libro->isbn);

    if (comparacion == 0) {
        *exito = 0;
        return raiz;
    }

    if (comparacion < 0) {
        raiz->izq = insertarRecAVL(
            raiz->izq, libro, exito, estadisticas
        );
    } else {
        raiz->der = insertarRecAVL(
            raiz->der, libro, exito, estadisticas
        );
    }

    /* Si hubo duplicado o falta de memoria, no se modifica el equilibrio. */
    if (!(*exito)) {
        return raiz;
    }

    return reequilibrarAVL(raiz, estadisticas);
}

/* Inserta un libro. Devuelve 1 si se inserto y 0 si no fue posible. */
int insertarAVL(
    NodoAVL **raiz,
    Libro *libro,
    EstadisticasAVL *estadisticas
) {
    int exito;

    if (raiz == NULL || libro == NULL) {
        return 0;
    }

    exito = 1;
    *raiz = insertarRecAVL(*raiz, libro, &exito, estadisticas);

    return exito;
}

/* Busqueda iterativa por ISBN. */
Libro *buscarAVL(NodoAVL *raiz, const char *isbn) {
    int comparacion;

    if (isbn == NULL) {
        return NULL;
    }

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

/*
 * Extrae el nodo de menor ISBN de un subarbol.
 * Solo libera el nodo AVL; devuelve aparte el Libro compartido.
 */
static NodoAVL *extraerMinimoAVL(NodoAVL *raiz, Libro **libroMinimo,
                                 EstadisticasAVL *estadisticas) {
    if (raiz->izq == NULL) {
        NodoAVL *nuevoSubarbol;

        *libroMinimo = raiz->libro;
        nuevoSubarbol = raiz->der;
        free(raiz);

        return nuevoSubarbol;
    }

    raiz->izq = extraerMinimoAVL(raiz->izq, libroMinimo, estadisticas);

    return reequilibrarAVL(raiz, estadisticas);
}

/* Eliminacion recursiva. No libera el Libro que sale del indice. */
static NodoAVL *eliminarRecAVL(
    NodoAVL *raiz,
    const char *isbn,
    int *eliminado,
    Libro **libroEliminado,
    EstadisticasAVL *estadisticas
) {
    int comparacion;
    Libro *libroSucesor;
    NodoAVL *temporal;

    if (raiz == NULL) {
        return NULL;
    }

    comparacion = strcmp(isbn, raiz->libro->isbn);

    if (comparacion < 0) {
        raiz->izq = eliminarRecAVL(
            raiz->izq, isbn, eliminado, libroEliminado, estadisticas
        );
    } else if (comparacion > 0) {
        raiz->der = eliminarRecAVL(
            raiz->der, isbn, eliminado, libroEliminado, estadisticas
        );
    } else {
        *eliminado = 1;
        *libroEliminado = raiz->libro;

        /* Caso 1: cero o un hijo. */
        if (raiz->izq == NULL) {
            temporal = raiz->der;
            free(raiz);
            return temporal;
        }

        if (raiz->der == NULL) {
            temporal = raiz->izq;
            free(raiz);
            return temporal;
        }

        /* Caso 2: dos hijos. Se reemplaza por el sucesor inorden. */
        raiz->der = extraerMinimoAVL(
            raiz->der, &libroSucesor, estadisticas
        );
        raiz->libro = libroSucesor;
    }

    /* Si no se encontro el ISBN, no hubo cambios en este camino. */
    if (!(*eliminado)) {
        return raiz;
    }

    return reequilibrarAVL(raiz, estadisticas);
}

/* Elimina el nodo del indice y devuelve el Libro para que el catalogo lo libere. */
int eliminarAVL(
    NodoAVL **raiz,
    const char *isbn,
    Libro **libroEliminado,
    EstadisticasAVL *estadisticas
) {
    int eliminado;
    Libro *descartado;

    if (raiz == NULL || isbn == NULL) {
        return 0;
    }

    descartado = NULL;
    if (libroEliminado == NULL) {
        libroEliminado = &descartado;
    }

    *libroEliminado = NULL;
    eliminado = 0;

    *raiz = eliminarRecAVL(
        *raiz, isbn, &eliminado, libroEliminado, estadisticas
    );

    return eliminado;
}

/* Recorrido inorden: izquierda, raiz, derecha. */
void inordenAVL(NodoAVL *raiz) {
    if (raiz == NULL) {
        return;
    }

    inordenAVL(raiz->izq);
    imprimirLibroAVL(raiz->libro);
    inordenAVL(raiz->der);
}

/* Recorrido preorden: raiz, izquierda, derecha. */
void preordenAVL(NodoAVL *raiz) {
    if (raiz == NULL) {
        return;
    }

    imprimirLibroAVL(raiz->libro);
    preordenAVL(raiz->izq);
    preordenAVL(raiz->der);
}

/* Recorrido postorden: izquierda, derecha, raiz. */
void postordenAVL(NodoAVL *raiz) {
    if (raiz == NULL) {
        return;
    }

    postordenAVL(raiz->izq);
    postordenAVL(raiz->der);
    imprimirLibroAVL(raiz->libro);
}

/* Altura en niveles: vacio = 0, hoja = 1. Es O(1) porque se almacena. */
int alturaAVL(NodoAVL *raiz) {
    return alturaNodo(raiz);
}

int factorEquilibrioAVL(NodoAVL *raiz) {
    return factorNodo(raiz);
}

int contarNodosAVL(NodoAVL *raiz) {
    if (raiz == NULL) {
        return 0;
    }

    return 1 + contarNodosAVL(raiz->izq) + contarNodosAVL(raiz->der);
}

/* Muestra el arbol girado 90 grados. */
void mostrarEstructuraAVL(NodoAVL *raiz, int nivel) {
    int i;

    if (raiz == NULL) {
        return;
    }

    mostrarEstructuraAVL(raiz->der, nivel + 1);

    for (i = 0; i < nivel; i++) {
        printf("    ");
    }

    printf("%s (h=%d, FE=%d)\n",
           raiz->libro->isbn, raiz->altura, factorNodo(raiz));

    mostrarEstructuraAVL(raiz->izq, nivel + 1);
}

/* Libera unicamente los nodos AVL; los libros son compartidos con el ABB. */
void liberarNodosAVL(NodoAVL *raiz) {
    if (raiz == NULL) {
        return;
    }

    liberarNodosAVL(raiz->izq);
    liberarNodosAVL(raiz->der);
    free(raiz);
}
