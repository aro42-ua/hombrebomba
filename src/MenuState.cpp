#include <MenuState.hpp>
#include <MainGameState.hpp>
#include <StateMachine.hpp>
#include <Constantes.hpp>
#include <Niveles.hpp>
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

    if (mostrandoNiveles) {
        handleInputNiveles();
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
        case 0:
            mostrandoNiveles = true;
            nivelSeleccionado = 0;
            break;
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
            break;
    }
}

void MenuState::handleInputNiveles()
{
    // ESC cierra la ventana (tecla de salida por defecto de raylib), así que
    // para volver se usa RETROCESO.
    if (IsKeyPressed(KEY_BACKSPACE)) {
        mostrandoNiveles = false;
        return;
    }

    if (IsKeyPressed(KEY_UP)) {
        nivelSeleccionado = (nivelSeleccionado + NUM_NIVELES - 1) % NUM_NIVELES;
    }
    if (IsKeyPressed(KEY_DOWN)) {
        nivelSeleccionado = (nivelSeleccionado + 1) % NUM_NIVELES;
    }

    if (IsKeyPressed(KEY_ENTER)) {
        state_machine->add_state(
            std::make_unique<MainGameState>(
                PartidaUnJugador{ nivelSeleccionado + 1, VIDAS_INICIALES },
                semilla
            ),
            true
        );
    }
}

void MenuState::renderNiveles() const
{
    dibujarCentrado("1 JUGADOR", 95, TAMANO_TITULO, YELLOW);
    dibujarCentrado("Elige el nivel", 165, 20, LIGHTGRAY);

    const int inicioY = 225;
    const int separacion = 52;

    for (int i = 0; i < NUM_NIVELES; ++i) {
        const int y = inicioY + i * separacion;
        if (i == nivelSeleccionado) {
            DrawRectangle(
                ANCHO_VENTANA / 2 - 205,
                y - 7,
                410,
                42,
                { 52, 75, 58, 255 }
            );
        }
        dibujarCentrado(
            TextFormat("Nivel %d: %s", i + 1, getNivel(i + 1).nombre),
            y,
            TAMANO_OPCION,
            i == nivelSeleccionado ? YELLOW : RAYWHITE
        );
    }

    dibujarCentrado(
        TextFormat("Empiezas con %d vidas. Mata a todos los enemigos.", VIDAS_INICIALES),
        inicioY + NUM_NIVELES * separacion + 10, 18, LIGHTGRAY
    );
    dibujarCentrado("Flechas y ENTER para jugar, RETROCESO para volver",
                    ALTO_VENTANA - 40, 18, GRAY);
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

    if (mostrandoNiveles) {
        renderNiveles();
        EndDrawing();
        return;
    }

    dibujarCentrado("BOMBERMAN", 125, TAMANO_TITULO, YELLOW);
    dibujarCentrado("Selecciona un modo de juego", 205, 20, LIGHTGRAY);

    const char* opciones[CANTIDAD_OPCIONES] = {
        "1 jugador",
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
