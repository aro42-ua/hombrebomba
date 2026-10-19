// ---------------------------------------------------------------------------
// test_powerups.cpp — Pruebas automáticas de los power-ups
//
// Programa independiente del juego (no abre ventana). Compilar y ejecutar
// desde la raíz del repo:
//
//   Linux:
//     g++ -o bin/test_powerups tests/test_powerups.cpp src/PowerUp.cpp src/Jugador.cpp src/Bomba.cpp src/Tablero.cpp -I src/ -I vendor/include/ -L vendor/lib -lraylib -lGL -lm -lpthread -lrt -lX11
//     ./bin/test_powerups
//   Windows:
//     g++ -o bin/test_powerups.exe tests/test_powerups.cpp src/PowerUp.cpp src/Jugador.cpp src/Bomba.cpp src/Tablero.cpp -I src/ -I vendor/include/ -L vendor/lib -lraylib -lopengl32 -lgdi32 -lwinmm
//     bin/test_powerups.exe
//
// Devuelve 0 si todas las pruebas pasan y 1 si alguna falla.
// ---------------------------------------------------------------------------
#include <PowerUp.hpp>
#include <Jugador.hpp>
#include <Bomba.hpp>
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

static bool contiene(const std::vector<Celda>& celdas, int col, int fila)
{
    for (const Celda& c : celdas) {
        if (c.col == col && c.fila == fila) return true;
    }
    return false;
}

static int contarBloques(const Tablero& tablero)
{
    int n = 0;
    for (int fila = 0; fila < FILAS; fila++)
        for (int col = 0; col < COLUMNAS; col++)
            if (tablero.getCelda(col, fila) == TipoCelda::Bloque) n++;
    return n;
}

// Avanza el gestor 'segundos' en pasos de un fotograma a 60 FPS.
static void avanzar(GestorPowerUps& gestor, const Tablero& tablero,
                    const std::vector<Celda>& llamas, float segundos)
{
    const float paso = 1.0f / 60.0f;
    for (float t = 0.0f; t < segundos; t += paso) {
        gestor.actualizar(paso, tablero, llamas);
    }
}

// --- Reparto -----------------------------------------------------------------

static void pruebaRepartoEscondidoBajoBloques()
{
    Tablero tablero;
    tablero.generar(42);
    GestorPowerUps gestor;
    gestor.repartir(tablero, 42);

    bool todosBajoBloque = true;
    for (const PowerUp& p : gestor.getPowerUps()) {
        if (p.visible || tablero.getCelda(p.celda.col, p.celda.fila) != TipoCelda::Bloque) {
            todosBajoBloque = false;
        }
    }

    comprobar((int)gestor.getPowerUps().size() == NUM_POWERUPS,
              "Se reparten NUM_POWERUPS power-ups");
    comprobar(todosBajoBloque, "Todos empiezan tapados por un bloque");
}

static void pruebaRepartoSinRepetirCeldas()
{
    Tablero tablero;
    tablero.generar(7);
    GestorPowerUps gestor;
    gestor.repartir(tablero, 7);

    const std::vector<PowerUp>& lista = gestor.getPowerUps();
    bool sinRepetir = true;
    for (size_t i = 0; i < lista.size(); i++)
        for (size_t j = i + 1; j < lista.size(); j++)
            if (lista[i].celda.col == lista[j].celda.col &&
                lista[i].celda.fila == lista[j].celda.fila)
                sinRepetir = false;

    comprobar(sinRepetir, "No hay dos power-ups en la misma celda");
}

static void pruebaRepartoReproducible()
{
    Tablero tablero;
    tablero.generar(1234);
    GestorPowerUps a, b;
    a.repartir(tablero, 1234);
    b.repartir(tablero, 1234);

    bool iguales = a.getPowerUps().size() == b.getPowerUps().size();
    for (size_t i = 0; iguales && i < a.getPowerUps().size(); i++) {
        const PowerUp& pa = a.getPowerUps()[i];
        const PowerUp& pb = b.getPowerUps()[i];
        iguales = pa.celda.col == pb.celda.col && pa.celda.fila == pb.celda.fila
                  && pa.tipo == pb.tipo;
    }

    comprobar(iguales, "La misma semilla reparte los mismos power-ups");
}

