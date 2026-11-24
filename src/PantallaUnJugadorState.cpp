#include <PantallaUnJugadorState.hpp>
#include <MainGameState.hpp>
#include <MenuState.hpp>
#include <StateMachine.hpp>
#include <Niveles.hpp>
#include <Constantes.hpp>
#include <memory>

extern "C" {
    #include <raylib.h>
}

namespace {
    void dibujarCentrado(const char* texto, int y, int tamano, Color color)
    {
        DrawText(
            texto,
            ANCHO_VENTANA / 2 - MeasureText(texto, tamano) / 2,
            y,
            tamano,
            color
        );
    }
}

PantallaUnJugadorState::PantallaUnJugadorState(TipoPantallaUnJugador tipo, int nivel,
                                               int vidas, unsigned int semilla)
    : tipo(tipo), nivel(nivel), vidas(vidas), semilla(semilla)
{
}

void PantallaUnJugadorState::init()
{
}

void PantallaUnJugadorState::jugarNivel(int numero, int vidasPartida)
{
    state_machine->add_state(
        std::make_unique<MainGameState>(PartidaUnJugador{ numero, vidasPartida }, semilla),
        true
    );
}

void PantallaUnJugadorState::volverAlMenu()
{
    state_machine->add_state(std::make_unique<MenuState>(semilla), true);
}

void PantallaUnJugadorState::handleInput()
{
    if (tipo == TipoPantallaUnJugador::GameOver && IsKeyPressed(KEY_BACKSPACE)) {
        volverAlMenu();
        return;
    }

    if (!IsKeyPressed(KEY_ENTER)) {
        return;
    }

    switch (tipo) {
        case TipoPantallaUnJugador::NivelSuperado:
            jugarNivel(nivel + 1, vidas);
            break;
        case TipoPantallaUnJugador::VidaPerdida:
            jugarNivel(nivel, vidas);
            break;
        case TipoPantallaUnJugador::GameOver:
            jugarNivel(nivel, VIDAS_INICIALES);
            break;
        case TipoPantallaUnJugador::Victoria:
            volverAlMenu();
            break;
    }
}

void PantallaUnJugadorState::update(float deltaTime)
{
    (void)deltaTime;
}

void PantallaUnJugadorState::render()
{
    BeginDrawing();
    ClearBackground({ 24, 33, 27, 255 });

    switch (tipo) {
        case TipoPantallaUnJugador::NivelSuperado:
            dibujarCentrado(TextFormat("NIVEL %d SUPERADO", nivel), 180, 46, YELLOW);
            dibujarCentrado(getNivel(nivel).nombre, 250, 24, LIGHTGRAY);
            dibujarCentrado(TextFormat("Siguiente: Nivel %d: %s", nivel + 1,
                                       getNivel(nivel + 1).nombre),
                            330, 24, RAYWHITE);
            dibujarCentrado(TextFormat("Vidas: %d", vidas), 375, 22, BLUE);
            dibujarCentrado("Pulsa ENTER para continuar", 470, 20, GRAY);
            break;

        case TipoPantallaUnJugador::VidaPerdida:
            dibujarCentrado("HAS PERDIDO UNA VIDA", 180, 46, RED);
            dibujarCentrado(TextFormat("Nivel %d: %s", nivel, getNivel(nivel).nombre),
                            250, 24, LIGHTGRAY);
            dibujarCentrado(vidas == 1 ? "Te queda 1 vida"
                                       : TextFormat("Te quedan %d vidas", vidas),
                            330, 26, RAYWHITE);
            dibujarCentrado("Pulsa ENTER para repetir el nivel", 470, 20, GRAY);
            break;

        case TipoPantallaUnJugador::GameOver:
            dibujarCentrado("GAME OVER", 180, 50, WHITE);
            dibujarCentrado(TextFormat("Te has quedado sin vidas en el nivel %d", nivel),
                            270, 24, LIGHTGRAY);
            dibujarCentrado("ENTER: repetir el nivel con todas las vidas", 420, 20, GRAY);
            dibujarCentrado("RETROCESO: volver al menu", 455, 20, GRAY);
            break;

        case TipoPantallaUnJugador::Victoria:
            dibujarCentrado("HAS GANADO!", 180, 50, YELLOW);
            dibujarCentrado(TextFormat("Has superado los %d niveles", NUM_NIVELES),
                            270, 26, RAYWHITE);
            dibujarCentrado(TextFormat("Vidas restantes: %d", vidas), 320, 22, BLUE);
            dibujarCentrado("Pulsa ENTER para volver al menu", 470, 20, GRAY);
            break;
    }

    EndDrawing();
}
