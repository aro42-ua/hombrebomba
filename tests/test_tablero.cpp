// ---------------------------------------------------------------------------
// test_tablero.cpp — Pruebas automáticas del escenario (Paquete 1)
//
// Programa independiente del juego (no abre ventana). Compilar y ejecutar
// desde la raíz del repo:
//
//   Linux:
//     g++ -o bin/test_tablero tests/test_tablero.cpp src/Tablero.cpp -I src/ -I vendor/include/ -L vendor/lib -lraylib -lGL -lm -lpthread -lrt -lX11
//     ./bin/test_tablero
//   Windows:
//     g++ -o bin/test_tablero.exe tests/test_tablero.cpp src/Tablero.cpp -I src/ -I vendor/include/ -L vendor/lib -lraylib -lopengl32 -lgdi32 -lwinmm
//     bin/test_tablero.exe
//
// Devuelve 0 si todas las pruebas pasan y 1 si alguna falla.
// ---------------------------------------------------------------------------
#include <Tablero.hpp>
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

static bool esBorde(int col, int fila)
{
    return col == 0 || col == COLUMNAS - 1 || fila == 0 || fila == FILAS - 1;
}

static bool esPilar(int col, int fila)
{
    return col % 2 == 0 && fila % 2 == 0;
}

static bool mismosTableros(const Tablero& a, const Tablero& b)
{
    for (int fila = 0; fila < FILAS; fila++)
        for (int col = 0; col < COLUMNAS; col++)
            if (a.getCelda(col, fila) != b.getCelda(col, fila)) return false;
    return true;
}

// 1. Borde y pilares son siempre muro, y no hay muros en ningún otro sitio.
static void pruebaMurosEnSuSitio()
{
    bool ok = true;
    Tablero t;
    for (unsigned int semilla = 1; semilla <= 2000 && ok; semilla++) {
        t.generar(semilla);
        for (int fila = 0; fila < FILAS; fila++)
            for (int col = 0; col < COLUMNAS; col++) {
                bool debeSerMuro = esBorde(col, fila) || esPilar(col, fila);
                bool esMuro = t.getCelda(col, fila) == TipoCelda::Muro;
                if (debeSerMuro != esMuro) ok = false;
            }
    }
    comprobar(ok, "Borde y pilares son muro, y no hay otros muros (2000 semillas)");
}

// 2. Las zonas de aparición (esquina + 2 vecinas) quedan siempre libres.
static void pruebaZonasLibres()
{
    const Celda zonas[] = {
        { 1, 1 }, { 2, 1 }, { 1, 2 },
        { COLUMNAS - 2, 1 }, { COLUMNAS - 3, 1 }, { COLUMNAS - 2, 2 },
        { 1, FILAS - 2 }, { 2, FILAS - 2 }, { 1, FILAS - 3 },
        { COLUMNAS - 2, FILAS - 2 }, { COLUMNAS - 3, FILAS - 2 }, { COLUMNAS - 2, FILAS - 3 },
    };
    bool ok = true;
    Tablero t;
    for (unsigned int semilla = 1; semilla <= 2000 && ok; semilla++) {
        t.generar(semilla);
        for (const Celda& c : zonas)
            if (!t.esTransitable(c.col, c.fila)) ok = false;
    }
    comprobar(ok, "Las 4 zonas de aparicion quedan libres (2000 semillas)");
}

// 3. La misma semilla genera el mismo tablero.
static void pruebaMismaSemilla()
{
    Tablero a, b;
    a.generar(1234);
    b.generar(1234);
    comprobar(mismosTableros(a, b), "La misma semilla genera el mismo tablero");
}

// 4. Regenerar tras destruir bloques deja el tablero como nuevo (reinicio).
static void pruebaReinicioLimpio()
{
    Tablero a, b;
    a.generar(99);
    for (int fila = 0; fila < FILAS; fila++)
        for (int col = 0; col < COLUMNAS; col++)
            a.destruirBloque(col, fila);
    a.generar(99);
    b.generar(99);
    comprobar(mismosTableros(a, b), "Regenerar tras destruir bloques no arrastra la partida anterior");
}

// 5. Semillas distintas dan tableros distintos (al menos en general).
static void pruebaSemillasDistintas()
{
    Tablero a, b;
    a.generar(1);
    b.generar(2);
    comprobar(!mismosTableros(a, b), "Semillas distintas generan tableros distintos");
}

// 6. Fuera del tablero todo es muro y no transitable.
static void pruebaFueraDeLimites()
{
    Tablero t;
    t.generar(7);
    bool ok = !t.dentroDeLimites(-1, 0) && !t.dentroDeLimites(0, -1)
           && !t.dentroDeLimites(COLUMNAS, 0) && !t.dentroDeLimites(0, FILAS)
           && t.dentroDeLimites(0, 0) && t.dentroDeLimites(COLUMNAS - 1, FILAS - 1)
           && t.getCelda(-5, 3) == TipoCelda::Muro
           && t.getCelda(COLUMNAS + 2, FILAS + 2) == TipoCelda::Muro
           && !t.esTransitable(-1, -1);
    comprobar(ok, "Fuera del tablero hay muro y no se puede pisar");
}

