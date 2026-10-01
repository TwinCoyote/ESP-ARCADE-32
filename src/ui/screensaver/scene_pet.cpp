#include "scenes.h"
#include "../../drivers/display/display.h"

// Mascota virtual al estilo Tamagotchi: un cachorro de coyote que nace de un huevo (la primera vez),
// pasea a saltitos, se come una pierna de pollo, juega a brincar la pelota, hace popo (y la ola la limpia)
// y al final se duerme. Los iconos de arriba y abajo se encienden segun lo que este haciendo.

namespace screensaver
{
    namespace
    {
        // ------------------------------------------------------------------ Sprites
        // '#' blanco, 'o' negro (tapa lo de atras), '.' transparente.
        // El cachorro se dibuja con la cola a la izquierda; cuando camina a la izquierda se voltea.

        const char *const PUP_IDLE[] = {
            "......#..........#..",
            "......##........##..",
            ".....#o#........#o#.",
            ".....#oo#......#oo#.",
            ".....#ooo######ooo#.",
            ".....##############.",
            ".....###oo####oo###.",
            ".#..####oo####oo####",
            "##..####oo####oo####",
            "###..######oo######.",
            "###...############..",
            ".###....########....",
            "..####.##########...",
            "...##############...",
            "......##.##..##.##..",
            "......##.##..##.##..",
        };

        const char *const PUP_BLINK[] = {
            "......#..........#..",
            "......##........##..",
            ".....#o#........#o#.",
            ".....#oo#......#oo#.",
            ".....#ooo######ooo#.",
            ".....##############.",
            ".....##############.",
            ".#..################",
            "##..####oo####oo####",
            "###..######oo######.",
            "###...############..",
            ".###....########....",
            "..####.##########...",
            "...##############...",
            "......##.##..##.##..",
            "......##.##..##.##..",
        };

        // Ojos felices (^ ^) y boca sonriendo
        const char *const PUP_HAPPY[] = {
            "......#..........#..",
            "......##........##..",
            ".....#o#........#o#.",
            ".....#oo#......#oo#.",
            ".....#ooo######ooo#.",
            ".....##############.",
            ".....####o####o####.",
            ".#..####o#o##o#o####",
            "##..################",
            "###..######oo######.",
            "###...####o##o####..",
            ".###....##oooo##....",
            "..####.##########...",
            "...##############...",
            "......##.##..##.##..",
            "......##.##..##.##..",
        };

        // Boca bien abierta para morder
        const char *const PUP_CHOMP[] = {
            "......#..........#..",
            "......##........##..",
            ".....#o#........#o#.",
            ".....#oo#......#oo#.",
            ".....#ooo######ooo#.",
            ".....##############.",
            ".....####o####o####.",
            ".#..####o#o##o#o####",
            "##..################",
            "###..######oo######.",
            "###...###oooooo###..",
            ".###....#oooooo#....",
            "..####.##########...",
            "...##############...",
            "......##.##..##.##..",
            "......##.##..##.##..",
        };

        // Paso: el cuerpo baja un pixel (va alternando con PUP_IDLE al caminar)
        const char *const PUP_STEP[] = {
            "....................",
            "......#..........#..",
            "......##........##..",
            ".....#o#........#o#.",
            ".....#oo#......#oo#.",
            ".....#ooo######ooo#.",
            ".....##############.",
            ".....###oo####oo###.",
            ".#..####oo####oo####",
            "##..####oo####oo####",
            "###..######oo######.",
            "###...############..",
            ".###....########....",
            "..####.##########...",
            "...##############...",
            "......##.##..##.##..",
        };

        // En el aire, con las patas recogidas
        const char *const PUP_JUMP[] = {
            "......#..........#..",
            "......##........##..",
            ".....#o#........#o#.",
            ".....#oo#......#oo#.",
            ".....#ooo######ooo#.",
            ".....##############.",
            ".....####o####o####.",
            ".#..####o#o##o#o####",
            "##..################",
            "###..######oo######.",
            "###...####oooo####..",
            ".###....##oooo##....",
            "..####.##########...",
            "...##############...",
            "......##........##..",
            "....................",
        };

        // Pujando (ojos > <)
        const char *const PUP_SQUAT[] = {
            "....................",
            "......#..........#..",
            "......##........##..",
            ".....#o#........#o#.",
            ".....#oo#......#oo#.",
            ".....#ooo######ooo#.",
            ".....##############.",
            ".....###o######o###.",
            ".#..#####o####o#####",
            "##..####o######o####",
            "###..######oo######.",
            "###...############..",
            ".###....########....",
            "..####.##########...",
            "...##############...",
            "....###........###..",
        };

