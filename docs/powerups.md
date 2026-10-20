# Memoria: power-ups

Autor: Ángel Rubio
Hito: 1 (Alpha)
Incidencia: #9

## 1. Qué pide la asignatura

Ni el enunciado del proyecto ni el del Hito 1 hablan de power-ups. Para este hito se pide la mecánica principal con un bucle jugable, gráficos hechos con figuras básicas (los sprites no son obligatorios) y, para el proyecto entero, una batería de al menos diez pruebas automáticas. Los power-ups son, por tanto, una decisión del grupo. Los hemos incluido porque forman parte del Bomberman original y dan motivos para romper bloques más allá de abrirse camino.

Del enunciado sí salen dos condiciones que he respetado: dibujarlos con figuras simples y no añadir librerías nuevas (solo se usa raylib y la librería estándar de C++).

## 2. Cómo funcionan en el juego

Al empezar cada partida se esconden 9 power-ups debajo de bloques elegidos al azar, 3 de cada tipo:

| Letra | Color | Efecto | Tope |
|---|---|---|---|
| B | gris oscuro | una bomba más a la vez | 5 bombas |
| F | rojo | la llama llega una celda más lejos | 5 celdas |
| V | azul | el jugador anda 20 px/s más rápido | 200 px/s |

Mientras el bloque siga en pie, el power-up no se ve. Cuando una explosión lo rompe, aparece en el suelo y se lo lleva el primer jugador que pisa la celda. Si una explosión posterior lo alcanza antes de que alguien lo coja, se pierde. Al pasar el tope, recoger otro del mismo tipo no hace nada.

Las mejoras duran lo que dura la partida: al reiniciar (tecla R o desde la pantalla de fin) cada jugador vuelve a tener una bomba, alcance 1 y 120 px/s.

En el marcador, debajo del estado de cada jugador, aparece una línea como `B2 F1 V140` con sus mejoras actuales. Con F1 se ven también los power-ups que siguen tapados, como un recuadro del color de su tipo con la letra en la esquina.

## 3. Diseño

### 3.1 Reparto del código

- `PowerUp.hpp` y `PowerUp.cpp` contienen `GestorPowerUps`, que reparte, destapa, entrega y dibuja los power-ups. No sabe nada de bombas ni de jugadores: recibe el tablero y las celdas en llamas, y responde a la pregunta "¿hay algo en esta celda?". Es el mismo criterio que seguí con el `Tablero`, que solo guarda lo estructural.
- `Jugador` guarda cuántas bombas puede tener a la vez, su alcance y su velocidad, y tiene un método `aplicarPowerUp`.
- `Bomba` recibe el alcance en el constructor.
- `MainGameState` une las piezas en cada fotograma.

### 3.2 Reparto reproducible con la semilla

El tablero se genera con `SetRandomSeed` y `GetRandomValue` de raylib. Para los power-ups he usado `std::mt19937`, de la librería estándar, sembrado con la misma semilla de la partida. Así el reparto no depende del generador interno de raylib, y `mt19937` está definido por el estándar de C++, de modo que da los mismos números con g++ en Linux y con MinGW en Windows.

Para barajar los bloques he escrito el algoritmo de Fisher-Yates a mano en lugar de usar `std::shuffle`, porque el estándar no fija cómo baraja `std::shuffle` y otra implementación podría dar otro orden.

El resultado es que la semilla que ya se apunta en las incidencias reproduce también los power-ups: `bin/bomberman 42` siempre los pone en las mismas celdas.

### 3.3 Aparecer al romperse el bloque

El gestor no se entera de qué bomba rompe cada bloque. En cada fotograma mira si la celda de un power-up tapado ya es transitable y, si lo es, lo destapa. Tiene dos ventajas. No hace falta tocar la propagación de la explosión, que es código del Paquete 3, y funciona con cualquier forma de romper un bloque, incluido el clic del banco de pruebas.

### 3.4 La explosión que lo destapa no lo destruye

