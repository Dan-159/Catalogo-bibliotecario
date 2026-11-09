#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>

#include "libro.h"
#include "abb.h"
#include "avl.h"
#include "bplus.h"

void mostrarMenu(void);
int leerOpcion(void);
void leerCadena(const char *mensaje, char *destino, int tamano);
int leerEntero(const char *mensaje);
void mostrarLibro(Libro *libro);

void registrarLibro(NodoABB **raizABB, NodoAVL **raizAVL,
                    NodoBPlus **raizBPlus, EstadisticasAVL *estadisticas);
void eliminarLibro(NodoABB **raizABB, NodoAVL **raizAVL,
                   NodoBPlus **raizBPlus, EstadisticasAVL *estadisticas);
void modificarLibro(NodoAVL *raizAVL, NodoBPlus **raizBPlus);
void consultarLibro(NodoAVL *raizAVL);
void consultarAutor(NodoABB *raizABB);
void mostrarRecorridos(NodoABB *raizABB, NodoAVL *raizAVL);
void mostrarEstadisticas(NodoABB *raizABB, NodoAVL *raizAVL,
                         NodoBPlus *raizBPlus, EstadisticasAVL *estadisticas);
void consultarRango(NodoBPlus *raizBPlus);
void probarOperacionesBPlus(void);
void demostrarCasoDegenerado(void);
void probarOperacionesYRotacionesAVL(void);

int main(void) {
    NodoABB *raizABB;
    NodoAVL *raizAVL;
    NodoBPlus *raizBPlus;
    EstadisticasAVL estadisticas;
    int opcion;

    raizABB = NULL;
    raizAVL = NULL;
    raizBPlus = NULL;
    inicializarEstadisticasAVL(&estadisticas);

    do {
        mostrarMenu();
        opcion = leerOpcion();

        switch (opcion) {
            case 1:
                registrarLibro(&raizABB, &raizAVL, &raizBPlus, &estadisticas);
                break;
            case 2:
                eliminarLibro(&raizABB, &raizAVL, &raizBPlus, &estadisticas);
                break;
            case 3:
                modificarLibro(raizAVL, &raizBPlus);
                break;
            case 4:
                consultarLibro(raizAVL);
                break;
            case 5:
                printf("\n--- LIBROS ORDENADOS POR ISBN (INDICE AVL) ---\n");
                if (raizAVL == NULL) {
                    printf("El catalogo esta vacio.\n");
                } else {
                    inordenAVL(raizAVL);
                }
                break;
            case 6:
                consultarRango(raizBPlus);
                break;
            case 7:
                consultarAutor(raizABB);
                break;
            case 8:
                mostrarRecorridos(raizABB, raizAVL);
                break;
            case 9:
                mostrarEstadisticas(raizABB, raizAVL, raizBPlus, &estadisticas);
                break;
            case 10:
                demostrarCasoDegenerado();
                break;
            case 11:
                probarOperacionesYRotacionesAVL();
                break;
            case 12:
                mostrarHojasBPlus(raizBPlus);
                break;
            case 13:
                probarOperacionesBPlus();
                break;
            case 0:
                printf("\nCerrando el catalogo bibliotecario...\n");
                break;
            default:
                printf("\nOpcion no valida.\n");
        }

        if (opcion != 0) {
            printf("\nPresione ENTER para continuar...");
            {
                char pausa[8];
                fgets(pausa, sizeof(pausa), stdin);
            }
            system ("cls");
        }

    } while (opcion != 0);

    /*
     * B+ y AVL liberan solo sus nodos indice.
     * El ABB es responsable de liberar los registros Libro compartidos.
     */
    liberarNodosBPlus(raizBPlus);
    liberarNodosAVL(raizAVL);
    liberarCatalogoABB(raizABB);

    return 0;
}

