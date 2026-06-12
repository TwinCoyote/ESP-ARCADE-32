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
    if (isPressed(BTN_RIGHT))
        indexMenu++;
    if (indexMenu >= totalOptions)
        indexMenu = 0;

    if (isPressed(BTN_LEFT))
        indexMenu--;
    if (indexMenu < 0)
        indexMenu = totalOptions - 1;
    return false;
}

void MenuRender()
{
    // ClearDisplay();
    DrawMenu();
    SetMenuFont();
    DrawText(44, 35, options[indexMenu]);
    ActDisplay();
}