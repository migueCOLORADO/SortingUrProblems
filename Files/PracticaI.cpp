/*
PRÁCTICA I – ORDENAMIENTO DE DATASET GRANDE (C++)
Hecho por: Alejandro Soto
Curso: ST0245 - SI001 - Estructuras de Datos y Algoritmos
Universidad EAFIT | Docente: Alexander Narváez Berrío
*/

#include <iostream>     // Entrada y salida estandar | cout, cerr
#include <fstream>      // Manejo de archivos | ifstream para leer el dataset
#include <vector>       // Contenedor dinamico | vector<string> para QuickSort y HeapSort
#include <string>       // Tipo de dato cadena | Las palabras del dataset se almacenan como string
#include <chrono>       // Medicion de tiempo de ejecucion | high_resolution_clock para mayor precision
#include <cmath>        // Funciones matematicas | log2 para el analisis teorico de complejidad
#include <cstdint>      // Tipos enteros de tamano fijo | uint16_t para manejar caracteres UTF-16
#include <algorithm>    // Algoritmos estandar | swap para intercambiar strings eficientemente (O(1) con move semantics)

using namespace std;


// ===================================================================
//   LECTURA DEL DATASET (UTF-16 LE)
// ===================================================================

vector<string> leerDataset(const string& nombreArchivo) {    // Lee el archivo dataset.txt codificado en UTF-16 LE y retorna un vector con todas las palabras | La lectura binaria de a 2 bytes maneja correctamente la codificacion
    ifstream archivo(nombreArchivo, ios::binary);            // Abrir el archivo en modo binario | El dataset esta en UTF-16 LE, debemos leer de a 2 bytes por caracter sin interpretacion automatica

    if (!archivo.is_open()) {    // Verificar que el archivo se abrio correctamente antes de intentar leerlo
        cerr << "| Error | No se pudo abrir el archivo: " << nombreArchivo << "\n";
        return {};               // Retornar vector vacio si hubo un error de apertura
    }

    char bom[2];
    archivo.read(bom, 2);    // Saltar el BOM (Byte Order Mark) | Los primeros 2 bytes de UTF-16 LE son FF FE | Son una marca que identifica la codificacion y no forman parte del contenido

    vector<string> palabras;    // palabras | Vector dinamico que almacenara las 100,000 palabras del dataset | Crece automaticamente conforme se agregan elementos
    string palabraActual;       // palabraActual | Acumula los caracteres de la palabra que se esta construyendo en el momento

    while (archivo) {           // Recorrer el archivo hasta llegar al fin de archivo
        char bytes[2];          // bytes | Par de bytes que juntos forman un caracter UTF-16 LE | En esta codificacion cada caracter ocupa exactamente 2 bytes
        if (!archivo.read(bytes, 2)) break;    // Intentar leer 2 bytes | Si no hay mas datos o falla, salir del bucle

        uint16_t c = (uint8_t)bytes[0] | ((uint8_t)bytes[1] << 8);    // Reconstruir el valor Unicode | Formula UTF-16 LE: c = byte_menor | (byte_mayor << 8)

        if      (c == 0x000A) {    // Salto de linea \n | Indica el fin de una palabra en el archivo
            if (!palabraActual.empty()) {           // Solo guardar si la palabra tiene contenido | Ignorar lineas vacias
                palabras.push_back(palabraActual);  // Insertar la palabra completa al final del vector
                palabraActual.clear();              // Reiniciar el acumulador para la siguiente palabra
            }
        } else if (c == 0x000D) {    // Retorno de carro \r | Parte del terminador CRLF de Windows | Se descarta sin agregar nada al acumulador
            continue;
        } else if (c < 0x80) {       // Caracter ASCII valido | Todas las palabras del dataset son letras ASCII en minuscula (a-z)
            palabraActual += (char)c;    // Agregar el caracter al acumulador de la palabra que se esta construyendo
        }
    }

    if (!palabraActual.empty()) palabras.push_back(palabraActual);    // Guardar la ultima palabra si el archivo no termina con salto de linea

    return palabras;    // Retornar el vector con todas las palabras listas para ser ordenadas por los tres algoritmos
}


// ===================================================================
//   QUICKSORT SOBRE VECTOR<STRING>
//   Adaptado de la implementacion en DoublyLinkedList.cpp
//   Logica equivalente a partitionHelper y quickSortHelper,
//   pero usando indices sobre vector en lugar de punteros a nodos
// ===================================================================

