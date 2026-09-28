// ---------------------------------------------------------------------------
// Tablero.cpp — Paquete 1 (Escenario)
//
// Generación del tablero, consultas, colisiones con el escenario y dibujo.
// Las decisiones de diseño están explicadas en docs/parte1-escenario.md.
// ---------------------------------------------------------------------------
#include <Tablero.hpp>
#include <cmath>    // std::floor
#include <cstdlib>  // std::abs

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

// ===========================================================================
// Modificación
// ===========================================================================

// Si getCelda dice Bloque, la celda está por fuerza dentro del tablero
// (fuera devuelve Muro), así que setCelda no puede salirse del vector.
bool Tablero::destruirBloque(int col, int fila) {
    if (getCelda(col, fila) != TipoCelda::Bloque) {
        return false;
    }
    setCelda(col, fila, TipoCelda::Vacio);
    return true;
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
    // Todo a Vacio: un reinicio no arrastra nada de la partida anterior.
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
                    // Una línea horizontal para que parezca un ladrillo
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
