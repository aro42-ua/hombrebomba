#pragma once

#include <GameState.hpp>

class GameOverState : public GameState
{
public:
    GameOverState(int ganador, unsigned int semilla);

    void init() override;
    void handleInput() override;
    void update(float deltaTime) override;
    void render() override;

    void pause() override {};
    void resume() override {};

private:
    int ganador;
    unsigned int semilla;
};