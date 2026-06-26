#ifndef UPDATE_MENU_H
#define UPDATE_MENU_H

#include <Arduino.h>
#include "../config_menu.h"
#include "../../../drivers/input/buttons.h"
#include "core0/services/ota/OTA.h"
#include "../../../drivers/display/display.h"

class UpdateMenu
{
private:
    OTAService _ota;

public:
    UpdateMenu();
    void logicUpdateMenu();
    void displayUpdate();
};

#endif