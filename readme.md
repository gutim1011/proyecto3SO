# Programa

Nuestro programa es una aplicación en C++ que permite procesar imágenes aplicando rotación y escalado, con la posibilidad de comparar el uso de memoria y tiempos de procesamiento utilizando un sistema de asignación convencional o el sistema **Buddy System**.  
Desde su última versión, **incluye procesamiento paralelo con OpenMP**, lo que mejora significativamente el rendimiento en imágenes de gran tamaño.

---

## Requisitos

- C++17 o superior
- OpenMP (normalmente incluido con `g++` en Linux/WSL)
- [STB Image Libraries](https://github.com/evalenciEAFIT/courses/tree/main/SistemasOperativos/LlamadaSistema/Memoria/buddySystem): `stb_image.h`, `stb_image_write.h` (ya están incluidos en `src/`)
- `make`
- g++ (recomendado en Linux o WSL si estás en Windows)

---

## Estructura del proyecto

project_root/ 
├── Makefile 
├── src/ # Código fuente (.cpp, .h) 
├── build/ # archivos de objetos 
├── bin/ # Binario compilado 
├── assets/ # Imágenes de entrada 
├── out/ # Imágenes de salida 
└── README.md

---

## Compilación

Desde la raíz del proyecto, ejecuta:

```bash
make
```
Esto compilará el proyecto y generará el ejecutable en la carpeta bin/.

---

## Ejecución

El programa se ejecuta desde la línea de comandos utilizando el siguiente formato:

```bash
./bin/program_image entrada.jpg salida.jpg -angulo <GRADOS> -escalar <FACTOR> [-buddy]
```
Estos son los parámetros para la ejecución del programa:

| Argumento            | Descripción                                                        |
|----------------------|---------------------------------------------------------------------|
| `input.jpg`          | Imagen de entrada (debe estar en la carpeta `assets/`)             |
| `output.jpg`         | Imagen de salida (se guarda en la carpeta `out/`)                  |
| `-angulo <ángulo>`   | Ángulo de rotación en grados (por ejemplo: 90, 45.5)               |
| `-escalar <escala>`  | Factor de escala (por ejemplo: 0.5, 1.0, 2.0)                       |
| `-buddy`             | *(Opcional)* Usar el sistema de memoria personalizado Buddy System |

---

## Ejemplos

# Rotar una imagen 90 grados y escalarla al 50% usando el sistema convencional
```bash
./bin/program_image assets/imagen.jpg out/salida.jpg -angulo 90 -escalar 0.5
```

# Lo mismo pero usando el Buddy System
```bash
./bin/program_image assets/imagen.jpg out/salida.jpg -angulo 90 -escalar 0.5 -buddy
```
---

## Detalles de salida

Después de cada ejecución, el programa imprimirá:

- Dimensiones de la imagen (antes y después)
- Tiempo de procesamiento
- Memoria usada (con y sin Buddy System)
- Confirmation de la imagen guardada

---
## Rendimiento y Paralelismo

Gracias a OpenMP, el programa ahora puede aprovechar múltiples núcleos del procesador.
Esto reduce significativamente el tiempo de ejecución en operaciones pesadas como rotación y escalado.

---

## Autores

- Ana Sofía Rodriguez
- Mariana Gutierrez Jaramillo
- Esteban Giraldo Llano

---

## Licencia

Este proyecto es para propósitos educativos. Siéntete libre de modificarlo o extenderlo.


