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
    if (currentState == STATE_MENU)
    {
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
            // Actualiza o inicia Snake
            break;
        case STATE_PONG:
            // Actualiza o inicia Pong
            break;
        case STATE_TETRIS:
            // Actualiza o inicia Tetris
            break;
        case STATE_CONFIG:
            // Actualiza o muestra la pantalla de configuración
            break;
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