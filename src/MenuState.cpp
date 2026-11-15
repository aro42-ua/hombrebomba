#include <MenuState.hpp>
#include <MainGameState.hpp>
#include <StateMachine.hpp>
#include <Constantes.hpp>
#include <memory>

extern "C" {
    #include <raylib.h>
}

namespace {
    constexpr int CANTIDAD_OPCIONES = 4;
    constexpr int TAMANO_TITULO = 48;
    constexpr int TAMANO_OPCION = 26;

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

MenuState::MenuState(unsigned int semilla)
    : semilla(semilla)
{
}

void MenuState::init()
{
}

void MenuState::handleInput()
{
    if (mostrandoCreditos) {
        if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_ENTER)) {
            mostrandoCreditos = false;
        }
        return;
    }

    if (IsKeyPressed(KEY_UP)) {
        opcionSeleccionada =
            (opcionSeleccionada + CANTIDAD_OPCIONES - 1) % CANTIDAD_OPCIONES;
    }
    if (IsKeyPressed(KEY_DOWN)) {
        opcionSeleccionada = (opcionSeleccionada + 1) % CANTIDAD_OPCIONES;
    }

    if (!IsKeyPressed(KEY_ENTER)) {
        return;
    }

    switch (opcionSeleccionada) {
        case 1:
            state_machine->add_state(
                std::make_unique<MainGameState>(semilla),
                true
            );
            break;
        case 2:
            mostrandoCreditos = true;
            break;
        case 3:
            state_machine->stop();
            break;
        default:
            // El modo de un jugador aun no esta implementado.
            break;
    }
}

void MenuState::update(float deltaTime)
{
}

void MenuState::render()
{
    BeginDrawing();
    ClearBackground({ 24, 33, 27, 255 });

    if (mostrandoCreditos) {
        dibujarCentrado("CREDITOS", 150, TAMANO_TITULO, YELLOW);
        dibujarCentrado("Bomberman DCA", 245, TAMANO_OPCION, RAYWHITE);
        dibujarCentrado("Desarrollo: Angel, Samuel, Antonio y Abel",
                        315, 18, LIGHTGRAY);
        dibujarCentrado("Hecho con raylib", 355, 18, LIGHTGRAY);
        dibujarCentrado("Pulsa ESC o ENTER para volver", 480, 18, GRAY);
        EndDrawing();
        return;
    }

    dibujarCentrado("BOMBERMAN", 125, TAMANO_TITULO, YELLOW);
    dibujarCentrado("Selecciona un modo de juego", 205, 20, LIGHTGRAY);

    const char* opciones[CANTIDAD_OPCIONES] = {
        "1 jugador (sin hacer)",
        "2 jugadores",
        "Creditos",
        "Salir"
    };
    const int inicioY = 280;
    const int separacion = 54;

    for (int i = 0; i < CANTIDAD_OPCIONES; ++i) {
        const int y = inicioY + i * separacion;
        if (i == opcionSeleccionada) {
            DrawRectangle(
                ANCHO_VENTANA / 2 - 205,
                y - 7,
                410,
                42,
                { 52, 75, 58, 255 }
            );
        }
        dibujarCentrado(
            opciones[i],
            y,
            TAMANO_OPCION,
            i == opcionSeleccionada ? YELLOW : RAYWHITE
        );
    }

    dibujarCentrado("Usa las flechas y ENTER", 520, 18, GRAY);
    EndDrawing();
}
