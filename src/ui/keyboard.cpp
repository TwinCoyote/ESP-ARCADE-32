#include "keyboard.h"
#include "../drivers/input/buttons.h"
#include "../drivers/display/display.h"

VirtualKeyboard::VirtualKeyboard(int x, int y)
{
    row = 0;
    column = 0;
}

/**
 * @brief Funcion que hace la logica del teclado
 * @param dir Ingresa la direccion que retornan los botones
 */
void VirtualKeyboard::handleInput(int dir)
{
    if (dir == 1)
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
    if (dir == 2)
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
    if (dir == 3)
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
    if (dir == 4)
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
        char keycap = keyboard[row][column];
        if (keycap == '<')
        {
            if (current_text.length() > 0)
            {
                current_text.remove(current_text.length() - 1);
            }
        }
        else if (keycap == '!')
        {
            current_text = EntireWord;
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

void VirtualKeyboard::render()
{
    const int cellWidth = 10;
    const int cellHeight = 10;
    const int hSpacing = 12;
    const int vSpacing = 14;
    const int labelY = y - 12;

    ClearDisplay();
    SetMenuFont();
    DrawText(x, labelY, "TECLADO");

    for (unsigned int r = 0; r < 3; r++)
    {
        for (unsigned int c = 0; c < 10; c++)
        {
            int cellX = x + c * hSpacing;
            int cellY = y + r * vSpacing;

            if (r == row && c == column)
            {
                DrawBox(cellX - 1, cellY - 1, cellWidth + 2, cellHeight + 2);
            }

            char letter[2] = {keyboard[r][c], '\0'};
            DrawText(cellX, cellY, letter);
        }
    }

    DrawText(x, y + 3 * vSpacing + 4, "Texto:");
    DrawText(x, y + 3 * vSpacing + 18, current_text.c_str());
    ActDisplay();
}