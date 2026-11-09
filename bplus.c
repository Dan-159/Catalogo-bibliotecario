#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bplus.h"

/* Reserva anticipada: evita fallos de memoria despues de modificar el arbol. */
typedef struct ReservaNodos {
    NodoBPlus **nodos;
    int total;
    int usados;
} ReservaNodos;

typedef struct ResultadoInsercion {
    int dividido;
    ClaveBPlus separador;
    NodoBPlus *nuevoDerecho;
} ResultadoInsercion;

static ClaveBPlus crearClave(Libro *libro) {
    ClaveBPlus clave;

    clave.anio = libro->anio;
    strcpy(clave.isbn, libro->isbn);
    clave.libro = libro;

    return clave;
}

static int compararClaves(const ClaveBPlus *a, const ClaveBPlus *b) {
    int comparacion;

    if (a->anio < b->anio) return -1;
    if (a->anio > b->anio) return 1;

    comparacion = strcmp(a->isbn, b->isbn);
    if (comparacion < 0) return -1;
    if (comparacion > 0) return 1;
    return 0;
}

static ClaveBPlus claveMinima(NodoBPlus *nodo) {
    while (nodo != NULL && !nodo->esHoja) {
        nodo = nodo->hijos[0];
    }

    if (nodo != NULL && nodo->cantidad > 0) {
        return nodo->claves[0];
    }

    /* Solo se utiliza con subarboles no vacios. */
    {
        ClaveBPlus vacia;
        vacia.anio = 0;
        vacia.isbn[0] = '\0';
        vacia.libro = NULL;
        return vacia;
    }
}

static void actualizarSeparadores(NodoBPlus *nodo) {
    int i;

    if (nodo == NULL || nodo->esHoja) return;

    for (i = 0; i < nodo->cantidad; i++) {
        nodo->claves[i] = claveMinima(nodo->hijos[i + 1]);
        /* Los separadores solo guardan (anio, ISBN), no son duenos del Libro. */
        nodo->claves[i].libro = NULL;
    }
}

static int alturaNiveles(NodoBPlus *raiz) {
    int altura;

    altura = 0;
    while (raiz != NULL) {
        altura++;
        if (raiz->esHoja) break;
        raiz = raiz->hijos[0];
    }
    return altura;
}

static int inicializarReserva(ReservaNodos *reserva, int cantidad) {
    int i;

    reserva->nodos = (NodoBPlus **)malloc(sizeof(NodoBPlus *) * cantidad);
    if (reserva->nodos == NULL) return 0;

    reserva->total = cantidad;
    reserva->usados = 0;

    for (i = 0; i < cantidad; i++) {
        reserva->nodos[i] = (NodoBPlus *)calloc(1, sizeof(NodoBPlus));
        if (reserva->nodos[i] == NULL) {
            int j;
            for (j = 0; j < i; j++) free(reserva->nodos[j]);
            free(reserva->nodos);
            reserva->nodos = NULL;
            reserva->total = 0;
            reserva->usados = 0;
            return 0;
        }
    }

    return 1;
}

static NodoBPlus *tomarNodo(ReservaNodos *reserva, int esHoja) {
    NodoBPlus *nodo;

    if (reserva->usados >= reserva->total) return NULL;

    nodo = reserva->nodos[reserva->usados];
    reserva->nodos[reserva->usados] = NULL;
    reserva->usados++;

    memset(nodo, 0, sizeof(NodoBPlus));
    nodo->esHoja = esHoja;
    return nodo;
}

static void liberarReserva(ReservaNodos *reserva) {
    int i;

    if (reserva->nodos == NULL) return;

    for (i = 0; i < reserva->total; i++) {
        if (reserva->nodos[i] != NULL) free(reserva->nodos[i]);
    }

    free(reserva->nodos);
    reserva->nodos = NULL;
}

/* Busca el primer separador mayor que la clave; si coincide, va a la derecha. */
static int buscarHijo(NodoBPlus *nodo, const ClaveBPlus *clave) {
    int i;

    i = 0;
    while (i < nodo->cantidad &&
           compararClaves(clave, &nodo->claves[i]) >= 0) {
        i++;
    }
    return i;
}