        const char *const PUP_YAWN[] = {
            "......#..........#..",
            "......##........##..",
            ".....#o#........#o#.",
            ".....#oo#......#oo#.",
            ".....#ooo######ooo#.",
            ".....##############.",
            ".....##############.",
            ".#..################",
            "##..####oo####oo####",
            "###..######oo######.",
            "###...####oooo####..",
            ".###....##oooo##....",
            "..####.##########...",
            "...##############...",
            "......##.##..##.##..",
            "......##.##..##.##..",
        };

        // Echado y dormido
        const char *const PUP_SLEEP[] = {
            "....................",
            "....................",
            "....................",
            "....................",
            "....................",
            "....................",
            "......#........#....",
            ".....#o#......#o#...",
            ".....#oo######oo#...",
            "....##############..",
            "....###oo####oo###..",
            "...#######oo#######.",
            "..####oo######oo###.",
            ".#################..",
            "###################.",
            ".################...",
        };

        const char *const EGG[] = {
            ".....##.....",
            "...##oo##...",
            "..#oooooo#..",
            ".#oo##oooo#.",
            ".#o####ooo#.",
            "#ooo##oooo##",
            "#oooooooo#o#",
            "#o#oooo#oo##",
            "##o#oo#o#oo#",
            "#ooo##ooo#o#",
            ".#oooooooo#.",
            ".#oo##oooo#.",
            "..##oooo##..",
            "....####....",
        };

        // Rajadura que se le dibuja encima al huevo antes de abrirse
        const char *const EGG_CRACK[] = {
            "............",
            "............",
            "............",
            "............",
            "............",
            "............",
            "#.#...#.#...",
            ".#.#.#.#.#.#",
            "....#.....#.",
        };

        // Pierna de pollo y como va quedando con cada mordida (al final solo el hueso)
        const char *const MEAT_0[] = {
            "....#####.",
            "...#######",
            "...#######",
            "...######.",
            "..#####...",
            ".##.......",
            "##........",
            "###.......",
            ".#........",
        };

        const char *const MEAT_1[] = {
            "....##....",
            "...###....",
            "...####...",
            "...######.",
            "..#####...",
            ".##.......",
            "##........",
            "###.......",
            ".#........",
        };

        const char *const MEAT_2[] = {
            "..........",
            "..........",
            "...##.....",
            "...###....",
            "..####....",
            ".##.......",
            "##........",
            "###.......",
            ".#........",
        };

        const char *const MEAT_3[] = {
            "..........",
            "..........",
            "....#.#...",
            "....###...",
            "...##.....",
            "..##......",
            ".##.......",
            "###.......",
            ".#........",
        };

        const char *const *const MEAT[] = {MEAT_0, MEAT_1, MEAT_2, MEAT_3};

        const char *const POOP[] = {
            ".....#...",
            "....##...",
            "...####..",
            "..##oo##.",
            "..######.",
            ".##oooo##",
            "#########",
        };

        const char *const HEART[] = {
            ".##.##.",
            "#######",
            "#######",
            ".#####.",
            "..###..",
            "...#...",
        };

        const char *const MOON[] = {
            "..####...",
            ".###.....",
            "###......",
            "##.......",
            "##.......",
            "##.......",
            "###......",
            ".###.....",
            "..####...",
        };

        // Iconos del borde, como los del Tamagotchi original
        const char *const ICON_FOOD[] = {
            "#.#.#..#.",
            "#.#.#.##.",
            "#.#.#.##.",
            ".###..##.",
            "..#...##.",
            "..#....#.",
            "..#....#.",
            "..#....#.",
        };

        const char *const ICON_LIGHT[] = {
            "..###..",
            ".#...#.",
            "#.....#",
            "#.....#",
            ".#...#.",
            "..###..",
            "..#.#..",
            "..###..",
        };

        const char *const ICON_GAME[] = {
            ".#######.",
            "##.###.##",
            "#...#.#.#",
            "##.###.##",
            "#.##.##.#",
            "##.....##",
        };

        const char *const ICON_MEDICINE[] = {
            "..###..",
            "..#.#..",
            "###.###",
            "#.....#",
            "###.###",
            "..#.#..",
            "..###..",
        };

        const char *const ICON_BATH[] = {
            "..##.....",
            ".####....",
            "##o##....",
            ".####...#",
            "..######.",
            ".#######.",
            ".#######.",
            "..#####..",
        };

        const char *const ICON_METER[] = {
            ".##.##.",
            "#..#..#",
            "#.....#",
            "#.....#",
            ".#...#.",
            "..#.#..",
            "...#...",
        };

