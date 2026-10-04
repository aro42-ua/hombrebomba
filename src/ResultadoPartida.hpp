#pragma once

enum class ResultadoPartida {
    EnCurso,
    Empate,
    Jugador1,
    Jugador2
};

ResultadoPartida calcularResultadoPartida(
    bool jugador1Vivo,
    bool jugador2Vivo
);