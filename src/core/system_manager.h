#pragma once

class SystemManager
{
public:
    enum State
    {
        STATE_MENU,
        STATE_SNAKE,
        STATE_PONG,
        STATE_TETRIS,
        STATE_CONFIG,
        STATE_BIRD,
        STATE_WIFI_CONFIG,
        STATE_UPDATE_CONFIG,
        STATE_INFO
    };

    void begin();
    void update();
    // Protector de pantalla por inactividad; true mientras tiene la pantalla
    bool animation();

    void setState(State s);
    State getState() const;

private:
    State currentState = STATE_MENU;
};