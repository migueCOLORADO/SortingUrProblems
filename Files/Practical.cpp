/*
Práctica I – Simulador de Algoritmos de Ordenamiento
Hecho por: Alejandro Soto Bermeo y Miguel Angel Colorado Castaño
Curso: ST0245 - SI001 - Estructuras de Datos y Algoritmos
Universidad EAFIT | Docente: Diego Iván Cruz Ordiéres 
*/

#include <iostream>     // Entrada y salida estandar | cout, cerr
#include <fstream>      // Manejo de archivos | ifstream para leer el dataset
#include <vector>       // Contenedor dinamico | vector<string> para QuickSort y HeapSort
#include <string>       // Tipo de dato cadena | Las palabras del dataset se almacenan como string
#include <chrono>       // Medicion de tiempo de ejecucion | high_resolution_clock para mayor precision
#include <cmath>        // Funciones matematicas | log2 para el analisis teorico de complejidad
#include <cstdint>      // Tipos enteros de tamano fijo | uint16_t para manejar caracteres UTF-16
#include <algorithm>    // Algoritmos estandar | swap para intercambiar strings eficientemente
#include <iomanip>      // Formato de salida | setw y left para alinear la tabla comparativa
#include <set>          // Conjunto ordenado | Usado internamente para detectar duplicados sin mostrarlo

using namespace std;

// Lectura del dataset

vector<string> leerDataset(const string& nombreArchivo) {    // Lee el archivo dataset.txt y retorna un vector con todas las palabras | Soporta UTF-16 LE y UTF-8/ASCII
    ifstream archivo(nombreArchivo, ios::binary);             // Abrir en modo binario | Necesario para leer el BOM y manejar correctamente ambas codificaciones

    if (!archivo.is_open()) {    // Verificar que el archivo se abrio correctamente antes de intentar leerlo
        cerr << "| Error | No se pudo abrir el archivo: " << nombreArchivo << "\n";
        return {};               // Retornar vector vacio si hubo un error de apertura
    }

    char bom[2] = {0, 0};    // bom | Primeros 2 bytes del archivo | Se usan para identificar la codificacion antes de leer el contenido
    archivo.read(bom, 2);    // Leer los primeros 2 bytes | Si son FF FE el archivo es UTF-16 LE; cualquier otro valor indica UTF-8/ASCII

    if (!archivo) {    // Verificar que la lectura del BOM fue exitosa | Un archivo de menos de 2 bytes se considera vacio
        cerr << "| Error | El archivo esta vacio o no se pudo leer el BOM.\n";
        return {};
    }

    bool esUTF16LE = ((uint8_t)bom[0] == 0xFF && (uint8_t)bom[1] == 0xFE);    // esUTF16LE | Bandera que indica el formato detectado | true si BOM = FF FE (UTF-16 LE), false para UTF-8/ASCII

    if (!esUTF16LE) archivo.seekg(0, ios::beg);    // Si no es UTF-16 LE, retroceder al inicio | Los 2 bytes leidos son datos reales, no un BOM, y deben procesarse

    vector<string> palabras;    // palabras | Vector dinamico que almacenara todas las palabras del dataset | Crece automaticamente conforme se agregan elementos
    string palabraActual;       // palabraActual | Acumula los caracteres de la palabra que se esta construyendo en el momento

    if (esUTF16LE) {
        while (archivo) {                              // Recorrer el archivo hasta llegar al fin de archivo
            char bytes[2];                             // bytes | Par de bytes que juntos forman un caracter UTF-16 LE | Cada caracter ocupa exactamente 2 bytes en esta codificacion
            if (!archivo.read(bytes, 2)) break;        // Intentar leer 2 bytes | Si no hay mas datos o falla, salir del bucle

            uint16_t c = (uint8_t)bytes[0] | ((uint8_t)bytes[1] << 8);    // c | Valor Unicode reconstruido | Formula UTF-16 LE: byte_menor | (byte_mayor << 8)

            if (c == 0x000A) {                             // Salto de linea \n | Indica el fin de una palabra en el archivo
                if (!palabraActual.empty()) {              // Solo guardar si la palabra tiene contenido | Ignorar lineas vacias
                    palabras.push_back(palabraActual);     // Insertar la palabra completa al final del vector
                    palabraActual.clear();                 // Reiniciar el acumulador para la siguiente palabra
                }
            } else if (c == 0x000D) {    // Retorno de carro \r | Parte del terminador CRLF de Windows | Se descarta sin agregar nada
                continue;
            } else if (c < 0x80) {       // Caracter ASCII valido | Todas las palabras del dataset son letras en minuscula (a-z)
                palabraActual += (char)c; // Agregar el caracter al acumulador de la palabra que se esta construyendo
            }
        }
    } else {
        string linea;    // linea | Variable auxiliar para leer el archivo linea por linea con getline | Se reutiliza en cada iteracion
        string inicio(bom, 2);    // inicio | Contiene los 2 bytes ya leidos antes de detectar el formato | Deben procesarse como datos normales
        for (char ch : inicio) {    // Procesar los 2 bytes del inicio como caracteres de texto normales
            if      (ch == '\n') { if (!palabraActual.empty()) { palabras.push_back(palabraActual); palabraActual.clear(); } }
            else if (ch != '\r') { palabraActual += ch; }    // Agregar el caracter si no es \r | \r se ignora igual que en la lectura UTF-16
        }
        while (getline(archivo, linea)) {    // Leer el resto del archivo linea por linea | getline maneja automaticamente el delimitador \n
            if (!linea.empty() && linea.back() == '\r') linea.pop_back();    // Eliminar \r si el archivo tiene terminadores CRLF de Windows
            if (!linea.empty()) palabras.push_back(linea);                   // Solo guardar lineas con contenido | Ignorar lineas vacias
        }
    }

    if (!palabraActual.empty()) palabras.push_back(palabraActual);    // Guardar la ultima palabra si el archivo no termina con salto de linea

    return palabras;    // Retornar el vector con todas las palabras listas para los tres algoritmos
}


