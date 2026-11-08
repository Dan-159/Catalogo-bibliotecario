#ifndef LIBRO_H
#define LIBRO_H

#define ISBN_TAM 20
#define TITULO_TAM 100
#define AUTOR_TAM 100

typedef struct {
    char isbn[ISBN_TAM];
    char titulo[TITULO_TAM];
    char autor[AUTOR_TAM];
    int anio;
} Libro;

#endif
