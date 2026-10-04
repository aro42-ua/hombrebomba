#include <Jugador.hpp>
#include <Tablero.hpp>

Jugador::Jugador(Vector2 posInicial, float vel, int arriba, int abajo, int izquierda, int derecha, Color colorJugador)
    : posicion(posInicial),
      velocidad(vel),
      tamano(32.0f),
      teclaArriba(arriba),
      teclaAbajo(abajo),
      teclaIzquierda(izquierda),
      teclaDerecha(derecha),
      color(colorJugador){}

void Jugador::init(Vector2 posInicial) {
    posicion = posInicial;
    vivo = true;
}

void Jugador::handleInput() {
    // Reservado para acciones como soltar bombas en el Paquete 3
}

void Jugador::update(float deltaTime, const Tablero& tablero) {
    //Si no existes no te mueves
    if (!vivo) {
        return;
    }

    Vector2 direccion = { 0.0f, 0.0f };

    // Captura de entradas (WASD / Flechas)
    if (IsKeyDown(teclaDerecha))   direccion.x += 1.0f;
    if (IsKeyDown(teclaIzquierda)) direccion.x -= 1.0f;
    if (IsKeyDown(teclaAbajo))     direccion.y += 1.0f;
    if (IsKeyDown(teclaArriba))    direccion.y -= 1.0f;

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
    Color colorActual = vivo ? color : GRAY;

    DrawRectangleV(
        { posicion.x - tamano / 2.0f, posicion.y - tamano / 2.0f },
        { tamano, tamano },
        colorActual
    );
}

Rectangle Jugador::getCajaColision() const {
    return { posicion.x - tamano / 2.0f, posicion.y - tamano / 2.0f, tamano, tamano };
}