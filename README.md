# hashing-lineal
Implementación del hashing lineal

## Sobre la tarea
El siguiente archivo corresponde a la implementación del hashing lineal en el contexto de la tarea 1 del curso Diseño y análisis de algoritmos CC4102 de la sección 2 de la Universidad de Chile. Esta tarea consiste en implementar las estructuras necesarias para la creación de una tabla hash que inserta elementos utilizando el hashing lineal. En base a esto se debe experimentar para poder sacar conclusiones y refutar o demostrar una hipótesis propuesta.

## Uso
Para poder ejecutar el código, se debe contar con el lenguaje de programación c en el sistema operativo. De ser así, se debe compilar el archivo y luego ejecutar usando los siguientes comandos en la terminal (solo para usuarios de Linux, macOS o wsl):

`gcc -o T1 T1.c`

`./T1 c_max n`

Para usuarios de windows que no cuenten con wsl en su sistema operativo, los comandos son los siguientes:

`gcc T1.c -o T1.exe`

`T1.exe c_max n`

**Para ejecutar el programa se requiere de dos argumentos númericos: c_max que corresponde al costo promedio máximo y n que cumple que |N|=$2^{n}$**