        const char *const ICON_DISCIPLINE[] = {
            "#######.",
            "#..#..#.",
            "#..#..#.",
            "#.....#.",
            "#..#..#.",
            "#######.",
            ".#......",
            "#.......",
        };

        const char *const ICON_ATTENTION[] = {
            "...#...",
            "..###..",
            ".#...#.",
            ".#...#.",
            ".#...#.",
            "#######",
            "...#...",
        };

        struct Icon
        {
            const char *const *rows;
            uint8_t rowCount;
        };
        // Arriba: comida, luz, juego, medicina. Abajo: bano, estado, disciplina, atencion
        const Icon ICONS[8] = {
            {ICON_FOOD, SPRITE_ROWS(ICON_FOOD)},
            {ICON_LIGHT, SPRITE_ROWS(ICON_LIGHT)},
            {ICON_GAME, SPRITE_ROWS(ICON_GAME)},
            {ICON_MEDICINE, SPRITE_ROWS(ICON_MEDICINE)},
            {ICON_BATH, SPRITE_ROWS(ICON_BATH)},
            {ICON_METER, SPRITE_ROWS(ICON_METER)},
            {ICON_DISCIPLINE, SPRITE_ROWS(ICON_DISCIPLINE)},
            {ICON_ATTENTION, SPRITE_ROWS(ICON_ATTENTION)},
        };
        enum IconId : int8_t
        {
            LIT_NONE = -1,
            LIT_FOOD = 0,
            LIT_LIGHT = 1,
            LIT_GAME = 2,
            LIT_BATH = 4,
        };
        const uint8_t ICON_ATTENTION_INDEX = 7;

        // ------------------------------------------------------------------ Medidas
        const uint8_t PX = 2;            // Cada pixel de los sprites grandes mide 2x2 en pantalla
        const int PUP_W = 20 * PX;       // 40
        const int PUP_H = 16 * PX;       // 32
        const int FLOOR = 53;            // Primera fila debajo de las patas
        const int ROOM_TOP = 10;         // Arriba de esto estan los iconos
        const int MIN_X = 2;             // Limites de la pantalla
        const int MAX_X = ANCHO_PANTALLA - PUP_W - 2;
        const int WALK_MIN_X = 14;       // Al pasear se queda lejos de las orillas: a los lados
        const int WALK_MAX_X = MAX_X - 12; // tienen que caber la comida y la popo
        const int CENTER_X = (ANCHO_PANTALLA - PUP_W) / 2;
        const unsigned long STEP_MS = 330; // Camina a saltitos, como en el Tamagotchi
        const int STEP_PX = 4;

        const int MEAT_W = 10 * PX;
        const int MEAT_H = 9 * PX;
        const int POOP_W = 9 * PX;
        const int POOP_H = 7 * PX;
        const int EGG_W = 12 * PX;
        const int EGG_H = 14 * PX;

        // Pelota del juego: entra rodando por la derecha, rebota en las paredes y al final se va
        const int BALL_R = 3;
        const float BALL_SPEED = 0.08f;        // px por ms
        const unsigned long BALL_STEP_MS = 10; // Paso de la simulacion
        const unsigned long BALL_EXIT_MS = 6200; // Desde aqui ya no rebota: se sale de la pantalla
        const unsigned long PLAY_MS = 8500;

        // ------------------------------------------------------------------ Guion
        enum Act : uint8_t
        {
            ACT_HATCH, // Sale del huevo (solo la primera vez desde que se prende la consola)
            ACT_HELLO, // Saluda (las siguientes veces)
            ACT_WALK,
            ACT_EAT,
            ACT_PLAY,
            ACT_POOP,
            ACT_SLEEP,
        };
        struct Part
        {
            Act act;
            unsigned long ms;
        };
        const Part FIRST_VISIT[] = {{ACT_HATCH, 6500}, {ACT_WALK, 6000}, {ACT_EAT, 8000}, {ACT_PLAY, PLAY_MS}, {ACT_WALK, 4500}, {ACT_POOP, 9000}, {ACT_SLEEP, 9500}};
        const Part NEXT_VISITS[] = {{ACT_HELLO, 3500}, {ACT_WALK, 6000}, {ACT_EAT, 8000}, {ACT_PLAY, PLAY_MS}, {ACT_WALK, 4500}, {ACT_POOP, 9000}, {ACT_SLEEP, 9500}};

        // ------------------------------------------------------------------ Estado
        bool hatched = false; // Despues de la primera vez ya no vuelve a salir del huevo
        const Part *script = FIRST_VISIT;
        uint8_t partCount = 0;
        uint8_t part = 0;
        unsigned long partStart = 0;

