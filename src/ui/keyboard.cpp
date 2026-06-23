#include "keyboard.h"
#include "../drivers/input/buttons.h"
#include "../drivers/display/display.h"

VirtualKeyboard::VirtualKeyboard(int x, int y)
{
    this->x = x;
    this->y = y;
    mode = 0; // start on lowercase mode
    row = 2;
    column = 6; // start on '/' key for mode toggle
    current_text = "";
    EntireWord = "";
    submitPressed = false;
}

/**
 * @brief Funcion que hace la logica del teclado
 * @param dir Ingresa la direccion que retornan los botones
 */
void VirtualKeyboard::handleInput(int dir)
{
    if (dir == 3) // 1
    {
        if (column == 9)
        {
            column = 0;
        }
        else
        {
            column++;
        }
    }
    if (dir == 4) // 2
    {
        if (column == 0)
        {
            column = 9;
        }
        else
        {
            column--;
        }
    }
    if (dir == 2) // 3
    {
        if (row == 2)
        {
            row = 0;
        }
        else
        {
            row++;
        }
    }
    if (dir == 1) // 4
    {
        if (row == 0)
        {
            row = 2;
        }
        else
        {
            row--;
        }
    }
    if (dir == 5)
    {
        if (row == 2 && column == 6)
        {
            mode = (mode + 1) % 3;
            return;
        }

        const char (*activeKeyboard)[10] = mode == 1 ? keyboard_Upper : (mode == 2 ? keyboard_Numbers : keyboard);
        char keycap = activeKeyboard[row][column];
        if (keycap == '<')
        {
            if (current_text.length() > 0)
            {
                current_text.remove(current_text.length() - 1);
            }
        }
        else if (keycap == '!')
        {
            submitPressed = true;
        }
        else
        {
            if (current_text.length() < 16)
            {
                current_text += keycap;
            }
        }
    }
}

/**
 * @brief Funcion que regresa la palabra completa
 */
String VirtualKeyboard::getWord()
{
    return current_text;
}

bool VirtualKeyboard::consumeSubmit()
{
    if (submitPressed)
    {
        submitPressed = false;
        EntireWord = current_text;
        return true;
    }
    return false;
}

void VirtualKeyboard::render()
{
    const int cellWidth = 10;
    const int cellHeight = 10;
    const int hSpacing = 12;
    const int vSpacing = 14;
    const int labelY = y - 5; // 12

    SetMenuFont();

    const int boxTopOffset = 12; // 2 10
    const char (*activeKeyboard)[10] = mode == 1 ? keyboard_Upper : (mode == 2 ? keyboard_Numbers : keyboard);
    // const char *modeLabel = mode == 2 ? "PSW: 1" : (mode == 1 ? "PSW: A" : "PSW: a");

    for (unsigned int r = 0; r < 3; r++)
    {
        for (unsigned int c = 0; c < 10; c++)
        {
            int cellX = x + c * hSpacing;
            int cellY = y + r * vSpacing + boxTopOffset;

            if (r == row && c == column)
            {
                DrawBox(cellX - 2, cellY - 18, cellWidth + 2, cellHeight + 2);
            }

            char letter[2] = {activeKeyboard[r][c], '\0'};
            DrawText(cellX, cellY - 8, letter);
        }
    }

    // DrawText(x, y - 5, modeLabel);
    // DrawText(x, y - 5, modeLabel);
}