static int insertarRec(NodoBPlus *nodo, const ClaveBPlus *clave,
                       ReservaNodos *reserva, ResultadoInsercion *resultado) {
    int i;
    int indice;
    NodoBPlus *nuevo;
    ResultadoInsercion resultadoHijo;

    resultado->dividido = 0;
    resultado->nuevoDerecho = NULL;

    if (nodo->esHoja) {
        i = nodo->cantidad;
        while (i > 0 && compararClaves(clave, &nodo->claves[i - 1]) < 0) {
            nodo->claves[i] = nodo->claves[i - 1];
            i--;
        }

        nodo->claves[i] = *clave;
        nodo->cantidad++;

        if (nodo->cantidad <= MAX_CLAVES_BPLUS) return 1;

        /* Desbordamiento de hoja: 4 claves se reparten 2 y 2. */
        nuevo = tomarNodo(reserva, 1);
        if (nuevo == NULL) return 0; /* La reserva previa evita este caso. */

        nuevo->cantidad = 2;
        nuevo->claves[0] = nodo->claves[2];
        nuevo->claves[1] = nodo->claves[3];
        nodo->cantidad = 2;

        nuevo->siguiente = nodo->siguiente;
        nodo->siguiente = nuevo;

        resultado->dividido = 1;
        resultado->separador = nuevo->claves[0];
        resultado->nuevoDerecho = nuevo;
        return 1;
    }

    indice = buscarHijo(nodo, clave);

    if (!insertarRec(nodo->hijos[indice], clave, reserva, &resultadoHijo)) {
        return 0;
    }

    if (!resultadoHijo.dividido) {
        actualizarSeparadores(nodo);
        return 1;
    }

    /* Inserta el nuevo hijo inmediatamente despues del hijo dividido. */
    for (i = nodo->cantidad + 1; i > indice + 1; i--) {
        nodo->hijos[i] = nodo->hijos[i - 1];
    }
    nodo->hijos[indice + 1] = resultadoHijo.nuevoDerecho;
    nodo->cantidad++;

    if (nodo->cantidad <= MAX_CLAVES_BPLUS) {
        actualizarSeparadores(nodo);
        return 1;
    }

    /* Desbordamiento interno: 5 hijos se reparten 2 y 3. */
    nuevo = tomarNodo(reserva, 0);
    if (nuevo == NULL) return 0;

    nuevo->hijos[0] = nodo->hijos[2];
    nuevo->hijos[1] = nodo->hijos[3];
    nuevo->hijos[2] = nodo->hijos[4];
    nuevo->cantidad = 2;

    nodo->hijos[2] = NULL;
    nodo->hijos[3] = NULL;
    nodo->hijos[4] = NULL;
    nodo->cantidad = 1;

    actualizarSeparadores(nodo);
    actualizarSeparadores(nuevo);

    resultado->dividido = 1;
    resultado->separador = claveMinima(nuevo);
    resultado->nuevoDerecho = nuevo;
    return 1;
}

int insertarBPlus(NodoBPlus **raiz, Libro *libro) {
    ReservaNodos reserva;
    ResultadoInsercion resultado;
    NodoBPlus *nuevaRaiz;
    ClaveBPlus clave;
    int cantidadReserva;

    if (raiz == NULL || libro == NULL || libro->isbn[0] == '\0') return 0;

    clave = crearClave(libro);

    if (buscarBPlus(*raiz, clave.anio, clave.isbn) != NULL) return 0;

    /* Como maximo se divide un nodo por nivel y se crea una nueva raiz. */
    cantidadReserva = alturaNiveles(*raiz) + 1;
    if (cantidadReserva < 1) cantidadReserva = 1;

    reserva.nodos = NULL;
    reserva.total = 0;
    reserva.usados = 0;

    if (!inicializarReserva(&reserva, cantidadReserva)) return 0;

    if (*raiz == NULL) {
        *raiz = tomarNodo(&reserva, 1);
        if (*raiz == NULL) {
            liberarReserva(&reserva);
            return 0;
        }
        (*raiz)->claves[0] = clave;
        (*raiz)->cantidad = 1;
        liberarReserva(&reserva);
        return 1;
    }

    if (!insertarRec(*raiz, &clave, &reserva, &resultado)) {
        liberarReserva(&reserva);
        return 0;
    }

    if (resultado.dividido) {
        nuevaRaiz = tomarNodo(&reserva, 0);
        if (nuevaRaiz == NULL) {
            /* No deberia ocurrir: se reservaron altura + 1 nodos. */
            liberarReserva(&reserva);
            return 0;
        }

        nuevaRaiz->hijos[0] = *raiz;
        nuevaRaiz->hijos[1] = resultado.nuevoDerecho;
        nuevaRaiz->cantidad = 1;
        actualizarSeparadores(nuevaRaiz);
        *raiz = nuevaRaiz;
    }

    liberarReserva(&reserva);
    return 1;
}