// ===================================================================
//   Creación de Quicksort
//   Tres consideraciones tenidas en cuenta sobre el pivote:
//   a) Pivote mediana de tres: elimina O(n^2) en datos ya ordenados.
//   b) Insertion sort para segmentos <= 20: menor overhead recursivo.
//   c) TCO manual (bucle + recursion corta): pila garantizada O(log n).
// ===================================================================

void medianaDeTres(vector<string>& arr, int l, int r) {    // Selecciona la mediana entre arr[l], arr[mid] y arr[r] y la coloca en arr[r] como pivote | Evita el peor caso O(n^2) con datos ordenados
    int mid = l + (r - l) / 2;                            // mid | Indice del elemento central del segmento | Calculado con desplazamiento para evitar desbordamiento de entero
    if (arr[l] > arr[mid]) swap(arr[l], arr[mid]);         // Ordenar los tres candidatos con 3 comparaciones | Paso 1: asegurar que arr[l] <= arr[mid]
    if (arr[l] > arr[r])   swap(arr[l], arr[r]);           // Paso 2: asegurar que arr[l] <= arr[r] | Ahora arr[l] es el minimo de los tres
    if (arr[mid] > arr[r]) swap(arr[mid], arr[r]);         // Paso 3: asegurar que arr[mid] <= arr[r] | Ahora arr[r] es el maximo de los tres
    swap(arr[mid], arr[r]);                                // Mover la mediana a arr[r] para que particion() la use como pivote sin modificaciones
}

void insertionSort(vector<string>& arr, int l, int r) {    // Ordena el segmento arr[l..r] por insercion | Mas eficiente que QuickSort para segmentos pequenos (n <= 20)
    for (int i = l + 1; i <= r; i++) {                    // Recorrer desde el segundo elemento hasta el final del segmento
        string key = arr[i];                               // key | Elemento actual que se va a insertar en su posicion correcta dentro de la parte ya ordenada
        int j = i - 1;                                     // j | Indice del ultimo elemento de la parte ya ordenada | Retrocede mientras encuentre elementos mayores que key
        while (j >= l && arr[j] > key) {                   // Desplazar hacia la derecha todos los elementos mayores que key para abrirle espacio
            arr[j + 1] = arr[j];                           // Mover el elemento una posicion a la derecha | Sobrescribe la posicion de key que ya fue guardada
            j--;                                           // Retroceder un paso para seguir comparando
        }
        arr[j + 1] = key;                                  // Insertar key en su posicion correcta | j+1 es el hueco que quedo tras los desplazamientos
    }
}

