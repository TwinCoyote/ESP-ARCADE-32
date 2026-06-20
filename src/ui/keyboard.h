#ifndef KEYBOARD_H
#define KEYBOARD_H
#include <Arduino.h>

class VirtualKeyboard
{
private:
    const char keyboard[3][10] = {
        {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J'},
        {'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T'},
        {'U', 'V', 'W', 'X', 'Y', 'Z', ' ', '<', '_', '!'}};

    int x;
    int y;
    unsigned int row;
    unsigned int column;
    String current_text = "";
    String EntireWord = "";

public:
    VirtualKeyboard(int x, int y);
    void render();
    void handleInput(int dir);
    String getWord();
};

#endif
