#include <Arduino.h>
#include "drivers/input/buttons.h"
#include "drivers/display/display.h"
const char *options[] = {
    "Snake",
    "Pong",
    "Tetris",
    "Config"};

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

    if (isPressed(BTN_RIGHT))
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

    if (isPressed(BTN_LEFT))
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