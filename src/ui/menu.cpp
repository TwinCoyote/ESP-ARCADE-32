#include <Arduino.h>
#include "drivers/input/buttons.h"
#include "drivers/display/display.h"
const char *options[] = {
    // "Menu"
    "Snake",
    "Pong",
    "Tetris",
    "Config",
    "Pruebas"};

int totalOptions = sizeof(options) / sizeof(options[0]);
int indexMenu = 0;

void MenuInit()
{
    indexMenu;
}

bool MenuUpdate()
{
    bool changed = false;

    // Simple per-button debounce: accept a new press only if
    // the last accepted press was more than 200 ms ago.
    static unsigned long lastRightMillis = 0;
    static unsigned long lastLeftMillis = 0;
    const unsigned long debounceMs = 200;

    bool rightPressed = isPressed(BTN_RIGHT);
    bool leftPressed = isPressed(BTN_LEFT);
    // Diagnostic: show pin numbers and raw digitalRead values
    // int rawRight = digitalRead(BTN_RIGHT);
    // int rawLeft = digitalRead(BTN_LEFT);
    // Serial.print("MENU raw BTN_LEFT=");
    // Serial.print(leftPressed ? "PRESSED" : "RELEASED");
    // Serial.print(" (pin=");
    // Serial.print(BTN_LEFT);
    // Serial.print(" val=");
    // Serial.print(rawLeft);
    // Serial.print(") BTN_RIGHT=");
    // Serial.print(rightPressed ? "PRESSED" : "RELEASED");
    // Serial.print(" (pin=");
    // Serial.print(BTN_RIGHT);
    // Serial.print(" val=");
    // Serial.println(rawRight);

    if (rightPressed)
    {
        unsigned long now = millis();
        if (now - lastRightMillis > debounceMs)
        {
            indexMenu++;
            if (indexMenu >= totalOptions)
                indexMenu = 0;
            changed = true;
            lastRightMillis = now;
            Serial.println("BTN_RIGHT pressed");
        }
    }

    if (leftPressed)
    {
        unsigned long now = millis();
        if (now - lastLeftMillis > debounceMs)
        {
            indexMenu--;
            if (indexMenu < 0)
                indexMenu = totalOptions - 1;
            changed = true;
            lastLeftMillis = now;
            Serial.println("BTN_LEFT pressed");
        }
    }

    if (changed)
    {
        Serial.print("indexMenu=");
        Serial.println(indexMenu);
    }

    return changed;
}

bool MenuConfirm()
{
    static unsigned long lastOkMillis = 0;
    static bool lastOkState = false;
    const unsigned long debounceMs = 200;

    bool current = isPressed(BTN_OK);
    unsigned long now = millis();

    if (current && !lastOkState && (now - lastOkMillis > debounceMs))
    {
        lastOkMillis = now;
        lastOkState = true;
        Serial.println("BTN_OK confirmed");
        return true;
    }

    if (!current)
    {
        lastOkState = false;
    }

    return false;
}

bool MenuBack()
{
    static unsigned long lastOkMillis = 0;
    static bool lastOkState = false;
    const unsigned long debounceMs = 200;

    bool current = isPressed(BTN_BACK);
    unsigned long now = millis();

    if (current && !lastOkState && (now - lastOkMillis > debounceMs))
    {
        lastOkMillis = now;
        lastOkState = true;
        Serial.println("BTN_BACK IS PRESSED");
        return true;
    }

    return false;
}

int MenuGetIndex()
{
    return indexMenu;
}

void MenuRender()
{
    // ClearDisplay();
    DrawMenu();
    SetMenuFont();
    DrawText(44, 35, options[indexMenu]);
    Serial.println(indexMenu);
    ActDisplay();
}