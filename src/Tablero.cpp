// ---------------------------------------------------------------------------
// Tablero.cpp — Paquete 1 (Escenario)
//
// Generación del tablero, consultas, colisiones con el escenario y dibujo.
// Las decisiones de diseño están explicadas en docs/parte1-escenario.md.
// ---------------------------------------------------------------------------
#include <Tablero.hpp>
#include <cmath>    // std::floor
#include <cstdlib>  // std::abs
#include <Jugador.hpp>
#include <Bomba.hpp>

// Colores del escenario (figuras elementales, sin sprites en el Hito 1)
static const Color COLOR_SUELO        = {  46, 125,  79, 255 };
static const Color COLOR_MURO         = {  74,  85,  80, 255 };
static const Color COLOR_MURO_BORDE   = {  48,  56,  52, 255 };
static const Color COLOR_BLOQUE       = { 183, 119,  58, 255 };
static const Color COLOR_BLOQUE_BORDE = { 120,  72,  30, 255 };

// ===========================================================================
// Construcción y acceso interno
// ===========================================================================

Tablero::Tablero()
    : celdas(COLUMNAS * FILAS, TipoCelda::Vacio), semilla(0) {}

int Tablero::indice(int col, int fila) const {
    return fila * COLUMNAS + col;
}

// Solo se usa desde dentro de la clase: quien llama ya ha comprobado límites.
void Tablero::setCelda(int col, int fila, TipoCelda tipo) {
    celdas[indice(col, fila)] = tipo;
}

unsigned int Tablero::getSemilla() const {
    return semilla;
}

// ===========================================================================
// Consultas
// ===========================================================================

bool Tablero::dentroDeLimites(int col, int fila) const {
    return col >= 0 && col < COLUMNAS && fila >= 0 && fila < FILAS;
}

// Decisión de diseño: fuera del tablero "hay muro". Así el resto de funciones
// (y el resto de paquetes) nunca leen memoria fuera del vector.
TipoCelda Tablero::getCelda(int col, int fila) const {
    if (!dentroDeLimites(col, fila)) {
        return TipoCelda::Muro;
    }
    return celdas[indice(col, fila)];
}

// Fuera del tablero getCelda devuelve Muro, así que eso ya sale false.
bool Tablero::esTransitable(int col, int fila) const {
    return getCelda(col, fila) == TipoCelda::Vacio;
}

// Zonas de aparición: las 4 esquinas jugables y sus vecinas pegadas
// (distancia Manhattan <= 1). Como los pilares ocupan las diagonales, cada
// zona queda en forma de "L" de 3 celdas.
bool Tablero::esZonaLibre(int col, int fila) const {
    const Celda esquinas[4] = {
        { 1,            1         },   // jugador
        { COLUMNAS - 2, 1         },   // enemigos (pendiente de confirmar con P4)
        { 1,            FILAS - 2 },
        { COLUMNAS - 2, FILAS - 2 },
    };

    for (const Celda& e : esquinas) {
        int distancia = std::abs(col - e.col) + std::abs(fila - e.fila);
        if (distancia <= 1) {
            return true;
        }
    }
    return false;
}


bool Tablero::destruirBloque(int col, int fila) {
    if (getCelda(col, fila) != TipoCelda::Bloque) {
        return false;
    }
    setCelda(col, fila, TipoCelda::Vacio);
    return true;
}

bool Tablero::colisionaConBombas(
    Rectangle caja,
    const std::vector<Celda>& bombas,
    const Celda* bombaIgnorada
) const {
    for (const Celda& celda : bombas) {
        if (bombaIgnorada != nullptr &&
            celda.col == bombaIgnorada->col &&
            celda.fila == bombaIgnorada->fila) {
            continue;
        }

        Vector2 esquina = Tablero::celdaAPixel(celda.col, celda.fila);
        Rectangle cajaBomba = {
            esquina.x,
            esquina.y,
            static_cast<float>(TAM_CELDA),
            static_cast<float>(TAM_CELDA)
        };

        if (CheckCollisionRecs(caja, cajaBomba)) {
            return true;
        }
    }

    return false;
}

