// ---------------------------------------------------------------------------
// test_ia_enemigos.cpp — Pruebas automáticas de la IA de los enemigos
//
// Programa independiente del juego (no abre ventana). Compilar y ejecutar
// desde la raíz del repo:
//
//   Linux:
//     g++ -o bin/test_ia_enemigos tests/test_ia_enemigos.cpp src/Enemigo.cpp src/IAEnemigo.cpp src/Tablero.cpp -I src/ -I vendor/include/ -L vendor/lib -lraylib -lGL -lm -lpthread -lrt -lX11
//     ./bin/test_ia_enemigos
//   Windows:
//     g++ -o bin/test_ia_enemigos.exe tests/test_ia_enemigos.cpp src/Enemigo.cpp src/IAEnemigo.cpp src/Tablero.cpp -I src/ -I vendor/include/ -L vendor/lib -lraylib -lopengl32 -lgdi32 -lwinmm
//     bin/test_ia_enemigos.exe
//
// Devuelve 0 si todas las pruebas pasan y 1 si alguna falla.
// ---------------------------------------------------------------------------
#include <Enemigo.hpp>
#include <IAEnemigo.hpp>
#include <Tablero.hpp>
#include <cstdlib>
#include <iostream>
#include <string>

static int fallos = 0;
static int pruebas = 0;

static void comprobar(bool condicion, const std::string& nombre)
{
    pruebas++;
    if (condicion) {
        std::cout << "[OK]    " << nombre << "\n";
    } else {
        std::cout << "[FALLO] " << nombre << "\n";
        fallos++;
    }
}

static bool igual(Celda a, Celda b) { return a.col == b.col && a.fila == b.fila; }

// Tablero sin bloques: solo el borde y los pilares. Así los caminos son
// fáciles de calcular a mano.
static Tablero tableroAbierto()
{
    Tablero tablero;
    tablero.generar(42);
    for (int fila = 0; fila < FILAS; fila++)
        for (int col = 0; col < COLUMNAS; col++)
            tablero.destruirBloque(col, fila);
    return tablero;
}

// Ejecuta 'segundos' de juego a 60 FPS.
static void simular(Enemigo& enemigo, const Tablero& tablero, const MapaPeligro& mapa,
                    const std::vector<Celda>& objetivos, float segundos)
{
    const float paso = 1.0f / 60.0f;
    for (float t = 0.0f; t < segundos; t += paso) {
        enemigo.update(paso, tablero, mapa, objetivos);
    }
}

// --- Árbol de decisión --------------------------------------------------------

static void pruebasArbolDeDecision()
{
    Percepcion p = {};
    p.vePeligro = true;
    p.puedePerseguir = true;
    p.enPeligro = true;
    p.hayRefugio = true;
    p.jugadorAlAlcance = true;
    p.tieneSalida = true;
    comprobar(decidir(p) == AccionEnemigo::Huir,
              "Árbol: en peligro y con refugio, huir va antes que perseguir");

    p.hayRefugio = false;
    comprobar(decidir(p) == AccionEnemigo::Perseguir,
              "Árbol: sin refugio posible, sigue con la rama de perseguir");

    Percepcion globo = {};
    globo.enPeligro = true;
    globo.hayRefugio = true;
    globo.jugadorAlAlcance = true;
    globo.tieneSalida = true;
    comprobar(decidir(globo) == AccionEnemigo::Deambular,
              "Árbol: el globo ni huye ni persigue, solo pasea");

    Percepcion encerrado = {};
    encerrado.vePeligro = true;
    encerrado.puedePerseguir = true;
    comprobar(decidir(encerrado) == AccionEnemigo::Esperar,
              "Árbol: sin salida, espera");
}

// --- Mapa de peligro ----------------------------------------------------------

