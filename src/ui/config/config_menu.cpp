#include "config_menu.h"
#include <Arduino.h>
#include "../../drivers/display/display.h"
#include "../../drivers/time/millis.h"

MenuConfig configMenu;

MenuConfig::MenuConfig()
{
    cursor = 0;
}

void MenuConfig::handleInput(int dir)
{
    if (dir == 1)
    {
        if (cursor == 2)
        {
            cursor = 0;
        }
        else
        {
            cursor++;
        }
    }
    if (dir == 2)
    {
        if (cursor == 0)
        {
            cursor = 2;
        }
        else
        {
            cursor--;
        }
    }
}

void MenuConfig::render()
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
