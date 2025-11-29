# Simulador Hidrodinámico interactivo
-------------

## Proyecto Final curso de Física Computacional para Data Science

Para el proyecto final se desarrolló un solver de hidrodinámica interactivo con render en tiempo real para observar el movimiento del fluido a través de un objeto diseñado en tiempo real. la 

**Estructura del proyecto:**

hydro-sim/

├── src/

│   ├── main.c           
│   ├── solver.c         
│   └── renderer.c       
├── include/     

├── platforms/

│   └── web/

│       └── shell.html   
├── CMakeLists.txt       
└── .gitignore

-----------------------------
## Solver

Como solver se utilizó el método de Lattice Boltzmann (Modelo D2Q9):

En lugar de resolver ecuaciones diferenciales complejas para la presión y la velocidad en cada punto, LBM simula "paquetes" de partículas microscópicas en una grilla que chocan y se propagan.

Para 2D, se usa el modelo D2Q9 (2 Dimensiones, 9 Velocidades). Cada celda de la grilla no tiene un solo vector velocidad, sino 9 densidades que representan partículas moviéndose en 8 direcciones + 1 quieta.

Si una partícula intenta moverse a una celda marcada como solid, simplemente invierte su dirección y vuelve a donde estaba.

Resultado: El fluido "choca" y rodea el obstáculo automáticamente, generando turbulencia y vórtices (calle de vórtices de Von Kármán).

El solver está definido en el archivo solver.c y maneja únicamente lógica del solver en C.

**Colisiones**

Para determinar las colusiones se chequea que la celda sea tipo *solid* reflejando la dirección del movimiento. Las partículas que llegaron a una celda chocan entre sí y tienden al equilibrio y se aplica una función de relajación simple para simular el equilibrio de los choques.

En el nivel macroscópico (rendedrizado) se muestra sólamente la suma de los vectores de velocidad como la velocidad final, aunque a nivel de cálculos se usa cada vector de manera individual.

--------------

## Renderizado

Para el renderizado se utilizó la librería Raylib, usada generalmente para el desarrollo de videojuegos, ya que permite generar un renderizado interactivo de manera sencilla usando funciones predefinidas para el movimiento del mouse, colores, pixeles entre otros.

EL renderizado ocurre en el archivo renderer.c, y consta únicamente con lógica de Raylib.

----------
## Main

En main se une la lógica de Raylib con el solver, donde básicamente a través de una conexión mediante un Struct *SimulationState* que almacena los valores del solver y los entrega al renderer se logra obtener el renderizado de las soluciones del modelo.

----------

# Build local

Para generar el build local se usa el archivo CMakeLists.txt, que tiene las instrucciones en CMake para la generación del ejecutable completo. 