        int pupX = CENTER_X;
        bool facingLeft = false;
        unsigned long nextStep = 0; // Momento (desde que empezo el acto) del siguiente saltito
        bool stepPose = false;
        uint8_t pauseSteps = 0;
        int itemX = 0; // Donde cayo la comida o quedo la popo

        // Pelota: momentos en que pasa por debajo del cachorro (para brincar a tiempo) y cuando se va
        const uint8_t MAX_PASSES = 8;
        unsigned long passTimes[MAX_PASSES];
        uint8_t passCount = 0;
        unsigned long ballGoneAt = PLAY_MS;

        // ------------------------------------------------------------------ Dibujo
        void drawPup(const char *const *sprite, int x, int lift = 0)
        {
            drawSprite(x, FLOOR - PUP_H - lift, sprite, 16, PX, facingLeft);
        }

        bool blinking(unsigned long t)
        {
            return (t % 2900) < 140;
        }

        // Un icono por cada cuarto de la pantalla; el encendido va en negativo
        void drawIcons(int8_t lit, bool attention)
        {
            for (uint8_t i = 0; i < 8; i++)
            {
                int cx = 16 + (i % 4) * 32;
                int top = i < 4 ? 0 : 55;
                int w = strlen(ICONS[i].rows[0]);
                int h = ICONS[i].rowCount;
                drawSprite(cx - w / 2, top + (9 - h) / 2, ICONS[i].rows, h);
                bool on = (i == lit) || (i == ICON_ATTENTION_INDEX && attention);
                if (on)
                {
                    InvertBox(cx - 9, top, 18, 9);
                }
            }
        }

        // Globo de dialogo junto a la cabeza (del lado contrario a la cola)
        void drawBubble(const char *text)
        {
            SetCustomFont(FONT_TINY_ES);
            int w = TextWidthUTF8(text) + 7;
            int h = 11;
            int headY = FLOOR - PUP_H + 6;
            int x = facingLeft ? pupX - w + 4 : pupX + PUP_W - 4;
            if (x + w > ANCHO_PANTALLA - 1)
            {
                x = pupX - w + 4;
            }
            if (x < 1)
            {
                x = pupX + PUP_W - 4;
            }
            int y = headY - h - 2;
            if (y < ROOM_TOP + 1)
            {
                y = ROOM_TOP + 1;
            }
            bool tailLeft = x > pupX + PUP_W / 2;
            ClearBox(x, y, w, h);
            DrawRoundFrame(x, y, w, h, 3);
            DrawTextUTF8(x + 4, y + 8, text);
            // Colita del globo apuntando a la cabeza
            int tx = tailLeft ? x + 3 : x + w - 4;
            ClearBox(tx, y + h - 1, 2, 1);
            DrawPixel(tx + (tailLeft ? -1 : 2), y + h);
            DrawPixel(tx + (tailLeft ? -2 : 3), y + h + 1);
        }

        void drawHeart(int x, int y)
        {
            drawSprite(x, y, HEART, SPRITE_ROWS(HEART));
        }

        // Corazones que suben desde la cabeza durante 'ms' a partir de 'from'
        void drawHearts(unsigned long lt, unsigned long from, unsigned long ms)
        {
            if (lt < from)
            {
                return;
            }
            for (uint8_t i = 0; i < 2; i++)
            {
                unsigned long age = lt - from;
                if (age < i * 450UL)
                {
                    continue;
                }
                age -= i * 450UL;
                if (age >= ms)
                {
                    continue;
                }
                int y = FLOOR - PUP_H - 4 - (int)(age * 20 / ms);
                int x = pupX + (i ? PUP_W - 8 : 2) + (int)((age / 150) % 2);
                if (y > ROOM_TOP)
                {
                    drawHeart(x, y);
                }
            }
        }

        // Destellos en cruz que crecen y se apagan
        void drawSparkle(int cx, int cy, unsigned long age)
        {
            int r = 1 + (int)(age / 80) % 4;
            DrawLine(cx - r, cy, cx + r, cy);
            DrawLine(cx, cy - r, cx, cy + r);
        }

        // ------------------------------------------------------------------ Movimiento
        // Camina a saltitos entre minX y maxX; de vez en cuando se detiene a mirar o se da la vuelta.
        // lt: ms desde que empezo el acto. wander = false camina derecho, sin pausas ni vueltas
        void walk(unsigned long lt, int minX, int maxX, bool wander = true)
        {
            while (lt >= nextStep)
            {
                nextStep += STEP_MS;
                if (pauseSteps > 0)
                {
                    pauseSteps--;
                    stepPose = false;
                    continue;
                }
                long r = wander ? random(100) : 100;
                if (r < 10)
                {
                    facingLeft = !facingLeft;
                }
                else if (r < 18)
                {
                    pauseSteps = random(3, 7);
                }
                int nx = pupX + (facingLeft ? -STEP_PX : STEP_PX);
                if (nx < minX || nx > maxX)
                {
                    if (!wander)
                    {
                        stepPose = false;
                        continue; // Llego a la orilla: se queda ahi
                    }
                    facingLeft = !facingLeft;
                    nx = pupX;
                }
                pupX = nx;
                stepPose = !stepPose;
            }
        }

