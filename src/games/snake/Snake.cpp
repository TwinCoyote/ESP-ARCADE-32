// #include "input.h"
#include "Snake.h"
#include "../../drivers/display/display.h"
#include "../../assets/images/snake_images/snake_background.h"
#include "../../assets/images/snake_images/snake_game_over.h"

// #define ANCHO_PANTALLA 128
// #define ALTO_PANTALLA 64
#define DIRECCION_OLED 0x3C
#define LONGITUD_MAXIMA 50

// extern Adafruit_SSD1306 display;
extern Input input;

const unsigned long eventInterval = 1000;
unsigned long previousTime = 0;

// int direccion = 1;
directions direccion = directions::DOWN;

states ACTUAL_STATE = states::INIT;

// int x = 0;   // posición X
// int y = 0;   // posición Y
int largo = 3;

int snake_x[LONGITUD_MAXIMA];
int snake_y[LONGITUD_MAXIMA];
// int is_alive = true;

int rans = 40;
int ran = 30;
const int len_block = 4;                 // 3
int comidaX = random(0, 32) * len_block; // 42
int comidaY = random(0, 16) * len_block; // 21

// Funciones Internas

void game_letters()
{
    int dir = input.realDirection();
    ClearDisplay();
    SetCustomFont(FONT_SMALL);
    DrawText(15, 10, "Iniciando Snake...");
    delay(100); // TODO: Cambiar Delay por millis...
    DrawText(15, 20, "Presion Start!");
    ActDisplay();

    if (dir == 5)
    {
        ACTUAL_STATE = states::START;
    }
}

void random_food()
{
    comidaX = random(0, 32) * len_block; // 42
    comidaY = random(0, 16) * len_block; // 21

    // ActDisplay();
    // display.fillRect(comidaX, comidaY, 3, 3, SSD1306_WHITE);
}

void check_food()
{

    DrawBox(comidaX, comidaY, 3, 3);
    if (snake_x[0] == comidaX && snake_y[0] == comidaY)
    {
        random_food();
        if (largo < LONGITUD_MAXIMA)
        {
            largo = largo + 1;
        }
        else
        {
            ClearDisplay();
            DrawText(0, 5, "Has Ganado!");
        }
    }
}

void direcciones()
{
    switch (direccion)
    {
    case directions::UP:
        snake_y[0] -= len_block;
        break;
    case directions::DOWN:
        snake_y[0] += len_block;
        break;
    case directions::RIGHT:
        snake_x[0] += len_block;
        break;
    case directions::LEFT:
        snake_x[0] -= len_block;
        break;
    };

    // if (direccion == 0)
    //     snake_y[0] -= len_block;
    // if (direccion == 1)
    //     snake_y[0] += len_block;
    // if (direccion == 2)
    //     snake_x[0] += len_block;
    // if (direccion == 3)
    //     snake_x[0] -= len_block;
}

// void buttons_read() {
//     if (digitalRead(botonArriba) == LOW) {
//         direccion = 2;
//     }
//     if (digitalRead(botonAbajo) == LOW) {
//         direccion = 19;
//     }
//     if (digitalRead(botonDerecha) == LOW) {
//         direccion = 4;
//     }
//     if (digitalRead(botonIzquierda) == LOW) {
//         direccion = 16;
//     }

// }

directions opposite(directions dire)
{
    switch(dire){
        case directions::UP:
        return directions::DOWN;
        
        case directions::DOWN:
        return directions::UP;

        case directions::LEFT:
        return directions::RIGHT;

        case directions::RIGHT:
        return directions::LEFT;

        default:
        return directions::DOWN;
    }
}

void buttons_read()
{

    directions newDirection = direccion;

    int dir = input.realDirection();
    if (dir == 1) // Arriba
    {
        newDirection = directions::UP;
    }
    else if (dir == 2) // Abajo
    {
        newDirection = directions::DOWN;
    }
    else if (dir == 3) // Derecha
    {
        newDirection = directions::RIGHT;
    }
    else if (dir == 4) // Izquierda
    {
        newDirection = directions::LEFT;
    }
    else if (dir == 5)
    {
    }
    if (newDirection != opposite(direccion))
    {
        direccion = newDirection;
    }
}

