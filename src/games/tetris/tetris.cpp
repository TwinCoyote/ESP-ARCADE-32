#include "tetris.h"
#include "../../drivers/display/display.h"
#include "../../drivers/input/buttons.h"
#include "../../config/debug_log.h"
#include "../../assets/images/tetris_images/tetris_background.h"

namespace tetris
{
    enum class states_tetris
    {
        INIT,      // Pozo vacio, esperando OK
        PLAYING,   // Pieza cayendo
        CLEARING,  // Parpadeo de las lineas completas antes de borrarlas
        GAME_OVER, // La pieza nueva no cabe
    };

    // ------------------------------------------------------------------ Medidas (segun tetris_preview.h)
    const int COLS = 10;
    const int ROWS = 20;
    const int WELL_X = 48; // Pixel de la celda (0, 0)
    const int WELL_Y = 2;
    const int WELL_W = 30; // Interior del pozo, entre las paredes x=47 y x=78
    const int CELL = 3;    // Separacion entre celdas
    const int BLOCK = 2;   // Lado del cuadro que se pinta en cada celda

    // Interior de los recuadros NEXT y HOLD
    const int BOX_X = 92;
    const int BOX_W = 24;
    const int BOX_H = 14;
    const int NEXT_Y = 11;
    const int HOLD_Y = 38;
    const int PREVIEW_CELL = 4;
    const int PREVIEW_BLOCK = 3;

    // Numeros del panel izquierdo (centrados bajo las etiquetas)
    const int NUMBERS_CENTER_X = 23;
    const int SCORE_Y = 10;
    const int LINES_Y = 31;
    const int LEVEL_Y = 52;

    // ------------------------------------------------------------------ Tiempos (ms)
    const unsigned long DAS_DELAY = 170;   // Cuanto hay que mantener IZQ/DER para que empiece a repetir
    const unsigned long DAS_REPEAT = 50;   // Cada cuanto se repite despues
    const unsigned long SOFT_DROP_MS = 40; // Caida con ABAJO presionado
    const unsigned long LOCK_DELAY = 400;  // Tiempo en el piso antes de fijar la pieza
    const int MAX_LOCK_RESETS = 15;        // Movimientos que reinician LOCK_DELAY (evita girar infinito)
    const unsigned long CLEAR_MS = 300;    // Duracion del parpadeo de lineas
    const unsigned long GAME_OVER_WAIT = 600;

    // Caida por nivel, aproximada a la del Tetris de NES (cuadros a 60 Hz convertidos a ms)
    const unsigned int GRAVITY_MS[] = {800, 717, 633, 550, 467, 383, 300, 217, 133, 100,
                                       83, 83, 83, 67, 67, 67, 50, 50, 50, 33};
    const int GRAVITY_LEVELS = sizeof(GRAVITY_MS) / sizeof(GRAVITY_MS[0]);

    const unsigned int LINE_POINTS[] = {0, 100, 300, 500, 800};

    // ------------------------------------------------------------------ Piezas
    // Cada rotacion es una rejilla de 4x4 en 16 bits: el bit 15 es la fila 0 columna 0 y
    // cada nibble es una fila. Las rotaciones siguen el sistema SRS (sentido horario).
    enum PieceType
    {
        PIECE_I,
        PIECE_O,
        PIECE_T,
        PIECE_S,
        PIECE_Z,
        PIECE_J,
        PIECE_L,
        PIECE_COUNT
    };

    const uint16_t SHAPES[PIECE_COUNT][4] = {
        {0x0F00, 0x2222, 0x00F0, 0x4444}, // I
        {0x6600, 0x6600, 0x6600, 0x6600}, // O
        {0x4E00, 0x4640, 0x0E40, 0x4C40}, // T
        {0x6C00, 0x4620, 0x06C0, 0x8C40}, // S
        {0xC600, 0x2640, 0x0C60, 0x4C80}, // Z
        {0x8E00, 0x6440, 0x0E20, 0x44C0}, // J
        {0x2E00, 0x4460, 0x0E80, 0xC440}, // L
    };

