#include <Jugador.hpp>
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
    }
    else {
        std::cout << "[FALLO] " << nombre << "\n";
        fallos++;
    }
}

static void pruebaJugadorEmpiezaVivo()
{
    Jugador jugador;

    comprobar(
        jugador.estaVivo(),
        "El jugador comienza vivo"
    );
}

static void pruebaEliminarJugador()
{
    Jugador jugador;

    jugador.eliminar();

    comprobar(
        !jugador.estaVivo(),
        "eliminar() marca al jugador como eliminado"
    );
}

static void pruebaReinicioJugador()
{
    Jugador jugador;

    jugador.eliminar();

    jugador.init(
        Tablero::centroDeCelda(1, 1)
    );

    comprobar(
        jugador.estaVivo(),
        "init() vuelve a dejar vivo al jugador"
    );
}

static void pruebaPosicionInicial()
{
    Vector2 posicionInicial =
        Tablero::centroDeCelda(1, 1);

    Jugador jugador(posicionInicial);

    Vector2 posicion = jugador.getPosicion();

    comprobar(
        posicion.x == posicionInicial.x &&
        posicion.y == posicionInicial.y,
        "El jugador conserva correctamente su posicion inicial"
    );
}

static void pruebaReinicioPosicion()
{
    Jugador jugador(
        Tablero::centroDeCelda(1, 1)
    );

    Vector2 nuevaPosicion =
        Tablero::centroDeCelda(COLUMNAS - 2, 1);

    jugador.init(nuevaPosicion);

    Vector2 posicion = jugador.getPosicion();

    comprobar(
        posicion.x == nuevaPosicion.x &&
        posicion.y == nuevaPosicion.y,
        "init() coloca al jugador en la nueva posicion"
    );
}

int main()
{
    SetTraceLogLevel(LOG_WARNING);

    pruebaJugadorEmpiezaVivo();
    pruebaEliminarJugador();
    pruebaReinicioJugador();
    pruebaPosicionInicial();
    pruebaReinicioPosicion();

    std::cout
        << "\n"
        << (pruebas - fallos)
        << "/"
        << pruebas
        << " pruebas superadas\n";

    return fallos == 0 ? 0 : 1;
}