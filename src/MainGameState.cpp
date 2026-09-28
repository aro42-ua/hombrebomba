#include <MainGameState.hpp>
#include <iostream>

extern "C" {
    #include <raylib.h>
}

MainGameState::MainGameState()
{
}

void MainGameState::init()
{

}

void MainGameState::handleInput()
{

}

void MainGameState::update(float deltaTime)
{

}

void MainGameState::render()
{
    BeginDrawing();
    ClearBackground(BLACK);
    DrawText("Bomberman DCA", 10, 10, 20, RAYWHITE);
    EndDrawing();
}