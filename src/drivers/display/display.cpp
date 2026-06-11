#include <Arduino.h>
#include <U8g2lib.h>
#include "display.h"
#include "../../assets/images/assets.h"
#include "../../config/display_config.h"

using namespace DisplayConfig;
static unsigned long waitStart = 0;
static unsigned long waitDuration = 0;
static bool waitActive = false;

#if DISPLAY_CONTROLLER == DISPLAY_CONTROLLER_SH1106
U8G2_SH1106_128X64_NONAME_F_HW_I2C display(U8G2_R0, U8X8_PIN_NONE);
#else
U8G2_SSD1306_128X64_NONAME_F_HW_I2C display(U8G2_R0, U8X8_PIN_NONE);
#endif

void InitDisplay()
{
    display.begin();
    display.setContrast(255);
    display.clearBuffer();
    display.sendBuffer();
}

void ClearDisplay()
{
    display.clearBuffer();
    display.sendBuffer();
}

void DrawBitmap(const unsigned char *bitmap, int width, int height)
{
    display.clearBuffer();
    display.drawXBMP(0, 0, width, height, bitmap);
    display.sendBuffer();
}

void DrawLogo()
{
    display.clearBuffer();
    DrawBitmap(Logo_R, logoWidth, logoHeight);
    display.sendBuffer();
}

void DrawMenu()
{
    display.clearBuffer();
    DrawBitmap(menu_principal, logoWidth, logoHeight);
    display.sendBuffer();
}

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