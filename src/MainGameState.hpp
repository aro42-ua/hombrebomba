#pragma once
#include <GameState.hpp>
#include <Tablero.hpp>
#include <ctime>

// Pantalla principal de la partida.
// De momento solo contiene el escenario (Paquete 1) y un banco de pruebas
// para él. El jugador (P2), las bombas (P3) y los enemigos (P4) se irán
// añadiendo aquí.
class MainGameState : public GameState
{
    public:
        // Con la misma semilla se genera el mismo tablero (útil para
        // reproducir una incidencia). Sin semilla, se usa la hora actual.
        MainGameState(unsigned int semilla = static_cast<unsigned int>(std::time(nullptr)));
        ~MainGameState() = default;

        void init() override;
        void handleInput() override;
        void update(float deltaTime) override;
        void render() override;

        void pause(){};
        void resume(){};

    
    private:
        Tablero tablero;
        unsigned int semilla;

        // --- Banco de pruebas del escenario (provisional) ---------------
        bool depuracion = false;      // F1: rejilla, coordenadas y semilla
        Rectangle cajaPrueba = {};    // caja del tamaño del jugador que sigue al ratón
        bool cajaChoca = false;       // ¿la caja colisiona con el escenario?
};
