#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "display.h"
// #include "assets.h"
#include "../../assets/images/assets.h"
#include "../../config/display_config.h"

using namespace DisplayConfig;

Adafruit_SSD1306 display(
    WIDTH,
    HEIGHT,
    &Wire,
    RESET_PIN);

void InitDisplay()
{
    // Inicializar pantalla (dirección I2C típica 0x3C)
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
    {
        Serial.println("Failed to initialize display");
        return;
    }
    display.invertDisplay(true); // Invertir display si está al revés
    display.clearDisplay();
    display.display();
}
void ClearDisplay()
{
    display.clearDisplay();
    display.display();
}

void DrawBitmap(const unsigned char *bitmap, int width, int height)
{
    display.clearDisplay();
    display.drawBitmap(0, 0, bitmap, width, height, 1);
    display.display();
}

void DrawLogo()
{
    display.display();
    DrawBitmap(Logo_R, logoWidth, logoHeight);
}

void DrawMenu()
{
    DrawBitmap(menu_principal, logoWidth, logoHeight);
}