static void pruebaRepartoDeTodosLosTipos()
{
    Tablero tablero;
    tablero.generar(42);
    GestorPowerUps gestor;
    gestor.repartir(tablero, 42);

    int bombas = 0, alcance = 0, velocidad = 0;
    for (const PowerUp& p : gestor.getPowerUps()) {
        if (p.tipo == TipoPowerUp::BombaExtra) bombas++;
        if (p.tipo == TipoPowerUp::Alcance) alcance++;
        if (p.tipo == TipoPowerUp::Velocidad) velocidad++;
    }

    comprobar(bombas == 3 && alcance == 3 && velocidad == 3,
              "Con 9 power-ups salen 3 de cada tipo");
}

static void pruebaRepartoLimitadoPorBloques()
{
    Tablero tablero;
    tablero.generar(42);
    GestorPowerUps gestor;
    gestor.repartir(tablero, 42, 10000);

    comprobar((int)gestor.getPowerUps().size() == contarBloques(tablero),
              "Nunca se reparten más power-ups que bloques");
}

// --- Aparición, recogida y destrucción -------------------------------------

static void pruebaApareceAlRomperElBloque()
{
    Tablero tablero;
    tablero.generar(42);
    GestorPowerUps gestor;
    gestor.repartir(tablero, 42, 1);
    Celda c = gestor.getPowerUps()[0].celda;

    gestor.actualizar(0.1f, tablero, {});
    bool tapadoConBloque = !gestor.getPowerUps()[0].visible;

    tablero.destruirBloque(c.col, c.fila);
    gestor.actualizar(0.1f, tablero, {});
    bool visibleSinBloque = gestor.getPowerUps()[0].visible;

    comprobar(tapadoConBloque, "Con el bloque encima el power-up no se ve");
    comprobar(visibleSinBloque, "Al romper el bloque el power-up aparece");
}

static void pruebaRecoger()
{
    Tablero tablero;
    tablero.generar(42);
    GestorPowerUps gestor;
    gestor.repartir(tablero, 42, 1);
    Celda c = gestor.getPowerUps()[0].celda;
    TipoPowerUp esperado = gestor.getPowerUps()[0].tipo;
    TipoPowerUp tipo;

    bool noSeRecogeTapado = !gestor.recoger(c, tipo);

    tablero.destruirBloque(c.col, c.fila);
    gestor.actualizar(0.1f, tablero, {});
    bool noHayEnOtraCelda = !gestor.recoger({ 1, 1 }, tipo);
    bool seRecoge = gestor.recoger(c, tipo) && tipo == esperado;
    bool desaparece = gestor.getPowerUps().empty() && !gestor.recoger(c, tipo);

    comprobar(noSeRecogeTapado, "Un power-up tapado no se puede recoger");
    comprobar(noHayEnOtraCelda, "recoger() en una celda sin power-up devuelve false");
    comprobar(seRecoge && desaparece, "Al recogerlo se devuelve su tipo y desaparece");
}

static void pruebaLaExplosionQueLoDestapaNoLoRompe()
{
    Tablero tablero;
    tablero.generar(42);
    GestorPowerUps gestor;
    gestor.repartir(tablero, 42, 1);
    Celda c = gestor.getPowerUps()[0].celda;

    // La bomba rompe el bloque y sus llamas siguen en la celda durante
    // DURACION_EXPLOSION.
    tablero.destruirBloque(c.col, c.fila);
    avanzar(gestor, tablero, { c }, DURACION_EXPLOSION);

    comprobar(gestor.getPowerUps().size() == 1,
              "La explosión que rompe el bloque no destruye el power-up");
}

static void pruebaUnaExplosionPosteriorLoRompe()
{
    Tablero tablero;
    tablero.generar(42);
    GestorPowerUps gestor;
    gestor.repartir(tablero, 42, 1);
    Celda c = gestor.getPowerUps()[0].celda;

    tablero.destruirBloque(c.col, c.fila);
    avanzar(gestor, tablero, {}, 2.0f);          // pasa un rato en el suelo
    avanzar(gestor, tablero, { c }, 0.1f);       // le llega otra explosión

    comprobar(gestor.getPowerUps().empty(),
              "Una explosión posterior destruye el power-up visible");
}

