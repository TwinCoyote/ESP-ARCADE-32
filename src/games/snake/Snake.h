#ifndef SNAKE_H
#define SNAKE_H
#include <Arduino.h>
#include "../../drivers/display/display.h"
#include "../../drivers/input/buttons.h"

#define ANCHO_PANTALLA 128
#define ALTO_PANTALLA 64
#define DIRECCION_OLED 0x3C
#define LONGITUD_MAXIMA 50

// extern Adafruit_SSD1306 display;
extern Input input;

enum class states
{
    INIT,
    START,
    GAME_OVER,
    AGAIN
};

extern states ACTUAL_STATE;

extern int direccion;
extern unsigned long previousTime;
extern int largo;

extern int snake_x[LONGITUD_MAXIMA];
extern int snake_y[LONGITUD_MAXIMA];

extern int comidaX;
extern int comidaY;

extern const int len_block;

// Funciones públicas del juego
void snake_init();
void snake_game();

#endif