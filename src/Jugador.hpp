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


    //Para que cada jugador guarde sus cuatro teclas
    int teclaArriba;
    int teclaAbajo;
    int teclaIzquierda;
    int teclaDerecha;

    //Color del jugador
    Color color;

    //Variable para saber si está vivo
    bool vivo = true;

public:
    // {72.0f, 120.0f} equivale al centro de la celda (1, 1) en píxeles y 
    Jugador(
        Vector2 posInicial = { 72.0f, 120.0f },
        float vel = 120.0f,
        int arriba = KEY_W,
        int abajo = KEY_S,
        int izquierda = KEY_A,
        int derecha = KEY_D,
        Color colorJugador = BLUE
    );

    void init(Vector2 posInicial);
    void handleInput();
    void update(float deltaTime, const Tablero& tablero);
    void render() const;

    Rectangle getCajaColision() const;
    Vector2 getPosicion() const { return posicion; }

    //Control de vida
    bool estaVivo() const { return vivo; }
    void eliminar() { vivo = false; }
};