    // Desplazamientos que se prueban si la pieza no cabe al girar (patada contra pared o piso)
    const int8_t KICKS[][2] = {{0, 0}, {-1, 0}, {1, 0}, {0, -1}, {-2, 0}, {2, 0}};
    const int KICK_COUNT = sizeof(KICKS) / sizeof(KICKS[0]);

    // Digitos de 3x5: cada fila son 3 bits, el bit 2 es la columna izquierda
    const uint8_t DIGITS[10][5] = {
        {7, 5, 5, 5, 7}, // 0
        {2, 6, 2, 2, 7}, // 1
        {7, 1, 7, 4, 7}, // 2
        {7, 1, 7, 1, 7}, // 3
        {5, 5, 7, 1, 1}, // 4
        {7, 4, 7, 1, 7}, // 5
        {7, 4, 7, 5, 7}, // 6
        {7, 1, 1, 1, 1}, // 7
        {7, 5, 7, 5, 7}, // 8
        {7, 5, 7, 1, 7}, // 9
    };

    struct Piece
    {
        int type;
        int rot;
        int x; // Columna de la esquina superior izquierda de la rejilla 4x4
        int y; // Fila (puede ser negativa: la pieza asoma por arriba del pozo)
    };

    // ------------------------------------------------------------------ Estado
    static states_tetris STATE = states_tetris::INIT;

    static uint8_t board[ROWS][COLS];
    static Piece current;
    static int nextType = PIECE_T;
    static int holdType = -1;
    static bool holdUsed = false;

    static uint8_t bag[PIECE_COUNT];
    static int bagIndex = PIECE_COUNT;

    static unsigned long score = 0;
    static unsigned int lines = 0;
    static unsigned int level = 1;

    static unsigned long lastFall = 0;
    static bool locking = false;
    static unsigned long lockStart = 0;
    static int lockResets = 0;

    static uint32_t clearMask = 0; // Bit por fila completa, mientras dura CLEARING
    static unsigned long stateStart = 0;

    // Botones: estado anterior para detectar flancos. Arrancan en true para que un boton que
    // venia presionado desde el menu (el OK que eligio Tetris) no cuente como pulsacion nueva.
    static bool prevLeft = true;
    static bool prevRight = true;
    static bool prevUp = true;
    static bool prevOk = true;
    static unsigned long dasStartLeft = 0;
    static unsigned long dasLastLeft = 0;
    static unsigned long dasStartRight = 0;
    static unsigned long dasLastRight = 0;

    // ------------------------------------------------------------------ Logica
    static bool cellOf(uint16_t mask, int r, int c)
    {
        return (mask >> (15 - (r * 4 + c))) & 1;
    }

    static bool collides(int type, int rot, int x, int y)
    {
        uint16_t mask = SHAPES[type][rot];
        for (int r = 0; r < 4; r++)
        {
            for (int c = 0; c < 4; c++)
            {
                if (!cellOf(mask, r, c))
                    continue;
                int bx = x + c;
                int by = y + r;
                if (bx < 0 || bx >= COLS || by >= ROWS)
                    return true;
                if (by >= 0 && board[by][bx])
                    return true;
            }
        }
        return false;
    }

    // Bolsa de 7: salen las siete piezas en orden aleatorio antes de repetir ninguna
    static int bagNext()
    {
        if (bagIndex >= PIECE_COUNT)
        {
            for (int i = 0; i < PIECE_COUNT; i++)
                bag[i] = i;
            for (int i = PIECE_COUNT - 1; i > 0; i--)
            {
                int j = random(0, i + 1);
                uint8_t tmp = bag[i];
                bag[i] = bag[j];
                bag[j] = tmp;
            }
            bagIndex = 0;
        }
        return bag[bagIndex++];
    }

    static void spawn(int type)
    {
        current.type = type;
        current.rot = 0;
        current.x = 3;
        current.y = (type == PIECE_I) ? -1 : 0; // La I ocupa la fila 1 de su rejilla
        locking = false;
        lockResets = 0;
        lastFall = millis();

        if (collides(current.type, current.rot, current.x, current.y))
        {
            STATE = states_tetris::GAME_OVER;
            stateStart = millis();
            DEV_PRINT("[Tetris] Game Over, score=");
            DEV_PRINTLN(score);
        }
    }

    static void spawnNext()
    {
        spawn(nextType);
        nextType = bagNext();
    }