void mostrarMenu(void) {
    printf("\n==============================================\n");
    printf("          CATALOGO BIBLIOTECARIO\n");
    printf("==============================================\n");
    printf("\n--- GESTION DE LIBROS ---\n");
    printf("1. Registrar libro\n");
    printf("2. Eliminar libro\n");
    printf("3. Modificar libro\n");
    printf("4. Buscar libro por ISBN (AVL)\n");
    printf("\n--- CONSULTAS ---\n");
    printf("5. Mostrar libros ordenados por ISBN (AVL)\n");
    printf("6. Consultar libros por rango de anios (B+)\n");
    printf("7. Buscar libros por autor\n");
    printf("\n--- ESTRUCTURAS Y PRUEBAS ---\n");
    printf("8. Mostrar recorridos ABB y AVL\n");
    printf("9. Mostrar alturas y rotaciones\n");
    printf("10. Comparar caso degenerado ABB frente a AVL\n");
    printf("11. Probar insercion, busqueda, eliminacion y cuatro rotaciones AVL\n");
    printf("12. Mostrar hojas enlazadas del arbol B+\n");
    printf("13. Ejecutar pruebas automaticas del B+\n");
    printf("\n0. Salir\n");
    printf("==============================================\n");
}

int leerOpcion(void) {
    char entrada[100];
    char *fin;
    long numero;

    while (1) {
        printf("Seleccione una opcion: ");

        if (fgets(entrada, sizeof(entrada), stdin) == NULL) {
            return 0;
        }

        errno = 0;
        numero = strtol(entrada, &fin, 10);

        while (isspace((unsigned char)*fin)) {
            fin++;
        }

        if (fin == entrada || *fin != '\0' || errno == ERANGE ||
            numero < INT_MIN || numero > INT_MAX) {
            printf("Entrada no valida. Ingrese un numero.\n");
        } else {
            return (int)numero;
        }
    }
}

void leerCadena(const char *mensaje, char *destino, int tamano) {
    size_t longitud;
    int caracter;

    printf("%s", mensaje);

    if (fgets(destino, tamano, stdin) == NULL) {
        destino[0] = '\0';
        return;
    }

    longitud = strlen(destino);

    if (longitud > 0 && destino[longitud - 1] == '\n') {
        destino[longitud - 1] = '\0';
    } else {
        while ((caracter = getchar()) != '\n' && caracter != EOF) {
            /* Descarta el resto de una linea demasiado larga. */
        }
    }
}

int leerEntero(const char *mensaje) {
    char entrada[100];
    char *fin;
    long numero;

    while (1) {
        printf("%s", mensaje);

        if (fgets(entrada, sizeof(entrada), stdin) == NULL) {
            return -1;
        }

        errno = 0;
        numero = strtol(entrada, &fin, 10);

        while (isspace((unsigned char)*fin)) {
            fin++;
        }

        if (fin == entrada || *fin != '\0' || errno == ERANGE ||
            numero < INT_MIN || numero > INT_MAX) {
            printf("Ingrese un numero entero valido.\n");
        } else {
            return (int)numero;
        }
    }
}

void mostrarLibro(Libro *libro) {
    if (libro == NULL) {
        return;
    }

    printf("\nISBN: %s\n", libro->isbn);
    printf("Titulo: %s\n", libro->titulo);
    printf("Autor: %s\n", libro->autor);
    printf("Anio: %d\n", libro->anio);
}

/*
 * Inserta el mismo puntero Libro en ambos indices.
 * Si falla la insercion AVL, revierte la insercion ABB.
 */