bool Tablero::moverJugador(Jugador& jugador, const std::vector<Celda>& bombas,
                          float deltaTime, Vector2 direccion) {
    if (!jugador.estaVivo()) {
        return false;
    }

    Vector2 posicionActual = jugador.getPosicion();
    float tamano = 32.0f;
    const Celda* bombaIgnorada = nullptr;
    Celda bombaAtravesable = { -1, -1 };

    if (jugador.puedeAtravesarUnaBomba()) {
        bombaAtravesable = jugador.getBombaAtravesable();
        bombaIgnorada = &bombaAtravesable;
    }

    if (direccion.x != 0.0f && direccion.y != 0.0f) {
        direccion.x *= 0.7071f;
        direccion.y *= 0.7071f;
    }

    auto cajaEn = [&](float x, float y) {
        return Rectangle {
            x - tamano / 2.0f,
            y - tamano / 2.0f,
            tamano,
            tamano
        };
    };

    float desplazamientoX = direccion.x * jugador.getVelocidad() * deltaTime;
    float desplazamientoY = direccion.y * jugador.getVelocidad() * deltaTime;

    // Intento principal: mover la caja completa en la dirección pedida.
    // Esto permite que W+D funcione como diagonal con velocidad reducida.
    Vector2 destinoFinal = {
        posicionActual.x + desplazamientoX,
        posicionActual.y + desplazamientoY
    };

    Rectangle cajaFinal = cajaEn(destinoFinal.x, destinoFinal.y);
    if ((desplazamientoX != 0.0f || desplazamientoY != 0.0f) &&
        !colisiona(cajaFinal) &&
        !colisionaConBombas(cajaFinal, bombas, bombaIgnorada)) {
        jugador.setPosicion(destinoFinal);
        return true;
    }

    // Si hay colisión en la diagonal, se hace un movimiento por ejes para que
    // siga siendo fluido y no se quede bloqueado en una esquina.
    if (desplazamientoX != 0.0f) {
        float nuevaX = posicionActual.x + desplazamientoX;
        Rectangle cajaX = cajaEn(nuevaX, posicionActual.y);
        if (!colisiona(cajaX) && !colisionaConBombas(cajaX, bombas, bombaIgnorada)) {
            jugador.setPosicion({ nuevaX, posicionActual.y });
        }
    }

    if (desplazamientoY != 0.0f) {
        float nuevaY = posicionActual.y + desplazamientoY;
        Rectangle cajaY = cajaEn(posicionActual.x, nuevaY);
        if (!colisiona(cajaY) && !colisionaConBombas(cajaY, bombas, bombaIgnorada)) {
            jugador.setPosicion({ posicionActual.x, nuevaY });
        }
    }

    return true;
}

bool Tablero::intentarPonerBomba(Jugador& jugador, std::vector<Bomba>& bombas) const {
    if (!jugador.estaVivo()) {
        return false;
    }

    int bombasActivas = 0;
    for (const Bomba& bomba : bombas) {
        if (bomba.estaEnCuentaAtras()) {
            bombasActivas++;
        }
    }
    if (bombasActivas >= jugador.getMaxBombas()) {
        return false;
    }

    Celda celdaBomba = Tablero::pixelACelda(jugador.getPosicion());
    for (const Bomba& bomba : bombas) {
        Celda p = bomba.getPosicion();
        if (p.col == celdaBomba.col && p.fila == celdaBomba.fila) {
            return false;
        }
    }

    bombas.emplace_back(celdaBomba, jugador.getAlcance());
    jugador.permitirAtravesarBomba(celdaBomba);
    return true;
}

bool Tablero::manejarEvento(const EventoTablero& evento,
                           Jugador& jugador1,
                           Jugador& jugador2,
                           std::vector<Bomba>& bombasJugador1,
                           std::vector<Bomba>& bombasJugador2,
                           float deltaTime) {
    switch (evento.tipo) {
        case TipoEventoTablero::MoverJugador: {
            Jugador* jugador = (evento.jugadorId == 1) ? &jugador1 : &jugador2;
            std::vector<Celda> bombas = (evento.jugadorId == 1) ?
                std::vector<Celda>{} : std::vector<Celda>{};
            std::vector<Celda> bombasActivas;
            for (const Bomba& bomba : bombasJugador1) {
                if (bomba.estaEnCuentaAtras()) bombasActivas.push_back(bomba.getPosicion());
            }
            for (const Bomba& bomba : bombasJugador2) {
                if (bomba.estaEnCuentaAtras()) bombasActivas.push_back(bomba.getPosicion());
            }
            return moverJugador(*jugador, bombasActivas, deltaTime, evento.direccion);
        }
        case TipoEventoTablero::PonerBomba: {
            Jugador* jugador = (evento.jugadorId == 1) ? &jugador1 : &jugador2;
            std::vector<Bomba>& destino = (evento.jugadorId == 1) ? bombasJugador1 : bombasJugador2;
            return intentarPonerBomba(*jugador, destino);
        }
    }
    return false;
}