static NodoBPlus *buscarHoja(NodoBPlus *raiz, const ClaveBPlus *clave) {
    int indice;

    while (raiz != NULL && !raiz->esHoja) {
        indice = buscarHijo(raiz, clave);
        raiz = raiz->hijos[indice];
    }
    return raiz;
}

Libro *buscarBPlus(NodoBPlus *raiz, int anio, const char *isbn) {
    ClaveBPlus clave;
    NodoBPlus *hoja;
    int i;

    if (isbn == NULL) return NULL;

    clave.anio = anio;
    strcpy(clave.isbn, isbn);
    clave.libro = NULL;

    hoja = buscarHoja(raiz, &clave);
    if (hoja == NULL) return NULL;

    for (i = 0; i < hoja->cantidad; i++) {
        if (compararClaves(&hoja->claves[i], &clave) == 0) {
            return hoja->claves[i].libro;
        }
    }

    return NULL;
}

static void eliminarHijo(NodoBPlus *padre, int indice) {
    int cantidadHijos;
    int i;

    cantidadHijos = padre->cantidad + 1;
    for (i = indice; i < cantidadHijos - 1; i++) {
        padre->hijos[i] = padre->hijos[i + 1];
    }
    padre->hijos[cantidadHijos - 1] = NULL;
    padre->cantidad--;
}

static void actualizarClavesInternas(NodoBPlus *nodo) {
    actualizarSeparadores(nodo);
}

