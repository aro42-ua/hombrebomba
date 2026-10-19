#include <MainGameState.hpp>
#include <iostream>
#include <GameOverState.hpp>
#include <StateMachine.hpp>
#include <memory>
#include <ResultadoPartida.hpp>

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
          VELOCIDAD_JUGADOR,
          KEY_W,
          KEY_S,
          KEY_A,
          KEY_D,
          BLUE
      ),
      jugador2(
          Tablero::centroDeCelda(COLUMNAS - 2, 1),
          VELOCIDAD_JUGADOR,
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
    reiniciarPartida();
}

void MainGameState::reiniciarPartida()
{
    // Se genera un tablero limpio al comenzar una partida.
    tablero.generar(semilla);
    powerUps.repartir(tablero, semilla);

    // Posiciones iniciales de los dos jugadores.
    jugador1.init(Tablero::centroDeCelda(1, 1));
    jugador2.init(Tablero::centroDeCelda(COLUMNAS - 2, 1));

    bombasJugador1.clear();
    bombasJugador2.clear();

    partidaTerminada = false;
    ganador = 0;
}

void MainGameState::intentarPonerBomba(Jugador& jugador, std::vector<Bomba>& bombas)
{
    if (!jugador.estaVivo()) {
        return;
    }

    int bombasActivas = 0;
    for (const Bomba& bomba : bombas) {
        if (bomba.estaEnCuentaAtras()) {
            bombasActivas++;
        }
    }
    if (bombasActivas >= jugador.getMaxBombas()) {
        return;
    }

    // Con varias bombas, el jugador podría pulsar dos veces sin moverse.
    Celda celdaBomba = Tablero::pixelACelda(jugador.getPosicion());
    for (const Celda& ocupada : celdasConBomba()) {
        if (ocupada.col == celdaBomba.col && ocupada.fila == celdaBomba.fila) {
            return;
        }
    }

    bombas.emplace_back(celdaBomba, jugador.getAlcance());
    jugador.permitirAtravesarBomba(celdaBomba);
}

std::vector<Celda> MainGameState::celdasConBomba() const
{
    std::vector<Celda> celdas;
    for (const std::vector<Bomba>* lista : { &bombasJugador1, &bombasJugador2 }) {
        for (const Bomba& bomba : *lista) {
            if (bomba.estaEnCuentaAtras()) {
                celdas.push_back(bomba.getPosicion());
            }
        }
    }
    return celdas;
}

std::vector<Celda> MainGameState::celdasEnLlamas() const
{
    std::vector<Celda> celdas;
    for (const std::vector<Bomba>* lista : { &bombasJugador1, &bombasJugador2 }) {
        for (const Bomba& bomba : *lista) {
            if (!bomba.estaEnCuentaAtras()) {
                const std::vector<Celda>& llamas = bomba.getCeldasExplosion();
                celdas.insert(celdas.end(), llamas.begin(), llamas.end());
            }
        }
    }
    return celdas;
}

void MainGameState::recogerPowerUps(Jugador& jugador)
{
    if (!jugador.estaVivo()) {
        return;
    }

    TipoPowerUp tipo;
    if (powerUps.recoger(Tablero::pixelACelda(jugador.getPosicion()), tipo)) {
        jugador.aplicarPowerUp(tipo);
    }
}

void MainGameState::handleInput()
{
    // Activar/desactivar información de depuración.
    if (IsKeyPressed(KEY_F1)) {
        depuracion = !depuracion;
    }

    // Reiniciar el tablero utilizando una nueva semilla.
    if (IsKeyPressed(KEY_R)) {
        ++semilla;
        reiniciarPartida();
    }

    if (IsKeyPressed(KEY_SPACE)) {
        intentarPonerBomba(jugador1, bombasJugador1);
    }

    if (IsKeyPressed(KEY_ENTER)) {
        intentarPonerBomba(jugador2, bombasJugador2);
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
    std::vector<Celda> celdasBombas = celdasConBomba();

    // Cada jugador procesa sus propios controles.
    jugador1.update(deltaTime, tablero, celdasBombas);
    jugador2.update(deltaTime, tablero, celdasBombas);

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

    // Power-ups: aparecen al romperse su bloque, se pierden si les alcanza
    // otra explosión y los recoge el primer jugador que pisa la celda.
    powerUps.actualizar(deltaTime, tablero, celdasEnLlamas());
    recogerPowerUps(jugador1);
    recogerPowerUps(jugador2);

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

    ResultadoPartida resultado = calcularResultadoPartida(
        jugador1.estaVivo(),
        jugador2.estaVivo()
    );

    if (resultado != ResultadoPartida::EnCurso) {

            if (resultado == ResultadoPartida::Jugador1) {
                ganador = 1;
            }
            else if (resultado == ResultadoPartida::Jugador2) {
                ganador = 2;
            }
            else {
                ganador = 0;
            }

            partidaTerminada = true;

            state_machine->add_state(
                std::make_unique<GameOverState>(ganador, semilla),
                true
            );
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

    // ---------------------------------------------------------------------
    // HUD
    // ---------------------------------------------------------------------

    const char* estadoJugador1 =
        jugador1.estaVivo() ? "VIVO" : "ELIMINADO";

    const char* estadoJugador2 =
        jugador2.estaVivo() ? "VIVO" : "ELIMINADO";

    DrawText(
        TextFormat("J1: %s", estadoJugador1),
        20,
        6,
        20,
        jugador1.estaVivo() ? BLUE : GRAY
    );

    const char* textoJugador2 =
        TextFormat("J2: %s", estadoJugador2);

    int anchoJugador2 = MeasureText(textoJugador2, 20);

    DrawText(
        textoJugador2,
        ANCHO_VENTANA - anchoJugador2 - 20,
        6,
        20,
        jugador2.estaVivo() ? RED : GRAY
    );

    // Mejoras de cada jugador: bombas a la vez, alcance y velocidad.
    DrawText(
        TextFormat("B%d F%d V%d", jugador1.getMaxBombas(), jugador1.getAlcance(),
                   (int)jugador1.getVelocidad()),
        20, 30, 10, LIGHTGRAY
    );

    const char* mejorasJugador2 =
        TextFormat("B%d F%d V%d", jugador2.getMaxBombas(), jugador2.getAlcance(),
                   (int)jugador2.getVelocidad());

    DrawText(
        mejorasJugador2,
        ANCHO_VENTANA - MeasureText(mejorasJugador2, 10) - 20,
        30, 10, LIGHTGRAY
    );

    // Línea que separa el HUD del tablero.
    DrawLine(
        0,
        ALTO_HUD - 1,
        ANCHO_VENTANA,
        ALTO_HUD - 1,
        DARKGRAY
    );

    // ---------------------------------------------------------------------
    // Juego
    // ---------------------------------------------------------------------

    tablero.dibujar();
    powerUps.dibujar();

    jugador1.render();
    jugador2.render();

    for (const Bomba& bomba : bombasJugador1) {
        bomba.dibujar();
    }

    for (const Bomba& bomba : bombasJugador2) {
        bomba.dibujar();
    }

    // ---------------------------------------------------------------------
    // Depuración
    // ---------------------------------------------------------------------

    if (depuracion) {
        tablero.dibujarDepuracion();
        powerUps.dibujarDepuracion();

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