void registrarLibro(
    NodoABB **raizABB,
    NodoAVL **raizAVL,
    NodoBPlus **raizBPlus,
    EstadisticasAVL *estadisticas
) {
    Libro *nuevo;
    Libro *descartado;

    nuevo = (Libro *)malloc(sizeof(Libro));

    if (nuevo == NULL) {
        printf("Error: no hay memoria disponible.\n");
        return;
    }

    printf("\n--- REGISTRAR LIBRO ---\n");
    leerCadena("ISBN: ", nuevo->isbn, ISBN_TAM);
    leerCadena("Titulo: ", nuevo->titulo, TITULO_TAM);
    leerCadena("Autor: ", nuevo->autor, AUTOR_TAM);
    nuevo->anio = leerEntero("Anio de publicacion: ");

    if (nuevo->isbn[0] == '\0' || nuevo->titulo[0] == '\0' ||
        nuevo->autor[0] == '\0' || nuevo->anio <= 0) {
        printf("Datos incompletos o anio no valido. Registro cancelado.\n");
        free(nuevo);
        return;
    }

    if (buscarABB(*raizABB, nuevo->isbn) != NULL ||
        buscarAVL(*raizAVL, nuevo->isbn) != NULL) {
        printf("Ya existe un libro con ese ISBN.\n");
        free(nuevo);
        return;
    }

    /* El B+ recibe la clave compuesta (anio, ISBN). */
    if (!insertarBPlus(raizBPlus, nuevo)) {
        printf("No se pudo insertar en el indice B+.\n");
        free(nuevo);
        return;
    }

    if (!insertarABB(raizABB, nuevo)) {
        eliminarBPlus(raizBPlus, nuevo->anio, nuevo->isbn);
        printf("No se pudo insertar el registro en el ABB.\n");
        free(nuevo);
        return;
    }

    if (!insertarAVL(raizAVL, nuevo, estadisticas)) {
        /* Revierte los dos indices ya actualizados. */
        descartado = NULL;
        eliminarABB(raizABB, nuevo->isbn, &descartado);
        eliminarBPlus(raizBPlus, nuevo->anio, nuevo->isbn);
        free(nuevo);
        printf("No se pudo insertar en el AVL. Se revirtio el registro.\n");
        return;
    }

    printf("\nLibro registrado correctamente en ABB, AVL y B+.\n");
}

/* Elimina la misma referencia de ambos indices y libera el Libro una sola vez. */
void eliminarLibro(
    NodoABB **raizABB,
    NodoAVL **raizAVL,
    NodoBPlus **raizBPlus,
    EstadisticasAVL *estadisticas
) {
    char isbn[ISBN_TAM];
    Libro *libroABB;
    Libro *libroAVL;
    Libro *libroBPlus;
    Libro *eliminadoABB;
    Libro *eliminadoAVL;
    int resultadoABB;
    int resultadoAVL;
    int resultadoBPlus;

    printf("\n--- ELIMINAR LIBRO ---\n");
    leerCadena("Ingrese el ISBN: ", isbn, ISBN_TAM);

    libroABB = buscarABB(*raizABB, isbn);
    libroAVL = buscarAVL(*raizAVL, isbn);

    if (libroABB == NULL || libroAVL == NULL || libroABB != libroAVL) {
        printf("El ISBN no existe o los indices ABB/AVL no estan sincronizados.\n");
        return;
    }

    libroBPlus = buscarBPlus(*raizBPlus, libroABB->anio, isbn);
    if (libroBPlus != libroABB) {
        printf("El indice B+ no esta sincronizado. No se elimina el registro.\n");
        return;
    }

    eliminadoABB = NULL;
    eliminadoAVL = NULL;

    /* Primero se retira la clave secundaria; el Libro sigue vivo. */
    resultadoBPlus = eliminarBPlus(raizBPlus, libroABB->anio, isbn);
    resultadoABB = eliminarABB(raizABB, isbn, &eliminadoABB);
    resultadoAVL = eliminarAVL(raizAVL, isbn, &eliminadoAVL, estadisticas);

    if (resultadoBPlus && resultadoABB && resultadoAVL &&
        eliminadoABB == eliminadoAVL && eliminadoABB == libroABB) {
        printf("Libro eliminado correctamente de ABB, AVL y B+:\n");
        mostrarLibro(eliminadoABB);
        free(eliminadoABB);
    } else {
        printf("Error de consistencia al eliminar. Revise los indices.\n");
        printf("No se libera el registro para evitar referencias colgantes.\n");
    }
}

