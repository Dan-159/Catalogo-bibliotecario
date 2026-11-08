CATALOGO BIBLIOTECARIO - ETAPA 4 (ABB + AVL + B+)
=================================================
ARCHIVOS DEL PROYECTO
---------------------
libro.h  Estructura compartida del registro Libro.
abb.h    Interfaz y nodo del arbol binario de busqueda.
abb.c    Insercion, busqueda, eliminacion y recorridos ABB.
avl.h    Interfaz, nodo AVL y estadisticas de rotaciones.
avl.c    Insercion, busqueda, eliminacion y rotaciones AVL.

COMPILACION EN DEV-C++
---------------------
1. Abra el proyecto  o cree un proyecto de consola en C.
2. Agregue libro.h, abb.h, abb.c, avl.h y avl.c.

DISENO DEL INDICE B+
--------------------
Se utiliza un arbol B+ de orden 4:
- Un nodo interno puede tener hasta 4 hijos y 3 claves separadoras.
- Una hoja puede almacenar hasta 3 registros.
- Los nodos no raiz mantienen al menos 2 hijos (internos) o 2 claves (hojas).
- La raiz tiene reglas especiales de ocupacion.
- Las hojas se enlazan mediante el campo siguiente.

La clave del indice secundario es compuesta: (anio, ISBN). Esto admite
varios libros publicados en el mismo anio y establece un orden total.
Los ISBN siguen siendo unicos en el catalogo completo porque ABB y AVL
validan la clave principal antes del alta.

En nodos internos, las claves separadoras contienen la clave minima del
subarbol derecho correspondiente. Los registros Libro se referencian desde
las hojas; las claves internas solo guardan los valores anio e ISBN.