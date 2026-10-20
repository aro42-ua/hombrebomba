#pragma once
// ---------------------------------------------------------------------------
// IAEnemigo.hpp — Inteligencia de los enemigos
//
// Aquí está la parte de la IA que no depende de raylib ni del estado de la
// partida, para poder probarla sin abrir ventana:
//
//   MapaPeligro       qué celdas va a barrer una bomba y dónde hay bombas
//   decidir()         árbol de decisión: qué hace el enemigo en cada cruce
//   primerPasoHacia() búsqueda en anchura (BFS) por la rejilla
//
// Enemigo.cpp usa estas piezas para mover a cada enemigo. El diseño está
// explicado en docs/ia-enemigos.md.
// ---------------------------------------------------------------------------
#include <Tablero.hpp>
#include <functional>
#include <vector>

// Lo que la IA necesita saber de una bomba en cuenta atrás.
struct BombaVista {
    Celda celda;
    int alcance;
};

class MapaPeligro {
public:
    MapaPeligro();

    // Marca como peligrosas las celdas que barrerá cada bomba en cuenta atrás
    // (con la misma regla de propagación que Bomba: el muro corta la llama y
    // el bloque la recibe pero la frena) y las celdas que ya están en llamas.
    void calcular(const Tablero& tablero,
                  const std::vector<BombaVista>& bombas,
                  const std::vector<Celda>& llamas);

    bool hayPeligro(Celda celda) const;
    bool hayBomba(Celda celda) const;

    // Celda por la que un enemigo puede andar: vacía y sin bomba.
    bool esPisable(const Tablero& tablero, Celda celda) const;

private:
    std::vector<char> peligro;   // char y no bool: vector<bool> no da referencias
    std::vector<char> bomba;

    static int indice(Celda c) { return c.fila * COLUMNAS + c.col; }
};

// --- Árbol de decisión ------------------------------------------------------

enum class AccionEnemigo {
    Huir,        // ir a la celda segura más cercana
    Perseguir,   // ir hacia el jugador por el camino más corto
    Deambular,   // pasear por el laberinto
    Esperar      // no hay a dónde ir
};

// Lo que el enemigo percibe al llegar al centro de una celda.
struct Percepcion {
    bool vePeligro;          // el tipo de enemigo sabe reconocer las bombas
    bool puedePerseguir;     // el tipo de enemigo persigue al jugador
    bool enPeligro;          // su celda está en llamas o la barrerá una bomba
    bool hayRefugio;         // hay una celda segura alcanzable
    bool jugadorAlAlcance;   // hay camino seguro a un jugador en RADIO_PERSECUCION pasos
    bool tieneSalida;        // alguna celda vecina es pisable
};

//   ¿Ve el peligro y está en peligro?
//   ├─ sí → ¿hay refugio? ── sí → Huir
//   │                     └─ no → (sigue por la rama de abajo)
//   └─ no → ¿persigue y tiene a un jugador al alcance?
//            ├─ sí → Perseguir
//            └─ no → ¿tiene salida? ── sí → Deambular
//                                    └─ no → Esperar
AccionEnemigo decidir(const Percepcion& p);

// --- Búsqueda de caminos ----------------------------------------------------

// Busca en anchura desde 'origen' la celda más cercana que cumpla
// 'esObjetivo', avanzando solo por celdas que acepte 'puedePisar' y como
// mucho 'maxPasos' pasos. Si la encuentra, deja en 'primerPaso' la celda
// vecina de 'origen' por la que empieza el camino, en 'distancia' su
// longitud y devuelve true. El origen no cuenta como objetivo.
bool primerPasoHacia(Celda origen,
                     const std::function<bool(Celda)>& esObjetivo,
                     const std::function<bool(Celda)>& puedePisar,
                     int maxPasos,
                     Celda& primerPaso,
                     int& distancia);
