#pragma once

#include <Arduino.h>

// Uso interno del protector de pantalla: las escenas y las ayudas que comparten.
namespace screensaver
{
    // Cada escena empieza con start(), que regresa cuanto dura en ms, y despues dibuja con
    // draw(t), donde t son los ms desde que empezo (0 <= t < duracion).
    // draw() solo llena el buffer: el despachador lo limpia antes, le aplica los fundidos y lo envia.
    // Con keepsBuffer el buffer no se limpia: la escena recibe el cuadro anterior (para dejar estelas).
    struct Scene
    {
        const char *name;
        unsigned long (*start)();
        void (*draw)(unsigned long t);
        bool keepsBuffer;
    };

    unsigned long petStart();
    void petDraw(unsigned long t);

    unsigned long eyesStart();
    void eyesDraw(unsigned long t);

    unsigned long musicStart();
    void musicDraw(unsigned long t);

    // ------------------------------------------------------------------ Ayudas

    // Sprite dibujado con texto, una cadena por fila: '#' pixel encendido, 'o' pixel apagado
    // (tapa lo que haya detras) y cualquier otro caracter es transparente.
    // scale agranda cada pixel y mirror voltea el sprite para que mire hacia el otro lado.
    void drawSprite(int x, int y, const char *const *rows, uint8_t rowCount, uint8_t scale = 1, bool mirror = false);

    // Numero pseudoaleatorio fijo para cada n: sirve para cosas que deben verse al azar
    // pero sin cambiar de un cuadro a otro (estrellas, confeti, barras del ecualizador)
    uint32_t hash32(uint32_t n);
}

// Cantidad de filas de un sprite de texto declarado como arreglo
#define SPRITE_ROWS(s) ((uint8_t)(sizeof(s) / sizeof(s[0])))
