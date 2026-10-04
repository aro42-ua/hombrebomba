#include <MainGameState.hpp>
#include <iostream>

extern "C" {
    #include <raylib.h>
}

// ---------------------------------------------------------------------------
// Banco de pruebas del escenario:
//   F1          mostrar/ocultar depuración
//   R           reiniciar tablero
//   Ratón       mueve una caja del tamaño del jugador
//   Clic izq.   destruye el bloque bajo el ratón
// ---------------------------------------------------------------------------

MainGameState::MainGameState(unsigned int semilla)
    : semilla(semilla),
      jugador1(
          Tablero::centroDeCelda(1, 1),
          120.0f,
          KEY_W,
          KEY_S,
          KEY_A,
          KEY_D,
          BLUE
      ),
      jugador2(
          Tablero::centroDeCelda(COLUMNAS - 2, 1),
          120.0f,
          KEY_UP,
          KEY_DOWN,
          KEY_LEFT,
          KEY_RIGHT,
          RED
      )
{
}

void MainGameState::init()
{
    // Se genera un tablero limpio al comenzar una partida.
    tablero.generar(semilla);

    // Posiciones iniciales de los dos jugadores.
    jugador1.init(Tablero::centroDeCelda(1, 1));
    jugador2.init(Tablero::centroDeCelda(COLUMNAS - 2, 1));

    bombas.clear();
}

void MainGameState::handleInput()
{
    // Activar/desactivar información de depuración.
    if (IsKeyPressed(KEY_F1)) {
        depuracion = !depuracion;
    }

    // Reiniciar el tablero utilizando una nueva semilla.
    if (IsKeyPressed(KEY_R)) {
        tablero.generar(++semilla);

        jugador1.init(Tablero::centroDeCelda(1, 1));
        jugador2.init(Tablero::centroDeCelda(COLUMNAS - 2, 1));

        bombas.clear();
    }

    // De momento se mantiene el sistema de bombas existente.
    // La bomba sigue perteneciendo provisionalmente al jugador 1.
    bool hayBombaEnCuentaAtras = false;

    for (const Bomba& bomba : bombas) {
        if (bomba.estaEnCuentaAtras()) {
            hayBombaEnCuentaAtras = true;
            break;
        }
    }

    if (IsKeyPressed(KEY_SPACE) && !hayBombaEnCuentaAtras) {
        bombas.emplace_back(
            Tablero::pixelACelda(jugador1.getPosicion())
        );
    }

    // Herramienta provisional de depuración.
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Celda c = Tablero::pixelACelda(GetMousePosition());
        tablero.destruirBloque(c.col, c.fila);
    }

    jugador1.handleInput();
    jugador2.handleInput();
}

void MainGameState::update(float deltaTime)
{
    // Cada jugador procesa sus propios controles.
    jugador1.update(deltaTime, tablero);
    jugador2.update(deltaTime, tablero);

    // Actualización de las bombas.
    for (auto bomba = bombas.begin(); bomba != bombas.end();) {
        if (bomba->actualizar(deltaTime, tablero)) {
            bomba = bombas.erase(bomba);
        } else {
            ++bomba;
        }
    }

    // Banco de pruebas del escenario.
    Vector2 raton = GetMousePosition();

    cajaPrueba = {
        raton.x - TAM_JUGADOR / 2.0f,
        raton.y - TAM_JUGADOR / 2.0f,
        (float)TAM_JUGADOR,
        (float)TAM_JUGADOR
    };

    cajaChoca = tablero.colisiona(cajaPrueba);
}

void MainGameState::render()
{
    BeginDrawing();

    ClearBackground({ 24, 33, 27, 255 });

    tablero.dibujar();

    // Dibujamos ambos jugadores.
    jugador1.render();
    jugador2.render();

    // Dibujamos bombas y explosiones.
    for (const Bomba& bomba : bombas) {
        bomba.dibujar();
    }

    if (depuracion) {
        tablero.dibujarDepuracion();

        DrawRectangleLinesEx(
            cajaPrueba,
            2,
            cajaChoca ? RED : GREEN
        );

        DrawRectangleLinesEx(
            jugador1.getCajaColision(),
            2,
            RED
        );

        DrawRectangleLinesEx(
            jugador2.getCajaColision(),
            2,
            RED
        );
    }

    EndDrawing();
}