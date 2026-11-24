# Bomberman DCA

Réplica de Bomberman en C++ con [raylib](https://www.raylib.com/), hecha para la asignatura Desarrollo Colaborativo de Aplicaciones del Grado en Ingeniería Informática (Universidad de Alicante).

El proyecto parte de la [plantilla oficial de la asignatura](https://github.com/antoniorv6/DCAGII-Game-Template) ([documentación](https://deepwiki.com/antoniorv6/DCAGII-Game-Template)): una máquina de estados (`StateMachine`) en la que cada pantalla del juego es un estado que hereda de `GameState`.

Ahora mismo estamos en el Hito 1 (Alpha), que se entrega el 28/10/2026.

## Estructura

```
assets/            recursos gráficos y de sonido
src/
  entities/        Jugador, Enemigo, Bomba y PowerUp
  game/            estados, constantes y resultados de partida
  ai/              IAEnemigo
  board/           Tablero
  core/            StateMachine
  main.cpp         punto de entrada
tests/             pruebas automáticas (se compilan aparte del juego)
vendor/include/    cabeceras de raylib (raylib.h, raymath.h, rlgl.h)
vendor/lib/        libraylib.a (no se sube: cada uno la compila, ver abajo)
bin/               ejecutable compilado (no se sube)
build/             objetos intermedios de make (no se sube)
Makefile           compilación del juego y las pruebas
```

## Preparar raylib (solo la primera vez)

Usamos **raylib 6.0**. Las cabeceras ya están en `vendor/include/`; lo único que hay que generar es la librería estática, que depende del sistema operativo. Se hace igual que en la Práctica 0, pero fijando la versión:

```bash
git clone --depth 1 --branch 6.0 https://github.com/raysan5/raylib.git
cd raylib/src
make PLATFORM=PLATFORM_DESKTOP
```

Copia `libraylib.a` a `vendor/lib/` de este repositorio. La carpeta `raylib/` clonada ya no hace falta.

- **Linux (Ubuntu 24.04, como en los laboratorios):** si el enlazado falla, instala las dependencias con `sudo apt install xorg-dev libx11-dev libxrandr-dev libxi-dev libgl1-mesa-dev libglu1-mesa-dev`.
- **Windows:** con [w64devkit](https://github.com/skeeto/w64devkit) o MinGW-w64, el mismo `make` funciona desde su terminal (en MinGW puede llamarse `mingw32-make`).

## Compilar y ejecutar

Se compila con el `Makefile` de la raíz (en Windows, `mingw32-make` si `make` no existe). Detecta recursivamente los `.cpp` de `src/` y añade las carpetas de cabeceras al include path, así que no hay que cambiarlo al añadir ficheros dentro de estas carpetas; solo recompila lo que ha cambiado, incluidas las cabeceras.

```bash
make            # compila el juego en bin/bomberman (objetivo por defecto, = make all)
make -j4        # igual, en paralelo
make clean      # borra build/ y los ejecutables
./bin/bomberman
```

Es un único Makefile para todas las carpetas (estrategia *monolítica*): `make` ve el grafo de dependencias completo, algo que se pierde con un Makefile por carpeta (*make* recursivo), y el proyecto es lo bastante pequeño como para no necesitarlo. Opciones útiles: `make CXXFLAGS="-std=c++17 -O2"` para optimizar y `make CXX="ccache g++"` para cachear la compilación.

El Makefile necesita una shell POSIX (usa `mkdir` y `rm`): en Windows, lánzalo desde Git Bash o w64devkit. Desde PowerShell funciona si antes añades el `sh` de Git al PATH: `$env:PATH = "C:\Program Files\Git\usr\bin;" + $env:PATH`.

## Controles

| | Jugador 1 | Jugador 2 |
|---|---|---|
| Moverse | W A S D | flechas |
| Poner bomba | espacio | Enter |

F1 activa el modo depuración (rejilla, semilla, power-ups tapados, celdas peligrosas y lo que decide cada enemigo) y R empieza otra partida con la semilla siguiente.

Al romper un bloque puede aparecer un power-up: B da una bomba más a la vez, F alarga la llama una celda y V sube la velocidad. Los detalles están en `docs/powerups.md`.

En cada partida hay tres enemigos. Los globos (círculos) pasean al azar y el perseguidor (rombo) huye de las bombas y va a por el jugador que tenga cerca. Tocar a un enemigo elimina al jugador, y una llama elimina al enemigo. Cómo funciona su IA está en `docs/ia-enemigos.md`.

## Pruebas

Las pruebas automáticas están en `tests/` y se compilan aparte del juego (no abren ventana). Cada `tests/*.cpp` es un ejecutable en `bin/`, y `make test` los compila y ejecuta todos (se detiene en el primero que falla):

```bash
make test
```

A mano, por ejemplo las del escenario en Linux:

```bash
g++ -o bin/test_tablero tests/test_tablero.cpp src/board/Tablero.cpp -I src/board/ -I src/game/ -I vendor/include/ -L vendor/lib -lraylib -lGL -lm -lpthread -lrt -lX11
./bin/test_tablero
```

Cada programa necesita sus propios ficheros de `src/`; el resto del comando es igual:

| Programa | Ficheros de `src/` |
|---|---|
| `tests/test_tablero.cpp` | `board/Tablero.cpp` |
| `tests/test_jugador.cpp` | `entities/Jugador.cpp board/Tablero.cpp` |
| `tests/test_bomba.cpp` | `entities/Bomba.cpp board/Tablero.cpp` |
| `tests/test_resultado_partida.cpp` | `game/ResultadoPartida.cpp` |
| `tests/test_powerups.cpp` | `entities/PowerUp.cpp entities/Jugador.cpp entities/Bomba.cpp board/Tablero.cpp` |
| `tests/test_ia_enemigos.cpp` | `entities/Enemigo.cpp ai/IAEnemigo.cpp board/Tablero.cpp` |

En Windows se cambian las librerías del final igual que al compilar el juego. Cada programa devuelve 0 si todo pasa.

## Cómo trabajamos

Seguimos **GitHub Flow**. `main` está protegida, tiene que compilar siempre y solo se cambia mediante pull request. No asignamos revisores: antes de hacer merge, el autor de la PR repasa la lista de comprobaciones de la plantilla de PR. Las PR se integran con *merge commit*.

Cualquier cambio empieza con una incidencia en Issues, y cada incidencia lleva su propia rama y su propia PR. Las ramas se llaman:

| Prefijo | Para qué |
|---|---|
| `feature/<nº>-<descripcion>` | funcionalidad nueva del juego |
| `fix/<nº>-<descripcion>` | corrección de un fallo |
| `docs/<nº>-<descripcion>` | solo documentación |
| `chore/<nº>-<descripcion>` | configuración del proyecto (estructura, dependencias) |

En los commits se cita la incidencia, por ejemplo `Añade generación de pilares (#4)`, y en la descripción de la PR se pone `Closes #<nº>` para que se cierre sola al hacer merge.

```bash
git switch main && git pull
git switch -c feature/4-tablero-base
# ...commits pequeños...
git push -u origin feature/4-tablero-base
# Abrir la PR en GitHub
```

## Equipo

- Ángel Rubio: Paquete 1, escenario y repositorio
- Samuel: Paquete 2, jugador y documento de la entrega
- Antonio: Paquete 3, bombas, explosiones y bugtracking
- Abel: Paquete 4, enemigos, flujo de partida y releases

## Licencia

MIT, heredada de la plantilla de la asignatura (ver `LICENSE`).
