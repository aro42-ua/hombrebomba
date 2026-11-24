#pragma once
#include <GameState.hpp>
#include <Tablero.hpp>
#include <ctime>
#include <Jugador.hpp>
#include <Bomba.hpp>
#include <PowerUp.hpp>
#include <Enemigo.hpp>
#include <IAEnemigo.hpp>
#include <vector>

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
        // Deja tablero, jugadores, bombas y power-ups como al empezar.
        void reiniciarPartida();

        // Pone una bomba en la celda del jugador si no ha llegado a su
        // máximo de bombas y la celda no tiene ya una.
        void intentarPonerBomba(Jugador& jugador, std::vector<Bomba>& bombas);

        // Celdas ocupadas por bombas en cuenta atrás y celdas en llamas.
        std::vector<Celda> celdasConBomba() const;
        std::vector<Celda> celdasEnLlamas() const;

        void recogerPowerUps(Jugador& jugador);

        // Un globo y un perseguidor en las esquinas de abajo y otro globo en
        // una celda vacía lejos de los dos jugadores.
        void crearEnemigos();

        // Mueve a los enemigos con la IA, elimina a los que alcanza una
        // llama y elimina a los jugadores que tocan a un enemigo.
        void actualizarEnemigos(float deltaTime, const std::vector<Celda>& llamas);

        Tablero tablero;
        unsigned int semilla;
        Jugador jugador1;
        Jugador jugador2;
        std::vector<Bomba> bombasJugador1;
        std::vector<Bomba> bombasJugador2;
        GestorPowerUps powerUps;
        std::vector<Enemigo> enemigos;
        MapaPeligro mapaPeligro;

        bool partidaTerminada = false;
        int ganador = 0;

        // --- Banco de pruebas del escenario (provisional) ---------------
        bool depuracion = false;      // F1: rejilla, coordenadas y semilla
        Rectangle cajaPrueba = {};    // caja del tamaño del jugador que sigue al ratón
        bool cajaChoca = false;       // ¿la caja colisiona con el escenario?
};
