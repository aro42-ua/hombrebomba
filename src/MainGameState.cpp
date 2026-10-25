#include <MainGameState.hpp>
#include <iostream>
#include <GameOverState.hpp>
#include <StateMachine.hpp>
#include <memory>
#include <ResultadoPartida.hpp>
#include <cstdlib>
#include <random>

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
    crearEnemigos();

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

void MainGameState::crearEnemigos()
{
    enemigos.clear();
    enemigos.emplace_back(TipoEnemigo::Globo, Celda{ 1, FILAS - 2 }, semilla + 1);
    enemigos.emplace_back(TipoEnemigo::Perseguidor, Celda{ COLUMNAS - 2, FILAS - 2 }, semilla + 2);

    // El tercero, en una celda vacía con alguna salida, lejos de los
    // jugadores y que no esté en las esquinas de los otros dos enemigos.
    const Celda inicioJugadores[2] = { { 1, 1 }, { COLUMNAS - 2, 1 } };
    std::vector<Celda> candidatas;

    for (int fila = 1; fila < FILAS - 1; fila++) {
        for (int col = 1; col < COLUMNAS - 1; col++) {
            if (!tablero.esTransitable(col, fila)) continue;

            bool tieneSalida = false;
            for (const auto& d : DIRECCIONES) {
                if (tablero.esTransitable(col + d[0], fila + d[1])) tieneSalida = true;
            }

            bool valida = tieneSalida;
            for (const Celda& j : inicioJugadores) {
                if (std::abs(col - j.col) + std::abs(fila - j.fila) < DISTANCIA_APARICION) {
                    valida = false;
                }
            }
            for (const Enemigo& e : enemigos) {
                Celda c = e.getCelda();
                if (std::abs(col - c.col) + std::abs(fila - c.fila) < 3) {
                    valida = false;
                }
            }
            if (valida) candidatas.push_back({ col, fila });
        }
    }

    if (!candidatas.empty()) {
        std::mt19937 generador(semilla + 3);
        Celda celda = candidatas[generador() % candidatas.size()];
        enemigos.emplace_back(TipoEnemigo::Globo, celda, semilla + 4);
    }
}