/* El ISBN no se modifica porque es la clave de ambos arboles. */
void modificarLibro(NodoAVL *raizAVL, NodoBPlus **raizBPlus) {
    char isbn[ISBN_TAM];
    char nuevoTitulo[TITULO_TAM];
    char nuevoAutor[AUTOR_TAM];
    int nuevoAnio;
    int anioAnterior;
    Libro *libro;

    printf("\n--- MODIFICAR LIBRO ---\n");
    leerCadena("Ingrese el ISBN del libro: ", isbn, ISBN_TAM);

    libro = buscarAVL(raizAVL, isbn);

    if (libro == NULL) {
        printf("No existe un libro con ese ISBN.\n");
        return;
    }

    printf("\nDatos actuales:");
    mostrarLibro(libro);

    leerCadena("Nuevo titulo: ", nuevoTitulo, TITULO_TAM);
    leerCadena("Nuevo autor: ", nuevoAutor, AUTOR_TAM);
    nuevoAnio = leerEntero("Nuevo anio: ");

    if (nuevoTitulo[0] == '\0' || nuevoAutor[0] == '\0' || nuevoAnio <= 0) {
        printf("Datos no validos. No se modifico el libro.\n");
        return;
    }

    anioAnterior = libro->anio;

    if (buscarBPlus(*raizBPlus, anioAnterior, libro->isbn) != libro) {
        printf("El indice B+ no esta sincronizado. No se modifica el libro.\n");
        return;
    }

    if (nuevoAnio != anioAnterior) {
        /* Inserta primero la nueva clave: si falla, la antigua sigue intacta. */
        libro->anio = nuevoAnio;

        if (!insertarBPlus(raizBPlus, libro)) {
            libro->anio = anioAnterior;
            printf("No se pudo actualizar el indice B+. No se modifico el libro.\n");
            return;
        }

        if (!eliminarBPlus(raizBPlus, anioAnterior, libro->isbn)) {
            eliminarBPlus(raizBPlus, nuevoAnio, libro->isbn);
            libro->anio = anioAnterior;
            printf("No se pudo retirar la clave anterior. Se revirtio el cambio.\n");
            return;
        }
    }

    strcpy(libro->titulo, nuevoTitulo);
    strcpy(libro->autor, nuevoAutor);
    libro->anio = nuevoAnio;

    printf("Libro modificado correctamente.\n");
    if (nuevoAnio != anioAnterior) {
        printf("El indice B+ tambien se actualizo con el nuevo anio.\n");
    }
}

void consultarLibro(NodoAVL *raizAVL) {
    char isbn[ISBN_TAM];
    Libro *libro;

    printf("\n--- BUSCAR LIBRO POR ISBN (AVL) ---\n");
    leerCadena("Ingrese el ISBN: ", isbn, ISBN_TAM);

    libro = buscarAVL(raizAVL, isbn);

    if (libro == NULL) {
        printf("No se encontro el libro.\n");
    } else {
        mostrarLibro(libro);
    }
}

void consultarAutor(NodoABB *raizABB) {
    char autor[AUTOR_TAM];
    int cantidad;

    printf("\n--- BUSCAR POR AUTOR ---\n");
    leerCadena("Ingrese el nombre del autor: ", autor, AUTOR_TAM);

    cantidad = mostrarPorAutorABB(raizABB, autor);

    if (cantidad == 0) {
        printf("No se encontraron libros de ese autor.\n");
    } else {
        printf("\nTotal encontrado: %d libro(s).\n", cantidad);
    }
}