int particion(vector<string>& arr, int l, int r) {    // Coloca el pivote en su posicion final correcta y deja menores a su izquierda y mayores a su derecha | Auxiliar para Quick Sort
    string pivot = arr[r];    // pivot | Valor del ultimo elemento del segmento actual | Sera el elemento de referencia para dividir los demas
    int i = l - 1;            // i | Indice del ultimo elemento menor o igual al pivote encontrado hasta ahora | Empieza un paso antes de l porque aun no encontramos ningun menor

    for (int j = l; j < r; j++) {     // j | Recorre el segmento desde l hasta antes de r (el pivote) evaluando cada elemento
        if (arr[j] <= pivot) {         // Si el elemento en j es menor o igual al pivote, pertenece al lado izquierdo
            i++;                       // Avanzar i un paso | Esta sera la posicion del proximo elemento menor encontrado
            swap(arr[i], arr[j]);      // Intercambiar arr[i] y arr[j] | Llevar el elemento menor hacia el lado izquierdo | swap es O(1) para strings por move semantics
        }
    }

    swap(arr[i + 1], arr[r]);    // Intercambiar el pivote (arr[r]) con arr[i+1] | El pivote queda colocado en su posicion definitiva correcta
    return i + 1;                // Retornar el indice donde quedo el pivote | quickSortHelper usara este indice como frontera para los dos segmentos a ordenar
}

void quickSortHelper(vector<string>& arr, int l, int r) {    // Ordena recursivamente los dos segmentos a cada lado del pivote | Auxiliar recursivo para Quick Sort
    if (l < r) {    // Condicion de parada | Solo hay algo que ordenar si el segmento tiene mas de un elemento (l < r)
        int p = particion(arr, l, r);      // p | Indice del pivote ya colocado en su posicion correcta | A su izquierda todo es menor o igual, a su derecha todo es mayor
        quickSortHelper(arr, l, p - 1);    // Ordenar recursivamente el segmento izquierdo | Desde l hasta el indice justo antes del pivote
        quickSortHelper(arr, p + 1, r);    // Ordenar recursivamente el segmento derecho | Desde el indice justo despues del pivote hasta r
    }
}

void quickSort(vector<string>& arr) {    // Elige el ultimo elemento como pivote, lo coloca en su posicion final y ordena recursivamente los dos segmentos | Usa quickSortHelper y particion
    quickSortHelper(arr, 0, (int)arr.size() - 1);    // Llamar al helper con los limites de todo el vector | 0 es el inicio, size()-1 es el final
}


// ===================================================================
//   HEAPSORT CON BINARY HEAP SOBRE VECTOR<STRING>
//   Adaptado de la implementacion en DoublyLinkedList.cpp
//   Logica equivalente a heapify y heapSort, pero el heap
//   es el propio vector (representacion implicita del arbol binario)
// ===================================================================

void heapify(vector<string>& arr, int n, int i) {    // Garantiza que el subarbol con raiz en el indice i cumpla la propiedad del max-heap (el padre siempre es mayor que sus hijos) | Auxiliar para Heap Sort
    int largest = i;            // largest | Indice del mayor entre la raiz y sus hijos | Asumimos la raiz como el mayor para comenzar
    int left    = 2 * i + 1;   // left  | Indice del hijo izquierdo | Formula del Binary Heap: hijo_izq = 2*padre + 1
    int right   = 2 * i + 2;   // right | Indice del hijo derecho  | Formula del Binary Heap: hijo_der = 2*padre + 2

    if (left  < n && arr[left]  > arr[largest]) largest = left;     // Si el hijo izquierdo existe (left < n) y es mayor que el actual largest, actualizar largest
    if (right < n && arr[right] > arr[largest]) largest = right;    // Si el hijo derecho existe (right < n) y es mayor que el actual largest, actualizar largest

    if (largest != i) {         // Si el mayor no es la raiz, hay que intercambiar para restaurar la propiedad del max-heap
        swap(arr[i], arr[largest]);    // Intercambiar la raiz con el hijo mayor | La raiz ahora tiene el valor correcto para el heap
        heapify(arr, n, largest);      // Llamada recursiva hacia abajo | El subarbol cuya raiz fue modificada puede haber roto la propiedad del heap, hay que verificarlo
    }
}

