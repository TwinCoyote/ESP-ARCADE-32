#ifndef CONFIG_MENU_H
#define CONFIG_MENU_H

#include <Arduino.h>

class MenuConfig
{
private:
    const char *options[3] = {
        "Wifi",
        "Display",
        "Info"};

    unsigned int cursor;
    unsigned int menuLen = 3;

public:
    MenuConfig();

    void handleInput(int dir);
    void render();
};

extern MenuConfig configMenu;
#endif // CONFIG_MENU_H
