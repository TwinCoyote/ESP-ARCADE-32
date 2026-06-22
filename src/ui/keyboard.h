#ifndef KEYBOARD_H
#define KEYBOARD_H
#include <Arduino.h>

class VirtualKeyboard
{
private:
    const char keyboard[3][10] = {
        {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j'},
        {'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't'},
        {'u', 'v', 'w', 'x', 'y', 'z', '/', '<', '_', '!'}};

    int x;
    int y;
    unsigned int row;
    unsigned int column;
    bool uppercase = false;
    String current_text = "";
    String EntireWord = "";
    bool submitPressed = false;

public:
    VirtualKeyboard(int x, int y);
    void render();
    void handleInput(int dir);
    String getWord();
    bool consumeSubmit();
};

#endif
