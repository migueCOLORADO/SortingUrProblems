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

La solución está implementada en C++ (GNU++17) y ejecuta una simulación completa de tres algoritmos de ordenamiento sobre el mismo dataset. El programa lee el archivo `dataset.txt`, detecta y decodifica UTF-16 LE (BOM `FF FE`) o cae a UTF-8/ASCII, carga las palabras en un `vector<string>`, y luego ejecuta:

1. QuickSort (con mejoras de pivot mediana de tres, Insertion Sort para segmentos de tamaño <= 20 y TCO manual de pila).
2. HeapSort (construcción de max-heap y extracción iterativa en un arreglo in-place).
3. Árbol AVL (inserción de todas las palabras en un BST balanceado y recorrido inorden para generar la ordenación de valores únicos).

Para cada algoritmo, el programa mide el tiempo de ejecución (milisegundos), muestra las primeras 5 palabras ordenadas, estima memoria utilizada y compara el comportamiento real.

### Funcionalidades

- Lectura robusta del dataset en `leerDataset`:
  - modo binario, detección de BOM UTF-16 LE
  - parsing de lineas para UTF-8/ASCII
  - manejo de CR/LF, salta lineas vacías

- QuickSort:
  - `medianaDeTres` para elegir pivote estable
  - `particion` con intercambio in-place
  - `quickSortHelper` con recursion en rama corta + bucle (TCO manual)
  - `insertionSort` en subsegmentos de longitud <= 20

- HeapSort:
  - `heapify` para max-heap
  - construcción del heap en O(n)
  - extracción ordenada intercambiando `arr[0]` con `arr[i]` y heapificando

- Árbol AVL:
  - `ArbolAVL` con rotaciones (`rotarIzquierda`, `rotarDerecha`) y rebalanceo automático
  - `insertar` ignora duplicados (conjunto ordenado)
  - `inorden` genera orden ascendente de palabras únicas
  - `alturaTotal`, `totalNodos` para métricas internas

- Métricas y reportes:
  - Cronometría con `chrono::high_resolution_clock`
  - `reportarMemoria`:
    - Estimaciones separadas para vector (QuickSort/HeapSort) y árbol AVL
    - Cálculo aproximado en KB de uso de memoria
    - Inspección de pila de recursion para QuickSort y altura de árbol AVL

- Comparativo y conclusiones:
  - Tabla de tiempos y complejidad asintótica en pantalla
  - Impresión de la estructura con el mejor tiempo
  - Análisis cualitativo de localización de memoria y constantes ocultas

## Autores

**Miguel Ángel Colorado Castaño**  
**Alejandro Soto Bermeo** <br>

## Información del Curso
**Curso |** Estructura Dat. y Algoritmos 1 - S2661-2111 <br>
**Asignación |** Practica - 01 <br>
**Universidad |** Universidad EAFIT <br>
**Año |** 2026-1

</div>


