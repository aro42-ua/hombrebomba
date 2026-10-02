#pragma once
#include <Constantes.hpp>

extern "C" {
    #include <raylib.h>
}

class Tablero; // Declaración adelantada para evitar inclusión circular

class Jugador {
private:
    Vector2 posicion;
    float velocidad;
    float tamano;

public:
    // {72.0f, 120.0f} equivale al centro de la celda (1, 1) en píxeles
    Jugador(Vector2 posInicial = { 72.0f, 120.0f }, float vel = 120.0f);

    void init(Vector2 posInicial);
    void handleInput();
    void update(float deltaTime, const Tablero& tablero);
    void render() const;

    Rectangle getCajaColision() const;
    Vector2 getPosicion() const { return posicion; }
};