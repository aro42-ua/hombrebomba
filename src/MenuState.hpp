#pragma once

#include <GameState.hpp>

class MenuState : public GameState
{
public:
    explicit MenuState(unsigned int semilla);

    void init() override;
    void handleInput() override;
    void update(float deltaTime) override;
    void render() override;

    void pause() override {};
    void resume() override {};

private:
    unsigned int semilla;
    int opcionSeleccionada = 0;
    bool mostrandoCreditos = false;
};
