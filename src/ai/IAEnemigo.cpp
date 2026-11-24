// ---------------------------------------------------------------------------
// IAEnemigo.cpp — Inteligencia de los enemigos
// ---------------------------------------------------------------------------
#include <IAEnemigo.hpp>
#include <Constantes.hpp>
#include <queue>

// ===========================================================================
// Mapa de peligro
// ===========================================================================

MapaPeligro::MapaPeligro()
    : peligro(COLUMNAS * FILAS, 0), bomba(COLUMNAS * FILAS, 0) {}

void MapaPeligro::calcular(const Tablero& tablero,
                           const std::vector<BombaVista>& bombas,
                           const std::vector<Celda>& llamas)
{
    peligro.assign(COLUMNAS * FILAS, 0);
    bomba.assign(COLUMNAS * FILAS, 0);

    for (const BombaVista& b : bombas) {
        if (!tablero.dentroDeLimites(b.celda.col, b.celda.fila)) continue;

        bomba[indice(b.celda)] = 1;
        peligro[indice(b.celda)] = 1;

        for (const auto& direccion : DIRECCIONES) {
            for (int paso = 1; paso <= b.alcance; paso++) {
                Celda c = { b.celda.col + direccion[0] * paso,
                            b.celda.fila + direccion[1] * paso };
                TipoCelda tipo = tablero.getCelda(c.col, c.fila);

                if (tipo == TipoCelda::Muro) break;
                peligro[indice(c)] = 1;
                if (tipo == TipoCelda::Bloque) break;
            }
        }
    }

    for (const Celda& c : llamas) {
        if (tablero.dentroDeLimites(c.col, c.fila)) {
            peligro[indice(c)] = 1;
        }
    }
}

// Fuera del tablero no hay peligro ni bombas; de todas formas nadie puede
// pisar ahí porque getCelda devuelve Muro.
bool MapaPeligro::hayPeligro(Celda c) const
{
    if (c.col < 0 || c.col >= COLUMNAS || c.fila < 0 || c.fila >= FILAS) return false;
    return peligro[indice(c)] != 0;
}

bool MapaPeligro::hayBomba(Celda c) const
{
    if (c.col < 0 || c.col >= COLUMNAS || c.fila < 0 || c.fila >= FILAS) return false;
    return bomba[indice(c)] != 0;
}

bool MapaPeligro::esPisable(const Tablero& tablero, Celda c) const
{
    return tablero.esTransitable(c.col, c.fila) && !hayBomba(c);
}

// ===========================================================================
// Árbol de decisión
// ===========================================================================

AccionEnemigo decidir(const Percepcion& p)
{
    if (p.vePeligro && p.enPeligro && p.hayRefugio) {
        return AccionEnemigo::Huir;
    }
    if (p.puedePerseguir && p.jugadorAlAlcance) {
        return AccionEnemigo::Perseguir;
    }
    if (p.tieneSalida) {
        return AccionEnemigo::Deambular;
    }
    return AccionEnemigo::Esperar;
}

// ===========================================================================
// Búsqueda en anchura
// ===========================================================================

// La rejilla es pequeña (15 x 13), así que un BFS por decisión no cuesta
// nada y siempre da el camino más corto en número de celdas.
bool primerPasoHacia(Celda origen,
                     const std::function<bool(Celda)>& esObjetivo,
                     const std::function<bool(Celda)>& puedePisar,
                     int maxPasos,
                     Celda& primerPaso,
                     int& distancia)
{
    const int total = COLUMNAS * FILAS;
    std::vector<int> pasos(total, -1);
    std::vector<Celda> primero(total, origen);   // primer paso del camino a cada celda

    auto indice = [](Celda c) { return c.fila * COLUMNAS + c.col; };
    auto dentro = [](Celda c) {
        return c.col >= 0 && c.col < COLUMNAS && c.fila >= 0 && c.fila < FILAS;
    };

    if (!dentro(origen)) return false;

    std::queue<Celda> cola;
    pasos[indice(origen)] = 0;
    cola.push(origen);

    while (!cola.empty()) {
        Celda actual = cola.front();
        cola.pop();

        int n = pasos[indice(actual)];
        if (n >= maxPasos) continue;

        for (const auto& direccion : DIRECCIONES) {
            Celda vecina = { actual.col + direccion[0], actual.fila + direccion[1] };
            if (!dentro(vecina) || pasos[indice(vecina)] != -1) continue;
            if (!puedePisar(vecina)) continue;

            pasos[indice(vecina)] = n + 1;
            primero[indice(vecina)] = (n == 0) ? vecina : primero[indice(actual)];

            if (esObjetivo(vecina)) {
                primerPaso = primero[indice(vecina)];
                distancia = n + 1;
                return true;
            }
            cola.push(vecina);
        }
    }
    return false;
}
