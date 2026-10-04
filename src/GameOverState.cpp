#include <GameOverState.hpp>
#include <MainGameState.hpp>
#include <StateMachine.hpp>
#include <Constantes.hpp>
#include <memory>

extern "C" {
    #include <raylib.h>
}

GameOverState::GameOverState(int ganador, unsigned int semilla)
    : ganador(ganador), semilla(semilla)
{
}

void GameOverState::init()
{
}

void GameOverState::handleInput()
{
    if (IsKeyPressed(KEY_ENTER)) {
        state_machine->add_state(
            std::make_unique<MainGameState>(semilla + 1),
            true
        );
    }
}

void GameOverState::update(float deltaTime)
{
}

void GameOverState::render()
{
    BeginDrawing();

    ClearBackground({ 24, 33, 27, 255 });

    const char* textoGameOver = "GAME OVER";
    const char* textoResultado;

    if (ganador == 1) {
        textoResultado = "JUGADOR 1 GANA!";
    }
    else if (ganador == 2) {
        textoResultado = "JUGADOR 2 GANA!";
    }
    else {
        textoResultado = "EMPATE!";
    }
    const char* textoReinicio = "Pulsa ENTER para jugar de nuevo";

    int anchoGameOver = MeasureText(textoGameOver, 50);
    int anchoGanador = MeasureText(textoResultado, 30);
    int anchoReinicio = MeasureText(textoReinicio, 20);

    DrawText(
        textoGameOver,
        ANCHO_VENTANA / 2 - anchoGameOver / 2,
        180,
        50,
        WHITE
    );

    DrawText(
        textoResultado,
        ANCHO_VENTANA / 2 - anchoGanador / 2,
        270,
        30,
        YELLOW
    );

    DrawText(
        textoReinicio,
        ANCHO_VENTANA / 2 - anchoReinicio / 2,
        350,
        20,
        LIGHTGRAY
    );

    EndDrawing();
}