void mostrarRecorridos(NodoABB *raizABB, NodoAVL *raizAVL) {
    if (raizABB == NULL && raizAVL == NULL) {
        printf("Los indices estan vacios.\n");
        return;
    }

    printf("\n========== RECORRIDOS ABB ==========" );
    if (raizABB == NULL) {
        printf("\nABB vacio.\n");
    } else {
        printf("\n--- INORDEN ---\n");
        inordenABB(raizABB);
        printf("\n--- PREORDEN ---\n");
        preordenABB(raizABB);
        printf("\n--- POSTORDEN ---\n");
        postordenABB(raizABB);
    }

    printf("\n========== RECORRIDOS AVL ==========");
    if (raizAVL == NULL) {
        printf("\nAVL vacio.\n");
    } else {
        printf("\n--- INORDEN ---\n");
        inordenAVL(raizAVL);
        printf("\n--- PREORDEN ---\n");
        preordenAVL(raizAVL);
        printf("\n--- POSTORDEN ---\n");
        postordenAVL(raizAVL);
    }
}

void mostrarEstadisticas(
    NodoABB *raizABB,
    NodoAVL *raizAVL,
    NodoBPlus *raizBPlus,
    EstadisticasAVL *estadisticas
) {
    printf("\n--- COMPARACION DE ESTADISTICAS ---\n");
    printf("Cantidad de libros ABB: %d\n", contarNodosABB(raizABB));
    printf("Altura ABB: %d nivel(es)\n", alturaABB(raizABB));
    printf("Cantidad de libros AVL: %d\n", contarNodosAVL(raizAVL));
    printf("Altura AVL: %d nivel(es)\n", alturaAVL(raizAVL));

    printf("\n--- ROTACIONES ACUMULADAS DEL AVL ---\n");
    printf("Casos LL: %lu\n", estadisticas->casosLL);
    printf("Casos RR: %lu\n", estadisticas->casosRR);
    printf("Casos LR: %lu\n", estadisticas->casosLR);
    printf("Casos RL: %lu\n", estadisticas->casosRL);
    printf("Reequilibrios simples: %lu\n", estadisticas->rotacionesSimples);
    printf("Reequilibrios dobles: %lu\n", estadisticas->rotacionesDobles);
    printf("Rotaciones primitivas ejecutadas: %lu\n",
           totalRotacionesPrimitivasAVL(estadisticas));
    printf("Nota: una rotacion doble equivale a dos rotaciones primitivas.\n");

    printf("\n--- ESTADISTICAS DEL B+ (INDICE POR ANIO) ---\n");
    printf("Libros indexados: %d\n", contarLibrosBPlus(raizBPlus));
    printf("Altura B+: %d nivel(es)\n", alturaBPlus(raizBPlus));
    printf("Nodos B+: %d\n", contarNodosBPlus(raizBPlus));
    printf("Hojas B+: %d\n", contarHojasBPlus(raizBPlus));
    printf("Validacion estructural: %s\n",
           validarBPlus(raizBPlus) ? "CORRECTA" : "ERROR");
}

/* Consulta inclusiva por intervalo de anios mediante las hojas enlazadas. */
void consultarRango(NodoBPlus *raizBPlus) {
    int anioInicial;
    int anioFinal;

    printf("\n--- CONSULTA POR RANGO DE ANIOS (B+) ---\n");
    anioInicial = leerEntero("Anio inicial: ");
    anioFinal = leerEntero("Anio final: ");

    if (anioInicial <= 0 || anioFinal <= 0 || anioInicial > anioFinal) {
        printf("Intervalo no valido.\n");
        return;
    }

    if (raizBPlus == NULL) {
        printf("El catalogo esta vacio.\n");
        return;
    }

    printf("\nLibros publicados entre %d y %d (incluidos):\n",
           anioInicial, anioFinal);
    if (consultarRangoBPlus(raizBPlus, anioInicial, anioFinal) == 0) {
        printf("No se encontraron libros en ese intervalo.\n");
    }
}

