#pragma once

#include <Arduino.h>
#include "../../drivers/display/display.h"

class Pajaro
{
private:
    unsigned int size = 4;
    int x = 0;
    int y = 0;
    float vel_y = 0;
    float gravity = 0;

public:
    // EL CONSTRUCTOR: Sin void, y recibe las coordenadas iniciales
    Pajaro(int xInicio, int yInicio);

    void move();
    void print();
    void jump();
    int getX();
    int getY();
    unsigned int getSize();
};

class Pipe
{
private:
    int x_pipe = ANCHO_PANTALLA;
    unsigned int high_superior = 0;
    unsigned int high_inferior = 0;
    unsigned int len_pipe = 6;
    unsigned int pipe_hole = 16;

public:
    Pipe(int xInicio);
    void move();
    void render();
    bool checkCollision(int pajaroX, int pajaroY, int pajaroSize);
};

void flappy_bird();