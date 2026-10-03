#include <MainGameState.hpp>
#include <iostream>

extern "C" {
    #include <raylib.h>
}

// ---------------------------------------------------------------------------
// Banco de pruebas del escenario (provisional, hasta que llegue el jugador):
//   F1          mostrar/ocultar depuración (rejilla, coordenadas, semilla y
//               caja de colisión)
//   R           nuevo tablero (siguiente semilla)
//   Ratón       mueve una caja del tamaño del jugador; se pone ROJA si
//               colisiona con el escenario
//   Clic izq.   destruye el bloque bajo el ratón (simula una explosión)
// ---------------------------------------------------------------------------

MainGameState::MainGameState(unsigned int semilla) : semilla(semilla)
{
}

void MainGameState::init()
{
    
    // init() se llama cada vez que el estado entra en la máquina de estados,
    // así que una partida nueva siempre empieza con un tablero limpio.
    tablero.generar(semilla);
    jugador.init(Tablero::centroDeCelda(1, 1));
    bombas.clear();
}

void MainGameState::handleInput()
{
    if (IsKeyPressed(KEY_F1)) depuracion = !depuracion;
    if (IsKeyPressed(KEY_R)) {
        tablero.generar(++semilla);
        jugador.init(Tablero::centroDeCelda(1, 1));
        bombas.clear();
    }
    bool hayBombaEnCuentaAtras = false;
    for (const Bomba& bomba : bombas) {
        if (bomba.estaEnCuentaAtras()) {
            hayBombaEnCuentaAtras = true;
            break;
        }
    }
    if (IsKeyPressed(KEY_SPACE) && !hayBombaEnCuentaAtras) {
        bombas.emplace_back(Tablero::pixelACelda(jugador.getPosicion()));
    }
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Celda c = Tablero::pixelACelda(GetMousePosition());
        tablero.destruirBloque(c.col, c.fila);
    }
    jugador.handleInput(); // Entradas propias del jugador
}

void MainGameState::update(float deltaTime)
{
    jugador.update(deltaTime, tablero);
    for (auto bomba = bombas.begin(); bomba != bombas.end();) {
        if (bomba->actualizar(deltaTime, tablero)) {
            bomba = bombas.erase(bomba);
        } else {
            ++bomba;
        }
    }
    Vector2 raton = GetMousePosition();
    cajaPrueba = { raton.x - TAM_JUGADOR / 2.0f,
                   raton.y - TAM_JUGADOR / 2.0f,
                   (float)TAM_JUGADOR, (float)TAM_JUGADOR };
    cajaChoca = tablero.colisiona(cajaPrueba);
}

void MainGameState::render()
{
    BeginDrawing();
    ClearBackground({ 24, 33, 27, 255 });   // fondo = franja del HUD

    tablero.dibujar();
    jugador.render(); // <-- Dibuja al jugador en pantalla
    for (const Bomba& bomba : bombas) bomba.dibujar();

    if (depuracion) {
        tablero.dibujarDepuracion();
        DrawRectangleLinesEx(cajaPrueba, 2, cajaChoca ? RED : GREEN);
        DrawRectangleLinesEx(jugador.getCajaColision(), 2, RED);
    }

    EndDrawing();
}
