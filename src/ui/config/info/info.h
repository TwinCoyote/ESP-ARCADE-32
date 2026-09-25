#ifndef INFO_MENU_H
#define INFO_MENU_H

#include <Arduino.h>

// Pantalla de Info del menu de configuracion: paginas CONSOLA / RED / CREDITOS (LEFT y RIGHT)
// y un easter egg escondido.
class InfoMenu
{
private:
    uint8_t page;
    bool secretVisible;
    bool leaveOnRelease;
    bool needsRedraw;
    int lastInput;
    unsigned long lastPressMs;
    unsigned long lastDrawMs;
    unsigned long secretStartMs;
    uint8_t history[10]; // ultimas teclas presionadas (para el codigo secreto)
    String version;

    void pushHistory(uint8_t button);
    bool codeTyped(uint8_t count) const;
    void drawValue(uint8_t row, const char *text, int maxWidth);
    void renderPage();
    void renderSecret();

public:
    InfoMenu();

    void reset();
    // Lee los botones. Regresa true cuando el usuario sale de Info con BACK
    bool update();
    void render();
};

#endif
