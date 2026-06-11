#pragma once

#include <cstdint>

#define DISPLAY_CONTROLLER_SH1106 1
#define DISPLAY_CONTROLLER_SSD1306 2
#define DISPLAY_CONTROLLER DISPLAY_CONTROLLER_SH1106

namespace DisplayConfig
{
    constexpr int WIDTH = 128;
    constexpr int HEIGHT = 64;

    constexpr int SDA_PIN = 21;
    constexpr int SCL_PIN = 22;

    constexpr int RESET_PIN = -1;

    constexpr uint8_t I2C_ADDRESS = 0x3C;
}