// Orden de decisión para cada celda (importa: si la zona libre se mirara antes
// que el borde, la celda (0,1) quedaría vacía y se podría salir del tablero):
//   1. Borde (primera/última fila o columna) -> Muro
//   2. Pilar (columna par y fila par)        -> Muro
//   3. Zona libre                            -> Vacio
//   4. Resto: Bloque con probabilidad PROB_BLOQUE, si no Vacio
void Tablero::generar(unsigned int semilla) {
    this->semilla = semilla;
    SetRandomSeed(semilla);
    celdas.assign(COLUMNAS * FILAS, TipoCelda::Vacio);

    for (int fila = 0; fila < FILAS; fila++) {
        for (int col = 0; col < COLUMNAS; col++) {
            bool esBorde = (col == 0 || col == COLUMNAS - 1 ||
                            fila == 0 || fila == FILAS - 1);
            bool esPilar = (col % 2 == 0 && fila % 2 == 0);

            if (esBorde || esPilar) {
                setCelda(col, fila, TipoCelda::Muro);
            } else if (esZonaLibre(col, fila)) {
                // Se queda Vacio. Importante: NO se consume número aleatorio,
                // así el resto del tablero depende solo de la semilla.
            } else if (GetRandomValue(0, 99) < PROB_BLOQUE) {
                setCelda(col, fila, TipoCelda::Bloque);
            }
            // else: Vacio (ya lo está por el assign de arriba)
        }
    }
}

// ===========================================================================
// Colisiones
// ===========================================================================

// Como la caja es más pequeña que una celda, solo puede tocar las celdas en
// las que caen sus 4 esquinas. El borde derecho/inferior (x + width,
// y + height) ya no pertenece a la caja: se resta un margen pequeño para que
// una caja que solo "roza" una celda por fuera no cuente como colisión.
bool Tablero::colisiona(Rectangle caja) const {
    const float MARGEN  = 0.01f;
    const float derecha = caja.x + caja.width  - MARGEN;
    const float abajo   = caja.y + caja.height - MARGEN;

    const Vector2 esquinas[4] = {
        { caja.x,  caja.y },   // superior izquierda
        { derecha, caja.y },   // superior derecha
        { caja.x,  abajo  },   // inferior izquierda
        { derecha, abajo  },   // inferior derecha
    };

    for (const Vector2& p : esquinas) {
        Celda c = pixelACelda(p);
        if (!esTransitable(c.col, c.fila)) {
            return true;
        }
    }
    return false;
}

// ===========================================================================
// Conversiones celda <-> píxel
// ===========================================================================

Vector2 Tablero::celdaAPixel(int col, int fila) {
    return { (float)(col * TAM_CELDA), (float)(fila * TAM_CELDA + ALTO_HUD) };
}

Vector2 Tablero::centroDeCelda(int col, int fila) {
    Vector2 esquina = celdaAPixel(col, fila);
    return { esquina.x + TAM_CELDA / 2.0f, esquina.y + TAM_CELDA / 2.0f };
}

// std::floor y no un simple (int): (int)(-0.5f) vale 0, pero floor(-0.5f)
// vale -1. Con (int), un punto un poco a la izquierda del tablero caería
// en la columna 0 en vez de fuera de él.
Celda Tablero::pixelACelda(Vector2 p) {
    int col  = (int)std::floor(p.x / TAM_CELDA);
    int fila = (int)std::floor((p.y - ALTO_HUD) / TAM_CELDA);
    return { col, fila };
}

// ===========================================================================
// Dibujo
// ===========================================================================

void Tablero::dibujar() const {
    for (int fila = 0; fila < FILAS; fila++) {
        for (int col = 0; col < COLUMNAS; col++) {
            Vector2 pos = celdaAPixel(col, fila);
            Rectangle r = { pos.x, pos.y, (float)TAM_CELDA, (float)TAM_CELDA };

            switch (celdas[indice(col, fila)]) {
                case TipoCelda::Vacio:
                    DrawRectangleRec(r, COLOR_SUELO);
                    break;
                case TipoCelda::Muro:
                    DrawRectangleRec(r, COLOR_MURO);
                    DrawRectangleLinesEx(r, 3, COLOR_MURO_BORDE);
                    break;
                case TipoCelda::Bloque:
                    DrawRectangleRec(r, COLOR_BLOQUE);
                    DrawRectangleLinesEx(r, 2, COLOR_BLOQUE_BORDE);
                    DrawLineEx({ r.x, r.y + r.height / 2 },
                               { r.x + r.width, r.y + r.height / 2 },
                               2, COLOR_BLOQUE_BORDE);
                    break;
            }
        }
    }
}

// Pulsa F1 en el juego para verla. Muestra (col,fila) de cada celda y la
// semilla, para poder escribirla en una incidencia y reproducir el tablero.
void Tablero::dibujarDepuracion() const {
    for (int fila = 0; fila < FILAS; fila++) {
        for (int col = 0; col < COLUMNAS; col++) {
            Vector2 pos = celdaAPixel(col, fila);
            DrawRectangleLines((int)pos.x, (int)pos.y, TAM_CELDA, TAM_CELDA,
                               Fade(WHITE, 0.25f));
            DrawText(TextFormat("%d,%d", col, fila),
                     (int)pos.x + 3, (int)pos.y + 3, 10, Fade(WHITE, 0.8f));
        }
    }
    DrawText(TextFormat("semilla: %u", semilla), 10, 14, 20, YELLOW);
}