/* Prueba insercion, busqueda, duplicados, rangos, eliminacion y fusiones. */
void probarOperacionesBPlus(void) {
    Libro datos[40];
    Libro duplicado;
    NodoBPlus *raiz;
    int orden[40];
    int i;
    int indice;
    int correctas;
    int esperado;
    int real;
    Libro *encontrado;

    raiz = NULL;
    correctas = 1;

    printf("\n--- PRUEBAS AUTOMATICAS DEL ARBOL B+ ---\n");

    /* La permutacion 17*i mod 40 evita insertar en orden de clave. */
    for (i = 0; i < 40; i++) {
        sprintf(datos[i].isbn, "97800000%04d", i + 1);
        sprintf(datos[i].titulo, "Libro de prueba %d", i + 1);
        strcpy(datos[i].autor, "Autor de prueba");
        datos[i].anio = 1980 + ((i * 13) % 25);
        orden[i] = (i * 17) % 40;
    }

    for (i = 0; i < 40; i++) {
        indice = orden[i];
        if (!insertarBPlus(&raiz, &datos[indice]) || !validarBPlus(raiz)) {
            correctas = 0;
            break;
        }
    }

    printf("Insercion de 40 libros y validacion tras cada insercion: %s\n",
           correctas ? "CORRECTA" : "FALLIDA");
    printf("Cantidad de registros: %d; altura: %d; hojas: %d.\n",
           contarLibrosBPlus(raiz), alturaBPlus(raiz), contarHojasBPlus(raiz));

    encontrado = buscarBPlus(raiz, datos[19].anio, datos[19].isbn);
    printf("Busqueda por clave compuesta (anio, ISBN): %s\n",
           encontrado == &datos[19] ? "CORRECTA" : "FALLIDA");

    duplicado = datos[19];
    printf("Rechazo de clave compuesta duplicada: %s\n",
           !insertarBPlus(&raiz, &duplicado) ? "CORRECTO" : "FALLIDO");

    esperado = 0;
    for (i = 0; i < 40; i++) {
        if (datos[i].anio >= 1990 && datos[i].anio <= 2000) esperado++;
    }
    real = contarRangoBPlus(raiz, 1990, 2000);
    printf("Consulta de rango 1990-2000: %s (%d registros).\n",
           real == esperado ? "CORRECTA" : "FALLIDA", real);

    /* Elimina en orden distinto para ejercitar prestamos y fusiones. */
    for (i = 0; i < 40; i++) {
        indice = (i * 13) % 40;
        if (!eliminarBPlus(&raiz, datos[indice].anio, datos[indice].isbn) ||
            buscarBPlus(raiz, datos[indice].anio, datos[indice].isbn) != NULL ||
            !validarBPlus(raiz)) {
            correctas = 0;
            break;
        }
    }

    printf("Eliminacion de 40 registros y validacion tras cada eliminacion: %s\n",
           correctas && raiz == NULL ? "CORRECTA" : "FALLIDA");
    printf("Estado final: %d libros, altura %d.\n",
           contarLibrosBPlus(raiz), alturaBPlus(raiz));

    liberarNodosBPlus(raiz);
}

