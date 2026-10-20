#pragma once
#include <Tablero.hpp>
#include <Constantes.hpp>
#include <vector>

class Bomba {
public:
    // alcance: celdas que recorre la llama en cada dirección. Por defecto
    // ALCANCE_EXPLOSION; con power-ups de alcance el jugador pone bombas más
    // potentes.
    explicit Bomba(Celda posicion, int alcance = ALCANCE_EXPLOSION);

    bool actualizar(float deltaTime, Tablero& tablero);
    void dibujar() const;
    Celda getPosicion() const { return posicion; }
    int getAlcance() const { return alcance; }
    bool estaEnCuentaAtras() const;
    const std::vector<Celda>& getCeldasExplosion() const { return celdasExplosion; }

private:
    enum class Fase { CuentaAtras, Explosion };

    void iniciarExplosion(Tablero& tablero);

    Celda posicion;
    int alcance;
    std::vector<Celda> celdasExplosion;
    float tiempoTranscurrido = 0.0f;
    Fase fase = Fase::CuentaAtras;
};