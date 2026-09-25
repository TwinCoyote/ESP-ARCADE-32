#pragma once

#include <Arduino.h>
#include "debug_config.h"

#if DEV_MODE

#define DEV_PRINT(...) Serial.print(__VA_ARGS__)
#define DEV_PRINTLN(...) Serial.println(__VA_ARGS__)
#define DEV_PRINTF(...) Serial.printf(__VA_ARGS__)

#else

#define DEV_PRINT(...)
#define DEV_PRINTLN(...)
#define DEV_PRINTF(...)

#endif