Hay un caso delicado: la misma llama que rompe el bloque sigue encendida en esa celda durante `DURACION_EXPLOSION` (0,6 s). Si el power-up se destruyera con cualquier llama, desaparecería en el mismo instante en que aparece.

Para evitarlo, cada power-up cuenta el tiempo que lleva a la vista y solo lo destruyen las llamas cuando ese tiempo supera `DURACION_EXPLOSION` más un margen de 0,05 s. El contador empieza en el fotograma en que explota la bomba, igual que el de la llama, así que cuando la llama que lo destapó se apaga el power-up ya está a punto de ser vulnerable. El margen cubre la diferencia de un fotograma que puede haber entre los dos contadores.

### 3.5 Varias bombas por jugador

Antes cada jugador solo podía tener una bomba en cuenta atrás. Ahora `MainGameState` cuenta las suyas y lo deja poner otra si no ha llegado a su máximo. Como con varias bombas se podría pulsar dos veces sin moverse, también comprueba que en esa celda no haya ya una bomba.

El código de poner bomba estaba repetido para los dos jugadores. Lo he juntado en `intentarPonerBomba(jugador, bombas)`. También he sacado a `reiniciarPartida()` lo que hacían igual `init()` y la tecla R, para que ninguno de los dos se olvide de repartir los power-ups.

## 4. Cambios en código de otros paquetes

He intentado que sean pequeños y compatibles con lo que ya había:

- `Bomba` (Paquete 3): el constructor recibe un alcance con `ALCANCE_EXPLOSION` como valor por defecto, y hay un `getAlcance()`. Las llamadas que ya existían siguen compilando igual.
- `Jugador` (Paquete 2): tres atributos nuevos, `aplicarPowerUp` y tres consultas. `init()` devuelve las mejoras a sus valores iniciales. La velocidad por defecto pasa a ser la constante `VELOCIDAD_JUGADOR` (120, el mismo valor que antes).
- Marcador: el texto `J1: VIVO` y `J2: VIVO` sube de y = 14 a y = 6 para que quepa la línea de mejoras debajo.

Las constantes nuevas están en `Constantes.hpp`, en la sección "Power-ups".

## 5. Pruebas

`tests/test_powerups.cpp` hace 18 comprobaciones sin abrir ventana:

- El reparto pone 9 power-ups tapados bajo bloques, sin repetir celda, 3 de cada tipo, y no pasa del número de bloques que hay.
- Dos repartos con la misma semilla son idénticos.
- Un power-up tapado no se ve ni se puede recoger; al romper su bloque aparece, y al recogerlo se devuelve su tipo y desaparece.
- La explosión que rompe el bloque no lo destruye y una explosión posterior sí.
- Cada tipo mejora al jugador hasta su tope, e `init()` quita las mejoras.
- Una bomba con alcance 2 rompe un bloque que una de alcance 1 no alcanza.

Se compila con:

```bash
g++ -o bin/test_powerups tests/test_powerups.cpp src/PowerUp.cpp src/Jugador.cpp src/Bomba.cpp src/Tablero.cpp -I src/ -I vendor/include/ -L vendor/lib -lraylib -lGL -lm -lpthread -lrt -lX11
./bin/test_powerups
```

Pasan las 18. También lo he probado jugando con la semilla 42: poniendo una bomba en (2,1) se rompe el bloque de (3,1), aparece una B y al pisarla el marcador pasa a `B2`.

## 6. Problemas encontrados

- `tests/test_bomba.cpp` falla en `main` desde antes de estos cambios. Espera que una bomba en (1,1) alcance el bloque de (3,1), cosa que solo pasa con alcance 2, pero `ALCANCE_EXPLOSION` vale 1. Hay que decidir si la prueba o la constante están mal; no lo he tocado porque es del Paquete 3.
- Con F1 activado, el texto de la semilla se dibuja encima del de `J1`. Es anterior a estos cambios y solo afecta al modo depuración.
