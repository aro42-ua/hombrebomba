// ---------------------------------------------------------------------------
// PowerUp.cpp — Power-ups
//
// Reparto, aparición, recogida y dibujo. Las decisiones están explicadas en
// docs/powerups.md.
// ---------------------------------------------------------------------------
#include <PowerUp.hpp>
#include <random>

// Margen para no confundir la explosión que destapa el power-up con una
// posterior: la primera dura DURACION_EXPLOSION desde el mismo fotograma.
static constexpr float MARGEN_EXPLOSION = 0.05f;

// ===========================================================================
// Reparto
// ===========================================================================

// Se usa std::mt19937 y no GetRandomValue de raylib por dos motivos: no altera
// la secuencia con la que Tablero genera los bloques, y mt19937 da los mismos
// números en cualquier compilador, así una semilla de una incidencia
// reproduce también los power-ups.
void GestorPowerUps::repartir(const Tablero& tablero, unsigned int semilla,
                              int cantidad)
{
    powerUps.clear();

    std::vector<Celda> bloques;
    for (int fila = 0; fila < FILAS; fila++) {
        for (int col = 0; col < COLUMNAS; col++) {
            if (tablero.getCelda(col, fila) == TipoCelda::Bloque) {
                bloques.push_back({ col, fila });
            }
        }
    }

    // Fisher-Yates a mano: std::shuffle depende de la implementación de la
    // librería estándar y podría dar otro orden en otro sistema.
    std::mt19937 generador(semilla);
    for (int i = static_cast<int>(bloques.size()) - 1; i > 0; i--) {
        int j = static_cast<int>(generador() % static_cast<unsigned int>(i + 1));
        std::swap(bloques[i], bloques[j]);
    }

    if (cantidad > static_cast<int>(bloques.size())) {
        cantidad = static_cast<int>(bloques.size());
    }

    // Los tipos se reparten por turnos para que haya de todos: con 9 salen
    // 3 de cada.
    const TipoPowerUp tipos[3] = {
        TipoPowerUp::BombaExtra, TipoPowerUp::Alcance, TipoPowerUp::Velocidad
    };
    for (int i = 0; i < cantidad; i++) {
        PowerUp p;
        p.celda = bloques[i];
        p.tipo = tipos[i % 3];
        powerUps.push_back(p);
    }
}

// ===========================================================================
// Actualización y recogida
// ===========================================================================

static bool estaEnLlamas(Celda celda, const std::vector<Celda>& llamas)
{
    for (const Celda& c : llamas) {
        if (c.col == celda.col && c.fila == celda.fila) return true;
    }
    return false;
}

void GestorPowerUps::actualizar(float deltaTime, const Tablero& tablero,
                                const std::vector<Celda>& celdasEnLlamas)
{
    for (auto it = powerUps.begin(); it != powerUps.end();) {
        if (!it->visible) {
            // Sin bloque encima: queda a la vista. El tiempo empieza en 0 en
            // este fotograma, que es cuando ha explotado la bomba.
            if (tablero.esTransitable(it->celda.col, it->celda.fila)) {
                it->visible = true;
                it->tiempoVisible = 0.0f;
            }
            ++it;
            continue;
        }

        it->tiempoVisible += deltaTime;

        bool explosionPosterior =
            it->tiempoVisible > DURACION_EXPLOSION + MARGEN_EXPLOSION;
        if (explosionPosterior && estaEnLlamas(it->celda, celdasEnLlamas)) {
            it = powerUps.erase(it);
        } else {
            ++it;
        }
    }
}

bool GestorPowerUps::recoger(Celda celda, TipoPowerUp& tipo)
{
    for (auto it = powerUps.begin(); it != powerUps.end(); ++it) {
        if (it->visible && it->celda.col == celda.col &&
            it->celda.fila == celda.fila) {
            tipo = it->tipo;
            powerUps.erase(it);
            return true;
        }
    }
    return false;
}

// ===========================================================================
// Dibujo (figuras básicas, sin sprites en el Hito 1)
// ===========================================================================

static Color colorDe(TipoPowerUp tipo)
{
    switch (tipo) {
        case TipoPowerUp::BombaExtra: return {  40,  40,  48, 255 };
        case TipoPowerUp::Alcance:    return { 214,  69,  44, 255 };
        case TipoPowerUp::Velocidad:  return {  52, 152, 219, 255 };
    }
    return WHITE;
}

// B = bomba extra, F = fuego (alcance), V = velocidad
static const char* letraDe(TipoPowerUp tipo)
{
    switch (tipo) {
        case TipoPowerUp::BombaExtra: return "B";
        case TipoPowerUp::Alcance:    return "F";
        case TipoPowerUp::Velocidad:  return "V";
    }
    return "?";
}

void GestorPowerUps::dibujar() const
{
    for (const PowerUp& p : powerUps) {
        if (!p.visible) continue;

        Vector2 esquina = Tablero::celdaAPixel(p.celda.col, p.celda.fila);
        Rectangle forma = { esquina.x + 8, esquina.y + 8,
                            TAM_CELDA - 16.0f, TAM_CELDA - 16.0f };

        DrawRectangleRounded(forma, 0.3f, 6, colorDe(p.tipo));
        DrawRectangleRoundedLines(forma, 0.3f, 6, RAYWHITE);

        const char* letra = letraDe(p.tipo);
        int ancho = MeasureText(letra, 20);
        DrawText(letra, (int)(forma.x + forma.width / 2 - ancho / 2.0f),
                 (int)(forma.y + forma.height / 2 - 10), 20, RAYWHITE);
    }
}

void GestorPowerUps::dibujarDepuracion() const
{
    for (const PowerUp& p : powerUps) {
        if (p.visible) continue;

        Vector2 esquina = Tablero::celdaAPixel(p.celda.col, p.celda.fila);
        DrawRectangleLines((int)esquina.x + 6, (int)esquina.y + 6,
                           TAM_CELDA - 12, TAM_CELDA - 12, colorDe(p.tipo));
        DrawText(letraDe(p.tipo), (int)esquina.x + TAM_CELDA - 16,
                 (int)esquina.y + TAM_CELDA - 18, 14, colorDe(p.tipo));
    }
}