    // Tras un movimiento o giro exitoso en el piso, se da un poco mas de tiempo antes de fijar
    static void onPieceMoved()
    {
        if (locking && lockResets < MAX_LOCK_RESETS)
        {
            lockStart = millis();
            lockResets++;
        }
    }

    static bool tryMove(int dx, int dy)
    {
        if (collides(current.type, current.rot, current.x + dx, current.y + dy))
            return false;
        current.x += dx;
        current.y += dy;
        return true;
    }

    static void tryRotate()
    {
        int rot = (current.rot + 1) % 4;
        for (int i = 0; i < KICK_COUNT; i++)
        {
            int x = current.x + KICKS[i][0];
            int y = current.y + KICKS[i][1];
            if (!collides(current.type, rot, x, y))
            {
                current.rot = rot;
                current.x = x;
                current.y = y;
                onPieceMoved();
                return;
            }
        }
    }

    static void holdPiece()
    {
        if (holdUsed)
            return;
        int type = current.type;
        if (holdType < 0)
        {
            spawnNext();
        }
        else
        {
            spawn(holdType);
        }
        holdType = type;
        holdUsed = true;
    }

    static bool rowFull(int r)
    {
        for (int c = 0; c < COLS; c++)
        {
            if (!board[r][c])
                return false;
        }
        return true;
    }

    static void removeClearedRows()
    {
        int dst = ROWS - 1;
        for (int src = ROWS - 1; src >= 0; src--)
        {
            if (clearMask & (1UL << src))
                continue;
            if (dst != src)
                memcpy(board[dst], board[src], COLS);
            dst--;
        }
        for (; dst >= 0; dst--)
            memset(board[dst], 0, COLS);
        clearMask = 0;
    }

    static void lockPiece()
    {
        uint16_t mask = SHAPES[current.type][current.rot];
        bool aboveTop = false;
        for (int r = 0; r < 4; r++)
        {
            for (int c = 0; c < 4; c++)
            {
                if (!cellOf(mask, r, c))
                    continue;
                int by = current.y + r;
                if (by < 0)
                {
                    aboveTop = true;
                    continue;
                }
                board[by][current.x + c] = current.type + 1;
            }
        }
        holdUsed = false;

        // Se fijo por encima del pozo: ya no hay lugar
        if (aboveTop)
        {
            STATE = states_tetris::GAME_OVER;
            stateStart = millis();
            DEV_PRINT("[Tetris] Game Over, score=");
            DEV_PRINTLN(score);
            return;
        }

        int cleared = 0;
        for (int r = 0; r < ROWS; r++)
        {
            if (rowFull(r))
            {
                clearMask |= (1UL << r);
                cleared++;
            }
        }

        if (cleared == 0)
        {
            spawnNext();
            return;
        }

        score += (unsigned long)LINE_POINTS[cleared] * level;
        lines += cleared;
        level = 1 + lines / 10;
        STATE = states_tetris::CLEARING;
        stateStart = millis();
    }

    static unsigned long gravityMs()
    {
        int idx = (int)level - 1;
        if (idx >= GRAVITY_LEVELS)
            idx = GRAVITY_LEVELS - 1;
        return GRAVITY_MS[idx];
    }

    // Movimiento lateral con autorepeticion (DAS): un paso al presionar y, si se mantiene,
    // despues de DAS_DELAY un paso cada DAS_REPEAT.
    static void handleShift(bool pressed, bool &prev, unsigned long &dasStart, unsigned long &dasLast, int dx)
    {
        unsigned long now = millis();
        if (pressed && !prev)
        {
            if (tryMove(dx, 0))
                onPieceMoved();
            dasStart = now;
            dasLast = now;
        }
        else if (pressed && now - dasStart >= DAS_DELAY && now - dasLast >= DAS_REPEAT)
        {
            if (tryMove(dx, 0))
                onPieceMoved();
            dasLast = now;
        }
        prev = pressed;
    }

    static void startGame()
    {
        memset(board, 0, sizeof(board));
        score = 0;
        lines = 0;
        level = 1;
        holdType = -1;
        holdUsed = false;
        clearMask = 0;
        bagIndex = PIECE_COUNT;
        nextType = bagNext();
        STATE = states_tetris::PLAYING;
        spawnNext();
        DEV_PRINTLN("[Tetris] Nueva partida");
    }