void MainGameState::actualizarEnemigos(float deltaTime, const std::vector<Celda>& llamas)
{
    std::vector<BombaVista> bombas;
    for (const std::vector<Bomba>* lista : { &bombasJugador1, &bombasJugador2 }) {
        for (const Bomba& bomba : *lista) {
            if (bomba.estaEnCuentaAtras()) {
                bombas.push_back({ bomba.getPosicion(), bomba.getAlcance() });
            }
        }
    }
    mapaPeligro.calcular(tablero, bombas, llamas);

    std::vector<Celda> objetivos;
    for (const Jugador* jugador : { &jugador1, &jugador2 }) {
        if (jugador->estaVivo()) {
            objetivos.push_back(Tablero::pixelACelda(jugador->getPosicion()));
        }
    }

    for (Enemigo& enemigo : enemigos) {
        enemigo.update(deltaTime, tablero, mapaPeligro, objetivos);

        Celda celda = enemigo.getCelda();
        for (const Celda& llama : llamas) {
            if (llama.col == celda.col && llama.fila == celda.fila) {
                enemigo.eliminar();
            }
        }
    }

    for (auto it = enemigos.begin(); it != enemigos.end();) {
        if (it->estaVivo()) ++it;
        else it = enemigos.erase(it);
    }

    // La caja del enemigo se encoge un poco: así un roce de esquina no mata.
    for (const Enemigo& enemigo : enemigos) {
        Rectangle caja = enemigo.getCajaColision();
        caja.x += MARGEN_CONTACTO;
        caja.y += MARGEN_CONTACTO;
        caja.width -= 2 * MARGEN_CONTACTO;
        caja.height -= 2 * MARGEN_CONTACTO;

        for (Jugador* jugador : { &jugador1, &jugador2 }) {
            if (jugador->estaVivo() &&
                CheckCollisionRecs(caja, jugador->getCajaColision())) {
                jugador->eliminar();
            }
        }
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
        EventoTablero evento{ TipoEventoTablero::PonerBomba, 1 };
        tablero.manejarEvento(evento, jugador1, jugador2, bombasJugador1, bombasJugador2, 0.0f);
    }

    if (IsKeyPressed(KEY_ENTER)) {
        EventoTablero evento{ TipoEventoTablero::PonerBomba, 2 };
        tablero.manejarEvento(evento, jugador1, jugador2, bombasJugador1, bombasJugador2, 0.0f);
    }

    // Herramienta provisional de depuración.
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Celda c = Tablero::pixelACelda(GetMousePosition());
        tablero.destruirBloque(c.col, c.fila);
    }

    Vector2 dirJugador1 = { 0.0f, 0.0f };
    if (IsKeyDown(KEY_D)) dirJugador1.x += 1.0f;
    if (IsKeyDown(KEY_A)) dirJugador1.x -= 1.0f;
    if (IsKeyDown(KEY_S)) dirJugador1.y += 1.0f;
    if (IsKeyDown(KEY_W)) dirJugador1.y -= 1.0f;

    Vector2 dirJugador2 = { 0.0f, 0.0f };
    if (IsKeyDown(KEY_RIGHT)) dirJugador2.x += 1.0f;
    if (IsKeyDown(KEY_LEFT)) dirJugador2.x -= 1.0f;
    if (IsKeyDown(KEY_DOWN)) dirJugador2.y += 1.0f;
    if (IsKeyDown(KEY_UP)) dirJugador2.y -= 1.0f;

    if (dirJugador1.x != 0.0f || dirJugador1.y != 0.0f) {
        EventoTablero evento{ TipoEventoTablero::MoverJugador, 1, dirJugador1 };
        tablero.manejarEvento(evento, jugador1, jugador2, bombasJugador1, bombasJugador2, 0.016f);
    }

    if (dirJugador2.x != 0.0f || dirJugador2.y != 0.0f) {
        EventoTablero evento{ TipoEventoTablero::MoverJugador, 2, dirJugador2 };
        tablero.manejarEvento(evento, jugador1, jugador2, bombasJugador1, bombasJugador2, 0.016f);
    }
}

void MainGameState::update(float deltaTime)
{
    std::vector<Celda> celdasBombas = celdasConBomba();

    // Movimiento del jugador queda centralizado en Tablero por evento.
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
    std::vector<Celda> llamas = celdasEnLlamas();
    powerUps.actualizar(deltaTime, tablero, llamas);
    recogerPowerUps(jugador1);
    recogerPowerUps(jugador2);

    actualizarEnemigos(deltaTime, llamas);

    // Comprobar si una explosión alcanza a un jugador.
    auto comprobarExplosion = [&](const std::vector<Bomba>& bombas) {
        for (const Bomba& bomba : bombas) {
            if (bomba.estaEnCuentaAtras()) {
                continue;
            }

            Celda celdaJugador1 = Tablero::pixelACelda(jugador1.getPosicion());
            Celda celdaJugador2 = Tablero::pixelACelda(jugador2.getPosicion());

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

    const char* textoEnemigos = TextFormat("Enemigos: %d", (int)enemigos.size());
    DrawText(
        textoEnemigos,
        ANCHO_VENTANA / 2 - MeasureText(textoEnemigos, 20) / 2,
        14,
        20,
        RAYWHITE
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

    for (const Enemigo& enemigo : enemigos) {
        enemigo.render();
    }

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

        // Celdas que la IA considera peligrosas y lo que decide cada enemigo.
        for (int fila = 0; fila < FILAS; fila++) {
            for (int col = 0; col < COLUMNAS; col++) {
                if (mapaPeligro.hayPeligro({ col, fila })) {
                    Vector2 esquina = Tablero::celdaAPixel(col, fila);
                    DrawRectangle((int)esquina.x, (int)esquina.y, TAM_CELDA, TAM_CELDA,
                                  Fade(RED, 0.25f));
                }
            }
        }
        for (const Enemigo& enemigo : enemigos) {
            enemigo.renderDepuracion();
        }

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