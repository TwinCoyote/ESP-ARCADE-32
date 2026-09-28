#include "config_menu.h"
#include <Arduino.h>
#include "../../drivers/display/display.h"
#include "../../drivers/time/millis.h"

MenuS::MenuS(const char *opt[], unsigned int len)
{
    cursor = 0;
    options = opt;
    menuLen = len;
    lastMoveMs = 0;
}

void MenuS::handleInput(int dir)
{
    // Una pulsacion mueve una sola opcion; si se mantiene presionado repite cada 200 ms
    const unsigned long repeatMs = 200;
    unsigned long now = millis();
    if ((dir != 1 && dir != 2) || now - lastMoveMs < repeatMs)
    {
        return;
    }
    lastMoveMs = now;

    if (dir == 2) // DOWN baja el cursor
    {
        if (cursor == menuLen - 1)
        {
            cursor = 0;
        }
        else
        {
            cursor++;
        }
    }
    if (dir == 1) // UP sube el cursor
    {
        if (cursor == 0)
        {
            cursor = menuLen - 1;
        }
        else
        {
            cursor--;
        }
    }
}

void MenuS::render()
{
    ClearDisplay();
    SetMenuFont();

    for (unsigned int i = 0; i < menuLen; i++)
    {
        int posY = 18 + i * 18; // Baseline Y for U8g2 text output

        if (i == cursor)
        {
            DrawBox(0, posY - 12, 6, 14);
        }

        DrawText(10, posY, options[i]);
        DrawBox(0, posY + 2, ANCHO_PANTALLA, 1);
    }

    ActDisplay();
    millis(50);
}

unsigned int MenuS::getIndex() const
{
    return cursor;
}
