#pragma once
// ---------------------------------------------------------------------------
// PantallaUnJugadorState.hpp — Modo un jugador (incidencia #17)
//
// Pantalla entre partidas del modo un jugador. MainGameState la abre al
// acabar un nivel y, según el tipo, ENTER lleva a un sitio u otro:
//
//   NivelSuperado  ENTER: siguiente nivel con las mismas vidas
//   VidaPerdida    ENTER: repite el nivel con las vidas que quedan
//   GameOver       ENTER: repite el nivel con todas las vidas
//                  RETROCESO: vuelve al menú
//   Victoria       ENTER: vuelve al menú (se han superado todos los niveles)
// ---------------------------------------------------------------------------
#include <GameState.hpp>

enum class TipoPantallaUnJugador {
    NivelSuperado,
    VidaPerdida,
    GameOver,
    Victoria
};

class PantallaUnJugadorState : public GameState
{
public:
    // nivel: el que se acaba de jugar. vidas: las que le quedan al jugador.
    PantallaUnJugadorState(TipoPantallaUnJugador tipo, int nivel, int vidas,
                           unsigned int semilla);

    void init() override;
    void handleInput() override;
    void update(float deltaTime) override;
    void render() override;

    void pause() override {};
    void resume() override {};

private:
    void jugarNivel(int numero, int vidasPartida);
    void volverAlMenu();

    TipoPantallaUnJugador tipo;
    int nivel;
    int vidas;
    unsigned int semilla;
};