/* Repara un hijo que quedo por debajo de la ocupacion minima. */
static void rebalancearHijo(NodoBPlus *padre, int indice) {
    NodoBPlus *hijo;
    NodoBPlus *hermanoIzq;
    NodoBPlus *hermanoDer;
    int cantidadHijos;
    int i;
    int cantidadHijosIzq;
    int cantidadHijosDer;

    cantidadHijos = padre->cantidad + 1;
    hijo = padre->hijos[indice];
    hermanoIzq = (indice > 0) ? padre->hijos[indice - 1] : NULL;
    hermanoDer = (indice + 1 < cantidadHijos) ? padre->hijos[indice + 1] : NULL;

    if (hijo->esHoja) {
        if (hijo->cantidad >= MIN_CLAVES_HOJA_BPLUS) return;

        /* Prestamo desde la hoja izquierda. */
        if (hermanoIzq != NULL && hermanoIzq->cantidad > MIN_CLAVES_HOJA_BPLUS) {
            for (i = hijo->cantidad; i > 0; i--) {
                hijo->claves[i] = hijo->claves[i - 1];
            }
            hijo->claves[0] = hermanoIzq->claves[hermanoIzq->cantidad - 1];
            hermanoIzq->cantidad--;
            hijo->cantidad++;
            return;
        }

        /* Prestamo desde la hoja derecha. */
        if (hermanoDer != NULL && hermanoDer->cantidad > MIN_CLAVES_HOJA_BPLUS) {
            hijo->claves[hijo->cantidad] = hermanoDer->claves[0];
            hijo->cantidad++;
            for (i = 0; i < hermanoDer->cantidad - 1; i++) {
                hermanoDer->claves[i] = hermanoDer->claves[i + 1];
            }
            hermanoDer->cantidad--;
            return;
        }

        /* No se puede prestar: fusiona con un hermano. */
        if (hermanoIzq != NULL) {
            for (i = 0; i < hijo->cantidad; i++) {
                hermanoIzq->claves[hermanoIzq->cantidad + i] = hijo->claves[i];
            }
            hermanoIzq->cantidad += hijo->cantidad;
            hermanoIzq->siguiente = hijo->siguiente;
            free(hijo);
            eliminarHijo(padre, indice);
        } else if (hermanoDer != NULL) {
            for (i = 0; i < hermanoDer->cantidad; i++) {
                hijo->claves[hijo->cantidad + i] = hermanoDer->claves[i];
            }
            hijo->cantidad += hermanoDer->cantidad;
            hijo->siguiente = hermanoDer->siguiente;
            free(hermanoDer);
            eliminarHijo(padre, indice + 1);
        }
        return;
    }

    cantidadHijos = hijo->cantidad + 1;
    if (cantidadHijos >= MIN_HIJOS_INTERNO_BPLUS) return;

    /* Prestamo de un hijo desde el hermano interno izquierdo. */
    if (hermanoIzq != NULL) {
        cantidadHijosIzq = hermanoIzq->cantidad + 1;
        if (cantidadHijosIzq > MIN_HIJOS_INTERNO_BPLUS) {
            for (i = cantidadHijos; i > 0; i--) {
                hijo->hijos[i] = hijo->hijos[i - 1];
            }
            hijo->hijos[0] = hermanoIzq->hijos[cantidadHijosIzq - 1];
            hermanoIzq->hijos[cantidadHijosIzq - 1] = NULL;
            hermanoIzq->cantidad--;
            hijo->cantidad++;
            actualizarClavesInternas(hermanoIzq);
            actualizarClavesInternas(hijo);
            return;
        }
    }

    /* Prestamo de un hijo desde el hermano interno derecho. */
    if (hermanoDer != NULL) {
        cantidadHijosDer = hermanoDer->cantidad + 1;
        if (cantidadHijosDer > MIN_HIJOS_INTERNO_BPLUS) {
            hijo->hijos[cantidadHijos] = hermanoDer->hijos[0];
            hijo->cantidad++;
            for (i = 0; i < cantidadHijosDer - 1; i++) {
                hermanoDer->hijos[i] = hermanoDer->hijos[i + 1];
            }
            hermanoDer->hijos[cantidadHijosDer - 1] = NULL;
            hermanoDer->cantidad--;
            actualizarClavesInternas(hijo);
            actualizarClavesInternas(hermanoDer);
            return;
        }
    }

    /* Fusion de nodos internos: se concatenan sus listas de hijos. */
    if (hermanoIzq != NULL) {
        cantidadHijosIzq = hermanoIzq->cantidad + 1;
        for (i = 0; i < cantidadHijos; i++) {
            hermanoIzq->hijos[cantidadHijosIzq + i] = hijo->hijos[i];
            hijo->hijos[i] = NULL;
        }
        hermanoIzq->cantidad = cantidadHijosIzq + cantidadHijos - 1;
        actualizarClavesInternas(hermanoIzq);
        free(hijo);
        eliminarHijo(padre, indice);
    } else if (hermanoDer != NULL) {
        cantidadHijosDer = hermanoDer->cantidad + 1;
        for (i = 0; i < cantidadHijosDer; i++) {
            hijo->hijos[cantidadHijos + i] = hermanoDer->hijos[i];
            hermanoDer->hijos[i] = NULL;
        }
        hijo->cantidad = cantidadHijos + cantidadHijosDer - 1;
        actualizarClavesInternas(hijo);
        free(hermanoDer);
        eliminarHijo(padre, indice + 1);
    }
}

static NodoBPlus *eliminarRec(NodoBPlus *nodo, const ClaveBPlus *clave,
                              int *eliminado) {
    int i;
    int indice;

    if (nodo == NULL) return NULL;

    if (nodo->esHoja) {
        for (i = 0; i < nodo->cantidad; i++) {
            if (compararClaves(&nodo->claves[i], clave) == 0) {
                break;
            }
        }

        if (i == nodo->cantidad) return nodo;

        for (; i < nodo->cantidad - 1; i++) {
            nodo->claves[i] = nodo->claves[i + 1];
        }
        nodo->cantidad--;
        *eliminado = 1;
        return nodo;
    }

    indice = buscarHijo(nodo, clave);
    nodo->hijos[indice] = eliminarRec(nodo->hijos[indice], clave, eliminado);

    if (*eliminado) {
        rebalancearHijo(nodo, indice);
        actualizarSeparadores(nodo);
    }

    return nodo;
}

