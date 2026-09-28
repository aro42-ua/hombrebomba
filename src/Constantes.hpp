#pragma once
// ---------------------------------------------------------------------------
// Constantes.hpp — Paquete 1 (Escenario)
//
// Todos los números "mágicos" del juego viven aquí. Si cambias el tamaño del
// tablero o de las celdas, el resto del código se adapta solo porque nadie
// escribe 15, 13 o 48 a mano en ningún otro fichero.
//
// Si otro paquete necesita una constante nueva, que la añada aquí en su propia
// rama y PR.
// ---------------------------------------------------------------------------

// --- Tablero (medido en celdas, contando el borde de muros) ----------------
constexpr int COLUMNAS = 15;      // 13 jugables + 2 de borde
constexpr int FILAS    = 13;      // 11 jugables + 2 de borde

// --- Tamaños en píxeles ----------------------------------------------------
constexpr int TAM_CELDA = 48;     // lado de cada celda
constexpr int ALTO_HUD  = 48;     // franja superior reservada al marcador (Paquete 4)

// La ventana se DERIVA del tablero: nunca se escribe a mano.
constexpr int ANCHO_VENTANA = COLUMNAS * TAM_CELDA;
constexpr int ALTO_VENTANA  = FILAS * TAM_CELDA + ALTO_HUD;

// --- Generación ------------------------------------------------------------
constexpr int PROB_BLOQUE = 60;   // % de probabilidad de bloque en una celda libre

// --- Provisional -----------------------------------------------------------
// Tamaño de la caja de colisión del jugador. Lo decide el Paquete 2; aquí solo
// se usa en el banco de pruebas de MainGameState. Debe ser MENOR que TAM_CELDA.
constexpr int TAM_JUGADOR = 36;