void heapSort(vector<string>& arr) {     // Construye un max-heap sobre el vector y extrae el maximo repetidamente colocandolo al final | Usa heapify | El vector mismo es la representacion del Binary Heap
    int n = (int)arr.size();             // n | Tamaño del vector | Controla cuantos elementos quedan por ordenar en cada iteracion del segundo bucle

    for (int i = n / 2 - 1; i >= 0; i--) heapify(arr, n, i);    // Construir el max-heap | Empezamos desde el ultimo nodo con hijos (n/2 - 1) y aplicamos heapify hacia la raiz | ¿Por que n/2 - 1? Porque los nodos desde n/2 en adelante son hojas (no tienen hijos) y no necesitan heapify

    for (int i = n - 1; i > 0; i--) {    // Extraer el maximo del heap uno a uno | Cada iteracion coloca el mayor elemento restante en su posicion final correcta
        swap(arr[0], arr[i]);             // Intercambiar la raiz (maximo actual del heap) con el ultimo elemento del heap | El maximo queda definitivamente al final
        heapify(arr, i, 0);              // Restaurar la propiedad del max-heap en el heap reducido (ahora de tamaño i) | La nueva raiz puede estar fuera de lugar
    }
}


// ===================================================================
//   ARBOL AVL (BALANCED BINARY SEARCH TREE)
//   El inorden de un BST produce los elementos ordenados,
//   y el balanceo AVL garantiza altura O(log n) para que
//   cada insercion y la busqueda sean eficientes
// ===================================================================

struct NodoAVL {         // Crear estructura | Esta nos permite crear nodos para el Arbol AVL
    string   dato;       // Crear campo de datos | Almacena la palabra del dataset
    NodoAVL* izq;        // Puntero al hijo izquierdo | Apunta a palabras lexicograficamente menores que dato
    NodoAVL* der;        // Puntero al hijo derecho  | Apunta a palabras lexicograficamente mayores o iguales a dato
    int      altura;     // Altura del nodo | Necesaria para calcular el factor de balance y decidir si hay que rotar | Un nodo hoja tiene altura 1

    NodoAVL(const string& val) : dato(val), izq(nullptr), der(nullptr), altura(1) {}    // Constructor | Inicializa el nodo con la palabra dada, sin hijos y altura 1
};


class ArbolAVL {

private:
    NodoAVL* raiz;    // Raiz del arbol | El nodo desde donde comienza toda la estructura jerarquica del AVL

    int alturaDeNodo(NodoAVL* n) const {      // Retorna la altura del nodo | Si el nodo es nulo retorna 0 para no acceder a memoria invalida | Auxiliar para calcular balances y actualizaciones
        return n ? n->altura : 0;       // Operador ternario | Si n existe retorna su altura; si es nulo retorna 0
    }

    int factorBalance(NodoAVL* n) {     // Calcula el factor de balance del nodo | Factor = altura(izq) - altura(der) | Si es > 1 o < -1 el nodo esta desbalanceado y hay que aplicar una rotacion
        return n ? alturaDeNodo(n->izq) - alturaDeNodo(n->der) : 0;    // Factor positivo | subarbol izquierdo mas alto | Factor negativo | subarbol derecho mas alto
    }

    void actualizarAltura(NodoAVL* n) {    // Actualiza la altura del nodo segun sus hijos | La altura de un nodo es 1 + la mayor altura entre sus dos subarboles
        if (n) n->altura = 1 + max(alturaDeNodo(n->izq), alturaDeNodo(n->der));    // Recalcular la altura | Solo si el nodo existe para evitar acceso a puntero nulo
    }

