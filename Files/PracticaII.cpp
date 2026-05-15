// ===================================================================================
// Practica II | EAFIT Social Network | Red Social Universitaria
// Materia: SI2001-10 Estructuras de Datos y Algoritmos 1
// Docente: Diego Ivan Cruz Ordieres
// ===================================================================================
// Este programa implementa un grafo no dirigido usando listas de adyacencia
// El escenario es una red social de estudiantes de la Universidad EAFIT
// Cada estudiante es un vertice (nodo) y cada amistad es una arista no dirigida
// | INFO | ¿Por que no dirigido? Porque la amistad es mutua, si A es amigo de B,
// entonces B es amigo de A. Es decir, la relacion se lee en ambos sentidos
// ===================================================================================

#include <iostream>     // Libreria | Permite usar cout y cin para entrada/salida
#include <vector>       // Libreria | Permite usar el contenedor dinamico vector
#include <string>       // Libreria | Permite usar el tipo string para los nombres
#include <queue>        // Libreria | Permite usar queue para el recorrido BFS
#include <unordered_map>// Libreria | Permite usar un mapa hash para busquedas rapidas de vertices por nombre
#include <unordered_set>// Libreria | Permite usar un conjunto hash para marcar vertices visitados en recorridos
#include <algorithm>    // Libreria | Permite usar find para buscar elementos en vectores
using namespace std;


// ==========================================
// Estructura Vertice | Representa un nodo del grafo, en este caso, un estudiante
// ==========================================
struct Vertice {
    string nombre;              // Nombre del estudiante
    vector<string> adyacentes;  // Lista de adyacencia | Guarda los nombres de los estudiantes con los que tiene amistad directa
    // | INFO | ¿Por que un vector de strings y no de punteros? Porque usamos los nombres como identificadores unicos
    // y el mapa del grafo nos permite acceder a cualquier vertice por su nombre en tiempo O(1)
};


// ==========================================
// Clase GrafoNoDirigido | Clase central del programa
// Modela la red social completa usando listas de adyacencia
// ==========================================
class GrafoNoDirigido {

private:
    unordered_map<string, Vertice> vertices;    // Mapa | Asocia cada nombre de estudiante con su estructura Vertice
    // | INFO | ¿Por que unordered_map? Porque nos permite buscar, insertar y acceder a vertices por nombre en tiempo O(1) promedio
    // Es como una tabla hash donde la clave es el nombre del estudiante y el valor es toda su informacion
    int totalAristas;   // Contador | Numero total de aristas en el grafo | Se mantiene actualizado con cada insercion

public:
    // Declaramos el constructor | El grafo comienza vacio, sin vertices ni aristas
    GrafoNoDirigido() : totalAristas(0) {}


    // ==========================================
    // a. Metodos de construccion del grafo
    // ==========================================

    void agregarVertice(string nombre) {    // Funcion | Agrega un nuevo estudiante (vertice) a la red social
        if (existeVertice(nombre)) {        // Verificamos si el estudiante ya existe en la red
            cout << "| Error | El estudiante \"" << nombre << "\" ya existe en la red social" << endl;
            return;
        }
        Vertice nuevo;              // Crear nuevo vertice
        nuevo.nombre = nombre;      // Asignar el nombre del estudiante
        // La lista de adyacencia se crea vacia por defecto | El estudiante aun no tiene amigos registrados
        vertices[nombre] = nuevo;   // Insertar el vertice en el mapa usando el nombre como clave
    }

    void agregarArista(string v1, string v2) {  // Funcion | Agrega una amistad (arista no dirigida) entre dos estudiantes
        // Validar que ambos vertices existan antes de crear la conexion
        if (!existeVertice(v1)) {
            cout << "| Error | El estudiante \"" << v1 << "\" no existe en la red social" << endl;
            return;
        }
        if (!existeVertice(v2)) {
            cout << "| Error | El estudiante \"" << v2 << "\" no existe en la red social" << endl;
            return;
        }
        if (v1 == v2) {     // Un estudiante no puede ser amigo de si mismo
            cout << "| Error | No se puede crear una amistad de un estudiante consigo mismo" << endl;
            return;
        }
        if (sonAdyacentes(v1, v2)) {    // Verificar que la amistad no exista ya
            cout << "| Error | La amistad entre \"" << v1 << "\" y \"" << v2 << "\" ya existe" << endl;
            return;
        }

        // | INFO | Como el grafo es no dirigido, la arista se agrega en AMBAS listas de adyacencia
        // Si Alejandro es amigo de Maria, entonces Maria tambien es amiga de Alejandro
        vertices[v1].adyacentes.push_back(v2);  // Agregar v2 a la lista de adyacencia de v1
        vertices[v2].adyacentes.push_back(v1);  // Agregar v1 a la lista de adyacencia de v2
        totalAristas++;     // Incrementar el contador de aristas | Solo se cuenta una vez aunque se inserte en ambas listas
    }


