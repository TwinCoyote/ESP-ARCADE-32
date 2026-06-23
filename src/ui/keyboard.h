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

    const char keyboard_Upper[3][10] = {
        {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J'},
        {'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T'},
        {'U', 'V', 'W', 'X', 'Y', 'Z', '/', '<', '_', '!'}};

    const char keyboard_Numbers[3][10] = {
        {'1', '2', '3', '4', '5', '6', '7', '8', '9', '0'},
        {'-', '/', ':', ';', '(', ')', '$', '&', '@', '"'},
        {'.', ',', '?', '!', '\'', '<', '/', '*', '+', '='}};

    int x;
    int y;
    unsigned int row;
    unsigned int column;
    int mode = 0; // 0 = lowercase, 1 = uppercase, 2 = numbers
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
