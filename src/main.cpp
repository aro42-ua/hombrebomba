#include <StateMachine.hpp>
#include <MainGameState.hpp>
#include <memory>
#include <chrono>

extern "C" {
    #include <raylib.h>
}

int main()
{ 
    // Ventana provisional: el escenario (#2) la ajustará al tamaño del tablero.
    InitWindow(720, 672, "Bomberman DCA");
    SetTargetFPS(60);

    float delta_time = 0.0f;

    StateMachine state_machine = StateMachine();
    state_machine.add_state(std::make_unique<MainGameState>(), false);
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