    // ==========================================
    // b. Metodos de representacion del grafo
    // ==========================================

    void mostrarGrafo() {   // Funcion | Muestra el grafo completo en formato de lista de adyacencia
        cout << "\n========================================" << endl;
        cout << " Red Social EAFIT | Lista de Adyacencia" << endl;
        cout << "========================================\n" << endl;

        if (vertices.empty()) {     // Si no hay vertices, el grafo esta vacio
            cout << "  La red social esta vacia." << endl;
            return;
        }

        for (auto& par : vertices) {    // Recorrer cada par (nombre, vertice) del mapa
            // par.first es el nombre del estudiante (clave del mapa)
            // par.second es la estructura Vertice completa
            cout << "  " << par.first << " -> [ ";
            for (int i = 0; i < par.second.adyacentes.size(); i++) {
                cout << par.second.adyacentes[i];
                if (i < par.second.adyacentes.size() - 1) cout << ", ";    // Separar con coma excepto el ultimo
            }
            cout << " ]" << endl;
        }
        cout << "\n  Total de estudiantes: " << obtenerNumeroVertices() << endl;
        cout << "  Total de amistades:   " << obtenerNumeroAristas() << endl;
        cout << endl;
    }


    // ==========================================
    // c. Consultas estructurales
    // ==========================================

    bool existeVertice(string nombre) {     // Funcion | Verifica si un estudiante existe en la red social
        // find busca la clave en el mapa | Si la encuentra retorna un iterador a ese elemento
        // Si no la encuentra retorna vertices.end(), que es el "final" del mapa
        return vertices.find(nombre) != vertices.end();
    }

    bool sonAdyacentes(string v1, string v2) {  // Funcion | Verifica si dos estudiantes son amigos (si existe una arista entre ellos)
        if (!existeVertice(v1) || !existeVertice(v2)) return false;     // Si alguno no existe, no pueden ser adyacentes

        // Buscar v2 en la lista de adyacencia de v1
        // | INFO | ¿Por que solo buscamos en una lista? Porque el grafo es no dirigido y mantenemos la simetria
        // Si v2 esta en la lista de v1, entonces v1 tambien esta en la lista de v2
        vector<string>& adj = vertices[v1].adyacentes;
        return find(adj.begin(), adj.end(), v2) != adj.end();
    }

    int obtenerGrado(string nombre) {   // Funcion | Retorna el grado de un vertice | El grado es la cantidad de amigos que tiene un estudiante
        // | INFO | En un grafo no dirigido, el grado de un vertice es simplemente el tamano de su lista de adyacencia
        // Segun Weiss (Cap. 9), el grado de un vertice v es el numero de aristas incidentes a v
        if (!existeVertice(nombre)) {
            cout << "| Error | El estudiante \"" << nombre << "\" no existe en la red social" << endl;
            return -1;      // Retornar -1 como indicador de error
        }
        return vertices[nombre].adyacentes.size();
    }

    int obtenerNumeroVertices() {   // Funcion | Retorna el numero total de estudiantes en la red social
        return vertices.size();
    }

    int obtenerNumeroAristas() {    // Funcion | Retorna el numero total de amistades en la red social
        // | INFO | ¿Por que no calculamos sumando los tamanos de las listas dividido 2?
        // Porque mantenemos un contador actualizado (totalAristas) para eficiencia O(1)
        // Pero si quisieramos verificar: total = sumatoria de todos los grados / 2
        return totalAristas;
    }


    // ==========================================
    // d. Recorridos del grafo
    // ==========================================

