#include <Arduino.h>
#include <U8g2lib.h>
#include "display.h"
#include "../../assets/images/assets.h"
#include "../../config/display_config.h"
#include "../../ui/menu.h"

using namespace DisplayConfig;
static unsigned long waitStart = 0;
static unsigned long waitDuration = 0;
static bool waitActive = false;

#if DISPLAY_CONTROLLER == DISPLAY_CONTROLLER_SH1106
U8G2_SH1106_128X64_NONAME_F_HW_I2C display(U8G2_R0, U8X8_PIN_NONE);
#else
U8G2_SSD1306_128X64_NONAME_F_HW_I2C display(U8G2_R0, U8X8_PIN_NONE);
#endif

/**
 * @brief Funcion anterior para el tamaño de la letra
 */
void SetMenuFont()
{
    display.setFont(u8g2_font_ncenB08_tr);
}

/**
 * @brief Funcion que permite Agregar Texto
 * @param x Cordenadas en el eje de las X
 * @param y Cordenadas en el Eje de las Y
 * @param text Ingresa el texto como String
 */
void DrawText(int x, int y, const char *text)
{
    display.drawStr(x, y, text);
    // display.sendBuffer();
}

/**
 * @brief Funcion que agrega texto centrado horizontalmente en la pantalla
 * @param y Coordenada en el eje Y (linea base del texto)
 * @param text Ingresa el texto como String
 */
void DrawTextCentered(int y, const char *text)
{
    int x = (ANCHO_PANTALLA - (int)display.getStrWidth(text)) / 2;
    display.drawStr(x, y, text);
}

/**
 * @brief Funcion que regresa el ancho en pixeles de un texto con la fuente actual
 * @param text Ingresa el texto como String
 */
int TextWidth(const char *text)
{
    return display.getStrWidth(text);
}

/**
 * @brief Funcion que inicia el display
 */
void InitDisplay()
{
    display.begin();
    display.setContrast(255);
    display.clearBuffer();
    display.sendBuffer();
}

/**
 * @brief Funcion que limpia el display
 */
void ClearDisplay()
{
    display.clearBuffer();
    // display.sendBuffer();
}

/**
 * @brief Funcion que imprime un mapa de bits
 * @param bitmap la direccion de mapa de bits
 * @param width El ancho del mapa
 * @param height El alto del mapa
 */
void DrawBitmap(const unsigned char *bitmap, int width, int height)
{
    display.clearBuffer();
    display.drawXBMP(0, 0, width, height, bitmap);
    display.sendBuffer();
}

/**
 * @brief Funcion que dibuja un mapa de bits en una posicion sin limpiar ni enviar el buffer.
 * Sirve para fondos y sprites: se dibuja primero y lo demas va encima.
 * @param x Coordenada en el eje X
 * @param y Coordenada en el eje Y
 * @param width El ancho del mapa
 * @param height El alto del mapa
 * @param bitmap la direccion de mapa de bits
 */
void DrawImage(int x, int y, int width, int height, const unsigned char *bitmap)
{
    display.drawXBMP(x, y, width, height, bitmap);
}

/**
 * @brief Funcion que imprime los logos con cordenandas preestablcidas
 */
void DrawLogo()
{
    display.clearBuffer();
    DrawBitmap(Logo_R, logoWidth, logoHeight);
    display.sendBuffer();
}

/**
 * @brief Funcion que dibuja el fondo del menu en el buffer, sin enviarlo.
 * Quien la llama agrega el texto encima y despues hace ActDisplay(): si se enviara aqui,
 * la pantalla mostraria un instante el fondo sin texto (parpadeo al cambiar de opcion).
 */
void DrawMenu()
{
    display.clearBuffer();
    display.drawXBMP(0, 0, logoWidth, logoHeight, menu_principal);
}

/**
 * @brief Funcion que actualiza el display
 */
void ActDisplay()
{
    display.sendBuffer();
}

