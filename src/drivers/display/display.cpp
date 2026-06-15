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
 * @brief Funcion que imprime los logos con cordenandas preestablcidas
 */
void DrawLogo()
{
    display.clearBuffer();
    DrawBitmap(Logo_R, logoWidth, logoHeight);
    display.sendBuffer();
}

/**
 * @brief Funcion que imprime el background de fondo
 */
void DrawMenu()
{
    // display.clearBuffer();
    DrawBitmap(menu_principal, logoWidth, logoHeight);
    // display.sendBuffer();
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
 * @param size El tamaño deseado (1 = Chico, 2 = Mediano, 3 = Grande).
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
    }
}