int eliminarBPlus(NodoBPlus **raiz, int anio, const char *isbn) {
    ClaveBPlus clave;
    int eliminado;
    NodoBPlus *anterior;

    if (raiz == NULL || *raiz == NULL || isbn == NULL) return 0;

    clave.anio = anio;
    strcpy(clave.isbn, isbn);
    clave.libro = NULL;
    eliminado = 0;

    *raiz = eliminarRec(*raiz, &clave, &eliminado);
    if (!eliminado) return 0;

    /* La raiz puede quedar vacia o con un unico hijo. */
    if ((*raiz)->esHoja && (*raiz)->cantidad == 0) {
        free(*raiz);
        *raiz = NULL;
    } else if (!(*raiz)->esHoja && (*raiz)->cantidad == 0) {
        anterior = *raiz;
        *raiz = anterior->hijos[0];
        anterior->hijos[0] = NULL;
        free(anterior);
    }

    return 1;
}

static NodoBPlus *buscarHojaInicial(NodoBPlus *raiz, int anioInicial) {
    ClaveBPlus clave;

    clave.anio = anioInicial;
    clave.isbn[0] = '\0';
    clave.libro = NULL;

    return buscarHoja(raiz, &clave);
}

int consultarRangoBPlus(NodoBPlus *raiz, int anioInicial, int anioFinal) {
    NodoBPlus *hoja;
    int i;
    int encontrados;
    ClaveBPlus *clave;

    if (raiz == NULL || anioInicial > anioFinal) return 0;

    hoja = buscarHojaInicial(raiz, anioInicial);
    encontrados = 0;

    while (hoja != NULL) {
        for (i = 0; i < hoja->cantidad; i++) {
            clave = &hoja->claves[i];

            if (clave->anio > anioFinal) {
                printf("\nTotal encontrado: %d libro(s).\n", encontrados);
                return encontrados;
            }

            if (clave->anio >= anioInicial) {
                printf("ISBN: %s | Titulo: %s | Autor: %s | Anio: %d\n",
                       clave->libro->isbn,
                       clave->libro->titulo,
                       clave->libro->autor,
                       clave->libro->anio);
                encontrados++;
            }
        }
        hoja = hoja->siguiente;
    }

    printf("\nTotal encontrado: %d libro(s).\n", encontrados);
    return encontrados;
}

int contarRangoBPlus(NodoBPlus *raiz, int anioInicial, int anioFinal) {
    NodoBPlus *hoja;
    int i;
    int encontrados;

    if (raiz == NULL || anioInicial > anioFinal) return 0;

    hoja = buscarHojaInicial(raiz, anioInicial);
    encontrados = 0;

    while (hoja != NULL) {
        for (i = 0; i < hoja->cantidad; i++) {
            if (hoja->claves[i].anio > anioFinal) return encontrados;
            if (hoja->claves[i].anio >= anioInicial) encontrados++;
        }
        hoja = hoja->siguiente;
    }
    return encontrados;
}

void mostrarHojasBPlus(NodoBPlus *raiz) {
    NodoBPlus *hoja;
    int i;

    if (raiz == NULL) {
        printf("El arbol B+ esta vacio.\n");
        return;
    }

    hoja = raiz;
    while (!hoja->esHoja) hoja = hoja->hijos[0];

    printf("\n--- HOJAS DEL ARBOL B+ (ORDEN ANIO, ISBN) ---\n");
    while (hoja != NULL) {
        printf("[ ");
        for (i = 0; i < hoja->cantidad; i++) {
            printf("(%d, %s) ", hoja->claves[i].anio, hoja->claves[i].isbn);
        }
        printf("] -> ");
        hoja = hoja->siguiente;
    }
    printf("NULL\n");
}

int alturaBPlus(NodoBPlus *raiz) {
    return alturaNiveles(raiz);
}

