// ---------------------------------------------------------------------------
// Niveles.cpp — Modo un jugador (incidencia #17)
//
// La dificultad sube de un nivel a otro: más enemigos, más perseguidores y
// menos espacio libre. Los power-ups también aumentan para compensar.
// ---------------------------------------------------------------------------
#include <Niveles.hpp>

namespace {

const Nivel NIVELES[NUM_NIVELES] = {
    {
        "Primeros pasos",
        {
            "###############",
            "#J..B...B.....#",
            "#.#.#B#.#.#B#.#",
            "#..B....B...B.#",
            "#.#.#.#B#.#.#.#",
            "#B....G.....B.#",
            "#.#B#.#.#.#B#.#",
            "#...B...B.....#",
            "#.#.#.#B#.#.#.#",
            "#.B.....B..G..#",
            "#.#B#.#.#.#B#.#",
            "#.....B.....G.#",
            "###############",
        },
        3, 101
    },
    {
        "Pasillos",
        {
            "###############",
            "#J.BB.B.B.BB..#",
            "#.#B#B#.#B#.#B#",
            "#..B...B.B..B.#",
            "#B#.#B#B#.#B#.#",
            "#..B.B..G.B..B#",
            "#B#B#.#B#B#.#.#",
            "#.B..B.B...B.B#",
            "#B#.#B#.#B#B#.#",
            "#..B.G.B.B..P.#",
            "#B#B#.#B#.#B#B#",
            "#..B.B...B.G..#",
            "###############",
        },
        4, 202
    },
    {
        "Fortaleza",
        {
            "###############",
            "#J..B.....B...#",
            "#.#B#B#B#B#B#.#",
            "#.B#########B.#",
            "#B#B.B.G.B.#B.#",
            "#..#.##.##.#..#",
            "#B#B.B.P.B.B#B#",
            "#..#.##.##.#..#",
            "#B##.B.B.B.#B.#",
            "#.B####.####B.#",
            "#.#B#B#.#B#B#.#",
            "#G..B..B..B.P.#",
            "###############",
        },
        5, 303
    },
    {
        "Laberinto",
        {
            "###############",
            "#J..#..B..#.G.#",
            "#.#.#.###.#.#.#",
            "#.#B..B.#B..#.#",
            "#B###.#.#.###B#",
            "#...#B#P..#...#",
            "###.#.#####.#B#",
            "#.B.B...G.B.#.#",
            "#.#####.#.###.#",
            "#.B.P.#.#B..B.#",
            "#B###.#.###.#B#",
            "#..B..B...B.P.#",
            "###############",
        },
        6, 404
    },
    {
        "Caos",
        {
            "###############",
            "#J.BBBBBBBBBP.#",
            "#.#B#B#B#B#B#.#",
            "#.BB.BBBBB.BBB#",
            "#B#B#B#.#B#B#B#",
            "#BBBBB.G.BBBBB#",
            "#B#B#.#P#.#B#B#",
            "#BBBBB...BBBBB#",
            "#B#B#B#.#B#B#B#",
            "#BB.BBBBBBB.BB#",
            "#B#B#B#B#B#B#.#",
            "#G.BBP.BBBBBP.#",
            "###############",
        },
        7, 505
    },
};

} // namespace

const Nivel& getNivel(int numero)
{
    if (numero < 1 || numero > NUM_NIVELES) {
        numero = 1;
    }
    return NIVELES[numero - 1];
}

Celda inicioJugador(const Nivel& nivel)
{
    for (int fila = 0; fila < FILAS; fila++) {
        for (int col = 0; col < COLUMNAS; col++) {
            if (nivel.mapa[fila][col] == 'J') {
                return { col, fila };
            }
        }
    }
    return { 1, 1 };
}

std::vector<EnemigoNivel> enemigosDelNivel(const Nivel& nivel)
{
    std::vector<EnemigoNivel> enemigos;
    for (int fila = 0; fila < FILAS; fila++) {
        for (int col = 0; col < COLUMNAS; col++) {
            char c = nivel.mapa[fila][col];
            if (c == 'G') {
                enemigos.push_back({ TipoEnemigo::Globo, { col, fila } });
            } else if (c == 'P') {
                enemigos.push_back({ TipoEnemigo::Perseguidor, { col, fila } });
            }
        }
    }
    return enemigos;
}