int particion(vector<string>& arr, int l, int r) {    // Coloca el pivote en su posicion final correcta y deja menores a su izquierda y mayores a su derecha | Auxiliar para Quick Sort
    string pivot = arr[r];                            // pivot | Valor del ultimo elemento del segmento | Es la mediana de tres colocada ahi por medianaDeTres()
    int i = l - 1;                                    // i | Indice del ultimo elemento menor o igual al pivote encontrado hasta ahora | Empieza un paso antes de l

    for (int j = l; j < r; j++) {      // j | Recorre el segmento desde l hasta antes de r evaluando cada elemento frente al pivote
        if (arr[j] <= pivot) {         // Si el elemento en j es menor o igual al pivote, pertenece al lado izquierdo
            i++;                       // Avanzar i un paso | Esta sera la posicion del proximo elemento menor encontrado
            swap(arr[i], arr[j]);      // Intercambiar arr[i] y arr[j] | Llevar el elemento menor hacia el lado izquierdo
        }
    }

    swap(arr[i + 1], arr[r]);    // Intercambiar el pivote con arr[i+1] | El pivote queda colocado en su posicion definitiva correcta
    return i + 1;                // Retornar el indice donde quedo el pivote | quickSortHelper usara este indice como frontera para los dos segmentos
}

void quickSortHelper(vector<string>& arr, int l, int r) {    // Ordena recursivamente los segmentos a cada lado del pivote | Usa TCO manual para garantizar pila O(log n)
    while (l < r) {                                          // Bucle en lugar de doble recursion | La rama larga continua en el bucle, la corta recursa
        if (r - l < 20) {                                    // Umbral de insercion | Para segmentos pequenos, Insertion Sort es mas rapido que seguir particionando
            insertionSort(arr, l, r);
            break;
        }

        medianaDeTres(arr, l, r);         // Seleccionar pivote como mediana de tres | Coloca la mediana en arr[r] lista para particion()
        int p = particion(arr, l, r);     // p | Indice del pivote ya en su posicion correcta | Divide el segmento en dos subsegmentos a ordenar

        if (p - l < r - p) {                   // Comparar tamanos de los dos subsegmentos | Recursar en el mas corto para garantizar profundidad de pila O(log n)
            quickSortHelper(arr, l, p - 1);    // Recursar en el segmento izquierdo si es el mas corto
            l = p + 1;                         // El segmento derecho (el mas largo) lo maneja el bucle, no la recursion
        } else {
            quickSortHelper(arr, p + 1, r);    // Recursar en el segmento derecho si es el mas corto
            r = p - 1;                         // El segmento izquierdo (el mas largo) lo maneja el bucle
        }
    }
}

void quickSort(vector<string>& arr) {    // Punto de entrada del Quick Sort | Valida que haya al menos 2 elementos y delega en quickSortHelper
    if (arr.size() < 2) return;          // Guardia | Un vector vacio o de un solo elemento ya esta ordenado, no hay nada que hacer
    quickSortHelper(arr, 0, (int)arr.size() - 1);    // Llamar al helper con los limites de todo el vector | 0 es el inicio, size()-1 es el final
}


// ===================================================================
//   Creación de Heapsort
// ===================================================================

void heapify(vector<string>& arr, int n, int i) {    // Garantiza que el subarbol con raiz en el indice i cumpla la propiedad del max-heap | Auxiliar para Heap Sort
    int largest = i;          // largest | Indice del mayor entre la raiz y sus hijos | Asumimos la raiz como el mayor para comenzar
    int left    = 2 * i + 1;  // left  | Indice del hijo izquierdo | Formula del Binary Heap: hijo_izq = 2*padre + 1
    int right   = 2 * i + 2;  // right | Indice del hijo derecho  | Formula del Binary Heap: hijo_der = 2*padre + 2

    if (left  < n && arr[left]  > arr[largest]) largest = left;     // Si el hijo izquierdo existe (left < n) y es mayor que el actual largest, actualizar largest
    if (right < n && arr[right] > arr[largest]) largest = right;    // Si el hijo derecho existe (right < n) y es mayor que el actual largest, actualizar largest

    if (largest != i) {                    // Si el mayor no es la raiz, hay que intercambiar para restaurar la propiedad del max-heap
        swap(arr[i], arr[largest]);        // Intercambiar la raiz con el hijo mayor | La raiz ahora tiene el valor correcto para el heap
        heapify(arr, n, largest);          // Llamada recursiva hacia abajo | El subarbol modificado puede haber roto la propiedad del heap
    }
}

