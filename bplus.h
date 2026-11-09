#ifndef BPLUS_H
#define BPLUS_H

#include "libro.h"

/* Orden 4: maximo 4 hijos y 3 claves por nodo. */
#define ORDEN_BPLUS 4
#define MAX_CLAVES_BPLUS (ORDEN_BPLUS - 1)
#define MIN_CLAVES_HOJA_BPLUS 2
#define MIN_HIJOS_INTERNO_BPLUS 2

typedef struct ClaveBPlus {
    int anio;
    char isbn[ISBN_TAM];
    Libro *libro;
} ClaveBPlus;

typedef struct NodoBPlus {
    int esHoja;
    int cantidad; /* Claves; en nodos internos = numero de hijos - 1. */
    ClaveBPlus claves[ORDEN_BPLUS]; /* Una posicion extra para desbordamiento. */
    struct NodoBPlus *hijos[ORDEN_BPLUS + 1];
    struct NodoBPlus *siguiente; /* Enlace entre hojas. */
} NodoBPlus;

int insertarBPlus(NodoBPlus **raiz, Libro *libro);
Libro *buscarBPlus(NodoBPlus *raiz, int anio, const char *isbn);
int eliminarBPlus(NodoBPlus **raiz, int anio, const char *isbn);

int consultarRangoBPlus(NodoBPlus *raiz, int anioInicial, int anioFinal);
int contarRangoBPlus(NodoBPlus *raiz, int anioInicial, int anioFinal);
void mostrarHojasBPlus(NodoBPlus *raiz);

int alturaBPlus(NodoBPlus *raiz);
int contarNodosBPlus(NodoBPlus *raiz);
int contarHojasBPlus(NodoBPlus *raiz);
int contarLibrosBPlus(NodoBPlus *raiz);
int validarBPlus(NodoBPlus *raiz);

/* Libera nodos e indices, pero no los Libro compartidos. */
void liberarNodosBPlus(NodoBPlus *raiz);

#endif
