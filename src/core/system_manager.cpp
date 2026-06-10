#include "system_manager.h"
#include <Arduino.h>
#include "../drivers/display/display.h"

void SystemManager::begin()
{
    Serial.begin(115200);
    InitDisplay();

    ClearDisplay();

    DrawLogo();
}

void SystemManager::update()
{
    // Lógica principal del sistema
}
