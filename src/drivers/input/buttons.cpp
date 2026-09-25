#include <Arduino.h>
#include "buttons.h"
#include "../../../src/config/debug_log.h"

// Global Input instance for the project
Input input(BTN_UP, BTN_DOWN, BTN_RIGHT, BTN_LEFT, BTN_OK, BTN_BACK);

// Define pin storage for buttons (file-scope)
static int pinUp;
static int pinDown;
static int pinRight;
static int pinLeft;
static int pinSelect;
static int pinBack;

Input::Input(int up, int down, int right, int left, int select, int back)
{
    pinUp = up;
    pinDown = down;
    pinRight = right;
    pinLeft = left;
    pinSelect = select;
    pinBack = back;
}

void Input::begin()
{
    // Delegate initialization to the global InitButtons() function
    InitButtons();
}

bool isPressed(int pin)
{
    return digitalRead(pin) == LOW;
}

int Input::realDirection()
{
    if (digitalRead(BTN_UP) == LOW)
        return 1;
    if (digitalRead(BTN_DOWN) == LOW)
        return 2;
    if (digitalRead(BTN_RIGHT) == LOW)
        return 3;
    if (digitalRead(BTN_LEFT) == LOW)
        return 4;
    if (digitalRead(BTN_OK) == LOW)
        return 5;
    if (digitalRead(BTN_BACK) == LOW)
        return 6;
    return 0;
}

void InitButtons()
{
    pinMode(BTN_OK, INPUT_PULLUP);
    pinMode(BTN_BACK, INPUT_PULLUP);
    pinMode(BTN_UP, INPUT_PULLUP);
    pinMode(BTN_DOWN, INPUT_PULLUP);
    pinMode(BTN_LEFT, INPUT_PULLUP);
    pinMode(BTN_RIGHT, INPUT_PULLUP);
    // Diagnostic output: list pins and their current digitalRead value
    DEV_PRINT("InitButtons pins: OK=");
    DEV_PRINT(BTN_OK);
    DEV_PRINT(" val=");
    DEV_PRINT(digitalRead(BTN_OK));
    DEV_PRINT(" BACK=");
    DEV_PRINT(BTN_BACK);
    DEV_PRINT(" val=");
    DEV_PRINT(digitalRead(BTN_BACK));
    DEV_PRINT(" UP=");
    DEV_PRINT(BTN_UP);
    DEV_PRINT(" val=");
    DEV_PRINT(digitalRead(BTN_UP));
    DEV_PRINT(" DOWN=");
    DEV_PRINT(BTN_DOWN);
    DEV_PRINT(" val=");
    DEV_PRINT(digitalRead(BTN_DOWN));
    DEV_PRINT(" LEFT=");
    DEV_PRINT(BTN_LEFT);
    DEV_PRINT(" val=");
    DEV_PRINT(digitalRead(BTN_LEFT));
    DEV_PRINT(" RIGHT=");
    DEV_PRINT(BTN_RIGHT);
    DEV_PRINT(" val=");
    DEV_PRINTLN(digitalRead(BTN_RIGHT));
}