#include "update.h"
static const char *OTA_CURRENT_VERSION = "v1.0.5";
UpdateMenu::UpdateMenu() : _ota(OTA_CURRENT_VERSION, "TwinCoyote", "ESP-ARCADE-32")
{
    // _otaChecked = false;
    ;
}

void UpdateMenu::displayUpdate()
{
    ClearDisplay();
    DrawText(50, 30, "Update..."); // TODO: Provisional, cambiar por un bitmap
    ActDisplay();
    wait(1000);
}

void UpdateMenu::logicUpdateMenu()
{
    static const char *opcionesUpdate[] = {"Update Console"};
    static const unsigned int totalOpciones = sizeof(opcionesUpdate) / sizeof(opcionesUpdate[0]);
    static MenuS menuUpdate(opcionesUpdate, totalOpciones);
    int dires = input.realDirection();
    menuUpdate.handleInput(dires);
    menuUpdate.render();

    if (MenuConfirm())
    {
        displayUpdate();
        Serial.println("[Update] Buscando actualizaciones de fondo...");
        // _otaChecked = true;
        _ota.performUpdate();
    };
}