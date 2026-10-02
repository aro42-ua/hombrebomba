#include <Bomba.hpp>
#include <iostream>

int main()
{
    Bomba bomba({ 4, 6 });
    Celda posicion = bomba.getPosicion();
    bool posicionCorrecta = posicion.col == 4 && posicion.fila == 6;
    bool sigueActivaAntesDelLimite = !bomba.actualizar(2.99f);
    bool expiraAlSuperarTresSegundos = bomba.actualizar(0.02f);

    if (!posicionCorrecta || !sigueActivaAntesDelLimite || !expiraAlSuperarTresSegundos) {
        std::cerr << "[FALLO] Posicion o tiempo de vida de la bomba incorrectos\n";
        return 1;
    }

    std::cout << "[OK] La bomba guarda su celda y expira tras 3 segundos\n";
    return 0;
}