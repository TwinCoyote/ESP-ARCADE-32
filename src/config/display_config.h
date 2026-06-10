#pragma once

#include <cstdint>

namespace DisplayConfig
{
    constexpr int WIDTH = 128;
    constexpr int HEIGHT = 64;

    constexpr int SDA_PIN = 21;
    constexpr int SCL_PIN = 22;

    constexpr int RESET_PIN = -1;

    constexpr uint8_t I2C_ADDRESS = 0x3C;
}