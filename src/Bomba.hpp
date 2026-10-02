#pragma once
#include <Tablero.hpp>

class Bomba {
public:
    explicit Bomba(Celda posicion);

    bool actualizar(float deltaTime);
    void dibujar() const;
    Celda getPosicion() const { return posicion; }

private:
    Celda posicion;
    float tiempoTranscurrido = 0.0f;
};