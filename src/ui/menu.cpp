#include <Arduino.h>
#include "drivers/input/buttons.h"
#include "drivers/display/display.h"
#include "../config/debug_log.h"

const char *options[] = {
    // "Menu"
    "Snake",
    "Pong",
    "Tetris",
    "Config",
    "Flappy Bird"};

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
    // DEV_PRINT("MENU raw BTN_LEFT=");
    // DEV_PRINT(leftPressed ? "PRESSED" : "RELEASED");
    // DEV_PRINT(" (pin=");
    // DEV_PRINT(BTN_LEFT);
    // DEV_PRINT(" val=");
    // DEV_PRINT(rawLeft);
    // DEV_PRINT(") BTN_RIGHT=");
    // DEV_PRINT(rightPressed ? "PRESSED" : "RELEASED");
    // DEV_PRINT(" (pin=");
    // DEV_PRINT(BTN_RIGHT);
    // DEV_PRINT(" val=");
    // DEV_PRINTLN(rawRight);

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
            DEV_PRINTLN("BTN_RIGHT pressed");
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
            DEV_PRINTLN("BTN_LEFT pressed");
        }
    }

    if (changed)
    {
        DEV_PRINT("indexMenu=");
        DEV_PRINTLN(indexMenu);
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
        DEV_PRINTLN("BTN_OK confirmed");
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
        DEV_PRINTLN("BTN_BACK IS PRESSED");
        return true;
    }
    lastOkState = false;

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
    DrawTextCentered(35, options[indexMenu]);
    DEV_PRINTLN(indexMenu);
    ActDisplay();
}