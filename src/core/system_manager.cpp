#include "system_manager.h"
#include <Arduino.h>
#include "../drivers/display/display.h"
#include "../ui/menu.h"
#include "../drivers/input/buttons.h"
#include "../games/snake/Snake.h"
#include "../games/pong/pong.h"
#include "../games/pruebas/pruebas.h"
#include "../ui/config/config_menu.h"

bool primeraVez = true;

void SystemManager::begin()
{
    Serial.begin(115200);
    input.begin();
    InitDisplay();
    ClearDisplay();
    DrawLogo();
    delay(500);
    ClearDisplay();
    DrawMenu();
    ActDisplay();
    delay(500);
    MenuInit();
    pinMode(2, OUTPUT);
}

void SystemManager::update()
{
    if (currentState == STATE_MENU)
    {
        digitalWrite(2, LOW);
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
                //     Serial.println("Switching to Menu");

                //     break;

            case 0:
                currentState = STATE_SNAKE;
                Serial.println("Switching to Snake");
                break;
            case 1:
                currentState = STATE_PONG;
                Serial.println("Switching to Pong");
                break;
            case 2:
                currentState = STATE_TETRIS;
                Serial.println("Switching to Tetris");
                break;
            case 3:
                currentState = STATE_CONFIG;
                Serial.println("Switching to Config");
                break;
            case 4:
                currentState = STATE_BIRD;
                Serial.print("Switching to Flappy Bird");
                break;

            default:
                break;
            }
        }
    }
    else
    {
        // Aquí se ejecuta la lógica del estado activo (juego o configuración)
        switch (currentState)
        {
        case STATE_SNAKE:
        {
            if (MenuBack())
            {
                currentState = STATE_MENU;
                primeraVez = true;
            }
            digitalWrite(2, HIGH);
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
            digitalWrite(2, HIGH);
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
            digitalWrite(2, HIGH);
            break;
        }

        case STATE_CONFIG:
        {
            if (MenuBack())
            {
                currentState = STATE_MENU;
                primeraVez = true;
            }
            digitalWrite(2, HIGH);
            int dires = input.realDirection();
            configMenu.handleInput(dires);
            configMenu.render();
            break;
        }

        case STATE_BIRD:
        {
            if (MenuBack())
            {
                currentState = STATE_MENU;
                primeraVez = true;
            }
            digitalWrite(2, HIGH);
            flappy_bird();
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