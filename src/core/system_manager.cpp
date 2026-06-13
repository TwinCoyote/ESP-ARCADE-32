#include "system_manager.h"
#include <Arduino.h>
#include "../drivers/display/display.h"
#include "../ui/menu.h"
#include "../drivers/input/buttons.h"

bool primeraVez = true;

void SystemManager::begin()
{
    Serial.begin(115200);
    InitButtons();
    InitDisplay();
    ClearDisplay();
    DrawLogo();
    delay(500);
    ClearDisplay();
    DrawMenu();
    ActDisplay();
    delay(500);
    MenuInit();
}

void SystemManager::update()
{
    // Actualizamos la lógica y guardamos si el usuario movió el menú
    bool huboMovimiento = MenuUpdate();

    // Solo renderizamos si es la primera vez que enciende o si se presionó un botón
    if (huboMovimiento || primeraVez)
    {
        MenuRender();
        primeraVez = false; // Ya se dibujó el estado inicial
    }

    // Lógica principal del sistema (juegos, etc.)
}