        const char *const *walkPose(unsigned long t)
        {
            if (blinking(t))
            {
                return PUP_BLINK;
            }
            return stepPose ? PUP_STEP : PUP_IDLE;
        }

        // Altura de un brinco: parabola de 'height' px que dura 'ms', empezando en 'from'
        int hop(unsigned long lt, unsigned long from, unsigned long ms, int height)
        {
            if (lt < from || lt >= from + ms)
            {
                return 0;
            }
            long s = (long)(lt - from) * 100 / (long)ms; // 0..99
            return (int)(4L * height * s * (100 - s) / 10000L);
        }

        // Un paso de la simulacion de la pelota. Solo rebota cuando va hacia la pared: asi puede
        // entrar desde fuera de la pantalla y, despues de BALL_EXIT_MS, salirse
        void ballStep(float &x, float &v, unsigned long ms)
        {
            x += v * BALL_STEP_MS;
            if (ms >= BALL_EXIT_MS)
            {
                return;
            }
            if ((x < BALL_R + 1 && v < 0) || (x > ANCHO_PANTALLA - BALL_R - 2 && v > 0))
            {
                v = -v;
            }
        }

        // ------------------------------------------------------------------ Actos
        void startAct(Act act)
        {
            nextStep = 0;
            stepPose = false;
            pauseSteps = 0;
            switch (act)
            {
            case ACT_HATCH:
            case ACT_HELLO:
                pupX = CENTER_X;
                facingLeft = false;
                break;
            case ACT_EAT:
                // Mira hacia el centro, que es donde hay espacio para que caiga la comida
                facingLeft = pupX > CENTER_X;
                itemX = facingLeft ? pupX - MEAT_W - 1 : pupX + PUP_W + 1;
                break;
            case ACT_POOP:
                // Igual: se aleja hacia el centro y la popo queda del lado de la cola, hacia la orilla
                facingLeft = pupX > CENTER_X;
                itemX = facingLeft ? pupX + PUP_W - 6 : pupX - POOP_W + 6;
                nextStep = 1000; // Empieza a alejarse despues de hacerla
                break;
            case ACT_PLAY:
            {
                // Se calcula de antemano cuando pasa la pelota por debajo del cachorro
                passCount = 0;
                ballGoneAt = PLAY_MS;
                int center = pupX + PUP_W / 2;
                float x = ANCHO_PANTALLA + 8;
                float v = -BALL_SPEED;
                for (unsigned long ms = 0; ms < PLAY_MS; ms += BALL_STEP_MS)
                {
                    float before = x;
                    ballStep(x, v, ms);
                    if ((before < center) != (x < center) && passCount < MAX_PASSES)
                    {
                        passTimes[passCount++] = ms;
                    }
                    if (ms >= BALL_EXIT_MS && (x < -BALL_R || x > ANCHO_PANTALLA + BALL_R) && ballGoneAt == PLAY_MS)
                    {
                        ballGoneAt = ms;
                    }
                }
                break;
            }
            default:
                break;
            }
        }

        void drawHatch(unsigned long lt)
        {
            int eggX = CENTER_X + (PUP_W - EGG_W) / 2;
            int eggY = FLOOR - EGG_H;
            if (lt < 3600)
            {
                // Se mece a ratos, cada vez mas rapido
                int wobble = 0;
                if ((lt > 500 && lt < 1300) || (lt > 1700 && lt < 2500) || lt > 2800)
                {
                    unsigned long period = lt < 1300 ? 220 : (lt < 2500 ? 140 : 90);
                    wobble = ((lt / period) % 2) ? 2 : -2;
                }
                drawSprite(eggX + wobble, eggY, EGG, SPRITE_ROWS(EGG), PX);
                if (lt >= 2500)
                {
                    drawSprite(eggX + wobble, eggY, EGG_CRACK, SPRITE_ROWS(EGG_CRACK), PX);
                }
                return;
            }

            unsigned long pop = lt - 3600;
            const char *const *pose = pop < 300 ? PUP_JUMP : (pop >= 900 && pop < 2400 ? PUP_HAPPY : PUP_IDLE);
            int lift = hop(pop, 0, 450, 10);
            drawPup(blinking(pop + 1500) ? PUP_BLINK : pose, pupX, lift);
            // La parte de arriba del cascaron sale volando y los pedazos de abajo caen a los lados
            if (pop < 700)
            {
                int fly = (int)(pop / 9);
                if (eggY - fly > ROOM_TOP - 4)
                {
                    drawSprite(eggX - fly, eggY - fly, EGG, 6, PX);
                }
                int slide = (int)(pop / 20);
                DrawBox(eggX - 6 - slide, FLOOR - 4, 6, 4);
                DrawBox(eggX + EGG_W + slide, FLOOR - 4, 6, 4);
            }
            if (pop < 600)
            {
                // Destellos alrededor
                drawSparkle(pupX - 2, FLOOR - 26, pop);
                drawSparkle(pupX + PUP_W + 1, FLOOR - 30, pop + 120);
                drawSparkle(pupX + PUP_W / 2, ROOM_TOP + 3, pop + 240);
            }
            if (pop >= 900)
            {
                drawBubble("¡Hola!");
            }
        }

