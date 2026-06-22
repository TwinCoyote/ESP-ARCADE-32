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
    };

    void begin();
    void update();

    void setState(State s);
    State getState() const;

private:
    State currentState = STATE_MENU;
};