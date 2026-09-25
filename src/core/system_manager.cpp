#include "system_manager.h"
#include <Arduino.h>
#include "../drivers/display/display.h"
#include "../ui/menu.h"
#include "../drivers/input/buttons.h"
#include "../games/snake/Snake.h"
#include "../games/pong/pong.h"
#include "../games/flappy_bird/flappy_bird.h"
#include "../ui/config/config_menu.h"
#include "../ui/config/WiFi/wifi_display.h"
#include "../ui/config/update/update.h"
#include "../ui/config/info/info.h"
#include "../config/debug_log.h"

bool primeraVez = true;
static WiFiService wifiService("", "");
static WifiMenu wifiMenu;
static InfoMenu infoMenu;

void SystemManager::begin()
{
    Serial.begin(115200);
    //
    // delay(500);

    // DEV_PRINTLN("--- INFORMACIÓN DEL SILICIO ---");

    // DEV_PRINT("Modelo de ESP32: ");
    // DEV_PRINTLN(ESP.getChipModel());

    // DEV_PRINT("Núcleos de CPU: ");
    // DEV_PRINTLN(ESP.getChipCores());

    // DEV_PRINT("Tamaño de Flash: ");
    // DEV_PRINT(ESP.getFlashChipSize() / (1024 * 1024));
    // DEV_PRINTLN(" MB");

    // DEV_PRINT("¿Tiene PSRAM?: ");
    // if (psramInit())
    // {
    //     DEV_PRINT("SÍ, tamaño: ");
    //     DEV_PRINT(ESP.getPsramSize() / 1024);
    //     DEV_PRINTLN(" KB");
    // }
    // else
    // {
    //     DEV_PRINTLN("NO (Solo los 520KB de SRAM interna)");
    // }
    // DEV_PRINTLN("--------------------------------");

    // //
    input.begin();
    InitDisplay();
    ClearDisplay();
    DrawLogo();
    delay(1000);
    ClearDisplay();
    DrawMenu();
    ActDisplay();
    delay(500);
    MenuInit();
    pinMode(2, OUTPUT);

    wifiService.begin();
}

void SystemManager::update()
{
    if (currentState == STATE_MENU)
    {
        // digitalWrite(2, LOW);
        // Actualizamos la lógica y guardamos si el usuario movió el menú
        bool huboMovimiento = MenuUpdate();

        // Solo renderizamos si es la primera vez que enciende o si se presionó un botón
        if (huboMovimiento || primeraVez)
        {
            MenuRender();
            primeraVez = false; // Ya se dibujó el estado inicial
        }

        if (MenuConfirm())
        {
            int valor = MenuGetIndex();
            switch (valor)
            {
                // case 0:
                //     currentState = STATE_MENU;
                //     DEV_PRINTLN("Switching to Menu");

                //     break;

            case 0:
                currentState = STATE_SNAKE;
                DEV_PRINTLN("Switching to Snake");
                break;
            case 1:
                currentState = STATE_PONG;
                DEV_PRINTLN("Switching to Pong");
                break;
            case 2:
                currentState = STATE_TETRIS;
                DEV_PRINTLN("Switching to Tetris");
                break;
            case 3:
                currentState = STATE_CONFIG;
                DEV_PRINTLN("Switching to Config");
                break;
            case 4:
                currentState = STATE_BIRD;
                DEV_PRINT("Switching to Flappy Bird");
                break;

            default:
                break;
            }
        }
    }
    else
    {

        switch (currentState)
        {
        case STATE_SNAKE:
        {
            if (MenuBack())
            {
                currentState = STATE_MENU;
                primeraVez = true;
            }
            // digitalWrite(2, HIGH);
            snake_game();
            break;
        }

        case STATE_PONG:
        {
            if (MenuBack())
            {
                currentState = STATE_MENU;
                primeraVez = true;
            }
            // digitalWrite(2, HIGH);
            pong::game_pong();
            break;
        }

        case STATE_TETRIS:
        {
            if (MenuBack())
            {
                currentState = STATE_MENU;
                primeraVez = true;
            }
            // digitalWrite(2, HIGH);
            break;
        }

        case STATE_CONFIG:
        {
            if (MenuBack())
            {
                currentState = STATE_MENU;
                primeraVez = true;
            }
            static const char *opcionesConfig[] = {"Wifi", "Update", "Info"};
            static const unsigned int totalOpciones = sizeof(opcionesConfig) / sizeof(opcionesConfig[0]);
            static MenuS menuConfig(opcionesConfig, totalOpciones);

            int dires = input.realDirection();
            menuConfig.handleInput(dires);
            menuConfig.render();

            if (MenuConfirm())
            {
                unsigned int sel = menuConfig.getIndex();
                switch (sel)
                {
                case 0:
                    if (MenuBack())
                    {
                        currentState = STATE_MENU;
                        primeraVez = true;
                    }
                    DEV_PRINTLN("Config -> Wifi selected");
                    currentState = STATE_WIFI_CONFIG;
                    wifiMenu.init(&wifiService);
                    break;
                case 1:
                    if (MenuBack())
                    {
                        currentState = STATE_MENU;
                        primeraVez = true;
                    }
                    DEV_PRINTLN("Config -> Update selected");
                    currentState = STATE_UPDATE_CONFIG;

                    // TODO: make the display for update firmware and the functions
                    break;
                case 2:
                    DEV_PRINTLN("Config -> Info selected");
                    currentState = STATE_INFO;
                    infoMenu.reset();
                    break;
                default:
                    break;
                }
            }
            break;
        }

        case STATE_BIRD:
        {
            if (MenuBack())
            {
                currentState = STATE_MENU;
                primeraVez = true;
            }
            // digitalWrite(2, HIGH);
            flappy_bird();
            break;
        }

        case STATE_WIFI_CONFIG:
        {
            if (MenuBack())
            {
                currentState = STATE_CONFIG;
                wifiMenu.reset();
            }
            wifiMenu.update();
            wifiMenu.render();
            break;
        }
        case STATE_UPDATE_CONFIG:
        {
            if (MenuBack())
            {
                currentState = STATE_CONFIG;
            }
            static UpdateMenu menuUpdate;
            menuUpdate.logicUpdateMenu();
            break;
        }
        case STATE_INFO:
        {
            // Info maneja su propio BACK: la B del codigo secreto no debe sacarte de la pantalla
            if (infoMenu.update())
            {
                currentState = STATE_CONFIG;
                break;
            }
            infoMenu.render();
            break;
        }

        default:
            break;
        }
    }
}

void SystemManager::setState(State s)
{
    currentState = s;
}

SystemManager::State SystemManager::getState() const
{
    return currentState;
}