#include <StateMachine.hpp>
#include <MainGameState.hpp>
#include <Constantes.hpp>
#include <memory>
#include <chrono>
#include <ctime>
#include <cstdlib>

extern "C" {
    #include <raylib.h>
}

// Uso: bomberman [semilla]
// Con una semilla concreta (p. ej. la de una incidencia) se reproduce el
// mismo tablero. Sin ella, se usa la hora actual.
int main(int argc, char* argv[])
{ 
    InitWindow(ANCHO_VENTANA, ALTO_VENTANA, "Bomberman DCA");
    SetTargetFPS(60);

    unsigned int semilla = static_cast<unsigned int>(std::time(nullptr));
    if (argc > 1) {
        semilla = static_cast<unsigned int>(std::strtoul(argv[1], nullptr, 10));
    }

    float delta_time = 0.0f;

    StateMachine state_machine = StateMachine();
    state_machine.add_state(std::make_unique<MainGameState>(semilla), false);
    state_machine.handle_state_changes(delta_time);

    while (!state_machine.is_game_ending() && !WindowShouldClose())
    {
        delta_time = GetFrameTime();

        state_machine.handle_state_changes(delta_time);
        state_machine.getCurrentState()->handleInput();
        state_machine.getCurrentState()->update(delta_time);
        state_machine.getCurrentState()->render();       
    }

    CloseWindow();
    return 0;
}