/**
 * @brief Funcion que imprime cuadros en cordenadas
 * @param x Cordenada en X
 * @param y Cordenadas en Y
 * @param l valor entero de la longiutd
 * @param w Valor entero de el ancho
 */
void DrawBox(int x, int y, int l, int w)
{
    display.drawBox(x, y, l, w);
}

/**
 * @brief Funcion que borra (pinta en negro) un cuadro del buffer.
 * Sirve para despejar el fondo detras de un texto y que se lea bien.
 * @param x Cordenada en X
 * @param y Cordenadas en Y
 * @param l valor entero de la longiutd
 * @param w Valor entero de el ancho
 */
void ClearBox(int x, int y, int l, int w)
{
    display.setDrawColor(0);
    display.drawBox(x, y, l, w);
    display.setDrawColor(1);
}

/**
 * @brief Funcion que dibuja un pixel
 */
void DrawPixel(int x, int y)
{
    display.drawPixel(x, y);
}

// Region de un punto respecto a la pantalla (algoritmo de Cohen-Sutherland): 0 = adentro
static uint8_t LineRegion(int x, int y)
{
    uint8_t code = 0;
    if (x < 0)
        code |= 1;
    else if (x >= ANCHO_PANTALLA)
        code |= 2;
    if (y < 0)
        code |= 4;
    else if (y >= ALTO_PANTALLA)
        code |= 8;
    return code;
}

/**
 * @brief Funcion que dibuja una linea. U8g2 no recorta lineas con coordenadas negativas
 * (las tomaria como numeros enormes), asi que primero se recorta contra la pantalla.
 */
void DrawLine(int x0, int y0, int x1, int y1)
{
    uint8_t c0 = LineRegion(x0, y0);
    uint8_t c1 = LineRegion(x1, y1);
    while (c0 | c1)
    {
        if (c0 & c1)
        {
            return; // Toda la linea queda del mismo lado, fuera de la pantalla
        }
        uint8_t c = c0 ? c0 : c1;
        long x, y;
        if (c & 8)
        {
            y = ALTO_PANTALLA - 1;
            x = x0 + (long)(x1 - x0) * (y - y0) / (y1 - y0);
        }
        else if (c & 4)
        {
            y = 0;
            x = x0 + (long)(x1 - x0) * (y - y0) / (y1 - y0);
        }
        else if (c & 2)
        {
            x = ANCHO_PANTALLA - 1;
            y = y0 + (long)(y1 - y0) * (x - x0) / (x1 - x0);
        }
        else
        {
            x = 0;
            y = y0 + (long)(y1 - y0) * (x - x0) / (x1 - x0);
        }
        if (c == c0)
        {
            x0 = x;
            y0 = y;
            c0 = LineRegion(x0, y0);
        }
        else
        {
            x1 = x;
            y1 = y;
            c1 = LineRegion(x1, y1);
        }
    }
    display.drawLine(x0, y0, x1, y1);
}

/**
 * @brief Funcion que dibuja el contorno de un rectangulo
 */
void DrawFrame(int x, int y, int w, int h)
{
    display.drawFrame(x, y, w, h);
}

/**
 * @brief Funcion que dibuja el contorno de un rectangulo con esquinas redondeadas de radio r
 */
void DrawRoundFrame(int x, int y, int w, int h, int r)
{
    display.drawRFrame(x, y, w, h, r);
}

/**
 * @brief Funcion que dibuja un rectangulo relleno con esquinas redondeadas de radio r
 */
void DrawRoundBox(int x, int y, int w, int h, int r)
{
    display.drawRBox(x, y, w, h, r);
}

/**
 * @brief Funcion que dibuja el contorno de un circulo con centro en (x, y)
 */
void DrawCircle(int x, int y, int r)
{
    display.drawCircle(x, y, r);
}

/**
 * @brief Funcion que dibuja un circulo relleno con centro en (x, y)
 */
void DrawDisc(int x, int y, int r)
{
    display.drawDisc(x, y, r);
}