        void drawHello(unsigned long lt)
        {
            int lift = hop(lt, 200, 450, 10) + hop(lt, 700, 400, 6);
            const char *const *pose = lift > 0 ? PUP_JUMP : (lt < 2200 ? PUP_HAPPY : walkPose(lt));
            drawPup(pose, pupX, lift);
            if (lt >= 500)
            {
                drawBubble("¡Hola!");
            }
        }

        // Pierna de pollo con 'bites' mordidas (0 a 3; con 3 ya solo queda el hueso).
        // La carne queda del lado del cachorro
        void drawMeat(int x, int y, uint8_t bites, uint8_t scale = PX)
        {
            drawSprite(x, y, MEAT[bites], SPRITE_ROWS(MEAT_0), scale, !facingLeft);
        }

        void drawEat(unsigned long lt, int8_t &lit, bool &attention)
        {
            int meatY = FLOOR - MEAT_H;
            if (lt < 1600)
            {
                // Tiene hambre: se le antoja una pierna de pollo
                attention = true;
                drawPup(blinking(lt) ? PUP_BLINK : PUP_IDLE, pupX);
                int bw = 17;
                int bh = 14;
                int bx = facingLeft ? pupX - bw + 6 : pupX + PUP_W - 6;
                int by = ROOM_TOP + 1;
                ClearBox(bx, by, bw, bh);
                DrawRoundFrame(bx, by, bw, bh, 4);
                drawMeat(bx + 4, by + 3, 0, 1);
                // Burbujitas de pensamiento hacia la cabeza
                int dx = facingLeft ? bw - 3 : 2;
                DrawPixel(bx + dx, by + bh + 2);
                DrawFrame(bx + dx + (facingLeft ? 1 : -2), by + bh + 4, 2, 2);
                return;
            }
            lit = LIT_FOOD;
            if (lt < 2300)
            {
                // Cae del cielo
                long s = lt - 1600;
                int y = ROOM_TOP + (int)((long)(meatY - ROOM_TOP) * s * s / (700L * 700L));
                drawMeat(itemX, y, 0);
                drawPup(PUP_IDLE, pupX);
                return;
            }
            if (lt < 5300)
            {
                // Tres mordidas, una por segundo
                unsigned long e = lt - 2300;
                uint8_t bites = e / 1000;
                unsigned long since = e % 1000;
                bool open = since < 380;
                drawMeat(itemX, meatY, open ? bites : bites + 1);
                int lean = open ? (facingLeft ? -2 : 2) : 0;
                drawPup(open ? PUP_CHOMP : PUP_HAPPY, pupX + lean);
                // Moronas que salen volando al morder
                if (since >= 380 && since < 700)
                {
                    int cx = facingLeft ? itemX + MEAT_W - 6 : itemX + 6;
                    int k = (int)(since - 380) / 40;
                    DrawPixel(cx - k, meatY + 2 - k + k * k / 6);
                    DrawPixel(cx + k, meatY + 1 - k + k * k / 5);
                    DrawPixel(cx + 1, meatY - 1 - k / 2 + k * k / 8);
                }
                return;
            }
            // Panza llena; el hueso se queda en el piso
            unsigned long e = lt - 5300;
            drawMeat(itemX, meatY, 3);
            int lift = hop(e, 100, 420, 8);
            drawPup(lift > 0 ? PUP_JUMP : PUP_HAPPY, pupX, lift);
            drawHearts(e, 200, 1600);
        }