// 7. destruirBloque solo actúa sobre bloques.
static void pruebaDestruirBloque()
{
    Tablero t;
    bool ok = true;
    bool probadoBloque = false;
    t.generar(42);
    for (int fila = 0; fila < FILAS; fila++)
        for (int col = 0; col < COLUMNAS; col++) {
            TipoCelda antes = t.getCelda(col, fila);
            bool destruido = t.destruirBloque(col, fila);
            if (antes == TipoCelda::Bloque) {
                probadoBloque = true;
                if (!destruido || t.getCelda(col, fila) != TipoCelda::Vacio) ok = false;
            } else {
                if (destruido || t.getCelda(col, fila) != antes) ok = false;
            }
        }
    ok = ok && probadoBloque && !t.destruirBloque(-1, 5) && !t.destruirBloque(COLUMNAS, 5);
    comprobar(ok, "destruirBloque solo convierte bloques en vacio");
}

// 8. Conversión celda <-> píxel de ida y vuelta.
static void pruebaConversiones()
{
    bool ok = true;
    for (int fila = 0; fila < FILAS; fila++)
        for (int col = 0; col < COLUMNAS; col++) {
            Celda c1 = Tablero::pixelACelda(Tablero::celdaAPixel(col, fila));
            Celda c2 = Tablero::pixelACelda(Tablero::centroDeCelda(col, fila));
            if (c1.col != col || c1.fila != fila || c2.col != col || c2.fila != fila) ok = false;
        }
    comprobar(ok, "celdaAPixel / centroDeCelda / pixelACelda son coherentes");
}

// 9. Un punto a la izquierda del tablero cae fuera (std::floor, no cast).
static void pruebaPixelNegativo()
{
    Celda c = Tablero::pixelACelda({ -0.5f, ALTO_HUD + 10.0f });
    Celda d = Tablero::pixelACelda({ 10.0f, ALTO_HUD - 0.5f });
    comprobar(c.col == -1 && d.fila == -1, "pixelACelda devuelve -1 justo fuera del tablero");
}

// 10. Una caja pegada al borde de un muro no colisiona; 1 px dentro, sí.
static void pruebaColisionBordes()
{
    Tablero t;
    t.generar(42);
    // Celda (1,1) libre, a su izquierda el muro del borde (0,1).
    Vector2 p = Tablero::celdaAPixel(1, 1);
    Rectangle pegada = { p.x, p.y, (float)TAM_JUGADOR, (float)TAM_JUGADOR };
    Rectangle dentro = { p.x - 1.0f, p.y, (float)TAM_JUGADOR, (float)TAM_JUGADOR };
    comprobar(!t.colisiona(pegada) && t.colisiona(dentro),
              "colisiona: sin falso positivo al rozar, detecta 1 px de solape");
}

// 11. El borde derecho de la caja no cuenta como dentro: una caja cuyo lado
//     derecho coincide con el inicio de un bloque no colisiona; 1 px más, sí.
static void pruebaColisionBordeDerecho()
{
    Tablero t;
    t.generar(42);
    // Con la semilla 42, la fila 1 es "#..BBBBBB.....#": (3,1) es un bloque.
    float xBloque = Tablero::celdaAPixel(3, 1).x;
    float y = Tablero::celdaAPixel(3, 1).y;
    Rectangle pegada = { xBloque - TAM_JUGADOR,        y, (float)TAM_JUGADOR, (float)TAM_JUGADOR };
    Rectangle dentro = { xBloque - TAM_JUGADOR + 1.0f, y, (float)TAM_JUGADOR, (float)TAM_JUGADOR };
    comprobar(t.getCelda(3, 1) == TipoCelda::Bloque && !t.colisiona(pegada) && t.colisiona(dentro),
              "colisiona: el borde derecho de la caja no invade la celda siguiente");
}

int main()
{
    SetTraceLogLevel(LOG_WARNING);   // sin mensajes INFO de raylib

    pruebaMurosEnSuSitio();
    pruebaZonasLibres();
    pruebaMismaSemilla();
    pruebaReinicioLimpio();
    pruebaSemillasDistintas();
    pruebaFueraDeLimites();
    pruebaDestruirBloque();
    pruebaConversiones();
    pruebaPixelNegativo();
    pruebaColisionBordes();
    pruebaColisionBordeDerecho();

    std::cout << "\n" << (pruebas - fallos) << "/" << pruebas << " pruebas superadas\n";
    return fallos == 0 ? 0 : 1;
}
