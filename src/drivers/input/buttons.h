#pragma once

#include "Arduino.h"
#include "../../config/pins.h"

void InitButtons();

bool isPressed(int pin);

class Input
{
public:
    Input(int up, int down, int right, int left, int select, int back);
    void begin();
    int realDirection();
    // Lee los botones: si hay alguno presionado (o se mantiene presionado) cuenta como actividad
    void updateActivity();
    // true si hubo actividad desde la ultima consulta (la consulta la borra)
    bool consumeButtonActivity();

private:
    int up;
    int down;
    int right;
    int left;
    int select;
    int back;
    bool buttonActivity = false; // Hubo algun boton presionado desde la ultima consulta

};

extern Input input;