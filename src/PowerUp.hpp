#pragma once
// ---------------------------------------------------------------------------
// PowerUp.hpp — Power-ups
//
// Los power-ups se esconden bajo algunos bloques al empezar la partida. Cuando
// una explosión rompe el bloque, el power-up queda a la vista y el primer
// jugador que pisa la celda se lo lleva. Si una explosión posterior lo alcanza
// mientras está en el suelo, se pierde (como en el Bomberman original).
//
// El gestor no sabe nada de bombas ni de jugadores: MainGameState le pasa las
// celdas en llamas y le pregunta qué hay en la celda de cada jugador.
// ---------------------------------------------------------------------------
#include <Tablero.hpp>
#include <Constantes.hpp>
#include <vector>

enum class TipoPowerUp {
    BombaExtra,   // una bomba más a la vez
    Alcance,      // la llama llega una celda más lejos
    Velocidad     // el jugador anda más rápido
};

struct PowerUp {
    Celda celda;
    TipoPowerUp tipo;
    bool visible = false;         // false mientras haya un bloque encima
    float tiempoVisible = 0.0f;   // segundos desde que quedó a la vista
};

class GestorPowerUps {
public:
    // Elige con la semilla qué bloques esconden un power-up. Hay que llamarla
    // justo después de tablero.generar(). Con la misma semilla y el mismo
    // tablero, los power-ups salen siempre en el mismo sitio.
    void repartir(const Tablero& tablero, unsigned int semilla,
                  int cantidad = NUM_POWERUPS);

    // Destapa los power-ups cuyo bloque ya no existe y elimina los que están a
    // la vista y los alcanza una explosión. La explosión que rompe el bloque
    // no cuenta: solo las que llegan después.
    void actualizar(float deltaTime, const Tablero& tablero,
                    const std::vector<Celda>& celdasEnLlamas);

    // Si en la celda hay un power-up visible lo quita, lo copia en 'tipo' y
    // devuelve true. Si no hay nada (o sigue tapado), devuelve false.
    bool recoger(Celda celda, TipoPowerUp& tipo);

    void dibujar() const;
    void dibujarDepuracion() const;   // marca también los que siguen tapados

    const std::vector<PowerUp>& getPowerUps() const { return powerUps; }
    void vaciar() { powerUps.clear(); }

private:
    std::vector<PowerUp> powerUps;
};