/* Compara dos estructuras temporales, sin modificar el catalogo principal. */
void demostrarCasoDegenerado(void) {
    Libro datos[7] = {
        {"0000000001", "Libro 1", "Autor A", 2001},
        {"0000000002", "Libro 2", "Autor A", 2002},
        {"0000000003", "Libro 3", "Autor A", 2003},
        {"0000000004", "Libro 4", "Autor A", 2004},
        {"0000000005", "Libro 5", "Autor A", 2005},
        {"0000000006", "Libro 6", "Autor A", 2006},
        {"0000000007", "Libro 7", "Autor A", 2007}
    };
    NodoABB *abbPrueba;
    NodoAVL *avlPrueba;
    EstadisticasAVL estadisticasPrueba;
    int i;

    abbPrueba = NULL;
    avlPrueba = NULL;
    inicializarEstadisticasAVL(&estadisticasPrueba);

    printf("\n--- CASO DEGENERADO: ISBN ASCENDENTES ---\n");
    printf("Se insertan los mismos 7 ISBN en ABB y AVL.\n");

    for (i = 0; i < 7; i++) {
        if (!insertarABB(&abbPrueba, &datos[i])) {
            printf("Error al construir el ABB temporal.\n");
            liberarNodosABB(abbPrueba);
            liberarNodosAVL(avlPrueba);
            return;
        }

        if (!insertarAVL(&avlPrueba, &datos[i], &estadisticasPrueba)) {
            printf("Error al construir el AVL temporal.\n");
            liberarNodosABB(abbPrueba);
            liberarNodosAVL(avlPrueba);
            return;
        }
    }

    printf("\nABB (girado 90 grados):\n");
    mostrarEstructuraABB(abbPrueba, 0);

    printf("\nAVL (girado 90 grados; se indica altura y FE):\n");
    mostrarEstructuraAVL(avlPrueba, 0);

    printf("\nResultado ABB: %d nodos, altura %d.\n",
           contarNodosABB(abbPrueba), alturaABB(abbPrueba));
    printf("Resultado AVL: %d nodos, altura %d.\n",
           contarNodosAVL(avlPrueba), alturaAVL(avlPrueba));
    printf("Rotaciones primitivas del AVL: %lu.\n",
           totalRotacionesPrimitivasAVL(&estadisticasPrueba));

    printf("\nEn el ABB la altura llega a n porque las claves se insertan ordenadas.\n");
    printf("El AVL reorganiza los nodos para conservar una altura O(log n).\n");

    /* Los libros son un arreglo local; solo se liberan nodos de los arboles. */
    liberarNodosABB(abbPrueba);
    liberarNodosAVL(avlPrueba);
}

/* Prueba cada uno de los cuatro casos de reequilibrio. */
static void demostrarCasoRotacion(
    const char *nombre,
    Libro datos[3],
    int secuencia[3]
) {
    NodoAVL *raiz;
    EstadisticasAVL estadisticas;
    int i;

    raiz = NULL;
    inicializarEstadisticasAVL(&estadisticas);

    for (i = 0; i < 3; i++) {
        if (!insertarAVL(&raiz, &datos[secuencia[i]], &estadisticas)) {
            printf("No se pudo completar la prueba %s.\n", nombre);
            liberarNodosAVL(raiz);
            return;
        }
    }

    printf("\nCaso %s: raiz final %s, altura %d.\n",
           nombre, raiz->libro->isbn, alturaAVL(raiz));
    printf("  LL=%lu, RR=%lu, LR=%lu, RL=%lu; primitivas=%lu.\n",
           estadisticas.casosLL, estadisticas.casosRR,
           estadisticas.casosLR, estadisticas.casosRL,
           totalRotacionesPrimitivasAVL(&estadisticas));
    mostrarEstructuraAVL(raiz, 0);

    liberarNodosAVL(raiz);
}

