#pragma once

#include "Arduino.h"

void InitButtons();

bool isPressed(int pin);

#define BTN_OK 35
#define BTN_BACK 34
#define BTN_UP 33
#define BTN_DOWN 32
#define BTN_LEFT 4
#define BTN_RIGHT 12

class Input
{
public:
    Input(int up, int down, int right, int left, int select, int back);
    void begin();
    int realDirection();

private:
    int up;
    int down;
    int right;
    int left;
    int select;
    int back;
};

extern Input input;