void heapSort(vector<string>& arr) {    // Construye un max-heap sobre el vector y extrae el maximo repetidamente colocandolo al final | Usa heapify
    int n = (int)arr.size();            // n | Tamaño del vector | Controla cuantos elementos quedan por ordenar en cada iteracion
    if (n < 2) return;                  // Guardia | Un vector vacio o de un solo elemento ya esta ordenado

    for (int i = n / 2 - 1; i >= 0; i--) heapify(arr, n, i);    // Construir el max-heap | Comenzamos desde el ultimo nodo con hijos (n/2-1) hacia la raiz | Los nodos desde n/2 en adelante son hojas y no necesitan heapify

    for (int i = n - 1; i > 0; i--) {    // Extraer el maximo del heap uno a uno | Cada iteracion coloca el mayor elemento restante en su posicion final
        swap(arr[0], arr[i]);             // Intercambiar la raiz (maximo actual) con el ultimo elemento del heap | El maximo queda definitivamente al final
        heapify(arr, i, 0);              // Restaurar la propiedad del max-heap en el heap reducido (tamaño i) | La nueva raiz puede estar fuera de lugar
    }
}


// ===================================================================
//   Creación de Árbol AVL
//   Consideraciones tomadas en cuenta:
//    a). Implementa un conjunto ordenado: los duplicados se ignoran.
//    b). El inorden produce la secuencia de palabras unicas ordenadas.
// ===================================================================

struct NodoAVL {         // Crear estructura | Cada nodo del arbol AVL almacena una palabra y los enlaces para mantener la jerarquia del arbol
    string   dato;       // dato | La palabra del dataset almacenada en este nodo
    NodoAVL* izq;        // Puntero al hijo izquierdo | Apunta a palabras lexicograficamente menores que dato
    NodoAVL* der;        // Puntero al hijo derecho  | Apunta a palabras lexicograficamente mayores que dato
    int      altura;     // altura | Altura del nodo en el arbol | Necesaria para calcular el factor de balance y decidir si hay que rotar

    NodoAVL(const string& val) : dato(val), izq(nullptr), der(nullptr), altura(1) {}    // Constructor | Inicializa el nodo con la palabra dada, sin hijos y altura 1
};

class ArbolAVL {

private:
    NodoAVL* raiz;         // raiz | Nodo desde donde comienza toda la estructura jerarquica del AVL
    int      nodosReales;  // nodosReales | Contador de palabras unicas insertadas | Excluye los duplicados ignorados

    int alturaDeNodo(NodoAVL* n) const {    // Retorna la altura del nodo | Si el nodo es nulo retorna 0 para no acceder a memoria invalida
        return n ? n->altura : 0;           // Operador ternario | Si n existe retorna su altura, si es nulo retorna 0
    }

    int factorBalance(NodoAVL* n) {    // Calcula el factor de balance | Factor = altura(izq) - altura(der) | Si es > 1 o < -1 el nodo esta desbalanceado
        return n ? alturaDeNodo(n->izq) - alturaDeNodo(n->der) : 0;    // Factor positivo | izquierda mas alta | Factor negativo | derecha mas alta
    }

    void actualizarAltura(NodoAVL* n) {    // Actualiza la altura del nodo segun sus hijos | La altura es 1 + la mayor altura entre los dos subarboles
        if (n) n->altura = 1 + max(alturaDeNodo(n->izq), alturaDeNodo(n->der));    // Solo si el nodo existe para evitar acceso a puntero nulo
    }

    NodoAVL* rotarDerecha(NodoAVL* y) {    // Rotacion simple a la derecha | Se aplica cuando el subarbol izquierdo esta demasiado cargado (factor > 1)
        NodoAVL* x  = y->izq;    // x  | Nuevo nodo raiz tras la rotacion | Es el hijo izquierdo del nodo desbalanceado que sube un nivel
        NodoAVL* T2 = x->der;    // T2 | Subarbol derecho de x | Pasara a ser el hijo izquierdo de y para mantener la propiedad del BST
        x->der = y;               // y pasa a ser hijo derecho de x | La rotacion sube a x y baja a y un nivel
        y->izq = T2;              // T2 pasa a ser el nuevo hijo izquierdo de y | Todos sus elementos son mayores que x y menores que y
        actualizarAltura(y);     // Actualizar la altura de y primero porque ahora es hijo de x
        actualizarAltura(x);     // Actualizar la altura de x porque ahora es la nueva raiz del subarbol
        return x;                 // Retornar x como nueva raiz | El nodo llamador actualizara su puntero hijo con este retorno
    }

