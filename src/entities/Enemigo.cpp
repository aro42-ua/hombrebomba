// ---------------------------------------------------------------------------
// Enemigo.cpp — Enemigos
// ---------------------------------------------------------------------------
#include <Enemigo.hpp>
#include <Constantes.hpp>
#include <cmath>

Enemigo::Enemigo(TipoEnemigo tipo, Celda inicio, unsigned int semilla)
    : tipo(tipo),
      posicion(Tablero::centroDeCelda(inicio.col, inicio.fila)),
      origen(inicio),
      destino(inicio),
      velocidad(tipo == TipoEnemigo::Globo ? VELOCIDAD_GLOBO : VELOCIDAD_PERSEGUIDOR),
      generador(semilla)
{
}

bool Enemigo::enMovimiento() const
{
    return origen.col != destino.col || origen.fila != destino.fila;
}

Celda Enemigo::getCelda() const
{
    return Tablero::pixelACelda(posicion);
}

Rectangle Enemigo::getCajaColision() const
{
    return { posicion.x - TAM_ENEMIGO / 2.0f, posicion.y - TAM_ENEMIGO / 2.0f,
             (float)TAM_ENEMIGO, (float)TAM_ENEMIGO };
}

// ===========================================================================
// Movimiento
// ===========================================================================

void Enemigo::update(float deltaTime, const Tablero& tablero,
                     const MapaPeligro& mapa, const std::vector<Celda>& objetivos)
{
    if (!vivo) return;

    // Si alguien pone una bomba en la celda a la que iba, da media vuelta.
    if (enMovimiento() && !mapa.esPisable(tablero, destino)) {
        std::swap(origen, destino);
        dirCol = -dirCol;
        dirFila = -dirFila;
    }

    if (!enMovimiento()) {
        elegirSiguienteCelda(tablero, mapa, objetivos);
    }

    if (enMovimiento()) {
        Vector2 meta = Tablero::centroDeCelda(destino.col, destino.fila);
        avanzarHacia(meta, velocidad * deltaTime);

        // Al llegar al centro decide ya el siguiente paso, para no quedarse
        // un fotograma parado en cada celda.
        if (posicion.x == meta.x && posicion.y == meta.y) {
            origen = destino;
            elegirSiguienteCelda(tablero, mapa, objetivos);
        }
    }
}

// Seek: avanza hacia la meta a velocidad constante. Si la meta está más cerca
// de lo que recorrería en este fotograma, se coloca justo encima (así no se
// pasa del centro de la celda ni oscila alrededor).
void Enemigo::avanzarHacia(Vector2 meta, float distanciaMaxima)
{
    float dx = meta.x - posicion.x;
    float dy = meta.y - posicion.y;
    float distancia = std::sqrt(dx * dx + dy * dy);

    if (distancia <= distanciaMaxima || distancia == 0.0f) {
        posicion = meta;
        return;
    }
    posicion.x += dx / distancia * distanciaMaxima;
    posicion.y += dy / distancia * distanciaMaxima;
}

// ===========================================================================
// Decisión
// ===========================================================================

void Enemigo::elegirSiguienteCelda(const Tablero& tablero, const MapaPeligro& mapa,
                                   const std::vector<Celda>& objetivos)
{
    const Celda aqui = origen;
    const bool esPerseguidor = (tipo == TipoEnemigo::Perseguidor);

    auto pisable = [&](Celda c) { return mapa.esPisable(tablero, c); };
    auto seguro  = [&](Celda c) { return pisable(c) && !mapa.hayPeligro(c); };
    auto aSalvo  = [&](Celda c) { return !mapa.hayPeligro(c); };
    auto esJugador = [&](Celda c) {
        for (const Celda& o : objetivos) {
            if (o.col == c.col && o.fila == c.fila) return true;
        }
        return false;
    };

    // --- Percepción -------------------------------------------------------
    Percepcion p = {};
    p.vePeligro = esPerseguidor;
    p.puedePerseguir = esPerseguidor;
    p.enPeligro = mapa.hayPeligro(aqui);

    Celda pasoRefugio = aqui;
    Celda pasoJugador = aqui;
    int distancia = 0;

    // Para huir puede cruzar celdas peligrosas (ya está en una); para
    // perseguir, no: prefiere no alcanzar al jugador antes que morir.
    if (p.vePeligro && p.enPeligro) {
        p.hayRefugio = primerPasoHacia(aqui, aSalvo, pisable, RADIO_HUIDA,
                                       pasoRefugio, distancia);
    }
    if (p.puedePerseguir) {
        // Si ya está encima del jugador, "perseguir" es quedarse ahí.
        p.jugadorAlAlcance = esJugador(aqui) ||
                             primerPasoHacia(aqui, esJugador, seguro,
                                             RADIO_PERSECUCION, pasoJugador, distancia);
    }
    for (const auto& d : DIRECCIONES) {
        if (pisable({ aqui.col + d[0], aqui.fila + d[1] })) {
            p.tieneSalida = true;
        }
    }

    // --- Decisión y acción ------------------------------------------------
    accion = decidir(p);

    switch (accion) {
        case AccionEnemigo::Huir:      destino = pasoRefugio; break;
        case AccionEnemigo::Perseguir: destino = pasoJugador; break;
        case AccionEnemigo::Deambular: destino = elegirPaseo(tablero, mapa); break;
        case AccionEnemigo::Esperar:   destino = aqui; break;
    }

    if (!enMovimiento()) {
        if (accion != AccionEnemigo::Perseguir) {
            accion = AccionEnemigo::Esperar;
        }
        return;
    }
    dirCol = destino.col - aqui.col;
    dirFila = destino.fila - aqui.fila;
}