// --- Efectos en el jugador y en la bomba ------------------------------------

static void pruebaBombaExtraConTope()
{
    Jugador jugador;
    jugador.aplicarPowerUp(TipoPowerUp::BombaExtra);
    bool sube = jugador.getMaxBombas() == 2;
    for (int i = 0; i < 20; i++) jugador.aplicarPowerUp(TipoPowerUp::BombaExtra);

    comprobar(sube && jugador.getMaxBombas() == MAX_BOMBAS,
              "Bomba extra suma una bomba hasta MAX_BOMBAS");
}

static void pruebaAlcanceConTope()
{
    Jugador jugador;
    jugador.aplicarPowerUp(TipoPowerUp::Alcance);
    bool sube = jugador.getAlcance() == ALCANCE_EXPLOSION + 1;
    for (int i = 0; i < 20; i++) jugador.aplicarPowerUp(TipoPowerUp::Alcance);

    comprobar(sube && jugador.getAlcance() == MAX_ALCANCE,
              "Alcance suma una celda hasta MAX_ALCANCE");
}

static void pruebaVelocidadConTope()
{
    Jugador jugador;
    jugador.aplicarPowerUp(TipoPowerUp::Velocidad);
    bool sube = jugador.getVelocidad() == VELOCIDAD_JUGADOR + INCREMENTO_VELOCIDAD;
    for (int i = 0; i < 20; i++) jugador.aplicarPowerUp(TipoPowerUp::Velocidad);

    comprobar(sube && jugador.getVelocidad() == MAX_VELOCIDAD_JUGADOR,
              "Velocidad sube hasta MAX_VELOCIDAD_JUGADOR");
}

static void pruebaInitQuitaLasMejoras()
{
    Jugador jugador;
    jugador.aplicarPowerUp(TipoPowerUp::BombaExtra);
    jugador.aplicarPowerUp(TipoPowerUp::Alcance);
    jugador.aplicarPowerUp(TipoPowerUp::Velocidad);
    jugador.init(Tablero::centroDeCelda(1, 1));

    comprobar(jugador.getMaxBombas() == 1 &&
              jugador.getAlcance() == ALCANCE_EXPLOSION &&
              jugador.getVelocidad() == VELOCIDAD_JUGADOR,
              "init() devuelve las mejoras a sus valores iniciales");
}

static void pruebaBombaConMasAlcance()
{
    // Tablero 42: (1,1) y (2,1) vacías, (3,1) bloque.
    Tablero corto, largo;
    corto.generar(42);
    largo.generar(42);

    Bomba normal({ 1, 1 });
    Bomba potente({ 1, 1 }, 2);
    normal.actualizar(DURACION_BOMBA + 0.01f, corto);
    potente.actualizar(DURACION_BOMBA + 0.01f, largo);

    bool normalSeQuedaCorta = !contiene(normal.getCeldasExplosion(), 3, 1)
                              && corto.getCelda(3, 1) == TipoCelda::Bloque;
    bool potenteLlega = contiene(potente.getCeldasExplosion(), 3, 1)
                        && largo.getCelda(3, 1) == TipoCelda::Vacio;

    comprobar(potente.getAlcance() == 2 && normalSeQuedaCorta && potenteLlega,
              "Una bomba con alcance 2 rompe un bloque que la normal no alcanza");
}

int main()
{
    pruebaRepartoEscondidoBajoBloques();
    pruebaRepartoSinRepetirCeldas();
    pruebaRepartoReproducible();
    pruebaRepartoDeTodosLosTipos();
    pruebaRepartoLimitadoPorBloques();
    pruebaApareceAlRomperElBloque();
    pruebaRecoger();
    pruebaLaExplosionQueLoDestapaNoLoRompe();
    pruebaUnaExplosionPosteriorLoRompe();
    pruebaBombaExtraConTope();
    pruebaAlcanceConTope();
    pruebaVelocidadConTope();
    pruebaInitQuitaLasMejoras();
    pruebaBombaConMasAlcance();

    std::cout << "\n" << (pruebas - fallos) << "/" << pruebas << " pruebas superadas\n";
    return fallos == 0 ? 0 : 1;
}