static void pruebasMapaDePeligro()
{
    Tablero abierto = tableroAbierto();
    MapaPeligro mapa;

    mapa.calcular(abierto, { { { 5, 5 }, 2 } }, {});
    comprobar(mapa.hayBomba({ 5, 5 }) && mapa.hayPeligro({ 5, 5 }) &&
              mapa.hayPeligro({ 7, 5 }) && mapa.hayPeligro({ 5, 3 }) &&
              !mapa.hayPeligro({ 8, 5 }) && !mapa.hayPeligro({ 6, 6 }),
              "Peligro: cruz del alcance de la bomba y nada más");

    mapa.calcular(abierto, { { { 2, 1 }, 3 } }, {});
    comprobar(!mapa.hayPeligro({ 2, 2 }) && !mapa.hayPeligro({ 2, 3 }) &&
              mapa.hayPeligro({ 5, 1 }),
              "Peligro: un pilar corta la llama hacia abajo");

    Tablero conBloques;
    conBloques.generar(42);   // (3,1) es un bloque
    mapa.calcular(conBloques, { { { 1, 1 }, 3 } }, {});
    comprobar(mapa.hayPeligro({ 3, 1 }) && !mapa.hayPeligro({ 4, 1 }),
              "Peligro: el bloque recibe la llama pero la frena");

    mapa.calcular(abierto, {}, { { 9, 9 } });
    comprobar(mapa.hayPeligro({ 9, 9 }) && !mapa.hayBomba({ 9, 9 }),
              "Peligro: las celdas en llamas también cuentan");

    comprobar(!mapa.esPisable(abierto, { 2, 2 }) && mapa.esPisable(abierto, { 1, 1 }),
              "Peligro: un pilar no es pisable y una celda vacía sí");
}

// --- Búsqueda de caminos ----------------------------------------------------

static void pruebasBusqueda()
{
    Tablero abierto = tableroAbierto();
    MapaPeligro mapa;
    mapa.calcular(abierto, {}, {});
    auto pisable = [&](Celda c) { return mapa.esPisable(abierto, c); };

    Celda paso = { -1, -1 };
    int distancia = 0;
    bool hay = primerPasoHacia({ 1, 1 }, [](Celda c) { return igual(c, { 5, 1 }); },
                               pisable, 20, paso, distancia);
    comprobar(hay && igual(paso, { 2, 1 }) && distancia == 4,
              "BFS: camino recto de (1,1) a (5,1) en 4 pasos");

    mapa.calcular(abierto, { { { 2, 1 }, 1 } }, {});
    hay = primerPasoHacia({ 1, 1 }, [](Celda c) { return igual(c, { 3, 1 }); },
                          pisable, 20, paso, distancia);
    comprobar(hay && igual(paso, { 1, 2 }) && distancia == 6,
              "BFS: rodea una bomba que corta el camino recto");

    mapa.calcular(abierto, {}, {});
    hay = primerPasoHacia({ 1, 1 }, [](Celda c) { return igual(c, { 5, 1 }); },
                          pisable, 3, paso, distancia);
    comprobar(!hay, "BFS: no encuentra nada más allá de maxPasos");
}

// --- Comportamiento de los enemigos ----------------------------------------

static void pruebaPerseguidorAlcanzaAlJugador()
{
    Tablero abierto = tableroAbierto();
    MapaPeligro mapa;
    mapa.calcular(abierto, {}, {});

    Enemigo enemigo(TipoEnemigo::Perseguidor, { 1, 1 }, 1);
    simular(enemigo, abierto, mapa, { { 5, 3 } }, 0.1f);
    bool persigue = enemigo.getAccion() == AccionEnemigo::Perseguir;

    // 6 pasos de 48 px a 80 px/s: menos de 4 segundos.
    simular(enemigo, abierto, mapa, { { 5, 3 } }, 4.0f);
    comprobar(persigue && igual(enemigo.getCelda(), { 5, 3 }),
              "El perseguidor llega hasta la celda del jugador");
}

static void pruebaPerseguidorNoVeLejos()
{
    Tablero abierto = tableroAbierto();
    MapaPeligro mapa;
    mapa.calcular(abierto, {}, {});

    // De (1,1) a (13,11) hay 22 pasos, más que RADIO_PERSECUCION.
    Enemigo enemigo(TipoEnemigo::Perseguidor, { 1, 1 }, 1);
    simular(enemigo, abierto, mapa, { { 13, 11 } }, 0.1f);
    comprobar(enemigo.getAccion() != AccionEnemigo::Perseguir,
              "El perseguidor no persigue a un jugador fuera de su radio");
}

static void pruebaPerseguidorHuyeDeUnaBomba()
{
    Tablero abierto = tableroAbierto();
    MapaPeligro mapa;
    mapa.calcular(abierto, { { { 5, 1 }, 2 } }, {});   // barre de (3,1) a (7,1)

    Enemigo enemigo(TipoEnemigo::Perseguidor, { 3, 1 }, 1);
    simular(enemigo, abierto, mapa, {}, 0.1f);
    bool huye = enemigo.getAccion() == AccionEnemigo::Huir;
    simular(enemigo, abierto, mapa, {}, 2.0f);

    comprobar(huye && !mapa.hayPeligro(enemigo.getCelda()),
              "El perseguidor sale de la zona que va a barrer una bomba");
}