/**
 * @brief Funcion que invierte un rectangulo del buffer (modo XOR): lo blanco queda negro y al reves.
 * Sirve para resaltar texto sin tener que redibujarlo.
 */
void InvertBox(int x, int y, int w, int h)
{
    display.setDrawColor(2);
    display.drawBox(x, y, w, h);
    display.setDrawColor(1);
}

// Umbrales de la matriz de Bayer de 4x4: con ellos se reparten puntos parejos para simular grises
static const uint8_t BAYER[4][4] = {{0, 8, 2, 10}, {12, 4, 14, 6}, {3, 11, 1, 9}, {15, 7, 13, 5}};

// El buffer va por paginas de 8 filas: cada byte es una columna de 8 pixeles (bit 0 arriba).
// Regresa el byte de la columna 'col' con encendidos los pixeles cuyo umbral es menor que level.
// El patron se repite cada 4 columnas y cada 4 filas, asi que sirve para cualquier pagina.
static uint8_t BayerColumn(int col, int shiftX, int shiftY, uint8_t level)
{
    uint8_t bits = 0;
    for (int bit = 0; bit < 8; bit++)
    {
        if (BAYER[(bit + shiftY) % 4][(col + shiftX) % 4] < level)
        {
            bits |= 1 << bit;
        }
    }
    return bits;
}

/**
 * @brief Funcion que oscurece todo el buffer con un tramado ordenado (matriz de Bayer de 4x4).
 * Con un solo bit por pixel no hay grises: apagar cada vez mas pixeles en un patron parejo
 * se ve como un fundido.
 * @param level 0 = sin cambio ... 16 = todo negro. Cada nivel apaga los mismos pixeles que el anterior y uno mas de cada 16.
 * @param shift Mueve el patron (0 a 15). Con el mismo shift en cada cuadro el fundido se ve parejo; cambiandolo,
 *              cada cuadro apaga pixeles distintos y lo que se dibujo antes se va borrando poco a poco (estelas).
 */
void DitherDisplay(uint8_t level, uint8_t shift)
{
    if (level == 0)
    {
        return;
    }
    uint8_t mask[4];
    for (int col = 0; col < 4; col++)
    {
        mask[col] = ~BayerColumn(col, shift & 3, (shift >> 2) & 3, level);
    }

    uint8_t *buffer = display.getBufferPtr();
    int width = display.getBufferTileWidth() * 8;
    int size = width * display.getBufferTileHeight();
    for (int i = 0; i < size; i++)
    {
        buffer[i] &= mask[(i % width) & 3];
    }
}

/**
 * @brief Funcion que pinta un rectangulo con un "gris" de puntos (el mismo tramado de Bayer).
 * Tapa lo que habia: los puntos apagados del patron quedan en negro. Escribe directo en el buffer,
 * asi que es rapida aunque se llame miles de veces por cuadro.
 * @param level 0 = negro ... 16 = blanco
 */
void FillDither(int x, int y, int w, int h, uint8_t level)
{
    // Patron de cada nivel, calculado una sola vez
    static uint8_t patterns[17][4];
    static bool ready = false;
    if (!ready)
    {
        for (int l = 0; l <= 16; l++)
        {
            for (int col = 0; col < 4; col++)
            {
                patterns[l][col] = BayerColumn(col, 0, 0, l);
            }
        }
        ready = true;
    }
    if (level > 16)
    {
        level = 16;
    }

    // Recorte contra la pantalla
    if (x < 0)
    {
        w += x;
        x = 0;
    }
    if (y < 0)
    {
        h += y;
        y = 0;
    }
    if (x + w > ANCHO_PANTALLA)
    {
        w = ANCHO_PANTALLA - x;
    }
    if (y + h > ALTO_PANTALLA)
    {
        h = ALTO_PANTALLA - y;
    }
    if (w <= 0 || h <= 0)
    {
        return;
    }

    uint8_t *buffer = display.getBufferPtr();
    int width = display.getBufferTileWidth() * 8;
    int lastPage = (y + h - 1) / 8;
    for (int page = y / 8; page <= lastPage; page++)
    {
        // Filas de esta pagina que caen dentro del rectangulo
        int top = y > page * 8 ? y - page * 8 : 0;
        int bottom = y + h < page * 8 + 8 ? y + h - page * 8 : 8;
        uint8_t rows = (uint8_t)((0xFF << top) & (0xFF >> (8 - bottom)));
        uint8_t *column = buffer + page * width + x;
        for (int i = 0; i < w; i++)
        {
            column[i] = (column[i] & ~rows) | (patterns[level][(x + i) & 3] & rows);
        }
    }
}