void space_limits()
{
    if (snake_x[0] >= ANCHO_PANTALLA)
    {
        snake_x[0] = 0;
    }
    else if (snake_x[0] < 0)
    {
        snake_x[0] = ANCHO_PANTALLA - len_block;
    }

    if (snake_y[0] >= ALTO_PANTALLA)
    {
        snake_y[0] = 0;
    }
    else if (snake_y[0] < 0)
    {
        snake_y[0] = ALTO_PANTALLA - len_block;
    }
}

void print_snake()
{
    for (int i = 0; i < largo; i++)
    {

        // display.fillRect(snake_x[i], snake_y[i], len_block, len_block, SSD1306_WHITE);
        DrawBox(snake_x[i], snake_y[i], len_block, len_block);
    }
}

// Funcion Beta para el recorrimiento de las cordenadas; la intencion es que se pueda usar tanto con
//  las cordenadas en x como en las de y

void body()
{
    for (int i = largo - 1; i > 0; i--)
    {
        snake_x[i] = snake_x[i - 1];
        snake_y[i] = snake_y[i - 1];
    }
}

// void beta(){
//   for (int i = 0; i < largo; i++) {
//   DEV_PRINT("Segmento ");
//   DEV_PRINT(i);
//   DEV_PRINT(": X=");
//   DEV_PRINT(snake_x[i]);
//   DEV_PRINT(" Y=");
//   DEV_PRINTLN(snake_y[i]);
// }
// DEV_PRINTLN("----");
// }

void lose_conditions()
{
    for (int i = 1; i < largo; i++)
    {
        if (snake_x[0] == snake_x[i] && snake_y[0] == snake_y[i])
        {

            ACTUAL_STATE = states::GAME_OVER;
            return;
        }
    }
}

void lose_display()
{
    // display.display();
    // display.clearDisplay();
    // display.setTextSize(2);
    // display.setTextColor(SSD1306_WHITE);
    // display.setCursor(4, 0);
    // display.println("Game Over!");
    // display.display();
    // display.setTextSize(1);
    // display.setTextColor(SSD1306_WHITE);
    // display.setCursor(15, 20);
    // display.println("Volver a jugar?");

    ActDisplay();
    ClearDisplay();
    DrawImage(0, 0, snakeGameOverWidth, snakeGameOverHeight, SnakeGameOverBitmap);
}

void reset_game()
{
    largo = 3;
    direccion = directions::DOWN;

    snake_x[0] = 8;
    snake_y[0] = 8;

    for (int i = 1; i < largo; i++)
    {
        snake_x[i] = snake_x[0] - i * len_block;
        snake_y[i] = snake_y[0];
    }

    random_food();
    ACTUAL_STATE = states::START;
}

// Funciones Externas

void snake_init()
{
    snake_x[0] = 8; // 9
    snake_y[0] = 8; // 9

    for (int i = 1; i < largo; i++)
    {
        snake_x[i] = snake_x[0] - i * len_block;
        snake_y[i] = snake_y[0];
    }
}

void snake_game()
{

    if (ACTUAL_STATE == states::INIT)
    {
        game_letters();
        snake_init();
    }

    else if (ACTUAL_STATE == states::START)
    {

        // display.clearDisplay();
        ClearDisplay();
        DrawImage(0, 0, snakeBackgroundWidth, snakeBackgroundHeight, SnakeBackgroundBitmap);
        buttons_read();
        // movement_logic();
        body();
        direcciones();
        space_limits();
        lose_conditions();
        print_snake();
        check_food();
        // display.display();
        ActDisplay();
        delay(120); // Todo: Cambiar a millis
    }
    else if (ACTUAL_STATE == states::GAME_OVER)
    {

        lose_display();
        int dir = input.realDirection();
        if (dir == 5)
        {

            delay(200); // Todo: Cambiat a millis
            reset_game();
        }
    }
}