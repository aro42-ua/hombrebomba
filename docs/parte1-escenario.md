# Memoria — Parte 1: Escenario

**Autor:** Ángel Rubio (Paquete 1 — Escenario y repositorio)
**Hito:** 1 — Alpha

## 1. Objetivo

La Parte 1 se encarga del escenario del juego: la estructura del tablero, los muros indestructibles, los bloques destructibles, las zonas de aparición libres, su representación en pantalla y el sistema de colisiones con el escenario. Es la base sobre la que trabajan el resto de paquetes, por lo que he intentado dejar una interfaz pequeña y estable (`Tablero.hpp`) que el jugador (P2), las bombas (P3) y los enemigos (P4) puedan usar sin conocer los detalles internos.

## 2. Integración con la plantilla de la asignatura

El proyecto parte de la [plantilla de la asignatura](https://github.com/antoniorv6/DCAGII-Game-Template), creada con la opción *Use this template* de GitHub, y respeta su organización:

- Todo el código está en `src/`, con las cabeceras (`.hpp`) junto a los fuentes (`.cpp`), y se incluye con `#include <Fichero.hpp>` gracias a la opción `-I src/`.
- raylib está en `vendor/`: las cabeceras (`raylib.h`, `raymath.h`, `rlgl.h`) en `vendor/include/` y la librería estática `libraylib.a` en `vendor/lib/`. Se incluye con `extern "C" { #include <raylib.h> }`, como indica la Práctica 0.
- La plantilla gestiona las pantallas con una máquina de estados (`StateMachine`) en la que cada pantalla hereda de `GameState`. El escenario no es una pantalla, sino una clase (`Tablero`) que la pantalla de juego (`MainGameState`) contiene y utiliza.

Sobre el código de la plantilla solo he hecho los cambios imprescindibles:

- En `main.cpp`: crear la ventana con `InitWindow` antes del bucle (con el tamaño derivado del tablero), calcular `delta_time` con `GetFrameTime`, salir también al cerrar la ventana (`WindowShouldClose`), cerrarla al final y leer una semilla opcional de la línea de comandos.
- En `MainGameState`: el constructor recibe la semilla; `init()` genera el tablero, de modo que cada vez que el estado entra en la máquina de estados (una partida nueva) el tablero empieza limpio; `render()` lo dibuja.

Mientras no existan el jugador ni las bombas, `MainGameState` hace además de banco de pruebas del escenario (ver apartado 5).

## 3. Diseño del tablero

### 3.1 Dimensiones y constantes

El tablero mide 15 × 13 celdas (13 × 11 jugables más un borde de muros), con celdas de 48 píxeles y una franja superior de 48 píxeles reservada para el marcador. Todos estos valores se definen en `Constantes.hpp` y el tamaño de la ventana se calcula a partir de ellos, de modo que cambiar el tamaño del tablero no obliga a tocar el resto del código.

### 3.2 Representación de los datos

Cada celda solo puede ser de tres tipos: Vacío, Muro o Bloque. He decidido que el tablero guarde únicamente lo estructural: las bombas, las llamas, el jugador y los enemigos se gestionan en sus propias clases. Así se evita que varios paquetes modifiquen la misma estructura de datos y se reducen los conflictos al integrar.

Las celdas se almacenan en un vector de `TipoCelda` de una sola dimensión, recorrido fila a fila. La posición de una celda (col, fila) dentro del vector es `fila * COLUMNAS + col`.

### 3.3 Sistemas de coordenadas

La lógica del juego trabaja en celdas, mientras que raylib dibuja en píxeles. Para no mezclar ambos sistemas se ofrecen tres funciones de conversión: `celdaAPixel`, `centroDeCelda` y `pixelACelda`. Esta última usa `std::floor` en lugar de una conversión directa a `int`, ya que `(int)(-0.5f)` vale 0 y un punto ligeramente a la izquierda del tablero se interpretaría como la columna 0 en vez de quedar fuera.

## 4. Funciones implementadas

### 4.1 dentroDeLimites

Comprueba que la columna esté entre 0 y `COLUMNAS - 1` y la fila entre 0 y `FILAS - 1`. Es la función que usa `getCelda` para decidir si puede acceder al vector.

### 4.2 getCelda y el criterio «fuera del tablero hay muro»

Si se consulta una celda fuera de los límites, `getCelda` devuelve Muro. Esta decisión simplifica mucho el resto del código: ninguna función necesita comprobar límites por su cuenta, nadie puede leer fuera del vector y, a efectos del juego, ningún personaje ni explosión puede salir del tablero.

### 4.3 esTransitable

Una celda es transitable únicamente si está vacía. Al apoyarse en `getCelda`, las celdas exteriores al tablero se consideran automáticamente no transitables.

### 4.4 destruirBloque

Si la celda contiene un bloque, lo convierte en vacío y devuelve `true`; en cualquier otro caso (muro, vacío o fuera del tablero) no modifica nada y devuelve `false`. El valor de retorno está pensado para la Parte 3, que lo necesita al propagar una explosión: cuando la llama destruye un bloque, debe detenerse en esa dirección.

No hace falta comprobar los límites antes de llamar a `setCelda`, porque si `getCelda` ha devuelto Bloque la celda está necesariamente dentro del tablero (fuera siempre devuelve Muro).

### 4.5 esZonaLibre

Define las zonas de aparición que deben quedar sin bloques. Son las cuatro esquinas jugables —(1,1), (13,1), (1,11) y (13,11)— y las celdas adyacentes a cada una. Para detectar la adyacencia se utiliza la distancia Manhattan (|Δcol| + |Δfila| ≤ 1). En la práctica, como los pilares ocupan las posiciones diagonales, cada zona libre queda con forma de «L» de tres celdas, que es el hueco mínimo para que un personaje pueda poner una bomba y apartarse.

El jugador aparece en (1,1) y las otras tres esquinas se reservan para los enemigos. Esta asignación puede cambiar según lo que decida el Paquete 4.

### 4.6 generar

Construye el tablero de una partida nueva a partir de una semilla. Primero guarda la semilla, la pasa a raylib con `SetRandomSeed` y deja todo el vector a Vacío, para que un reinicio de partida no arrastre bloques destruidos de la partida anterior. Después recorre todas las celdas y decide su tipo en este orden:

1. Borde (primera o última fila o columna) → Muro.
2. Pilar (columna par y fila par) → Muro.
3. Zona libre → se queda Vacío.
4. Resto → Bloque con una probabilidad de `PROB_BLOQUE` (60 %), Vacío en caso contrario.

El orden es importante: si la zona libre se comprobara antes que el borde, la celda (0,1) —que es vecina de (1,1)— quedaría vacía y el jugador podría salir del tablero por ahí.

Otro detalle relevante es que en las celdas de borde, pilar y zona libre no se consume ningún número aleatorio. De esta forma, la misma semilla genera siempre exactamente el mismo tablero. Esto resulta muy útil para el bugtracking: la semilla se muestra en el modo de depuración (F1) y basta con indicarla en una incidencia para que cualquier miembro del equipo reproduzca el mismo tablero con `bin/bomberman <semilla>`.

### 4.7 colisiona

Indica si una caja de colisión (en píxeles) se solapa con alguna celda no transitable. Como la caja del jugador es más pequeña que una celda (36 px frente a 48 px), solo puede tocar las celdas en las que caen sus cuatro esquinas, por lo que basta con convertir esas cuatro esquinas a celdas y comprobar si alguna no es transitable.

Para las esquinas derecha e inferior se resta un pequeño margen (0.01f), ya que el punto `x + width` pertenece a la celda siguiente y no a la caja. Sin este ajuste, un personaje perfectamente alineado junto a un muro se detectaría como colisionando con él y no podría moverse por los pasillos.

## 5. Pruebas

### 5.1 Banco de pruebas interactivo

Mientras no existan el jugador ni las bombas, la pantalla de juego permite probar el escenario a mano:

| Tecla | Acción |
|---|---|
| F1 | Modo depuración: rejilla, coordenadas de cada celda, semilla y caja de colisión |
| R | Genera un tablero nuevo (siguiente semilla) |
| Ratón | Mueve una caja del tamaño del jugador, que se pone roja al colisionar |
| Clic izquierdo | Destruye el bloque que hay debajo (simula una explosión) |

### 5.2 Pruebas automáticas

El programa `tests/test_tablero.cpp` se compila aparte del juego (el comando está en el README) y comprueba automáticamente 11 casos:

- Para 2000 semillas distintas: el borde y los pilares son siempre muros, no aparece ningún muro fuera de ellos y las tres celdas de cada zona de aparición son transitables.
- La generación es fija: dos tableros con la misma semilla son idénticos, incluso después de destruir bloques y volver a generar, y dos semillas distintas dan tableros distintos.
- `dentroDeLimites`, `getCelda` y `esTransitable` se comportan correctamente fuera del tablero.
- `destruirBloque` solo actúa sobre bloques y devuelve `false` con muros, celdas vacías y posiciones exteriores.
- Las conversiones celda ↔ píxel son coherentes entre sí, y `pixelACelda` devuelve −1 justo fuera del tablero.
- `colisiona` no da falsos positivos con una caja pegada exactamente al borde de una celda (tanto por la izquierda como por la derecha), pero sí detecta una caja que se mete un solo píxel en un muro o un bloque.

Todas las comprobaciones se superan. Como referencia, este es el tablero generado con la semilla 42 («#» muro, «B» bloque, «.» vacío):

```
###############
#..BBBBBB.....#
#.#B#B#B#.#B#.#
#B..B.BB.B.BB.#
#B#B#B#B#.#.#B#
#BB.B..B.BBB..#
#.#B#.#B#B#B#B#
#BB..BB.BB.BB.#
#B#B#.#B#B#B#B#
#B.BBB.BBBBBBB#
#.#.#B#B#.#.#.#
#..BBBBBBB....#
###############
```

## 6. Interfaz para el resto de paquetes

| Paquete | Funciones que usa |
|---|---|
| P2 — Jugador | `colisiona`, `esTransitable`, `centroDeCelda`, `pixelACelda` |
| P3 — Bombas | `getCelda`, `destruirBloque`, `centroDeCelda` |
| P4 — Enemigos y flujo | `generar` (al empezar y reiniciar), `dibujar`, `esTransitable` |

Para empezar una partida nueva desde otra pantalla (por ejemplo, desde el *game over*) basta con `state_machine->add_state(std::make_unique<MainGameState>(), true);`: sin argumento, el constructor usa una semilla nueva basada en la hora.

## 7. Librerías utilizadas

Únicamente raylib 6.0, que es la base técnica indicada en la Práctica 0. No se ha añadido ninguna librería adicional.