    NodoAVL* rotarDerecha(NodoAVL* y) {    // Rotacion simple a la derecha | Se aplica cuando el subarbol izquierdo esta demasiado cargado (factor de balance > 1) | Auxiliar para balancear
        NodoAVL* x  = y->izq;    // x  | Nuevo nodo raiz tras la rotacion | Es el hijo izquierdo del nodo desbalanceado que sube un nivel
        NodoAVL* T2 = x->der;    // T2 | Subarbol derecho de x | Pasara a ser el hijo izquierdo de y para mantener la propiedad del BST
        x->der = y;               // y pasa a ser el hijo derecho de x | La rotacion sube a x y baja a y un nivel en el arbol
        y->izq = T2;              // El subarbol T2 pasa a ser el nuevo hijo izquierdo de y | Todos sus elementos son mayores que x y menores que y, asi se mantiene el BST
        actualizarAltura(y);     // Actualizar la altura de y primero porque ahora es hijo de x y su posicion en el arbol cambio
        actualizarAltura(x);     // Actualizar la altura de x porque ahora es la nueva raiz del subarbol y su estructura cambio
        return x;                 // Retornar x como nueva raiz de este subarbol | El nodo llamador actualizara su puntero hijo con este retorno
    }

    NodoAVL* rotarIzquierda(NodoAVL* x) {    // Rotacion simple a la izquierda | Se aplica cuando el subarbol derecho esta demasiado cargado (factor de balance < -1) | Auxiliar para balancear
        NodoAVL* y  = x->der;    // y  | Nuevo nodo raiz tras la rotacion | Es el hijo derecho del nodo desbalanceado que sube un nivel
        NodoAVL* T2 = y->izq;    // T2 | Subarbol izquierdo de y | Pasara a ser el hijo derecho de x para mantener la propiedad del BST
        y->izq = x;               // x pasa a ser el hijo izquierdo de y | La rotacion sube a y y baja a x un nivel en el arbol
        x->der = T2;              // El subarbol T2 pasa a ser el nuevo hijo derecho de x | Todos sus elementos son mayores que x y menores que y, asi se mantiene el BST
        actualizarAltura(x);     // Actualizar la altura de x primero porque ahora es hijo de y y su posicion en el arbol cambio
        actualizarAltura(y);     // Actualizar la altura de y porque ahora es la nueva raiz del subarbol y su estructura cambio
        return y;                 // Retornar y como nueva raiz de este subarbol | El nodo llamador actualizara su puntero hijo con este retorno
    }

    NodoAVL* balancear(NodoAVL* n) {    // Verifica el factor de balance y aplica la rotacion necesaria | Garantiza que el arbol permanezca equilibrado (|factor| <= 1) tras cada insercion
        actualizarAltura(n);            // Actualizar la altura del nodo actual antes de calcular su balance | Siempre se recalcula al regresar de la recursion
        int fb = factorBalance(n);      // fb | Factor de balance del nodo | Determina si el nodo esta desbalanceado y hacia cual lado

        if (fb > 1) {                   // Desbalance hacia la izquierda | El subarbol izquierdo es demasiado alto (mas de 1 nivel de diferencia)
            if (factorBalance(n->izq) < 0)           // Caso Izquierda-Derecha | El hijo izquierdo esta cargado hacia la derecha
                n->izq = rotarIzquierda(n->izq);     // Rotar el hijo izquierdo a la izquierda primero | Lo convierte en Caso Izquierda-Izquierda
            return rotarDerecha(n);                  // Rotar el nodo actual a la derecha | Resuelve el desbalance Izquierda-Izquierda
        }

        if (fb < -1) {                  // Desbalance hacia la derecha | El subarbol derecho es demasiado alto (mas de 1 nivel de diferencia)
            if (factorBalance(n->der) > 0)           // Caso Derecha-Izquierda | El hijo derecho esta cargado hacia la izquierda
                n->der = rotarDerecha(n->der);       // Rotar el hijo derecho a la derecha primero | Lo convierte en Caso Derecha-Derecha
            return rotarIzquierda(n);                // Rotar el nodo actual a la izquierda | Resuelve el desbalance Derecha-Derecha
        }

        return n;    // Factor de balance entre -1 y 1 | El nodo ya esta balanceado, retornar sin cambios
    }

    NodoAVL* insertarHelper(NodoAVL* n, const string& val) {    // Inserta recursivamente una palabra en el subarbol con raiz n y rebalancea si es necesario | Auxiliar recursivo para insertar
        if (n == nullptr) return new NodoAVL(val);      // Caso base | Posicion de insercion encontrada | Crear un nuevo nodo aqui con altura 1
        if (val < n->dato) n->izq = insertarHelper(n->izq, val);    // Si la palabra es menor, buscar la posicion correcta en el subarbol izquierdo
        else               n->der = insertarHelper(n->der, val);    // Si la palabra es mayor o igual, buscar en el subarbol derecho | Los duplicados van a la derecha para no perderlos
        return balancear(n);    // Balancear el nodo actual al regresar de la recursion | Garantiza que el arbol AVL se mantenga equilibrado en cada nivel de la pila de llamadas
    }