static void pruebaGloboNoAtraviesaBombas()
{
    Tablero abierto = tableroAbierto();
    MapaPeligro mapa;
    mapa.calcular(abierto, { { { 2, 1 }, 1 }, { { 1, 2 }, 1 } }, {});

    Enemigo globo(TipoEnemigo::Globo, { 1, 1 }, 1);
    simular(globo, abierto, mapa, {}, 2.0f);

    Vector2 centro = Tablero::centroDeCelda(1, 1);
    comprobar(igual(globo.getCelda(), { 1, 1 }) &&
              globo.getPosicion().x == centro.x && globo.getPosicion().y == centro.y &&
              globo.getAccion() == AccionEnemigo::Esperar,
              "Un globo encerrado entre bombas se queda quieto");
}

static void pruebaMediaVueltaAnteUnaBomba()
{
    Tablero abierto = tableroAbierto();
    MapaPeligro mapa;
    mapa.calcular(abierto, { { { 1, 2 }, 1 } }, {});   // solo puede ir a (2,1)

    Enemigo globo(TipoEnemigo::Globo, { 1, 1 }, 1);
    simular(globo, abierto, mapa, {}, 0.2f);
    bool sale = globo.getPosicion().x > Tablero::centroDeCelda(1, 1).x;

    // Le ponen una bomba justo delante.
    mapa.calcular(abierto, { { { 1, 2 }, 1 }, { { 2, 1 }, 1 } }, {});
    simular(globo, abierto, mapa, {}, 2.0f);

    Vector2 centro = Tablero::centroDeCelda(1, 1);
    comprobar(sale && globo.getPosicion().x == centro.x && globo.getPosicion().y == centro.y,
              "Si le ponen una bomba delante, vuelve a su celda");
}

static void pruebaPaseoReproducible()
{
    Tablero tablero;
    tablero.generar(42);
    MapaPeligro mapa;
    mapa.calcular(tablero, {}, {});

    Enemigo a(TipoEnemigo::Globo, { 1, FILAS - 2 }, 99);
    Enemigo b(TipoEnemigo::Globo, { 1, FILAS - 2 }, 99);
    simular(a, tablero, mapa, {}, 10.0f);
    simular(b, tablero, mapa, {}, 10.0f);

    comprobar(a.getPosicion().x == b.getPosicion().x && a.getPosicion().y == b.getPosicion().y,
              "Con la misma semilla dos globos hacen el mismo paseo");
}

static void pruebaPaseoSinAtravesarParedes()
{
    Tablero abierto = tableroAbierto();
    MapaPeligro mapa;
    mapa.calcular(abierto, {}, {});

    Enemigo globo(TipoEnemigo::Globo, { 7, 5 }, 3);
    bool siempreEnSuelo = true;
    bool seMueve = false;
    Vector2 inicio = globo.getPosicion();

    const float paso = 1.0f / 60.0f;
    for (int i = 0; i < 60 * 30; i++) {
        globo.update(paso, abierto, mapa, {});
        Celda c = globo.getCelda();
        if (!abierto.esTransitable(c.col, c.fila)) siempreEnSuelo = false;
        if (globo.getPosicion().x != inicio.x || globo.getPosicion().y != inicio.y) seMueve = true;
    }

    comprobar(seMueve && siempreEnSuelo,
              "El globo pasea 30 segundos sin meterse en muros ni pilares");
}

int main()
{
    pruebasArbolDeDecision();
    pruebasMapaDePeligro();
    pruebasBusqueda();
    pruebaPerseguidorAlcanzaAlJugador();
    pruebaPerseguidorNoVeLejos();
    pruebaPerseguidorHuyeDeUnaBomba();
    pruebaGloboNoAtraviesaBombas();
    pruebaMediaVueltaAnteUnaBomba();
    pruebaPaseoReproducible();
    pruebaPaseoSinAtravesarParedes();

    std::cout << "\n" << (pruebas - fallos) << "/" << pruebas << " pruebas superadas\n";
    return fallos == 0 ? 0 : 1;
}
