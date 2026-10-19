#pragma once
#include <Constantes.hpp>
#include <vector>
#include <Tablero.hpp>
#include <PowerUp.hpp>

extern "C" {
    #include <raylib.h>
}

class Jugador {
private:
    Vector2 posicion;
    float velocidad;
    float velocidadBase;   // la de inicio de partida; init() la recupera
    float tamano;

    // Mejoras que dan los power-ups. init() las devuelve a sus valores
    // iniciales en cada partida.
    int maxBombas = 1;
    int alcance = ALCANCE_EXPLOSION;


    //Para que cada jugador guarde sus cuatro teclas
    int teclaArriba;
    int teclaAbajo;
    int teclaIzquierda;
    int teclaDerecha;

    //Color del jugador
    Color color;

    //Variable para saber si está vivo
    bool vivo = true;

    bool puedeAtravesarBomba = false;
    Celda bombaAtravesable = { -1, -1 };

    bool colisionaConBombas(
        Rectangle caja,
        const std::vector<Celda>& bombas
    ) const;

public:
    // {72.0f, 120.0f} equivale al centro de la celda (1, 1) en píxeles y 
    Jugador(
        Vector2 posInicial = { 72.0f, 120.0f },
        float vel = VELOCIDAD_JUGADOR,
        int arriba = KEY_W,
        int abajo = KEY_S,
        int izquierda = KEY_A,
        int derecha = KEY_D,
        Color colorJugador = BLUE
    );

    void init(Vector2 posInicial);
    void handleInput();
    void update(float deltaTime, const Tablero& tablero, const std::vector<Celda>& bombas);
    void render() const;

    Rectangle getCajaColision() const;
    Vector2 getPosicion() const { return posicion; }

    //Control de vida
    bool estaVivo() const { return vivo; }
    void eliminar() { vivo = false; }

    void permitirAtravesarBomba(Celda celda);

    // Power-ups. Cada mejora tiene un tope (MAX_BOMBAS, MAX_ALCANCE,
    // MAX_VELOCIDAD_JUGADOR); por encima del tope el power-up no hace nada.
    void aplicarPowerUp(TipoPowerUp tipo);
    int getMaxBombas() const { return maxBombas; }
    int getAlcance() const { return alcance; }
    float getVelocidad() const { return velocidad; }
};