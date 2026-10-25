#pragma once
// ---------------------------------------------------------------------------
// Tablero.hpp — Paquete 1 (Escenario)
//
// Este fichero es el CONTRATO con el resto del equipo: jugador (P2),
// bombas (P3) y enemigos/flujo (P4) solo usan lo que hay en la parte public.
// Si hay que cambiar una firma, se avisa al equipo antes de hacerlo.
//
// Coordenadas:
//   - La lógica trabaja en CELDAS: (col, fila). col = eje X, fila = eje Y.
//     (0,0) es la esquina superior izquierda del tablero (un muro del borde).
//   - raylib dibuja en PÍXELES: Vector2 {x, y}.
//   - Para pasar de un mundo a otro usa SIEMPRE celdaAPixel / pixelACelda.
// ---------------------------------------------------------------------------
#include <vector>
#include <Constantes.hpp>

extern "C" {
    #include <raylib.h>
}

class Jugador;
class Bomba;

// Lo que puede haber en una celda del tablero. Solo lo ESTRUCTURAL:
// bombas, llamas, jugador y enemigos viven en sus propias clases.
enum class TipoCelda { Vacio, Muro, Bloque };

// Una posición en la rejilla.
struct Celda {
    int col;
    int fila;
};

enum class TipoEventoTablero {
    MoverJugador,
    PonerBomba
};

struct EventoTablero {
    TipoEventoTablero tipo;
    int jugadorId = 0;
    Vector2 direccion = { 0.0f, 0.0f };
};

class Tablero {
public:
    Tablero();

    // --- Ciclo de vida -----------------------------------------------------
    // Rellena el tablero entero: borde, pilares, zonas de inicio libres y
    // bloques aleatorios. Se llama al empezar cada partida (y al reiniciar).
    // Con la misma semilla genera exactamente el mismo tablero.
    void generar(unsigned int semilla);
    unsigned int getSemilla() const;

    // --- Eventos ------------------------------------------------------------
    bool manejarEvento(const EventoTablero& evento, Jugador& jugador1, Jugador& jugador2,
                       std::vector<Bomba>& bombasJugador1, std::vector<Bomba>& bombasJugador2,
                       float deltaTime);

    // --- Dibujo ------------------------------------------------------------
    void dibujar() const;
    void dibujarDepuracion() const;   // rejilla, coordenadas y semilla

    // --- Consultas ---------------------------------------------------------
    // Fuera de los límites devuelve Muro: así nadie se sale del tablero.
    TipoCelda getCelda(int col, int fila) const;
    bool dentroDeLimites(int col, int fila) const;
    bool esTransitable(int col, int fila) const;

    // ¿La caja (en píxeles) toca alguna celda no transitable?
    // Requisito: la caja debe ser más pequeña que una celda.
    bool colisiona(Rectangle caja) const;
    bool colisionaConBombas(Rectangle caja, const std::vector<Celda>& bombas,
                            const Celda* bombaIgnorada = nullptr) const;

    // --- Modificación ------------------------------------------------------
    // Si en (col, fila) hay un Bloque, lo convierte en Vacio y devuelve true.
    // En cualquier otro caso (muro, vacío, fuera de límites) no hace nada y
    // devuelve false. La usa el Paquete 3 al propagar una explosión.
    bool destruirBloque(int col, int fila);
    bool moverJugador(Jugador& jugador, const std::vector<Celda>& bombas,
                      float deltaTime, Vector2 direccion);
    bool intentarPonerBomba(Jugador& jugador, std::vector<Bomba>& bombas) const;

    // --- Conversiones celda <-> píxel ----------------------------------------
    static Vector2 celdaAPixel(int col, int fila);    // esquina sup. izquierda
    static Vector2 centroDeCelda(int col, int fila);  // centro de la celda
    static Celda   pixelACelda(Vector2 p);           // celda que contiene p

private:
    std::vector<TipoCelda> celdas;   // COLUMNAS * FILAS, recorrido fila a fila
    unsigned int semilla;

    int  indice(int col, int fila) const;             // (col, fila) -> posición en el vector
    void setCelda(int col, int fila, TipoCelda tipo);
    bool esZonaLibre(int col, int fila) const;        // zonas de aparición
};
