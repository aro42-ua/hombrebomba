#pragma once
#include <Tablero.hpp>
#include <vector>

class Bomba {
public:
    explicit Bomba(Celda posicion);

    bool actualizar(float deltaTime, Tablero& tablero);
    void dibujar() const;
    Celda getPosicion() const { return posicion; }
    bool estaEnCuentaAtras() const;
    const std::vector<Celda>& getCeldasExplosion() const { return celdasExplosion; }

private:
    enum class Fase { CuentaAtras, Explosion };

    void iniciarExplosion(Tablero& tablero);

    Celda posicion;
    std::vector<Celda> celdasExplosion;
    float tiempoTranscurrido = 0.0f;
    Fase fase = Fase::CuentaAtras;
};