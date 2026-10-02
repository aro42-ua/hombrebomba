#include <Jugador.hpp>
#include <Tablero.hpp>

Jugador::Jugador(Vector2 posInicial, float vel) 
    : posicion(posInicial), velocidad(vel), tamano(32.0f) {}

void Jugador::init(Vector2 posInicial) {
    posicion = posInicial;
}

void Jugador::handleInput() {
    // Reservado para acciones como soltar bombas en el Paquete 3
}

void Jugador::update(float deltaTime, const Tablero& tablero) {
    Vector2 direccion = { 0.0f, 0.0f };

    // Captura de entradas (WASD / Flechas)
    if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) direccion.x += 1.0f;
    if (IsKeyDown(KEY_LEFT)  || IsKeyDown(KEY_A)) direccion.x -= 1.0f;
    if (IsKeyDown(KEY_DOWN)  || IsKeyDown(KEY_S)) direccion.y += 1.0f;
    if (IsKeyDown(KEY_UP)    || IsKeyDown(KEY_W)) direccion.y -= 1.0f;

    // Normalización para mantener la misma velocidad en diagonal
    if (direccion.x != 0.0f && direccion.y != 0.0f) {
        direccion.x *= 0.7071f;
        direccion.y *= 0.7071f;
    }

    // --- Movimiento en Eje X con colisión ---
    if (direccion.x != 0.0f) {
        float nuevaX = posicion.x + direccion.x * velocidad * deltaTime;
        Rectangle cajaX = { nuevaX - tamano / 2.0f, posicion.y - tamano / 2.0f, tamano, tamano };
        
        if (!tablero.colisiona(cajaX)) {
            posicion.x = nuevaX;
        }
    }

    // --- Movimiento en Eje Y con colisión ---
    if (direccion.y != 0.0f) {
        float nuevaY = posicion.y + direccion.y * velocidad * deltaTime;
        Rectangle cajaY = { posicion.x - tamano / 2.0f, nuevaY - tamano / 2.0f, tamano, tamano };

        if (!tablero.colisiona(cajaY)) {
            posicion.y = nuevaY;
        }
    }
}

void Jugador::render() const {
    // Dibujado del personaje centrado en su posición actual
    DrawRectangleV({ posicion.x - tamano / 2.0f, posicion.y - tamano / 2.0f }, { tamano, tamano }, BLUE);
}

Rectangle Jugador::getCajaColision() const {
    return { posicion.x - tamano / 2.0f, posicion.y - tamano / 2.0f, tamano, tamano };
}