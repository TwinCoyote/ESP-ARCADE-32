#ifndef CONFIG_MENU_H
#define CONFIG_MENU_H

#include <Arduino.h>

class MenuS
{
private:
    const char **options;
    unsigned int cursor;
    unsigned int menuLen;

public:
    MenuS(const char *opt[], unsigned int len);

    void handleInput(int dir);
    void render();
    unsigned int getIndex() const;
};

#endif // CONFIG_MENU_H
