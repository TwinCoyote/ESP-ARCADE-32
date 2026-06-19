
#include "../../drivers/display/display.h"
#include "../../drivers/input/buttons.h"
#include "../../drivers/time/millis.h"
#include "pruebas.h"

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

Pajaro miPajaro(ANCHO_PANTALLA / 2, ALTO_PANTALLA / 2);
Pipe myPipe(ANCHO_PANTALLA / 2);

void flappy_bird()
{
    ClearDisplay();
    miPajaro.jump();
    miPajaro.move();
    miPajaro.print();
    myPipe.move();
    myPipe.render();

    if (myPipe.checkCollision(miPajaro.getX(), miPajaro.getY(), miPajaro.getSize()))
    {
        Serial.println("Game Over");
        // TODO: Make a gameOver Function
    }
    ActDisplay();
    millis(20);
}