// Paseo: casi siempre sigue recto; en los cruces gira al azar y solo da media
// vuelta si no le queda otra. El perseguidor, además, no se mete en celdas
// que vaya a barrer una bomba: si todas las salidas lo son, se queda quieto.
Celda Enemigo::elegirPaseo(const Tablero& tablero, const MapaPeligro& mapa)
{
    const Celda aqui = origen;

    std::vector<Celda> opciones;
    for (const auto& d : DIRECCIONES) {
        Celda c = { aqui.col + d[0], aqui.fila + d[1] };
        if (!mapa.esPisable(tablero, c)) continue;
        if (tipo == TipoEnemigo::Perseguidor && mapa.hayPeligro(c)) continue;
        opciones.push_back(c);
    }
    if (opciones.empty()) return aqui;

    auto contiene = [&](const std::vector<Celda>& lista, Celda c) {
        for (const Celda& x : lista) {
            if (x.col == c.col && x.fila == c.fila) return true;
        }
        return false;
    };

    bool teniaDireccion = (dirCol != 0 || dirFila != 0);
    Celda recto = { aqui.col + dirCol, aqui.fila + dirFila };
    if (teniaDireccion && contiene(opciones, recto) &&
        static_cast<int>(generador() % 100) < PROB_SEGUIR_RECTO) {
        return recto;
    }

    Celda atras = { aqui.col - dirCol, aqui.fila - dirFila };
    std::vector<Celda> sinMediaVuelta;
    for (const Celda& c : opciones) {
        if (!teniaDireccion || c.col != atras.col || c.fila != atras.fila) {
            sinMediaVuelta.push_back(c);
        }
    }
    const std::vector<Celda>& candidatas = sinMediaVuelta.empty() ? opciones : sinMediaVuelta;
    return candidatas[generador() % candidatas.size()];
}

// ===========================================================================
// Dibujo (figuras básicas)
// ===========================================================================

void Enemigo::render() const
{
    if (!vivo) return;

    if (tipo == TipoEnemigo::Globo) {
        DrawCircleV(posicion, TAM_ENEMIGO / 2.0f, { 236, 112, 99, 255 });
    } else {
        // Rombo: polígono de 4 lados.
        DrawPoly(posicion, 4, TAM_ENEMIGO / 2.0f + 3, 0.0f, { 142, 68, 173, 255 });
    }

    // Ojos mirando hacia donde va.
    float mx = dirCol * 3.0f;
    float my = dirFila * 3.0f;
    DrawCircleV({ posicion.x - 6 + mx, posicion.y - 3 + my }, 4, RAYWHITE);
    DrawCircleV({ posicion.x + 6 + mx, posicion.y - 3 + my }, 4, RAYWHITE);
    DrawCircleV({ posicion.x - 6 + mx * 1.5f, posicion.y - 3 + my * 1.5f }, 2, BLACK);
    DrawCircleV({ posicion.x + 6 + mx * 1.5f, posicion.y - 3 + my * 1.5f }, 2, BLACK);
}

void Enemigo::renderDepuracion() const
{
    if (!vivo) return;

    const char* letra = "E";
    switch (accion) {
        case AccionEnemigo::Huir:      letra = "H"; break;
        case AccionEnemigo::Perseguir: letra = "P"; break;
        case AccionEnemigo::Deambular: letra = "D"; break;
        case AccionEnemigo::Esperar:   letra = "E"; break;
    }

    Vector2 meta = Tablero::centroDeCelda(destino.col, destino.fila);
    DrawLineEx(posicion, meta, 2, YELLOW);
    DrawText(letra, (int)posicion.x - 4, (int)posicion.y - TAM_ENEMIGO, 16, YELLOW);
}
