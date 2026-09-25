
#include "../../drivers/display/display.h"
#include "../../drivers/input/buttons.h"
#include "../../drivers/time/millis.h"
#include "../../assets/images/game_over.h"
#include "../../assets/images/flappy_bird_images/flappy_background.h"
#include "flappy_bird.h"
#include "../../config/debug_log.h"

Pajaro::Pajaro(int xInicio, int yInicio)
{
    x = xInicio;
    y = yInicio;
    vel_y = 0;
    gravity = 0.4;
}

void Pajaro::move()
{

    vel_y += gravity;
    y += vel_y;
    if (y >= 64)
    {
        y = 60;
    }
}

void Pajaro::print()
{
    // ClearDisplay();
    DrawBox(x, y, size, size);
    // ActDisplay();
}

void Pajaro::jump()
{
    int dir = input.realDirection();
    if (dir == 5)
    {
        vel_y = -3.5;
    }
}

int Pajaro::getX()
{
    return x;
}

int Pajaro::getY()
{
    return y;
}

unsigned Pajaro::getSize()
{
    return size;
}

Pipe::Pipe(int x)
{
    x_pipe = x;
    high_superior = random(8, 40);
    high_inferior = 64 - high_superior - pipe_hole;
}

// void Pipe::move()
// {
//     x_pipe -= 2;
// }

void Pipe::move()
{
    x_pipe -= 2;

    if (x_pipe < -(int)len_pipe)
    {
        x_pipe = ANCHO_PANTALLA;

        high_superior = random(8, 40);
        high_inferior = 64 - high_superior - pipe_hole;
    }
}

void Pipe::render()
{
    // ClearDisplay();
    DrawBox(x_pipe, 0, len_pipe, high_superior);
    DrawBox(x_pipe, high_superior + pipe_hole, len_pipe, high_inferior);
    // ActDisplay();
}

bool Pipe::checkCollision(int pajaroX, int pajaroY, int pajaroSize)
{
    if (pajaroX + pajaroSize > x_pipe && pajaroX < x_pipe + len_pipe)
    {
        if (pajaroY < high_superior || pajaroY + pajaroSize > high_superior + pipe_hole)
        {
            return true;
        }
    }
    return false;
}

void Pipe::GameOver()
{
    // Tomas los ultimos valores de 'x' y 'y' cuando se detecte la colision
    //  y paras la velocidad y la gravedad
    // x,y,bool de la colision, tamaño,
    DrawBitmap(GameOverBitmap, gameOverWidth, gameOverHeight);
}

Pajaro miPajaro(ANCHO_PANTALLA / 2, ALTO_PANTALLA / 2);
Pipe myPipe(ANCHO_PANTALLA / 2);

// Estado propio de Flappy Bird. Va en un namespace anonimo para no chocar con el
// 'states' y el 'ACTUAL_STATE' de Snake (system_manager.cpp incluye los dos juegos).
namespace
{
    enum class states
    {
        INIT,
        START,
        GAME_OVER,
        AGAIN
    };

    states ACTUAL_STATE = states::INIT;
    bool okLiberado = false; // En Game Over hay que soltar OK antes de poder reiniciar
}

void flappy_bird()
{
    // Game Over: la imagen se queda en pantalla hasta soltar y volver a presionar OK
    if (ACTUAL_STATE == states::GAME_OVER)
    {
        myPipe.GameOver();
        if (!isPressed(BTN_OK))
        {
            okLiberado = true;
            return;
        }
        if (!okLiberado)
        {
            return;
        }
        miPajaro = Pajaro(ANCHO_PANTALLA / 2, ALTO_PANTALLA / 2);
        myPipe = Pipe(ANCHO_PANTALLA / 2);
    }

    ACTUAL_STATE = states::START;

    while (ACTUAL_STATE == states::START)
    {
        ClearDisplay();
        DrawImage(0, 0, flappyBackgroundWidth, flappyBackgroundHeight, FlappyBackgroundBitmap);
        miPajaro.jump();
        miPajaro.move();
        miPajaro.print();
        myPipe.move();
        myPipe.render();

        if (myPipe.checkCollision(miPajaro.getX(), miPajaro.getY(), miPajaro.getSize()))
        {
            DEV_PRINTLN("Game Over");
            ACTUAL_STATE = states::GAME_OVER;
            okLiberado = false;
            break;
        }
        ActDisplay();
        millis(20);
    }
    myPipe.GameOver();
    miPajaro.print();
    myPipe.render();
}