    NodoAVL* rotarIzquierda(NodoAVL* x) {    // Rotacion simple a la izquierda | Se aplica cuando el subarbol derecho esta demasiado cargado (factor < -1)
        NodoAVL* y  = x->der;    // y  | Nuevo nodo raiz tras la rotacion | Es el hijo derecho del nodo desbalanceado que sube un nivel
        NodoAVL* T2 = y->izq;    // T2 | Subarbol izquierdo de y | Pasara a ser el hijo derecho de x para mantener la propiedad del BST
        y->izq = x;               // x pasa a ser hijo izquierdo de y | La rotacion sube a y y baja a x un nivel
        x->der = T2;              // T2 pasa a ser el nuevo hijo derecho de x | Todos sus elementos son mayores que x y menores que y
        actualizarAltura(x);     // Actualizar la altura de x primero porque ahora es hijo de y
        actualizarAltura(y);     // Actualizar la altura de y porque ahora es la nueva raiz del subarbol
        return y;                 // Retornar y como nueva raiz | El nodo llamador actualizara su puntero hijo con este retorno
    }

    NodoAVL* balancear(NodoAVL* n) {    // Verifica el factor de balance y aplica la rotacion necesaria | Garantiza que el arbol permanezca equilibrado tras cada insercion
        actualizarAltura(n);            // Actualizar la altura del nodo actual antes de calcular su balance
        int fb = factorBalance(n);      // fb | Factor de balance del nodo | Determina si esta desbalanceado y hacia cual lado

        if (fb > 1) {                        // Desbalance hacia la izquierda | Subarbol izquierdo demasiado alto
            if (factorBalance(n->izq) < 0)                     // Caso Izquierda-Derecha | El hijo izquierdo esta cargado hacia la derecha
                n->izq = rotarIzquierda(n->izq);               // Rotar el hijo izquierdo primero para convertirlo en Caso Izquierda-Izquierda
            return rotarDerecha(n);                   // Rotar el nodo actual a la derecha para resolver el desbalance
        }

        if (fb < -1) {                    // Desbalance hacia la derecha | Subarbol derecho demasiado alto
            if (factorBalance(n->der) > 0)                     // Caso Derecha-Izquierda | El hijo derecho esta cargado hacia la izquierda
                n->der = rotarDerecha(n->der);                 // Rotar el hijo derecho primero para convertirlo en Caso Derecha-Derecha
            return rotarIzquierda(n);                 // Rotar el nodo actual a la izquierda para resolver el desbalance
        }

        return n;    // Factor de balance entre -1 y 1 | El nodo ya esta balanceado, retornar sin cambios
    }

    NodoAVL* insertarHelper(NodoAVL* n, const string& val) {    // Inserta recursivamente una palabra en el subarbol con raiz n y rebalancea si es necesario
        if (n == nullptr) { nodosReales++; return new NodoAVL(val); }    // Caso base | Posicion de insercion encontrada | Crear un nuevo nodo y contarlo
        if      (val < n->dato) n->izq = insertarHelper(n->izq, val);   // Si la palabra es menor, buscar posicion en el subarbol izquierdo
        else if (val > n->dato) n->der = insertarHelper(n->der, val);   // Si la palabra es mayor, buscar posicion en el subarbol derecho
        // Si val == n->dato: duplicado | No insertar, no contar | El AVL es un conjunto ordenado, no admite repetidos
        return balancear(n);    // Balancear el nodo actual al regresar de la recursion | Garantiza que el AVL se mantenga equilibrado en cada nivel
    }

    void inordenHelper(NodoAVL* n, vector<string>& resultado) const {    // Recorre el arbol en inorden (izq -> raiz -> der) | El inorden de un BST produce la secuencia ordenada
        if (n == nullptr) return;               // Caso base | Si el nodo es nulo no hay nada que recorrer
        inordenHelper(n->izq, resultado);       // Recorrer el subarbol izquierdo primero | Palabras menores se procesan antes
        resultado.push_back(n->dato);           // Agregar el dato del nodo actual | Llega en el orden correcto gracias al recorrido inorden
        inordenHelper(n->der, resultado);       // Recorrer el subarbol derecho al final | Palabras mayores se procesan despues
    }

    void limpiarHelper(NodoAVL* n) {    // Libera recursivamente la memoria de todos los nodos | Auxiliar para el destructor | Evita memory leaks
        if (n == nullptr) return;        // Caso base | Si el nodo es nulo no hay nada que liberar
        limpiarHelper(n->izq);           // Liberar primero el subarbol izquierdo para no perder la referencia
        limpiarHelper(n->der);           // Liberar el subarbol derecho
        delete n;                        // Liberar el nodo actual solo despues de sus hijos
    }

public:
    ArbolAVL() : raiz(nullptr), nodosReales(0) {}    // Constructor | Inicializa el arbol vacio con raiz nula y contador de nodos en cero
    ~ArbolAVL() { limpiarHelper(raiz); }  // Destructor | Libera toda la memoria del arbol llamando al helper recursivo

