#include <Bomba.hpp>
#include <Constantes.hpp>

Bomba::Bomba(Celda posicion) : posicion(posicion) {}

bool Bomba::estaEnCuentaAtras() const {
    return fase == Fase::CuentaAtras;
}

void Bomba::iniciarExplosion(Tablero& tablero) {
    celdasExplosion.push_back(posicion);

    for (const auto& direccion : DIRECCIONES) {
        for (int paso = 1; paso <= ALCANCE_EXPLOSION; paso++) {
            Celda celda = {
                posicion.col + direccion[0] * paso,
                posicion.fila + direccion[1] * paso,
            };
            TipoCelda tipo = tablero.getCelda(celda.col, celda.fila);

            if (tipo == TipoCelda::Muro) break;

            celdasExplosion.push_back(celda);
            if (tipo == TipoCelda::Bloque) {
                tablero.destruirBloque(celda.col, celda.fila);
                break;
            }
        }
    }
}

bool Bomba::actualizar(float deltaTime, Tablero& tablero) {
    tiempoTranscurrido += deltaTime;
    if (fase == Fase::CuentaAtras) {
        if (tiempoTranscurrido >= DURACION_BOMBA) {
            iniciarExplosion(tablero);
            tiempoTranscurrido = 0.0f;
            fase = Fase::Explosion;
        }
        return false;
    }
    return tiempoTranscurrido >= DURACION_EXPLOSION;
}

void Bomba::dibujar() const {
    if (fase == Fase::CuentaAtras) {
        DrawCircleV(Tablero::centroDeCelda(posicion.col, posicion.fila),
                    TAM_CELDA / 4.0f, BLACK);
        return;
    }

    for (const Celda& celda : celdasExplosion) {
        Vector2 esquina = Tablero::celdaAPixel(celda.col, celda.fila);
        Rectangle forma = {
            esquina.x + 4.0f,
            esquina.y + 4.0f,
            TAM_CELDA - 8.0f,
            TAM_CELDA - 8.0f,
        };
        DrawRectangleRec(forma, ORANGE);
    }
}