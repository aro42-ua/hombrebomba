#include <ResultadoPartida.hpp>

ResultadoPartida calcularResultadoPartida(
    bool jugador1Vivo,
    bool jugador2Vivo
)
{
    if (jugador1Vivo && jugador2Vivo) {
        return ResultadoPartida::EnCurso;
    }

    if (!jugador1Vivo && !jugador2Vivo) {
        return ResultadoPartida::Empate;
    }

    if (jugador1Vivo) {
        return ResultadoPartida::Jugador1;
    }

    return ResultadoPartida::Jugador2;
}