/* Pruebas autocontenidas: usan Libros locales y no afectan al catalogo. */
void probarOperacionesYRotacionesAVL(void) {
    Libro datos[5] = {
        {"0000000030", "Libro 30", "Autor T", 2000},
        {"0000000020", "Libro 20", "Autor T", 2001},
        {"0000000040", "Libro 40", "Autor T", 2002},
        {"0000000010", "Libro 10", "Autor T", 2003},
        {"0000000025", "Libro 25", "Autor T", 2004}
    };
    Libro duplicado;
    Libro *encontrado;
    Libro *eliminado;
    NodoAVL *raiz;
    EstadisticasAVL estadisticas;
    int secuenciaLL[3] = {2, 1, 0};
    int secuenciaRR[3] = {0, 1, 2};
    int secuenciaLR[3] = {2, 0, 1};
    int secuenciaRL[3] = {0, 2, 1};
    int insercionesCorrectas;

    printf("\n--- PRUEBAS AUTOMATICAS DEL AVL ---\n");

    /* Las secuencias producen respectivamente LL, RR, LR y RL. */
    {
        Libro ll[3] = {
            {"0000000010", "10", "Prueba", 2000},
            {"0000000020", "20", "Prueba", 2000},
            {"0000000030", "30", "Prueba", 2000}
        };
        Libro rr[3] = {
            {"0000000010", "10", "Prueba", 2000},
            {"0000000020", "20", "Prueba", 2000},
            {"0000000030", "30", "Prueba", 2000}
        };
        Libro lr[3] = {
            {"0000000010", "10", "Prueba", 2000},
            {"0000000020", "20", "Prueba", 2000},
            {"0000000030", "30", "Prueba", 2000}
        };
        Libro rl[3] = {
            {"0000000010", "10", "Prueba", 2000},
            {"0000000020", "20", "Prueba", 2000},
            {"0000000030", "30", "Prueba", 2000}
        };

        demostrarCasoRotacion("LL", ll, secuenciaLL);
        demostrarCasoRotacion("RR", rr, secuenciaRR);
        demostrarCasoRotacion("LR", lr, secuenciaLR);
        demostrarCasoRotacion("RL", rl, secuenciaRL);
    }

    printf("\n--- PRUEBA DE INSERCION, BUSQUEDA Y ELIMINACION ---\n");
    raiz = NULL;
    inicializarEstadisticasAVL(&estadisticas);
    insercionesCorrectas = 1;

    if (!insertarAVL(&raiz, &datos[0], &estadisticas) ||
        !insertarAVL(&raiz, &datos[1], &estadisticas) ||
        !insertarAVL(&raiz, &datos[2], &estadisticas) ||
        !insertarAVL(&raiz, &datos[3], &estadisticas) ||
        !insertarAVL(&raiz, &datos[4], &estadisticas)) {
        insercionesCorrectas = 0;
    }

    printf("Insercion de 5 libros: %s\n",
           insercionesCorrectas ? "CORRECTA" : "FALLIDA");

    strcpy(duplicado.isbn, "0000000020");
    strcpy(duplicado.titulo, "Duplicado");
    strcpy(duplicado.autor, "Autor T");
    duplicado.anio = 2020;

    printf("Rechazo de ISBN duplicado: %s\n",
           !insertarAVL(&raiz, &duplicado, &estadisticas) ? "CORRECTO" : "FALLIDO");

    encontrado = buscarAVL(raiz, "0000000025");
    printf("Busqueda de ISBN existente: %s\n",
           encontrado == &datos[4] ? "CORRECTA" : "FALLIDA");

    eliminado = NULL;
    printf("Eliminacion de nodo con dos hijos: ");
    if (eliminarAVL(&raiz, "0000000020", &eliminado, &estadisticas) &&
        eliminado == &datos[1] &&
        buscarAVL(raiz, "0000000020") == NULL &&
        buscarAVL(raiz, "0000000025") == &datos[4]) {
        printf("CORRECTA\n");
    } else {
        printf("FALLIDA\n");
    }

    eliminado = NULL;
    printf("Eliminacion de nodo hoja: ");
    if (eliminarAVL(&raiz, "0000000010", &eliminado, &estadisticas) &&
        eliminado == &datos[3] &&
        buscarAVL(raiz, "0000000010") == NULL) {
        printf("CORRECTA\n");
    } else {
        printf("FALLIDA\n");
    }

    printf("Cantidad final esperada de nodos: 3; cantidad real: %d.\n",
           contarNodosAVL(raiz));
    printf("Altura final del arbol de prueba: %d.\n", alturaAVL(raiz));

    liberarNodosAVL(raiz);
}