    void insertar(const string& val) {      // Funcion publica para insertar una palabra | Delega en insertarHelper para la insercion recursiva con rebalanceo
        raiz = insertarHelper(raiz, val);   // La raiz puede cambiar tras una rotacion, por eso siempre actualizamos el puntero con el retorno del helper
    }

    vector<string> inorden() const {        // Funcion publica para obtener el recorrido inorden | Retorna un vector con las palabras unicas ordenadas
        vector<string> resultado;           // resultado | Vector donde se almacenaran las palabras durante el recorrido | Se llenara de menor a mayor
        inordenHelper(raiz, resultado);     // Delegar el recorrido al helper recursivo | El vector se llena por referencia para evitar copias intermedias
        return resultado;         // Retornar el vector con las palabras en orden alfabetico
    }

    int alturaTotal() const { return alturaDeNodo(raiz); }    // Retorna la altura total del arbol | Debe ser aproximadamente log2(n) para validar el balanceo AVL
    int totalNodos()  const { return nodosReales; }           // Retorna el numero de nodos unicos insertados | Excluye duplicados ignorados durante la insercion
};


// ===================================================================
//   Cálculo / Estimación de la Memoria
// ===================================================================

void reportarMemoria(const string& algoritmo, size_t n, size_t bytesPorPalabra,    // Imprime una estimacion de la memoria utilizada segun la estructura de datos del algoritmo
                     bool esArbol, int alturaArbol) {
    cout << "Estimacion de Memoria | " << algoritmo << " |\n";

    if (!esArbol) {
        size_t memoriaVector = n * (sizeof(string) + bytesPorPalabra);    // memoriaVector | Estimacion total del vector | No incluye la capacidad extra que el vector puede reservar internamente
        cout << "Estructura: vector<string> con " << n << " elementos\n";
        cout << "Tamano por elemento : " << sizeof(string) << " bytes (objeto string) + "
             << bytesPorPalabra << " bytes (cadena) = "
             << sizeof(string) + bytesPorPalabra << " bytes aprox\n";
        cout << "Memoria total aprox: " << memoriaVector / 1024 << " KB\n";
        if (algoritmo == "QuickSort") {    // El stack de recursion solo aplica a QuickSort, HeapSort es completamente iterativo
            cout << "Pila de recursion: O(log n) = ~" << (int)log2((double)n)
                 << "niveles garantizados (pivote mediana de tres + TCO manual)\n";
        }
    } else {
        size_t bytesPorNodo = sizeof(NodoAVL) + bytesPorPalabra;    // bytesPorNodo | Tamano total de cada nodo | sizeof(NodoAVL) incluye el string, los 2 punteros y el int altura
        size_t memoriaArbol = n * bytesPorNodo;          // memoriaArbol | Estimacion total del arbol | n nodos * bytes por nodo
        cout << "Estructura: Arbol AVL con " << n << " nodos\n";
        cout << "Altura real: " << alturaArbol
             << "Altura teorica O(log n) = ~" << (int)log2((double)n) << "\n";
        cout << "Tamano por nodo: " << bytesPorNodo
             << "bytes (NodoAVL + contenido cadena)\n";
        cout << "Memoria total aprox: " << memoriaArbol / 1024 << " KB\n";
        cout << "INFO | El arbol usa mas memoria que el vector porque cada nodo"
             << "agrega 2 punteros (izq, der) y el int altura ademas del string\n";
    }
}


// ===================================================================
//   Conclusiones:
//   Razonamiento inferencial sobre por que los tiempos difieren
//   pese a que los tres algoritmos comparten O(n log n) en teoria.
//   No repite estadisticas individuales ni la tabla comparativa.
// ===================================================================