    static void updatePlaying()
    {
        bool left = isPressed(BTN_LEFT);
        bool right = isPressed(BTN_RIGHT);
        bool up = isPressed(BTN_UP);
        bool down = isPressed(BTN_DOWN);
        bool ok = isPressed(BTN_OK);

        handleShift(left, prevLeft, dasStartLeft, dasLastLeft, -1);
        handleShift(right, prevRight, dasStartRight, dasLastRight, 1);

        if (up && !prevUp)
            tryRotate();
        prevUp = up;

        if (ok && !prevOk)
            holdPiece();
        prevOk = ok;

        // HOLD puede haber terminado la partida si la pieza guardada no cabe
        if (STATE != states_tetris::PLAYING)
            return;

        unsigned long now = millis();
        if (!collides(current.type, current.rot, current.x, current.y + 1))
        {
            locking = false;
            unsigned long interval = gravityMs();
            if (down && SOFT_DROP_MS < interval)
                interval = SOFT_DROP_MS;
            if (now - lastFall >= interval)
            {
                current.y++;
                lastFall = now;
                if (down)
                    score++;
            }
        }
        else
        {
            if (!locking)
            {
                locking = true;
                lockStart = now;
            }
            if (now - lockStart >= LOCK_DELAY)
                lockPiece();
        }
    }

    // ------------------------------------------------------------------ Dibujo
    static void drawDigit(int d, int x, int y, int scale)
    {
        for (int r = 0; r < 5; r++)
        {
            for (int c = 0; c < 3; c++)
            {
                if ((DIGITS[d][r] >> (2 - c)) & 1)
                    DrawBox(x + c * scale, y + r * scale, scale, scale);
            }
        }
    }

    // Numero centrado bajo las etiquetas del panel izquierdo, con los digitos de 3x5 (escala 2 = 6x10 como en la maqueta)
    static void drawNumber(unsigned long value, int y, int scale)
    {
        char buf[12];
        snprintf(buf, sizeof(buf), "%lu", value);
        int n = strlen(buf);
        int width = n * 4 * scale - scale;
        int x = NUMBERS_CENTER_X - width / 2;
        for (int i = 0; i < n; i++)
        {
            drawDigit(buf[i] - '0', x, y, scale);
            x += 4 * scale;
        }
    }

    static void drawScore()
    {
        // Hasta 5 digitos caben en grande; mas alla se usa el tamano chico para no invadir el pozo
        if (score > 99999)
            drawNumber(score > 9999999 ? 9999999 : score, SCORE_Y + 2, 1);
        else
            drawNumber(score, SCORE_Y, 2);
    }

    static void drawPreview(int type, int boxY, bool hollow)
    {
        if (type < 0)
            return;
        uint16_t mask = SHAPES[type][0];
        int minR = 4, maxR = -1, minC = 4, maxC = -1;
        for (int r = 0; r < 4; r++)
        {
            for (int c = 0; c < 4; c++)
            {
                if (!cellOf(mask, r, c))
                    continue;
                if (r < minR)
                    minR = r;
                if (r > maxR)
                    maxR = r;
                if (c < minC)
                    minC = c;
                if (c > maxC)
                    maxC = c;
            }
        }
        int pw = (maxC - minC + 1) * PREVIEW_CELL - 1;
        int ph = (maxR - minR + 1) * PREVIEW_CELL - 1;
        int x0 = BOX_X + (BOX_W - pw) / 2;
        int y0 = boxY + (BOX_H - ph) / 2;
        for (int r = minR; r <= maxR; r++)
        {
            for (int c = minC; c <= maxC; c++)
            {
                if (!cellOf(mask, r, c))
                    continue;
                int x = x0 + (c - minC) * PREVIEW_CELL;
                int y = y0 + (r - minR) * PREVIEW_CELL;
                DrawBox(x, y, PREVIEW_BLOCK, PREVIEW_BLOCK);
                // HOLD ya usado en esta pieza: se dibuja hueca
                if (hollow)
                    ClearBox(x + 1, y + 1, 1, 1);
            }
        }
    }