    // Recorrido BFS (Breadth-First Search | Busqueda en Anchura)
    // | INFO | BFS explora el grafo por niveles, es decir, primero visita todos los vecinos directos,
    // luego los vecinos de esos vecinos, y asi sucesivamente | Usa una cola (queue) para mantener el orden
    // Conceptualmente: si empezamos en un estudiante, BFS visita primero a TODOS sus amigos directos,
    // luego a los amigos de sus amigos, etc. Es como una onda expansiva desde el origen
    void BFS(string inicio) {
        if (!existeVertice(inicio)) {   // Verificar que el vertice de inicio exista
            cout << "| Error | El estudiante \"" << inicio << "\" no existe en la red social" << endl;
            return;
        }

        cout << "\n--- BFS (Busqueda en Anchura) desde \"" << inicio << "\" ---" << endl;
        cout << "  Recorrido: ";

        unordered_set<string> visitados;    // Conjunto | Guarda los nombres de los estudiantes ya visitados para no repetirlos
        queue<string> cola;                 // Cola | Estructura FIFO que determina el orden de visita | Primero en entrar, primero en salir

        visitados.insert(inicio);   // Marcar el vertice inicial como visitado
        cola.push(inicio);          // Insertar el vertice inicial en la cola para comenzar el recorrido

        while (!cola.empty()) {             // Mientras haya vertices pendientes por procesar
            string actual = cola.front();   // Aux | Tomar el primer elemento de la cola (el que lleva mas tiempo esperando)
            cola.pop();                     // Eliminarlo de la cola ya que lo estamos procesando
            cout << actual << " ";          // Imprimir el estudiante visitado

            // Recorrer todos los amigos del estudiante actual
            for (string& vecino : vertices[actual].adyacentes) {
                if (visitados.find(vecino) == visitados.end()) {    // Si el vecino NO ha sido visitado
                    visitados.insert(vecino);   // Marcarlo como visitado para no procesarlo de nuevo
                    cola.push(vecino);          // Agregarlo a la cola para visitarlo despues
                    // | INFO | ¿Por que se marca como visitado al insertarlo en la cola y no al sacarlo?
                    // Porque si lo marcamos al sacarlo, podria insertarse varias veces en la cola
                    // desde diferentes vecinos, desperdiciando memoria y tiempo
                }
            }
        }
        cout << endl;
    }

    // Recorrido DFS (Depth-First Search | Busqueda en Profundidad)
    // | INFO | DFS explora el grafo yendo lo mas profundo posible por cada camino antes de retroceder
    // Conceptualmente: si empezamos en un estudiante, DFS sigue la cadena de amigos lo mas lejos posible
    // antes de "devolverse" y explorar otro camino | Usa recursion (o una pila) para mantener el orden
    // | INFO | ¿Cual es la diferencia con BFS? BFS explora por niveles (anchura), DFS explora por profundidad
    // BFS usa cola (FIFO), DFS usa la pila de llamadas recursivas (LIFO)
    // BFS encuentra el camino mas corto en grafos sin peso, DFS no garantiza eso
    void DFS(string inicio) {
        if (!existeVertice(inicio)) {   // Verificar que el vertice de inicio exista
            cout << "| Error | El estudiante \"" << inicio << "\" no existe en la red social" << endl;
            return;
        }

        cout << "\n--- DFS (Busqueda en Profundidad) desde \"" << inicio << "\" ---" << endl;
        cout << "  Recorrido: ";

        unordered_set<string> visitados;    // Conjunto | Guarda los nombres de los estudiantes ya visitados
        DFSAuxiliar(inicio, visitados);     // Llamar al metodo recursivo auxiliar que realiza el recorrido
        cout << endl;
    }

private:
    // Metodo auxiliar recursivo para DFS | Es privado porque solo lo usa DFS internamente
    // | INFO | ¿Por que un metodo auxiliar? Porque DFS necesita pasar el conjunto de visitados entre llamadas recursivas
    // y no queremos que el usuario tenga que manejar ese detalle, por eso DFS() publica solo recibe el inicio
    void DFSAuxiliar(string actual, unordered_set<string>& visitados) {
        visitados.insert(actual);   // Marcar el vertice actual como visitado
        cout << actual << " ";      // Imprimir el estudiante visitado

        // Recorrer todos los amigos del estudiante actual
        for (string& vecino : vertices[actual].adyacentes) {
            if (visitados.find(vecino) == visitados.end()) {    // Si el vecino NO ha sido visitado
                DFSAuxiliar(vecino, visitados);     // Llamada recursiva | Se "sumerge" en ese vecino antes de seguir con los demas
                // | INFO | Aqui es donde DFS se diferencia de BFS: en vez de guardar el vecino en una cola para despues,
                // DFS entra inmediatamente a explorar ese vecino y toda su rama antes de volver aqui
            }
        }
    }
};


