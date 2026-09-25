#include "update.h"
#include "../../../assets/images/updating.h"
#include "../../../config/debug_log.h"

// static const char *OTA_CURRENT_VERSION = "v1.0.5";
UpdateMenu::UpdateMenu() : _ota(OTAService::readVersion().c_str(), "TwinCoyote", "ESP-ARCADE-32")
{
    // _otaChecked = false;
    ;
}

void UpdateMenu::displayUpdate()
{
    // Se queda en pantalla mientras performUpdate() busca y descarga la actualizacion
    DrawBitmap(UpdatingBitmap, updatingWidth, updatingHeight);
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
        DEV_PRINTLN("[Update] Buscando actualizaciones de fondo...");
        // _otaChecked = true;
        _ota.performUpdate();
    };
}