void imprimirConclusion(double tQS, double tHS, double tAVL, size_t n) {

    // Determinar el algoritmo mas rapido para nombrarlo en la conclusion
    string mejor  = "QuickSort";    // mejor | Nombre del algoritmo con menor tiempo real de ejecucion
    double tMejor = tQS;
    if (tHS  < tMejor) { tMejor = tHS;  mejor = "HeapSort"; }    // Si HeapSort fue mas rapido, actualizar el mejor
    if (tAVL < tMejor) { tMejor = tAVL; mejor = "AVL Tree"; }    // Si AVL fue mas rapido, actualizar el mejor

    // Rendimiento: palabras procesadas por milisegundo por cada algoritmo
    double tpQS  = n / tQS;     // tpQS  | Rendimiento de QuickSort  en palabras/ms | Metrica de eficiencia independiente del tamano del dataset
    double tpHS  = n / tHS;     // tpHS  | Rendimiento de HeapSort
    double tpAVL = n / tAVL;    // tpAVL | Rendimiento del AVL (insercion + inorden)

    cout << "\n=======================================================\n";
    cout << "  CONCLUSION\n";
    cout << "=======================================================\n";

    cout << "\n  Rendimiento real (palabras procesadas por ms):\n";
    cout << "    QuickSort : " << fixed << setprecision(0) << tpQS  << " palabras/ms\n";
    cout << "    HeapSort  : " << fixed << setprecision(0) << tpHS  << " palabras/ms\n";
    cout << "    AVL Tree  : " << fixed << setprecision(0) << tpAVL << " palabras/ms\n";

    cout << "\n  Los tres algoritmos tienen la misma complejidad teorica O(n log n),\n";
    cout << "  sin embargo sus tiempos reales difieren de forma significativa.\n";
    cout << "  Esto ocurre porque la notacion asintótica describe el crecimiento\n";
    cout << "  a escala pero omite la constante oculta, que en la practica esta\n";
    cout << "  determinada principalmente por la localidad de cache de la CPU:\n\n";

    cout << "  - QuickSort opera sobre un vector contiguo en memoria. Sus accesos\n";
    cout << "    son casi siempre secuenciales, lo que maximiza los cache hits.\n\n";

    cout << "  - HeapSort tambien usa el vector, pero los saltos padre->hijo\n";
    cout << "    (i -> 2i+1 -> 2i+2) son accesos no contiguos que producen\n";
    cout << "    cache misses con mayor frecuencia, elevando su tiempo real.\n\n";

    cout << "  - El AVL realiza " << n << " allocaciones dinamicas de nodos dispersos\n";
    cout << "    en memoria. Los punteros izq/der apuntan a direcciones arbitrarias,\n";
    cout << "    causando la peor localidad de cache de los tres. Ademas, cada\n";
    cout << "    insercion puede desencadenar rotaciones de rebalanceo adicionales.\n\n";

    cout << "  Para ordenamiento puro de un dataset fijo, " << mejor << " es la\n";
    cout << "  eleccion optima. El AVL agrega valor solo si la estructura debe\n";
    cout << "  permanecer dinamica: busquedas O(log n), inserciones incrementales\n";
    cout << "  o consultas de rango posteriores al ordenamiento inicial.\n";
    cout << "=======================================================\n";
}


// ===================================================================
//   Función Main
// ===================================================================