    void inordenHelper(NodoAVL* n, vector<string>& resultado) const {    // Recorre el arbol en inorden (izq -> raiz -> der) y agrega cada palabra al vector | El inorden de un BST siempre produce la secuencia ordenada
        if (n == nullptr) return;               // Caso base | Si el nodo es nulo no hay nada que recorrer, retornar inmediatamente
        inordenHelper(n->izq, resultado);       // Recorrer el subarbol izquierdo primero | Palabras lexicograficamente menores se procesan antes
        resultado.push_back(n->dato);           // Agregar el dato del nodo actual al resultado | En este momento llega en el orden correcto gracias al recorrido inorden
        inordenHelper(n->der, resultado);       // Recorrer el subarbol derecho al final | Palabras lexicograficamente mayores se procesan despues
    }

    void limpiarHelper(NodoAVL* n) {    // Libera recursivamente la memoria de todos los nodos del arbol | Auxiliar para el destructor | Evita memory leaks
        if (n == nullptr) return;        // Caso base | Si el nodo es nulo no hay nada que liberar
        limpiarHelper(n->izq);           // Liberar primero el subarbol izquierdo
        limpiarHelper(n->der);           // Liberar el subarbol derecho
        delete n;                        // Liberar el nodo actual | Solo despues de sus hijos para no perder las referencias y provocar memory leaks
    }

public:
    ArbolAVL() : raiz(nullptr) {}           // Constructor | Inicializa el arbol completamente vacio con raiz nula
    ~ArbolAVL() { limpiarHelper(raiz); }    // Destructor | Libera toda la memoria del arbol llamando al helper recursivo | Funcion que borrara todos los nodos del arbol

    void insertar(const string& val) {      // Funcion publica para insertar una palabra | Delega en insertarHelper para la insercion recursiva con rebalanceo automatico
        raiz = insertarHelper(raiz, val);   // La raiz puede cambiar tras una rotacion, por eso siempre actualizamos el puntero raiz con el retorno del helper
    }

    vector<string> inorden() const {        // Funcion publica para obtener el recorrido inorden | Retorna un vector con todas las palabras ordenadas alfabeticamente
        vector<string> resultado;           // resultado | Vector donde se almacenaran las palabras durante el recorrido | Se llenara de menor a mayor
        inordenHelper(raiz, resultado);     // Delegar el recorrido al helper recursivo | El vector se llena por referencia para evitar copias intermedias
        return resultado;                   // Retornar el vector con las palabras en orden alfabetico
    }

    int alturaTotal() const { return alturaDeNodo(raiz); }    // Retorna la altura total del arbol | Util para validar el balanceo AVL | Debe ser aproximadamente log2(n)
};


// ===================================================================
//   ESTIMACION DE MEMORIA
// ===================================================================

void reportarMemoria(const string& algoritmo, size_t n, size_t bytesPorPalabra, bool esArbol, int alturaArbol) {    // Imprime una estimacion de la memoria utilizada segun la estructura de datos del algoritmo | Basada en sizeof de los tipos usados y el numero de elementos
    cout << "\n  | Estimacion de Memoria | " << algoritmo << " |\n";

    if (!esArbol) {
        // vector<string> | Cada elemento ocupa: sizeof(string) para el objeto en si + bytes del contenido de la cadena en el heap
        size_t memoriaVector = n * (sizeof(string) + bytesPorPalabra);    // Estimacion total del vector | No incluye la capacidad extra que el vector puede reservar internamente
        cout << "    Estructura          : vector<string> con " << n << " elementos\n";
        cout << "    Tamano por elemento : " << sizeof(string) << " bytes (objeto string) + " << bytesPorPalabra << " bytes (cadena) = " << sizeof(string) + bytesPorPalabra << " bytes aprox\n";
        cout << "    Memoria total aprox : " << memoriaVector / 1024 << " KB\n";
        if (algoritmo == "QuickSort") {
            // QuickSort usa pila de llamadas recursivas | Profundidad promedio O(log n) con dataset aleatorio | Cada nivel guarda l, r y variables locales
            cout << "    Pila de recursion   : O(log n) = ~" << (int)log2((double)n) << " niveles en promedio con dataset aleatorio\n";
        }
    } else {
        // Arbol AVL | Cada nodo contiene: objeto string + 2 punteros a NodoAVL + int altura + bytes del contenido de la cadena
        size_t bytesPorNodo = sizeof(NodoAVL) + bytesPorPalabra;    // Tamano total de cada nodo | sizeof(NodoAVL) incluye el objeto string, los 2 punteros y el int altura
        size_t memoriaArbol = n * bytesPorNodo;                      // Estimacion total del arbol | n nodos * bytes por nodo
        cout << "    Estructura          : Arbol AVL con " << n << " nodos\n";
        cout << "    Altura real         : " << alturaArbol << " | Altura teorica O(log n) = ~" << (int)log2((double)n) << "\n";
        cout << "    Tamano por nodo     : " << bytesPorNodo << " bytes (NodoAVL + contenido cadena)\n";
        cout << "    Memoria total aprox : " << memoriaArbol / 1024 << " KB\n";
        cout << "    | INFO | El arbol usa mas memoria que el vector porque cada nodo agrega 2 punteros (izq, der) y el int altura ademas del string\n";
    }
}


