#include "pong.h"
#include "../../drivers/input/buttons.h"
#include "../../drivers/display/display.h"
// #include "states_displays.h" // Todo: Mejorar por codigo y no por bitmap

namespace pong
{

    void ball_init();
    void ball_start();
    void move_ball();

    int pong_direccion = 1;
    const int botonArriba = 2;
    const int botonAbajo = 19;
    const int botonDerecha = 4;
    const int botonIzquierda = 16;
    const int botonSelect = 5;

    const int pong_len_block = 4;
    const int len_block = 4;
    const int width = 12;
    const int limite_inferior_player = ALTO_PANTALLA - width;
    const int ball_size = 4;
    const int limite_inferior = ALTO_PANTALLA;
    const int limite_superior_ball = ALTO_PANTALLA + ball_size;
    const int limite_superior = 0;

    int ball_directionX = 1;
    int ball_directionY = 1;

    int y_AI = 0 + len_block;
    int x_AI = ANCHO_PANTALLA - len_block;
    int x = 2;
    int y = 0;
    int ball_x = 0;
    int ball_y = 0;
    int ball_velocity = 4;
    int user_points = 0;
    int count_user = 0;
    int count_enemy = 0;
    bool start = false; // TODO: Hacer la condicion de que cuando alguno pierda se le ponga el marcador y
    // TODO -- para eso se necesita el bool start para que indique cuando aparezca y desaparezca

    const int calcPositionX()
    {
        return ball_x + (ball_velocity * ball_directionX);
    }

    const int calcPositionY()
    {
        return ball_y + (ball_velocity * ball_directionY);
    }

    void ball_start()
    {
        // Simplemente le da un empujón inicial a la bola si fuera necesario
        // o cambia la dirección inicial.
    }

    void ball_init()
    {
        ball_x = ANCHO_PANTALLA / 2;
        ball_y = ALTO_PANTALLA / 2;
        ball_directionX = (random(0, 2) == 0) ? 1 : -1;
        ball_directionY = (random(0, 2) == 0) ? 1 : -1;
    }

    void move_ball()
    {
        int dir = input.realDirection();
        int expectedX = calcPositionX();
        int expectedY = calcPositionY();

        if (expectedY <= 0 || expectedY >= limite_superior_ball)
        {
            ball_directionY *= -1;
            expectedY = calcPositionY();
        }

        if (expectedX <= x + len_block)
        {
            if (expectedY + ball_size >= y && expectedY <= y + width)
            {
                ball_directionX = 1;
                expectedX = calcPositionX();
            }
            else if (expectedX < 0)
            {
                count_user++;
                // user_counter();
                if (dir == 5)
                {
                    ball_init();
                    return;
                }
            }
        }

        if (expectedX >= x_AI - ball_size)
        {
            if (expectedY + ball_size >= y_AI && expectedY <= y_AI + width)
            {
                ball_directionX = -1;
                expectedX = calcPositionX();
            }
            else if (expectedX > ANCHO_PANTALLA)
            {
                count_enemy++;
                // user_counter();
                if (dir == 5)
                {
                    ball_init();
                    return;
                }
            }
        }

        ball_x = expectedX;
        ball_y = expectedY;
    }

    int moveAi()
    {
        if (ball_y > y_AI + (width / 2))
            y_AI += 1;
        else if (ball_y < y_AI + (width / 2))
            y_AI -= 1;

        if (y_AI < 0)
            y_AI = 0;
        if (y_AI > limite_inferior_player)
            y_AI = limite_inferior_player;
        return y_AI;
    }

    void pong_buttons_read()
    {
        int dir = input.realDirection();
        if (dir == 1)
            pong_direccion = 0;
        else if (dir == 2)
            pong_direccion = 1;
    }

    void pong_direcciones()
    {
        if (pong_direccion == 0)
            y -= 2;
        if (pong_direccion == 1)
            y += 2;
    }

    void screen_limits()
    {
        if (y <= limite_superior)
            y = limite_superior;
        else if (y > limite_inferior_player)
            y = limite_inferior_player;
    }

    void print_player()
    {
        // display.fillRect(x, y, len_block, width, SSD1306_WHITE);
        DrawBox(x, y, len_block, width);
    }

    void print_AI()
    {
        // display.fillRect(x_AI, moveAi(), len_block, width, SSD1306_WHITE);
        DrawBox(x_AI, moveAi(), len_block, width);
    }

    void print_ball()
    {
        // display.fillRect(ball_x, ball_y, ball_size, ball_size, SSD1306_WHITE);
        DrawBox(ball_x, ball_y, ball_size, ball_size);
    }

    void setup_pong()
    {
        ball_init();
    }

    void user_counter()
    {
        // pong_counter(display, count_user, count_enemy);
        // TODO: Hacer la pantalla de puntuacion
    }

    void game_pong()
    {
        // display.clearDisplay();
        ClearDisplay();

        pong_buttons_read();
        pong_direcciones();
        screen_limits();

        print_player();
        print_AI();
        move_ball();
        print_ball();

        // display.display();
        ActDisplay();
        delay(20); // Todo: Actualizar a millis()
    }

}