#ifndef PONG_H
#define PONG_H
#include "../../drivers/display/display.h"
#include "../../drivers/input/buttons.h"

// extern Adafruit_SSD1306 display;
extern Input input;

extern int direccion;

extern const int botonArriba;
extern const int botonAbajo;
extern const int botonDerecha;
extern const int botonIzquierda;
extern const int botonSelect;

extern const int len_block;
extern const int width;
extern const int limite_inferior;
extern const int limite_inferior_ball;
extern const int limite_superior;
extern const int ball_size;

extern int x_AI;
extern int x;
extern int y;
extern int ball_x;
extern int ball_y;

namespace pong
{
    void pong_direcciones();
    void pong_buttons_read();
    void screen_limits();
    void print_player();
    void print_AI();
    void ball_limits();
    void caida();
    void game_pong();
}
#endif