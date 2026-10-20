#pragma once
// ---------------------------------------------------------------------------
// Enemigo.hpp — Enemigos
//
// Los enemigos se mueven de centro de celda en centro de celda. Cada vez que
// llegan a uno, perciben lo que tienen alrededor, el árbol de decisión de
// IAEnemigo elige qué hacer y de ahí sale la siguiente celda. Entre dos
// centros se desplazan con un "seek" (steering behavior): van hacia el centro
// de la celda destino a su velocidad sin pasarse.
//
// Hay dos tipos:
//   Globo        lento y despistado: pasea al azar, no reconoce las bombas
//                (solo las esquiva porque no puede atravesarlas).
//   Perseguidor  más rápido: huye de las bombas, persigue al jugador que
//                tenga cerca y, si no ve a nadie, pasea evitando el peligro.
// ---------------------------------------------------------------------------
#include <IAEnemigo.hpp>
#include <Tablero.hpp>
#include <random>
#include <vector>

extern "C" {
    #include <raylib.h>
}

enum class TipoEnemigo { Globo, Perseguidor };

class Enemigo {
public:
    // La semilla hace que el paseo aleatorio sea reproducible.
    Enemigo(TipoEnemigo tipo, Celda inicio, unsigned int semilla);

    // objetivos: celdas de los jugadores vivos.
    void update(float deltaTime, const Tablero& tablero,
                const MapaPeligro& mapa, const std::vector<Celda>& objetivos);
    void render() const;
    void renderDepuracion() const;   // acción actual y celda destino

    Celda getCelda() const;          // celda en la que está su centro
    Vector2 getPosicion() const { return posicion; }
    Rectangle getCajaColision() const;
    TipoEnemigo getTipo() const { return tipo; }
    AccionEnemigo getAccion() const { return accion; }

    bool estaVivo() const { return vivo; }
    void eliminar() { vivo = false; }

private:
    TipoEnemigo tipo;
    Vector2 posicion;
    Celda origen;            // celda de la que sale
    Celda destino;           // celda a la que va (igual que origen si está parado)
    int dirCol = 0;          // última dirección en la que se movió
    int dirFila = 0;
    float velocidad;
    bool vivo = true;
    AccionEnemigo accion = AccionEnemigo::Esperar;
    std::mt19937 generador;

    bool enMovimiento() const;
    void elegirSiguienteCelda(const Tablero& tablero, const MapaPeligro& mapa,
                              const std::vector<Celda>& objetivos);
    Celda elegirPaseo(const Tablero& tablero, const MapaPeligro& mapa);
    void avanzarHacia(Vector2 meta, float distanciaMaxima);
};
