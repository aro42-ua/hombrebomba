#pragma once
// ---------------------------------------------------------------------------
// Niveles.hpp — Modo un jugador (incidencia #17)
//
// Los niveles del modo un jugador están dibujados a mano con caracteres, una
// cadena por fila del tablero (COLUMNAS x FILAS):
//
//   #  muro            B  bloque destructible     .  vacío
//   J  salida del jugador (celda vacía)
//   G  globo           P  perseguidor            (ambos sobre celda vacía)
//
// Tablero::cargar convierte el mapa en celdas; inicioJugador y
// enemigosDelNivel sacan del mismo mapa dónde empieza cada uno.
// ---------------------------------------------------------------------------
#include <Tablero.hpp>
#include <Enemigo.hpp>
#include <vector>

constexpr int NUM_NIVELES = 5;

struct Nivel {
    const char* nombre;
    const char* mapa[FILAS];       // FILAS cadenas de COLUMNAS caracteres
    int cantidadPowerUps;          // power-ups escondidos bajo los bloques
    unsigned int semilla;          // fija: los power-ups salen siempre igual
};

struct EnemigoNivel {
    TipoEnemigo tipo;
    Celda celda;
};

// numero va de 1 a NUM_NIVELES. Fuera de ese rango devuelve el nivel 1.
const Nivel& getNivel(int numero);

// Celda marcada con 'J'. Si no hay ninguna devuelve { 1, 1 }.
Celda inicioJugador(const Nivel& nivel);

// Enemigos marcados con 'G' y 'P', recorriendo el mapa fila a fila.
std::vector<EnemigoNivel> enemigosDelNivel(const Nivel& nivel);