        void drawPlay(unsigned long lt, int8_t &lit)
        {
            lit = LIT_GAME;
            // Brinca justo cuando la pelota pasa por debajo; al caer queda contento un momento
            int lift = 0;
            bool landed = false;
            for (uint8_t i = 0; i < passCount; i++)
            {
                int h = hop(lt + 330, passTimes[i], 660, 12);
                if (h > lift)
                {
                    lift = h;
                }
                if (lt >= passTimes[i] + 330 && lt < passTimes[i] + 800)
                {
                    landed = true;
                }
            }
            bool done = lt >= ballGoneAt;
            const char *const *pose = lift > 0 ? PUP_JUMP : ((landed || done) ? PUP_HAPPY : (blinking(lt) ? PUP_BLINK : PUP_IDLE));
            drawPup(pose, pupX, lift);
            if (done)
            {
                drawHearts(lt, ballGoneAt, 1300);
                return;
            }

            // Pelota rodando: misma simulacion que en startAct, avanzada hasta lt
            float x = ANCHO_PANTALLA + 8;
            float v = -BALL_SPEED;
            for (unsigned long ms = 0; ms + BALL_STEP_MS <= lt; ms += BALL_STEP_MS)
            {
                ballStep(x, v, ms);
            }
            int bx = (int)x;
            int by = FLOOR - BALL_R - 1;
            DrawDisc(bx, by, BALL_R);
            // Dos puntos negros que giran segun avanza: se ve que rueda
            const int8_t DX[4] = {0, 1, 1, 1};
            const int8_t DY[4] = {-1, -1, 0, 1};
            int a = (bx / 2) & 3;
            ClearBox(bx + DX[a], by + DY[a], 1, 1);
            ClearBox(bx - DX[a], by - DY[a], 1, 1);
        }

        // Ola que barre la pantalla de derecha a izquierda
        void drawWave(int x, unsigned long lt)
        {
            for (int y = ROOM_TOP + 2; y < FLOOR; y++)
            {
                int wob = (int)((y + lt / 30) % 6);
                int off = wob < 3 ? wob : 6 - wob;
                ClearBox(x - 1 + off, y, 9, 1);
                DrawPixel(x + off, y);
                DrawPixel(x + off + 3, y);
                if ((y + lt / 60) % 4 == 0)
                {
                    DrawPixel(x + off + 6, y);
                }
            }
            // Burbujas
            for (uint8_t i = 0; i < 3; i++)
            {
                int by = FLOOR - 6 - (int)((lt / 25 + i * 13) % 36);
                DrawCircle(x + 10 + i * 3, by, 1 + (i % 2));
            }
        }

        void drawPoop(unsigned long lt, int8_t &lit, bool &attention)
        {
            int poopY = FLOOR - POOP_H;
            if (lt < 800)
            {
                // Pujando
                int shake = ((lt / 70) % 2) ? 1 : -1;
                drawPup(PUP_SQUAT, pupX + shake);
                return;
            }
            // Se aleja unos pasos, apenado
            if (lt >= 1000 && lt < 2000)
            {
                walk(lt, MIN_X, MAX_X, false);
            }
            unsigned long washStart = 4000;
            unsigned long washMs = 1600;
            int waveX = ANCHO_PANTALLA + 4 - (int)((long)(lt > washStart ? lt - washStart : 0) * (ANCHO_PANTALLA + 40) / (long)washMs);

            // La popo: con olor mientras nadie la limpia; la ola se la lleva
            int px = itemX;
            if (lt > washStart && waveX < itemX + 4)
            {
                px = waveX - 4;
            }
            if (px > -POOP_W)
            {
                drawSprite(px, poopY, POOP, SPRITE_ROWS(POOP), PX);
                if (lt < washStart)
                {
                    int f = (lt / 300) % 2;
                    for (uint8_t i = 0; i < 3; i++)
                    {
                        int sx = px + 3 + i * 6;
                        for (int k = 0; k < 6; k++)
                        {
                            DrawPixel(sx + (((k + f + i) / 2) % 2), poopY - 3 - k);
                        }
                    }
                }
            }
            if (lt < 1000)
            {
                drawSparkle(itemX + POOP_W / 2, poopY + 4, lt - 800);
            }

            if (lt < washStart)
            {
                drawPup(lt < 2000 ? walkPose(lt) : (blinking(lt) ? PUP_BLINK : PUP_IDLE), pupX);
                if (lt >= 2600)
                {
                    attention = true;
                    drawBubble("!");
                }
                return;
            }
            lit = LIT_BATH;
            unsigned long after = lt - washStart;
            bool clean = after >= washMs;
            int lift = clean ? hop(after, washMs + 100, 420, 8) : 0;
            drawPup(lift > 0 ? PUP_JUMP : (clean ? PUP_HAPPY : PUP_IDLE), pupX, lift);
            if (!clean)
            {
                drawWave(waveX, lt);
            }
            else
            {
                drawHearts(after, washMs + 300, 1500);
                if (after < washMs + 900)
                {
                    drawSparkle(itemX + POOP_W / 2, poopY + 2, after - washMs);
                    drawSparkle(itemX + 2, poopY - 6, after - washMs + 160);
                }
            }
        }

