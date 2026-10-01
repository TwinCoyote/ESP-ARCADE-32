#include "screensaver.h"
#include "scenes.h"
#include "../../drivers/display/display.h"
#include "../../config/debug_log.h"

namespace screensaver
{
    namespace
    {
        // Orden en que se turnan las escenas
        const Scene SCENES[] = {
            {"Mascota", petStart, petDraw, false},
            {"Ojos", eyesStart, eyesDraw, false},
            {"Video musical", musicStart, musicDraw, true},
        };
        const uint8_t SCENE_COUNT = sizeof(SCENES) / sizeof(SCENES[0]);

        const unsigned long INTRO_MS = 700; // La pantalla que habia se disuelve antes de la primera escena
        const unsigned long FADE_MS = 400;  // Fundido de entrada y de salida de cada escena

        uint8_t nextScene = 0; // Se conserva entre activaciones: cada vez sigue la que tocaba
        uint8_t current = 0;
        unsigned long sceneStart = 0;
        unsigned long sceneLength = 0;
        unsigned long introStart = 0;
        bool intro = false;

        void startScene()
        {
            current = nextScene;
            nextScene = (nextScene + 1) % SCENE_COUNT;
            sceneStart = millis();
            sceneLength = SCENES[current].start();
            DEV_PRINT("[Protector] Escena: ");
            DEV_PRINTLN(SCENES[current].name);
        }
    }

    void begin()
    {
        intro = true;
        introStart = millis();
        DEV_PRINTLN("[Protector] Inicio");
    }

    void update()
    {
        unsigned long now = millis();
        if (intro)
        {
            unsigned long t = now - introStart;
            if (t < INTRO_MS)
            {
                // Lo ultimo que dibujo el menu o el juego sigue en el buffer: se va apagando con el tramado
                DitherDisplay(1 + t * 15 / INTRO_MS);
                ActDisplay();
                return;
            }
            intro = false;
            startScene();
        }

        unsigned long t = now - sceneStart;
        if (t >= sceneLength)
        {
            startScene();
            t = 0;
        }

        if (!SCENES[current].keepsBuffer)
        {
            ClearDisplay();
        }
        SCENES[current].draw(t);

        unsigned long left = sceneLength - t;
        if (t < FADE_MS)
        {
            DitherDisplay(16 - t * 16 / FADE_MS);
        }
        else if (left < FADE_MS)
        {
            DitherDisplay(16 - left * 16 / FADE_MS);
        }
        ActDisplay();
    }

    // ------------------------------------------------------------------ Ayudas

    void drawSprite(int x, int y, const char *const *rows, uint8_t rowCount, uint8_t scale, bool mirror)
    {
        for (uint8_t r = 0; r < rowCount; r++)
        {
            const char *row = rows[r];
            int len = strlen(row);
            int c = 0;
            while (c < len)
            {
                char ch = row[c];
                if (ch != '#' && ch != 'o')
                {
                    c++;
                    continue;
                }
                // Los pixeles seguidos del mismo tipo se dibujan como un solo rectangulo
                int start = c;
                while (c < len && row[c] == ch)
                {
                    c++;
                }
                int col = mirror ? len - c : start;
                if (ch == '#')
                {
                    DrawBox(x + col * scale, y + r * scale, (c - start) * scale, scale);
                }
                else
                {
                    ClearBox(x + col * scale, y + r * scale, (c - start) * scale, scale);
                }
            }
        }
    }

    uint32_t hash32(uint32_t n)
    {
        n ^= n >> 16;
        n *= 0x7feb352dUL;
        n ^= n >> 15;
        n *= 0x846ca68bUL;
        n ^= n >> 16;
        return n;
    }
}