int main() {
    cout << "\n=======================================================\n";
    cout << "  PRACTICA I - ORDENAMIENTO DE DATASET GRANDE (C++)   \n";
    cout << "  ST0245 - Estructuras de Datos y Algoritmos - EAFIT   \n";
    cout << "=======================================================\n\n";

    // ----- Cargar el dataset -----
    cout << "Cargando dataset desde 'dataset.txt'...\n";
    vector<string> dataset = leerDataset("dataset.txt");    // dataset | Vector con las 100k palabras en orden aleatorio | Fuente de datos comun para los tres algoritmos

    if (dataset.empty()) {    // Verificar que la lectura fue exitosa antes de proceder con los algoritmos
        cerr << "| Error | El dataset esta vacio o no se pudo leer correctamente.\n";
        return 1;             // Terminar con codigo de error
    }
    cout << "Dataset cargado | " << dataset.size() << " palabras\n";

    // Calcular promedio de bytes por palabra para la estimacion de memoria
    size_t totalBytes = 0;
    for (const auto& p : dataset) totalBytes += p.size() + 1;    // Sumar la longitud de cada palabra + 1 byte del caracter nulo de terminacion
    size_t bytesPorPalabra = totalBytes / dataset.size();     // bytesPorPalabra | Promedio de bytes por string en este dataset especifico

    // Deteccion silenciosa de duplicados para el reporte del AVL (no se imprime aqui)
    set<string> unicos(dataset.begin(), dataset.end());    // unicos | Conjunto de palabras sin repeticion | Solo se usa para la bandera interna
    bool hayDuplicados = (unicos.size() < dataset.size());     // hayDuplicados | Si es true, el AVL tendra menos nodos que el dataset y se reporta al ejecutar el AVL


    // ===== [1] QUICKSORT =====
    vector<string> qs = dataset;    // qs | Copia independiente del dataset para QuickSort | Cada algoritmo trabaja sobre su propia copia para garantizar igualdad de condiciones
    cout << "\n[1] Ejecutando QuickSort sobre vector<string>...\n";
    auto ini = chrono::high_resolution_clock::now();    // ini | Captura el instante exacto antes de ordenar | high_resolution_clock es el reloj de maxima precision disponible
    quickSort(qs);
    auto fin = chrono::high_resolution_clock::now();    // fin | Captura el instante exacto despues de ordenar | La diferencia fin - ini es el tiempo real de ejecucion
    double tiempoQS = chrono::duration<double, milli>(fin - ini).count();    // tiempoQS | Tiempo en milisegundos que tardo QuickSort | duration<double,milli> convierte la duracion a ms con decimales
    cout << "Tiempo QuickSort: " << fixed << setprecision(3) << tiempoQS << " ms\n";
    cout << "Primeras 5 palabras: ";
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
    cout << "Tiempo HeapSort: " << fixed << setprecision(3) << tiempoHS << " ms\n";
    cout << "Primeras 5 palabras: ";
    for (int i = 0; i < 5 && i < (int)hs.size(); i++) cout << hs[i] << " | ";
    cout << "\n";
    reportarMemoria("HeapSort", hs.size(), bytesPorPalabra, false, 0);


    // ===== [3] ARBOL AVL =====
    cout << "\n[3] Ejecutando Arbol AVL (insercion + recorrido inorden)...\n";
    ArbolAVL avl;    // avl | Arbol AVL donde se insertaran todas las palabras | Se autobalancea en cada insercion para garantizar altura O(log n)
    ini = chrono::high_resolution_clock::now();    // Registrar el tiempo de inicio | Incluye tanto la insercion como el recorrido inorden
    for (const auto& p : dataset) avl.insertar(p);    // Insertar cada palabra en el arbol | Cada insercion rebalancea el arbol si es necesario mediante rotaciones
    vector<string> avlResult = avl.inorden();          // avlResult | Vector con las palabras unicas obtenidas por el recorrido inorden | El inorden de un BST produce la secuencia ordenada
    fin = chrono::high_resolution_clock::now();    // Registrar el tiempo de fin | Despues del inorden completo
    double tiempoAVL = chrono::duration<double, milli>(fin - ini).count();    // tiempoAVL | Tiempo total del AVL: insercion de todas las palabras + recorrido inorden
    cout << "Tiempo AVL: " << fixed << setprecision(3) << tiempoAVL << " ms (insercion + inorden)\n";
    if (hayDuplicados)    // Informar sobre duplicados solo si el dataset los tiene | Si no hay duplicados, el conteo del AVL sera igual al del dataset
        cout << "Nodos unicos en AVL: " << avl.totalNodos() << " (se ignoraron duplicados)\n";
    cout << "Primeras 5 palabras: ";
    for (int i = 0; i < 5 && i < (int)avlResult.size(); i++) cout << avlResult[i] << " | ";
    cout << "\n";
    reportarMemoria("AVL Tree", avlResult.size(), bytesPorPalabra, true, avl.alturaTotal());


    // ===== ANALISIS COMPARATIVO =====
    cout << "\n=======================================================\n";
    cout << "  ANALISIS COMPARATIVO\n";
    cout << "=======================================================\n";
    cout << "  " << left
         << setw(12) << "Algoritmo"   << " | "
         << setw(12) << "Tiempo (ms)" << " | "
         << "Complejidad temporal\n";
    cout << "  " << left << setw(12) << "QuickSort"
         << "   " << setw(12) << (to_string((int)tiempoQS)  + " ms")
         << "   O(n log n) promedio y garantizado (pivote mediana de tres)\n";
    cout << "  " << left << setw(12) << "HeapSort"
         << "   " << setw(12) << (to_string((int)tiempoHS)  + " ms")
         << "   O(n log n) garantizado en todos los casos\n";
    cout << "  " << left << setw(12) << "AVL Tree"
         << "   " << setw(12) << (to_string((int)tiempoAVL) + " ms")
         << "   O(n log n) insercion + O(n) inorden\n";

    cout << "\n  Complejidad espacial:\n";
    cout << "    QuickSort: O(n) vector + O(log n) pila de recursion\n";
    cout << "    HeapSort: O(n) vector (in-place, sin memoria adicional)\n";
    cout << "    AVL Tree: O(n) nodos con 2 punteros y campo altura (mayor costo por nodo)\n";
    cout << "=======================================================\n";


    // ===== CONCLUSION =====
    imprimirConclusion(tiempoQS, tiempoHS, tiempoAVL, dataset.size());

    return 0;    // Indica que el programa finalizo correctamente
}