int contarNodosBPlus(NodoBPlus *raiz) {
    int total;
    int i;

    if (raiz == NULL) return 0;
    total = 1;
    if (!raiz->esHoja) {
        for (i = 0; i <= raiz->cantidad; i++) {
            total += contarNodosBPlus(raiz->hijos[i]);
        }
    }
    return total;
}

int contarHojasBPlus(NodoBPlus *raiz) {
    int total;
    int i;

    if (raiz == NULL) return 0;
    if (raiz->esHoja) return 1;

    total = 0;
    for (i = 0; i <= raiz->cantidad; i++) {
        total += contarHojasBPlus(raiz->hijos[i]);
    }
    return total;
}

int contarLibrosBPlus(NodoBPlus *raiz) {
    int total;
    int i;

    if (raiz == NULL) return 0;
    if (raiz->esHoja) return raiz->cantidad;

    total = 0;
    for (i = 0; i <= raiz->cantidad; i++) {
        total += contarLibrosBPlus(raiz->hijos[i]);
    }
    return total;
}

/* Validacion estructural utilizada por las pruebas del proyecto. */
static int validarRec(NodoBPlus *nodo, int esRaiz, int profundidad,
                      int *profundidadHoja, ClaveBPlus *ultima,
                      int *hayUltima, NodoBPlus **ultimaHoja) {
    int i;
    int cantidadHijos;

    if (nodo == NULL) return 0;

    if (nodo->esHoja) {
        if (nodo->cantidad < (esRaiz ? 1 : MIN_CLAVES_HOJA_BPLUS) ||
            nodo->cantidad > MAX_CLAVES_BPLUS) {
            /* La raiz hoja vacia se representa como raiz NULL. */
            return 0;
        }

        if (*profundidadHoja == -1) *profundidadHoja = profundidad;
        else if (*profundidadHoja != profundidad) return 0;

        for (i = 0; i < nodo->cantidad; i++) {
            if (i > 0 && compararClaves(&nodo->claves[i - 1], &nodo->claves[i]) >= 0) {
                return 0;
            }
            if (*hayUltima && compararClaves(ultima, &nodo->claves[i]) >= 0) {
                return 0;
            }
            *ultima = nodo->claves[i];
            *hayUltima = 1;
        }

        if (*ultimaHoja != NULL && (*ultimaHoja)->siguiente != nodo) return 0;
        *ultimaHoja = nodo;
        return 1;
    }

    if (nodo->cantidad < (esRaiz ? 1 : MIN_HIJOS_INTERNO_BPLUS - 1) ||
        nodo->cantidad > MAX_CLAVES_BPLUS) {
        return 0;
    }

    cantidadHijos = nodo->cantidad + 1;
    for (i = 0; i < cantidadHijos; i++) {
        if (nodo->hijos[i] == NULL || nodo->hijos[i]->esHoja !=
            (nodo->hijos[0]->esHoja)) {
            /* Todos los hijos directos deben estar al mismo nivel. */
            return 0;
        }

        if (i > 0) {
            ClaveBPlus minimo;
            minimo = claveMinima(nodo->hijos[i]);
            if (compararClaves(&nodo->claves[i - 1], &minimo) != 0) return 0;
        }

        if (!validarRec(nodo->hijos[i], 0, profundidad + 1,
                        profundidadHoja, ultima, hayUltima, ultimaHoja)) {
            return 0;
        }
    }

    return 1;
}

int validarBPlus(NodoBPlus *raiz) {
    int profundidadHoja;
    int hayUltima;
    ClaveBPlus ultima;
    NodoBPlus *ultimaHoja;

    if (raiz == NULL) return 1;

    profundidadHoja = -1;
    hayUltima = 0;
    ultimaHoja = NULL;

    if (!validarRec(raiz, 1, 1, &profundidadHoja,
                    &ultima, &hayUltima, &ultimaHoja)) {
        return 0;
    }

    return ultimaHoja != NULL && ultimaHoja->siguiente == NULL;
}

void liberarNodosBPlus(NodoBPlus *raiz) {
    int i;

    if (raiz == NULL) return;

    if (!raiz->esHoja) {
        for (i = 0; i <= raiz->cantidad; i++) {
            liberarNodosBPlus(raiz->hijos[i]);
        }
    }

    free(raiz);
}
