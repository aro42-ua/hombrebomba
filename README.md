# Bomberman DCA

Réplica de Bomberman en C++ con [raylib](https://www.raylib.com/), hecha para la asignatura Desarrollo Colaborativo de Aplicaciones del Grado en Ingeniería Informática (Universidad de Alicante).

El proyecto parte de la [plantilla oficial de la asignatura](https://github.com/antoniorv6/DCAGII-Game-Template) ([documentación](https://deepwiki.com/antoniorv6/DCAGII-Game-Template)): una máquina de estados (`StateMachine`) en la que cada pantalla del juego es un estado que hereda de `GameState`.

Ahora mismo estamos en el Hito 1 (Alpha), que se entrega el 28/10/2026.

## Estructura

```
assets/            recursos gráficos y de sonido
src/               código fuente: cabeceras (.hpp) y fuentes (.cpp) juntas
tests/             pruebas automáticas (se compilan aparte del juego)
vendor/include/    cabeceras de raylib (raylib.h, raymath.h, rlgl.h)
vendor/lib/        libraylib.a (no se sube: cada uno la compila, ver abajo)
bin/               ejecutable compilado (no se sube)
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

No usamos Makefiles (lo prohíbe el enunciado del Hito 1). Todo se compila con un único comando desde la raíz del repo. Si no existe la carpeta `bin/`, créala antes.

Linux:

```bash
g++ -o bin/bomberman src/*.cpp -I src/ -I vendor/include/ -L vendor/lib -lraylib -lGL -lm -lpthread -lrt -lX11
./bin/bomberman
```

Windows:

```bash
g++ -o bin/bomberman.exe src/*.cpp -I src/ -I vendor/include/ -L vendor/lib -lraylib -lopengl32 -lgdi32 -lwinmm
bin/bomberman.exe
```

## Pruebas

Las pruebas automáticas están en `tests/` y se compilan aparte del juego (no abren ventana). Por ejemplo, las del escenario en Linux:

```bash
g++ -o bin/test_tablero tests/test_tablero.cpp src/Tablero.cpp -I src/ -I vendor/include/ -L vendor/lib -lraylib -lGL -lm -lpthread -lrt -lX11
./bin/test_tablero
```

En Windows se cambian las librerías del final igual que al compilar el juego. El programa devuelve 0 si todo pasa.

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
