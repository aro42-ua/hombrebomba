#include <ResultadoPartida.hpp>
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

static void pruebaPartidaContinua()
{
    comprobar(
        calcularResultadoPartida(true, true)
            == ResultadoPartida::EnCurso,

        "La partida continua si ambos jugadores estan vivos"
    );
}

static void pruebaGanaJugador1()
{
    comprobar(
        calcularResultadoPartida(true, false)
            == ResultadoPartida::Jugador1,

        "Jugador 1 gana si es el unico jugador vivo"
    );
}

static void pruebaGanaJugador2()
{
    comprobar(
        calcularResultadoPartida(false, true)
            == ResultadoPartida::Jugador2,

        "Jugador 2 gana si es el unico jugador vivo"
    );
}

static void pruebaEmpate()
{
    comprobar(
        calcularResultadoPartida(false, false)
            == ResultadoPartida::Empate,

        "Hay empate si ambos jugadores estan eliminados"
    );
}

int main()
{
    pruebaPartidaContinua();
    pruebaGanaJugador1();
    pruebaGanaJugador2();
    pruebaEmpate();

    std::cout
        << "\n"
        << (pruebas - fallos)
        << "/"
        << pruebas
        << " pruebas superadas\n";

    return fallos == 0 ? 0 : 1;
}