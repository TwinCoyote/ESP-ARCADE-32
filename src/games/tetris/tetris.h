#ifndef TETRIS_H
#define TETRIS_H
#include <Arduino.h>

// Controles:
//   IZQUIERDA / DERECHA  mover la pieza (si se mantiene, se repite)
//   ARRIBA               girar
//   ABAJO                bajar rapido (+1 punto por fila)
//   OK                   guardar la pieza en HOLD (una vez por pieza)
//   BACK                 volver al menu (lo maneja SystemManager)

namespace tetris
{
    // Deja el juego en la pantalla de inicio. Se llama al entrar desde el menu.
    void reset();
    // Un cuadro del juego: lee los botones, avanza la logica y dibuja. Va en cada vuelta del loop.
    void game_tetris();
}
#endif