// ==========================================
// Funcion principal | Ejemplo de ejecucion con la red social EAFIT
// ==========================================
int main() {
    GrafoNoDirigido red;    // Crear la red social como un grafo no dirigido vacio

    cout << "\n===========================================================" << endl;
    cout << " EAFIT SOCIAL NETWORK | Prototipo de Red Social Estudiantil" << endl;
    cout << "===========================================================" << endl;

    // --- Agregar estudiantes (vertices) a la red ---
    cout << "\n>> Registrando estudiantes en la red social...\n" << endl;
    red.agregarVertice("Alejandro");
    red.agregarVertice("Maria");
    red.agregarVertice("Carlos");
    red.agregarVertice("Valentina");
    red.agregarVertice("Santiago");
    red.agregarVertice("Isabella");
    red.agregarVertice("Juan");
    red.agregarVertice("Camila");
    cout << "  Se registraron 8 estudiantes." << endl;

    // --- Agregar amistades (aristas no dirigidas) entre estudiantes ---
    cout << "\n>> Creando amistades entre estudiantes...\n" << endl;
    red.agregarArista("Alejandro", "Maria");        // Alejandro y Maria son amigos
    red.agregarArista("Alejandro", "Carlos");       // Alejandro y Carlos son amigos
    red.agregarArista("Alejandro", "Valentina");    // Alejandro y Valentina son amigos
    red.agregarArista("Maria", "Santiago");          // Maria y Santiago son amigos
    red.agregarArista("Maria", "Isabella");          // Maria e Isabella son amigas
    red.agregarArista("Carlos", "Santiago");         // Carlos y Santiago son amigos
    red.agregarArista("Carlos", "Juan");             // Carlos y Juan son amigos
    red.agregarArista("Valentina", "Camila");        // Valentina y Camila son amigas
    red.agregarArista("Santiago", "Isabella");       // Santiago e Isabella son amigos
    red.agregarArista("Juan", "Camila");             // Juan y Camila son amigos
    cout << "  Se crearon 10 amistades." << endl;

    // --- Mostrar el grafo completo como lista de adyacencia ---
    red.mostrarGrafo();

    // --- Validaciones | Probar que las validaciones funcionan correctamente ---
    cout << ">> Probando validaciones...\n" << endl;
    red.agregarVertice("Alejandro");                // Error | El estudiante ya existe
    red.agregarArista("Alejandro", "Maria");        // Error | La amistad ya existe
    red.agregarArista("Alejandro", "Alejandro");    // Error | Amistad consigo mismo
    red.agregarArista("Alejandro", "Pedro");        // Error | Pedro no existe en la red

    // --- Consultas estructurales ---
    cout << "\n>> Consultas estructurales...\n" << endl;

    // Verificar existencia de vertices
    cout << "  ¿Existe \"Alejandro\"?  -> " << (red.existeVertice("Alejandro") ? "Si" : "No") << endl;
    cout << "  ¿Existe \"Pedro\"?      -> " << (red.existeVertice("Pedro") ? "Si" : "No") << endl;

    // Verificar adyacencia entre vertices
    cout << "\n  ¿Son amigos Alejandro y Maria?     -> " << (red.sonAdyacentes("Alejandro", "Maria") ? "Si" : "No") << endl;
    cout << "  ¿Son amigos Alejandro y Isabella?  -> " << (red.sonAdyacentes("Alejandro", "Isabella") ? "Si" : "No") << endl;

    // Consultar grado de un vertice | Numero de amigos de cada estudiante
    cout << "\n  Grado de Alejandro (amigos): " << red.obtenerGrado("Alejandro") << endl;
    cout << "  Grado de Maria (amigos):     " << red.obtenerGrado("Maria") << endl;
    cout << "  Grado de Camila (amigos):    " << red.obtenerGrado("Camila") << endl;

    // Consultar totales del grafo
    cout << "\n  Total de estudiantes: " << red.obtenerNumeroVertices() << endl;
    cout << "  Total de amistades:   " << red.obtenerNumeroAristas() << endl;

    // --- Recorridos del grafo ---
    cout << "\n>> Recorridos del grafo...\n" << endl;

    // BFS | Busqueda en Anchura desde Alejandro
    // Explora por niveles: primero los amigos directos de Alejandro, luego los amigos de esos amigos, etc.
    red.BFS("Alejandro");

    // DFS | Busqueda en Profundidad desde Alejandro
    // Explora lo mas profundo posible por cada rama antes de devolverse
    red.DFS("Alejandro");

    cout << "\n===========================================================" << endl;
    cout << " Fin del programa" << endl;
    cout << "===========================================================" << endl;

    return 0;   // Indica que el programa finalizo correctamente
}