    static void drawBoard(bool blinkOff)
    {
        for (int r = 0; r < ROWS; r++)
        {
            if (blinkOff && (clearMask & (1UL << r)))
                continue;
            for (int c = 0; c < COLS; c++)
            {
                if (board[r][c])
                    DrawBox(WELL_X + c * CELL, WELL_Y + r * CELL, BLOCK, BLOCK);
            }
        }
    }

    static void drawPiece(const Piece &p, bool ghost)
    {
        uint16_t mask = SHAPES[p.type][p.rot];
        for (int r = 0; r < 4; r++)
        {
            for (int c = 0; c < 4; c++)
            {
                if (!cellOf(mask, r, c) || p.y + r < 0)
                    continue;
                int x = WELL_X + (p.x + c) * CELL;
                int y = WELL_Y + (p.y + r) * CELL;
                // La sombra es un punto por celda, como en la maqueta
                if (ghost)
                    DrawBox(x, y, 1, 1);
                else
                    DrawBox(x, y, BLOCK, BLOCK);
            }
        }
    }

    // Texto chico centrado dentro del pozo (quien llama despeja antes el fondo con ClearBox)
    static void drawWellText(int baseline, const char *text)
    {
        int x = WELL_X + (WELL_W - TextWidth(text)) / 2;
        DrawText(x, baseline, text);
    }

    static void render()
    {
        unsigned long now = millis();
        bool blink = (now / 400) % 2 == 0;

        ClearDisplay();
        DrawImage(0, 0, tetrisBackgroundWidth, tetrisBackgroundHeight, TetrisBackgroundBitmap);

        drawScore();
        drawNumber(lines, LINES_Y, 2);
        drawNumber(level, LEVEL_Y, 2);

        bool clearingOff = STATE == states_tetris::CLEARING && ((now - stateStart) / 75) % 2 == 0;
        drawBoard(clearingOff);

        if (STATE == states_tetris::PLAYING)
        {
            Piece ghost = current;
            while (!collides(ghost.type, ghost.rot, ghost.x, ghost.y + 1))
                ghost.y++;
            if (ghost.y != current.y)
                drawPiece(ghost, true);
            drawPiece(current, false);
        }

        if (STATE != states_tetris::INIT)
            drawPreview(nextType, NEXT_Y, false);
        drawPreview(holdType, HOLD_Y, holdUsed);

        SetCustomFont(FONT_TINY);
        if (STATE == states_tetris::INIT)
        {
            ClearBox(WELL_X, 20, WELL_W, 22);
            if (blink)
            {
                drawWellText(29, "PULSA");
                drawWellText(39, "OK");
            }
        }
        else if (STATE == states_tetris::GAME_OVER)
        {
            ClearBox(WELL_X, 16, WELL_W, 33);
            drawWellText(25, "GAME");
            drawWellText(34, "OVER");
            if (blink && now - stateStart >= GAME_OVER_WAIT)
                drawWellText(45, "OK");
        }

        ActDisplay();
    }

    // ------------------------------------------------------------------ API
    void reset()
    {
        memset(board, 0, sizeof(board));
        score = 0;
        lines = 0;
        level = 1;
        holdType = -1;
        holdUsed = false;
        clearMask = 0;
        prevLeft = prevRight = prevUp = prevOk = true;
        STATE = states_tetris::INIT;
        stateStart = millis();
    }

    void game_tetris()
    {
        switch (STATE)
        {
        case states_tetris::INIT:
        {
            bool ok = isPressed(BTN_OK);
            if (ok && !prevOk)
                startGame();
            prevOk = ok;
            break;
        }
        case states_tetris::PLAYING:
            updatePlaying();
            break;
        case states_tetris::CLEARING:
            if (millis() - stateStart >= CLEAR_MS)
            {
                removeClearedRows();
                STATE = states_tetris::PLAYING;
                spawnNext();
            }
            break;
        case states_tetris::GAME_OVER:
        {
            bool ok = isPressed(BTN_OK);
            if (ok && !prevOk && millis() - stateStart >= GAME_OVER_WAIT)
                startGame();
            prevOk = ok;
            break;
        }
        }
        render();
    }
}