// ===================================================================
//   MAIN
// ===================================================================

int main() {
    cout << "\n=======================================================\n";
    cout << "  PRACTICA I - ORDENAMIENTO DE DATASET GRANDE (C++)   \n";
    cout << "  ST0245 - Estructuras de Datos y Algoritmos - EAFIT   \n";
    cout << "=======================================================\n\n";

    // ----- Cargar el dataset desde el archivo -----
    cout << "| Cargando dataset desde 'dataset.txt'...\n";
    vector<string> dataset = leerDataset("dataset.txt");    // dataset | Vector con las palabras en orden aleatorio cargadas desde el archivo | Sera la fuente de datos para los tres algoritmos

    if (dataset.empty()) {    // Verificar que la lectura fue exitosa antes de proceder con los algoritmos
        cerr << "| Error | El dataset esta vacio o no se pudo leer correctamente.\n";
        return 1;             // Terminar el programa con codigo de error
    }
    cout << "| Dataset cargado | " << dataset.size() << " palabras\n";

    // Calcular promedio de bytes por palabra | Necesario para la estimacion de memoria de las tres estructuras
    size_t totalBytes = 0;
    for (const auto& p : dataset) totalBytes += p.size() + 1;    // Sumar la longitud de cada palabra + 1 byte del caracter nulo de terminacion
    size_t bytesPorPalabra = totalBytes / dataset.size();          // bytesPorPalabra | Promedio de bytes por string en este dataset especifico


    // ===== [1] QUICKSORT =====
    vector<string> qs = dataset;    // qs | Copia independiente del dataset para QuickSort | Cada algoritmo trabaja sobre su propia copia para garantizar igualdad de condiciones en la comparacion
    cout << "\n[1] Ejecutando QuickSort sobre vector<string>...\n";
    auto ini = chrono::high_resolution_clock::now();    // ini | Captura el instante exacto antes de ordenar | high_resolution_clock es el reloj de maxima precision disponible en el sistema
    quickSort(qs);
    auto fin = chrono::high_resolution_clock::now();    // fin | Captura el instante exacto despues de ordenar | La diferencia fin - ini es el tiempo real de ejecucion del algoritmo
    double tiempoQS = chrono::duration<double, milli>(fin - ini).count();    // tiempoQS | Tiempo en milisegundos que tardo QuickSort | duration<double,milli> convierte la duracion a ms con decimales
    cout << "| Tiempo QuickSort      : " << tiempoQS << " ms\n";
    cout << "| Primeras 5 palabras   : ";
    for (int i = 0; i < 5 && i < (int)qs.size(); i++) cout << qs[i] << " | ";
    cout << "\n";
    reportarMemoria("QuickSort", qs.size(), bytesPorPalabra, false, 0);


    // ===== [2] HEAPSORT =====
    vector<string> hs = dataset;    // hs | Copia independiente del dataset para HeapSort
    cout << "\n[2] Ejecutando HeapSort (Binary Heap sobre vector<string>)...\n";
    ini = chrono::high_resolution_clock::now();    // Registrar el tiempo de inicio justo antes de ordenar
    heapSort(hs);
    fin = chrono::high_resolution_clock::now();    // Registrar el tiempo de fin justo despues de ordenar
    double tiempoHS = chrono::duration<double, milli>(fin - ini).count();    // tiempoHS | Tiempo en milisegundos que tardo HeapSort
    cout << "| Tiempo HeapSort       : " << tiempoHS << " ms\n";
    cout << "| Primeras 5 palabras   : ";
    for (int i = 0; i < 5 && i < (int)hs.size(); i++) cout << hs[i] << " | ";
    cout << "\n";
    reportarMemoria("HeapSort", hs.size(), bytesPorPalabra, false, 0);


    // ===== [3] ARBOL AVL =====
    cout << "\n[3] Ejecutando Arbol AVL (insercion de 100k palabras + recorrido inorden)...\n";
    ArbolAVL avl;    // avl | Arbol AVL donde insertaremos todas las palabras | Se autobalancea en cada insercion para garantizar altura O(log n)
    ini = chrono::high_resolution_clock::now();    // Registrar el tiempo de inicio | Incluye tanto la insercion como el recorrido inorden
    for (const auto& p : dataset) avl.insertar(p);    // Insertar cada palabra en el arbol | Cada insercion rebalancea el arbol si es necesario mediante rotaciones
    vector<string> avlResult = avl.inorden();          // avlResult | Vector con las palabras obtenidas por el recorrido inorden | El inorden de un BST produce la secuencia ordenada
    fin = chrono::high_resolution_clock::now();    // Registrar el tiempo de fin | Despues del inorden
    double tiempoAVL = chrono::duration<double, milli>(fin - ini).count();    // tiempoAVL | Tiempo total del AVL: insercion de todas las palabras + recorrido inorden
    cout << "| Tiempo AVL            : " << tiempoAVL << " ms (insercion + inorden)\n";
    cout << "| Primeras 5 palabras   : ";
    for (int i = 0; i < 5 && i < (int)avlResult.size(); i++) cout << avlResult[i] << " | ";
    cout << "\n";
    reportarMemoria("AVL Tree", avlResult.size(), bytesPorPalabra, true, avl.alturaTotal());


    // ===== ANALISIS COMPARATIVO =====
    cout << "\n=======================================================\n";
    cout << "  ANALISIS COMPARATIVO\n";
    cout << "=======================================================\n";
    cout << "  Algoritmo    | Tiempo (ms)   | Complejidad temporal\n";
    cout << "  -------------|---------------|------------------------------------------\n";
    cout << "  QuickSort    | " << tiempoQS  << "\t\t| O(n log n) promedio | O(n^2) peor caso\n";
    cout << "  HeapSort     | " << tiempoHS  << "\t\t| O(n log n) garantizado en todos los casos\n";
    cout << "  AVL Tree     | " << tiempoAVL << "\t\t| O(n log n) insercion + O(n) inorden\n";

    cout << "\n  Complejidad espacial:\n";
    cout << "  QuickSort    : O(n) vector + O(log n) pila de recursion en promedio\n";
    cout << "  HeapSort     : O(n) vector (ordenamiento in-place, sin memoria adicional)\n";
    cout << "  AVL Tree     : O(n) nodos con 2 punteros y campo altura (mayor costo por elemento)\n";

    cout << "\n  Conclusiones:\n";
    cout << "  - QuickSort suele ser el mas rapido en la practica por mejor aprovechamiento\n";
    cout << "    del cache de la CPU y menor numero de operaciones en el caso promedio\n";
    cout << "  - HeapSort garantiza O(n log n) en el peor caso pero tiene mayor constante\n";
    cout << "    oculta que QuickSort debido a patrones de acceso menos amigables con el cache\n";
    cout << "  - El AVL Tree usa significativamente mas memoria por elemento (punteros + altura)\n";
    cout << "    pero ofrece busquedas eficientes O(log n) en la estructura y produce el orden\n";
    cout << "    directamente del inorden sin necesidad de comparaciones adicionales\n";
    cout << "  - Para ordenar un dataset puro, QuickSort o HeapSort sobre vector son preferibles\n";
    cout << "    al AVL por menor sobrecarga de memoria y mejor localidad espacial\n";
    cout << "=======================================================\n";

    return 0;    // Indica que el programa finalizo correctamente
}
