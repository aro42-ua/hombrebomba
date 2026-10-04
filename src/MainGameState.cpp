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

    bombasJugador1.clear();
    bombasJugador2.clear();

    partidaTerminada = false;
    ganador = 0;
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

        bombasJugador1.clear();
        bombasJugador2.clear();
    }

    // --- Bomba del jugador 1 -------------------------------------------------
    bool bombaJugador1Activa = false;

    for (const Bomba& bomba : bombasJugador1) {
        if (bomba.estaEnCuentaAtras()) {
            bombaJugador1Activa = true;
            break;
        }
    }

    if (jugador1.estaVivo() && IsKeyPressed(KEY_SPACE) && !bombaJugador1Activa) {
        bombasJugador1.emplace_back(
            Tablero::pixelACelda(jugador1.getPosicion())
        );
    }


    // --- Bomba del jugador 2 -------------------------------------------------
    bool bombaJugador2Activa = false;

    for (const Bomba& bomba : bombasJugador2) {
        if (bomba.estaEnCuentaAtras()) {
            bombaJugador2Activa = true;
            break;
        }
    }

    if (jugador2.estaVivo() && IsKeyPressed(KEY_ENTER) && !bombaJugador2Activa) {
        bombasJugador2.emplace_back(
            Tablero::pixelACelda(jugador2.getPosicion())
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
    for (auto bomba = bombasJugador1.begin(); bomba != bombasJugador1.end();) {
        if (bomba->actualizar(deltaTime, tablero)) {
            bomba = bombasJugador1.erase(bomba);
        } else {
            ++bomba;
        }
    }

    for (auto bomba = bombasJugador2.begin(); bomba != bombasJugador2.end();) {
        if (bomba->actualizar(deltaTime, tablero)) {
            bomba = bombasJugador2.erase(bomba);
        } else {
            ++bomba;
        }
    }

    // Comprobar si una explosión alcanza a un jugador.
    auto comprobarExplosion = [&](const std::vector<Bomba>& bombas) {
        for (const Bomba& bomba : bombas) {

            // Si sigue en cuenta atrás todavía no hace daño.
            if (bomba.estaEnCuentaAtras()) {
                continue;
            }

            Celda celdaJugador1 =
                Tablero::pixelACelda(jugador1.getPosicion());

            Celda celdaJugador2 =
                Tablero::pixelACelda(jugador2.getPosicion());

            for (const Celda& celdaExplosion : bomba.getCeldasExplosion()) {

                if (jugador1.estaVivo() &&
                    celdaJugador1.col == celdaExplosion.col &&
                    celdaJugador1.fila == celdaExplosion.fila) {

                    jugador1.eliminar();
                }

                if (jugador2.estaVivo() &&
                    celdaJugador2.col == celdaExplosion.col &&
                    celdaJugador2.fila == celdaExplosion.fila) {

                    jugador2.eliminar();
                }
            }
        }
    };

    comprobarExplosion(bombasJugador1);
    comprobarExplosion(bombasJugador2);

    if (!partidaTerminada) {

    if (jugador1.estaVivo() && !jugador2.estaVivo()) {
        ganador = 1;
        partidaTerminada = true;
    }
    else if (!jugador1.estaVivo() && jugador2.estaVivo()) {
        ganador = 2;
        partidaTerminada = true;
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
    for (const Bomba& bomba : bombasJugador1) {
        bomba.dibujar();
    }

    for (const Bomba& bomba : bombasJugador2) {
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

    if (partidaTerminada) {
        DrawText(
            TextFormat("Jugador %d gana", ganador),
            250,
            12,
            24,
            WHITE
        );
    }

    EndDrawing();
}