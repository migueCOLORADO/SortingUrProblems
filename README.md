# Practica - 01 | Simulador de Algoritmos de Ordamiento

## Tabla de Contenidos
1. [Introducción](#introducción)
2. [Presentación del Problema](#presentación-del-problema)
3. [Solución planteada](#solución-planteada)
4. [Autores](#autores)
5. [Información del Curso](#información-del-curso)

<div style = "text-align: justify;">

## Introducción

En este documento, presentaremos el desarrollo de la Práctica I del curso de Estructuras de Datos y Algoritmos, en la que se implementan y comparan tres algoritmos de ordenamiento para un dataset grande de palabras. La práctica busca demostrar el dominio de conceptos fundamentales como algoritmos de ordenamiento, estructuras de datos (vectores, heaps, árboles AVL), complejidad algorítmica y medición de rendimiento.

## Presentación del Problema

La práctica consiste en ordenar un dataset de 100,000 palabras almacenadas en un archivo de texto codificado en UTF-16 LE (Little Endian). El objetivo es implementar y comparar tres métodos de ordenamiento diferentes: QuickSort, HeapSort y un Árbol AVL con inserción de todas las palabras seguida de un recorrido inorder. 

Para cada algoritmo, se debe medir el tiempo de ejecución y estimar el uso de memoria, considerando tanto la estructura de datos utilizada como el contenido de las cadenas. Finalmente, se realiza un análisis comparativo de los resultados, evaluando la eficiencia temporal y espacial de cada enfoque, así como sus ventajas y desventajas en el contexto de ordenar un dataset grande.

## Solución planteada

La solución se implementa en C++ utilizando la biblioteca estándar (STL) para aprovechar contenedores eficientes como vector y algoritmos optimizados. El programa lee el archivo dataset.txt, aplica los tres algoritmos de ordenamiento sobre copias independientes del dataset para garantizar condiciones equitativas, mide los tiempos de ejecución con alta precisión utilizando la biblioteca chrono, estima el uso de memoria basado en el tamaño de las estructuras de datos y el contenido de las cadenas, y presenta un análisis comparativo con las complejidades teóricas.

### Funcionalidades

- **Lectura del dataset**: Función `leerDataset` que abre el archivo en modo binario, salta el BOM (Byte Order Mark) de UTF-16 LE, lee los caracteres de a 2 bytes, reconstruye los valores Unicode y construye un vector de strings con todas las palabras del dataset.

- **Implementación de QuickSort**: Algoritmo de ordenamiento rápido que utiliza particionamiento recursivo sobre un vector<string>, con pivote en el último elemento y partición in-place.

- **Implementación de HeapSort**: Algoritmo de ordenamiento por montículo que construye un max-heap sobre el vector y extrae el máximo repetidamente, operando in-place sin memoria adicional significativa.

- **Implementación de Árbol AVL**: Estructura de árbol binario de búsqueda balanceado que inserta todas las palabras, rebalanceando automáticamente tras cada inserción mediante rotaciones, y realiza un recorrido inorder para obtener la secuencia ordenada.

- **Medición de tiempos**: Utiliza `chrono::high_resolution_clock` para capturar instantes precisos antes y después de cada ordenamiento, calculando la duración en milisegundos.

- **Estimación de memoria**: Función `reportarMemoria` que calcula el uso aproximado de memoria basado en el tamaño de los objetos (vector, nodos AVL) y el contenido de las cadenas, diferenciando entre estructuras para vector y árbol.

- **Análisis comparativo**: Imprime una tabla con tiempos medidos, complejidades temporales y espaciales, y conclusiones sobre el rendimiento práctico de cada algoritmo.

## Autores

**Miguel Ángel Colorado Castaño**  
**Alejandro Soto Bermeo** <br>

## Información del Curso
**Curso |** Estructura Dat. y Algoritmos 1 - S2661-2111 <br>
**Asignación |** Practica - 01 <br>
**Universidad |** Universidad EAFIT <br>
**Año |** 2026-1

</div>


