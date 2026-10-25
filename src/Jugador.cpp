#include <Jugador.hpp>
#include <Tablero.hpp>

Jugador::Jugador(Vector2 posInicial, float vel, int arriba, int abajo, int izquierda, int derecha, Color colorJugador)
    : posicion(posInicial),
      velocidad(vel),
      velocidadBase(vel),
      tamano(32.0f),
      teclaArriba(arriba),
      teclaAbajo(abajo),
      teclaIzquierda(izquierda),
      teclaDerecha(derecha),
      color(colorJugador){}

void Jugador::init(Vector2 posInicial) {
    posicion = posInicial;
    vivo = true;

    velocidad = velocidadBase;
    maxBombas = 1;
    alcance = ALCANCE_EXPLOSION;

    puedeAtravesarBomba = false;
    bombaAtravesable = { -1, -1 };
}

void Jugador::handleInput() {
    // Reservado para acciones como soltar bombas en el Paquete 3
}

void Jugador::update(float deltaTime, const Tablero& tablero, const std::vector<Celda>& bombas) {
    // La lógica de movimiento y colisión se delega en Tablero.
    // Este método queda solo como punto de compatibilidad con el flujo actual.
    (void)deltaTime;
    (void)tablero;
    (void)bombas;

    if (puedeAtravesarBomba) {
        Vector2 esquina = Tablero::celdaAPixel(
            bombaAtravesable.col,
            bombaAtravesable.fila
        );

        Rectangle cajaBomba = {
            esquina.x,
            esquina.y,
            static_cast<float>(TAM_CELDA),
            static_cast<float>(TAM_CELDA)
        };

        if (!CheckCollisionRecs(getCajaColision(), cajaBomba)) {
            puedeAtravesarBomba = false;
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

void Jugador::permitirAtravesarBomba(Celda celda) {
    puedeAtravesarBomba = true;
    bombaAtravesable = celda;
}

void Jugador::aplicarPowerUp(TipoPowerUp tipo) {
    switch (tipo) {
        case TipoPowerUp::BombaExtra:
            if (maxBombas < MAX_BOMBAS) maxBombas++;
            break;
        case TipoPowerUp::Alcance:
            if (alcance < MAX_ALCANCE) alcance++;
            break;
        case TipoPowerUp::Velocidad:
            velocidad += INCREMENTO_VELOCIDAD;
            if (velocidad > MAX_VELOCIDAD_JUGADOR) velocidad = MAX_VELOCIDAD_JUGADOR;
            break;
    }
}
