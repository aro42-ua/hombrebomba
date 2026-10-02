#pragma once
#include <GameState.hpp>
#include <Tablero.hpp>
#include <ctime>
#include <Jugador.hpp>
#include <Bomba.hpp>
#include <memory>

// Pantalla principal de la partida.
// Contiene el escenario, el jugador y una bomba activa, además de un banco de
// pruebas provisional para el escenario.
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
        Jugador jugador;
        std::unique_ptr<Bomba> bomba;

        // --- Banco de pruebas del escenario (provisional) ---------------
        bool depuracion = false;      // F1: rejilla, coordenadas y semilla
        Rectangle cajaPrueba = {};    // caja del tamaño del jugador que sigue al ratón
        bool cajaChoca = false;       // ¿la caja colisiona con el escenario?
};