/**
 * @brief Funcion que limita todo lo que se dibuja a un rectangulo (x1 y y1 no se incluyen)
 */
void SetClipWindow(int x0, int y0, int x1, int y1)
{
    x0 = constrain(x0, 0, ANCHO_PANTALLA);
    x1 = constrain(x1, 0, ANCHO_PANTALLA);
    y0 = constrain(y0, 0, ALTO_PANTALLA);
    y1 = constrain(y1, 0, ALTO_PANTALLA);
    display.setClipWindow(x0, y0, x1, y1);
}

/**
 * @brief Funcion que vuelve a dejar dibujar en toda la pantalla
 */
void ResetClipWindow()
{
    display.setMaxClipWindow();
}

/**
 * @brief Funcion que dibuja texto UTF-8 (acentos, ñ, ¿ y ¡). Necesita una fuente con esos caracteres (*_ES)
 */
void DrawTextUTF8(int x, int y, const char *text)
{
    display.drawUTF8(x, y, text);
}

/**
 * @brief Funcion que regresa el ancho en pixeles de un texto UTF-8 con la fuente actual
 */
int TextWidthUTF8(const char *text)
{
    return display.getUTF8Width(text);
}

/**
 * @brief Funcion que regresa cuantos pixeles sube la fuente actual sobre la linea base
 */
int TextAscent()
{
    return display.getAscent();
}

/**
 * @brief Funcion que regresa cuantos pixeles baja la fuente actual bajo la linea base (numero negativo)
 */
int TextDescent()
{
    return display.getDescent();
}

/**
 * @brief Funcion que permite esperar un tiempo usando millis()
 */
bool wait(unsigned long durationMs)
{
    unsigned long current = millis();

    if (!waitActive || waitDuration != durationMs)
    {
        waitStart = current;
        waitDuration = durationMs;
        waitActive = true;
        return false;
    }

    if (current - waitStart >= waitDuration)
    {
        waitActive = false;
        return true;
    }

    return false;
}

/**
 * @brief Cambia el tamaño de la fuente de la pantalla OLED.
 * @param size El tamaño deseado (1 = Chico, 2 = Mediano, 3 = Grande, FONT_TINY = 5x7 para datos;
 *             las *_ES traen acentos y ñ para DrawTextUTF8).
 */
void SetCustomFont(FontSize size)
{
    switch (size)
    {
    case FONT_SMALL:
        display.setFont(u8g2_font_ncenB08_tr);
        break;
    case FONT_MEDIUM:
        display.setFont(u8g2_font_ncenB14_tr);
        break;
    case FONT_LARGE:
        display.setFont(u8g2_font_ncenB24_tr);
        break;
    case FONT_TINY:
        display.setFont(u8g2_font_5x7_tr);
        break;
    case FONT_TINY_ES:
        display.setFont(u8g2_font_5x7_tf);
        break;
    case FONT_SMALL_ES:
        display.setFont(u8g2_font_helvB08_tf);
        break;
    case FONT_MEDIUM_ES:
        display.setFont(u8g2_font_helvB14_tf);
        break;
    case FONT_BUBBLE:
        display.setFont(u8g2_font_bubble_tr);
        break;
    }
}