        void drawSleep(unsigned long lt, int8_t &lit)
        {
            if (lt < 1300)
            {
                // Bostezo
                drawPup(lt < 900 ? PUP_YAWN : PUP_BLINK, pupX);
                return;
            }
            lit = LIT_LIGHT;
            // De noche: luna del lado contrario al cachorro y estrellas que titilan
            bool pupOnLeft = pupX + PUP_W / 2 < ANCHO_PANTALLA / 2;
            int moonX = pupOnLeft ? ANCHO_PANTALLA - 18 : 8;
            drawSprite(moonX, ROOM_TOP + 3, MOON, SPRITE_ROWS(MOON));
            for (uint8_t i = 0; i < 9; i++)
            {
                uint32_t h = hash32(i * 7919 + 17);
                int sx = 4 + (int)(h % 120);
                int sy = ROOM_TOP + 2 + (int)((h >> 8) % 22);
                if (sx > pupX - 4 && sx < pupX + PUP_W + 4)
                {
                    continue;
                }
                if (sx > moonX - 3 && sx < moonX + 12 && sy < ROOM_TOP + 14)
                {
                    continue;
                }
                bool twinkle = ((lt / 400) + (h >> 16)) % 5 == 0;
                if (lt > 1300 + i * 120UL)
                {
                    DrawPixel(sx, sy);
                    if (twinkle)
                    {
                        DrawPixel(sx - 1, sy);
                        DrawPixel(sx + 1, sy);
                        DrawPixel(sx, sy - 1);
                        DrawPixel(sx, sy + 1);
                    }
                }
            }
            // Respira despacio
            int breath = ((lt / 900) % 2) ? 1 : 0;
            drawSprite(pupX, FLOOR - PUP_H - breath, PUP_SLEEP, SPRITE_ROWS(PUP_SLEEP), PX, facingLeft);

            // Zzz que suben desde la cabeza
            for (uint8_t i = 0; i < 8; i++)
            {
                unsigned long born = 1800 + i * 950UL;
                if (lt < born || lt >= born + 2600)
                {
                    continue;
                }
                unsigned long age = lt - born;
                int zx = pupX + (facingLeft ? 8 : PUP_W - 10) + (int)(age / 200) + (int)((age / 350) % 2);
                int zy = FLOOR - 22 - (int)(age * 22 / 2600);
                if (zy < ROOM_TOP + 7)
                {
                    continue;
                }
                SetCustomFont(age < 1100 ? FONT_TINY_ES : FONT_SMALL_ES);
                DrawText(zx, zy, age < 1100 ? "z" : "Z");
            }
        }
    }

    unsigned long petStart()
    {
        script = hatched ? NEXT_VISITS : FIRST_VISIT;
        partCount = hatched ? sizeof(NEXT_VISITS) / sizeof(NEXT_VISITS[0]) : sizeof(FIRST_VISIT) / sizeof(FIRST_VISIT[0]);
        hatched = true;
        part = 0;
        partStart = 0;
        startAct(script[0].act);

        unsigned long total = 0;
        for (uint8_t i = 0; i < partCount; i++)
        {
            total += script[i].ms;
        }
        return total;
    }

    void petDraw(unsigned long t)
    {
        // Avanza el guion
        while (part + 1 < partCount && t >= partStart + script[part].ms)
        {
            partStart += script[part].ms;
            part++;
            startAct(script[part].act);
        }
        unsigned long lt = t - partStart;
        int8_t lit = LIT_NONE;
        bool attention = false;

        switch (script[part].act)
        {
        case ACT_HATCH:
            drawHatch(lt);
            break;
        case ACT_HELLO:
            drawHello(lt);
            break;
        case ACT_WALK:
            walk(lt, WALK_MIN_X, WALK_MAX_X);
            drawPup(walkPose(t), pupX);
            break;
        case ACT_EAT:
            drawEat(lt, lit, attention);
            break;
        case ACT_PLAY:
            drawPlay(lt, lit);
            break;
        case ACT_POOP:
            drawPoop(lt, lit, attention);
            break;
        case ACT_SLEEP:
            drawSleep(lt, lit);
            break;
        }

        drawIcons(lit, attention && (t / 300) % 2 == 0);
    }
}
