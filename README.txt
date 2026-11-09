CATALOGO BIBLIOTECARIO - ETAPA 4 (ABB + AVL + B+)
=================================================
ARCHIVOS DEL PROYECTO
---------------------
main.c   Menu principal e integracion de los tres indices.
libro.h  Estructura compartida del registro Libro.
abb.h    Interfaz y nodo del arbol binario de busqueda.
abb.c    Insercion, busqueda, eliminacion y recorridos ABB.
avl.h    Interfaz, nodo AVL y estadisticas de rotaciones.
avl.c    Insercion, busqueda, eliminacion y rotaciones AVL.
bplus.h  Estructuras y prototipos del indice B+.
bplus.c  Insercion, busqueda, eliminacion, rangos y hojas enlazadas.

COMPILACION EN DEV-C++
---------------------
1. Abra el proyecto de la etapa 3 o cree un proyecto de consola en C.
2. Reemplace main.c por el main.c de esta etapa.
3. Agregue bplus.c y bplus.h al proyecto.
4. Mantenga libro.h, abb.h, abb.c, avl.h y avl.c.
5. Verifique que main.c, abb.c, avl.c y bplus.c se compilen.
6. Compile y ejecute con F11.

Compilacion equivalente con GCC:
    gcc -std=c89 -Wall -Wextra -pedantic main.c abb.c avl.c bplus.c -o CatalogoBiblioteca

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

OPERACIONES B+
--------------
- insertarBPlus(): inserta una clave compuesta. Divide hojas e internos si
  se supera la capacidad. La division de hoja es 2/2; la division de un
  interno con 5 hijos es 2/3. Puede crear una nueva raiz.
- buscarBPlus(): busca una clave compuesta (anio, ISBN).
- eliminarBPlus(): elimina la clave, presta desde hermanos o fusiona nodos,
  actualiza separadores y reduce la raiz cuando queda con un solo hijo.
- consultarRangoBPlus(): busca la hoja inicial y recorre hojas enlazadas.
- validarBPlus(): comprueba ocupacion, orden, separadores, profundidad y
  enlaces entre hojas.

SINCRONIZACION DE LOS TRES INDICES
----------------------------------
Cada Libro se reserva una sola vez. ABB, AVL y las hojas B+ guardan
referencias al mismo registro. El ABB es responsable de liberar los libros.

Alta:
1. Comprueba que el ISBN no exista en ABB ni AVL.
2. Inserta (anio, ISBN) en B+.
3. Inserta ISBN en ABB y AVL.
4. Si falla una insercion posterior, revierte los indices anteriores.

Baja:
1. Comprueba que los tres indices referencien el mismo Libro.
2. Elimina (anio, ISBN) del B+ y el ISBN del ABB y AVL.
3. Libera el Libro una sola vez.

Modificacion:
- El ISBN se conserva porque es la clave principal.
- Si cambia el anio, primero se inserta la nueva clave (nuevo anio, ISBN).
  Si la insercion falla, se restaura el anio anterior. Si funciona, se elimina
  la clave antigua. Asi se evita perder el indice previo por falta de memoria.

CONSULTA POR RANGO
------------------
La consulta de anios es inclusiva: [anioInicial, anioFinal]. Se busca la hoja
que puede contener la primera clave del anio inicial y luego se recorren las
hojas mediante siguiente hasta superar el anio final.

Complejidad esperada:
- Busqueda, insercion y eliminacion: O(log n), con orden fijo.
- Consulta por rango: O(log n + k), donde k es la cantidad de registros
  devueltos (mas el recorrido de las hojas que contienen esos registros).
- Recorrido completo de hojas: O(n).
- Espacio: O(n).

MENU
----
1. Registrar libro en ABB, AVL y B+.
2. Eliminar libro de los tres indices.
3. Modificar titulo, autor y anio; reindexa el anio en B+.
4. Buscar por ISBN mediante AVL.
5. Listar por ISBN mediante inorden AVL.
6. Consultar intervalo de anios mediante B+.
7. Buscar por autor mediante recorrido ABB.
8. Mostrar recorridos ABB y AVL.
9. Mostrar alturas, rotaciones AVL y estadisticas B+.
10. Comparar caso degenerado ABB frente a AVL.
11. Ejecutar pruebas automaticas del AVL.
12. Mostrar hojas enlazadas del B+.
13. Ejecutar pruebas automaticas del B+.
0. Salir y liberar memoria.

PRUEBAS INCLUIDAS
-----------------
La opcion 13 crea 40 libros de prueba con anios repetidos y claves compuestas,
los inserta en un orden permutado y valida la estructura despues de cada alta.
Tambien comprueba busqueda, rechazo de una clave compuesta duplicada y el
rango 1990-2000. Finalmente elimina los 40 registros en otro orden y valida
la estructura despues de cada baja, incluyendo prestamos, fusiones y reduccion
de la raiz.

La prueba ejecutada en esta version obtuvo:
- 40 inserciones correctas.
- Busqueda compuesta correcta.
- Duplicado rechazado.
- 18 libros en el intervalo 1990-2000.
- 40 eliminaciones correctas y arbol vacio al final.

Adicionalmente, la implementacion fue compilada en C89 con -Wall -Wextra
-pedantic y sometida a pruebas de estres con sanitizadores AddressSanitizer
y UndefinedBehaviorSanitizer, incluyendo 1000 operaciones de insercion y
eliminacion del B+ y 300 libros sincronizados en ABB, AVL y B+.

LIMITACION ACTUAL
-----------------
Los datos se almacenan en memoria durante la ejecucion. Al cerrar el
programa se liberan; no existe persistencia en archivos ni en base de datos.