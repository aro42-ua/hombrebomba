#include <Bomba.hpp>
#include <Constantes.hpp>

Bomba::Bomba(Celda posicion) : posicion(posicion) {}

bool Bomba::actualizar(float deltaTime) {
    tiempoTranscurrido += deltaTime;
    return tiempoTranscurrido >= 3.0f;
}

void Bomba::dibujar() const {
    DrawCircleV(Tablero::centroDeCelda(posicion.col, posicion.fila),
                TAM_CELDA / 4.0f, BLACK);
}