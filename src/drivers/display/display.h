#pragma once
#include <stdint.h>
// Inicializa la pantalla OLED (llama a begin y limpia)
void InitDisplay();
// Limpia el buffer sin llamar display.display()
void ClearDisplay();
void SetMenuFont();
void DrawText(int x, int y, const char *text);
// Dibuja texto centrado horizontalmente en la pantalla (y = linea base)
void DrawTextCentered(int y, const char *text);
// Ancho en pixeles de un texto con la fuente actual
int TextWidth(const char *text);
// Dibuja un bitmap completo del tamaño indicado y hace display()
void DrawBitmap(const unsigned char *bitmap, int w, int h);
// Dibuja un bitmap en (x, y) sin limpiar ni enviar el buffer (para fondos y sprites)
void DrawImage(int x, int y, int width, int height, const unsigned char *bitmap);
// Conveniencia: dibuja el logo inicial
void DrawLogo();
// Dibuja el fondo del menu principal en el buffer (no lo envia; despues va ActDisplay())
void DrawMenu();
// Funcion para esperar con millis
bool wait(unsigned long durationMs);
void ActDisplay();
void DrawBox(int x, int y, int l, int w);
// Borra (pinta en negro) un cuadro del buffer, para despejar el fondo detras de un texto
void ClearBox(int x, int y, int l, int w);

// Primitivas extra (las usa el protector de pantalla). Dibujan en el buffer sin enviarlo.
void DrawPixel(int x, int y);
// Linea entre dos puntos; lo que cae fuera de la pantalla se recorta
void DrawLine(int x0, int y0, int x1, int y1);
// Contorno de un rectangulo
void DrawFrame(int x, int y, int w, int h);
// Rectangulo con esquinas redondeadas (contorno o relleno). w y h deben ser mayores que 2 * (r + 1)
void DrawRoundFrame(int x, int y, int w, int h, int r);
void DrawRoundBox(int x, int y, int w, int h, int r);
// Circulo (contorno) y disco (relleno) con centro en (x, y)
void DrawCircle(int x, int y, int r);
void DrawDisc(int x, int y, int r);
// Invierte un rectangulo: lo blanco queda negro y lo negro blanco
void InvertBox(int x, int y, int w, int h);
// Oscurece todo el buffer con un tramado: 0 = sin cambio, 16 = negro. Sirve para fundidos en una pantalla de 1 bit.
// shift mueve el patron: cambiandolo en cada cuadro lo de cuadros anteriores se va apagando (estelas)
void DitherDisplay(uint8_t level, uint8_t shift = 0);
// Rellena un rectangulo con un "gris" de puntos: 0 = negro ... 16 = blanco. Tapa lo que habia debajo
void FillDither(int x, int y, int w, int h, uint8_t level);
// Limita el dibujo a un rectangulo (x1 e y1 no se incluyen); ResetClipWindow vuelve a toda la pantalla
void SetClipWindow(int x0, int y0, int x1, int y1);
void ResetClipWindow();
// Texto UTF-8 (acentos, ñ, ¿ y ¡). Usar con una fuente *_ES
void DrawTextUTF8(int x, int y, const char *text);
int TextWidthUTF8(const char *text);
// Cuanto sube y baja el texto de la fuente actual respecto a la linea base (la bajada es negativa)
int TextAscent();
int TextDescent();

#define ANCHO_PANTALLA 128
#define ALTO_PANTALLA 64

enum FontSize
{
    FONT_SMALL,
    FONT_MEDIUM,
    FONT_LARGE,
    FONT_TINY,
    // Con acentos, ñ y signos de apertura (se dibujan con DrawTextUTF8)
    FONT_TINY_ES,   // 5x7
    FONT_SMALL_ES,  // Helvetica negrita, 8 px
    FONT_MEDIUM_ES, // Helvetica negrita, 14 px
    // Letras de burbuja de 18 px, solo contorno (sin acentos)
    FONT_BUBBLE,
};

void SetCustomFont(FontSize size);