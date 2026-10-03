#include <Bomba.hpp>
#include <iostream>

static bool contiene(const std::vector<Celda>& celdas, int col, int fila)
{
    for (const Celda& celda : celdas) {
        if (celda.col == col && celda.fila == fila) return true;
    }
    return false;
}

int main()
{
    Tablero tablero;
    tablero.generar(42);

    Bomba bomba({ 4, 6 });
    Celda posicion = bomba.getPosicion();
    bool posicionCorrecta = posicion.col == 4 && posicion.fila == 6;
    Bomba bombaPrueba({ 1, 1 });
    bool sigueArmadaAntesDelLimite = !bombaPrueba.actualizar(2.99f, tablero)
                                  && bombaPrueba.estaEnCuentaAtras();
    bool detonaAlSuperarTresSegundos = !bombaPrueba.actualizar(0.02f, tablero)
                                    && !bombaPrueba.estaEnCuentaAtras();
    const std::vector<Celda>& celdas = bombaPrueba.getCeldasExplosion();
    bool centroIncluido = contiene(celdas, 1, 1);
    bool muroDetieneExplosion = !contiene(celdas, 0, 1) && !contiene(celdas, 1, 0);
    bool bloqueAlcanzadoYDestruido = contiene(celdas, 2, 1)
                                  && contiene(celdas, 3, 1)
                                  && tablero.getCelda(3, 1) == TipoCelda::Vacio;
    bool desapareceTrasDuracion = !bombaPrueba.actualizar(0.59f, tablero)
                               && bombaPrueba.actualizar(0.02f, tablero);

    if (!posicionCorrecta || !sigueArmadaAntesDelLimite || !detonaAlSuperarTresSegundos
        || !centroIncluido || !muroDetieneExplosion || !bloqueAlcanzadoYDestruido
        || !desapareceTrasDuracion) {
        std::cerr << "[FALLO] Ciclo de vida o propagacion de la bomba incorrectos\n";
        return 1;
    }

    std::cout << "[OK] Cuenta atras, propagacion, destruccion y duracion de la explosion\n";
    return 0;
}