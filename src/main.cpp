#include <Arduino.h>
#include "core/system_manager.h"

SystemManager systemManager;

void setup()
{
  systemManager.begin();
}

void loop()
{
  systemManager.update();
  delay(16);
}