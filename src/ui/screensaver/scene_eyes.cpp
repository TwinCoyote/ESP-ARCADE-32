#include "scenes.h"
#include "../../drivers/display/display.h"
#include <math.h>

// Ojos flotantes: dos ojos de esquinas redondeadas que flotan, parpadean, miran alrededor
// (el ojo del lado al que miran se acerca y se ve mas grande) y van cambiando de humor:
// feliz, sorprendido, guino, enamorado, enojado, confundido y al final se duermen.

namespace screensaver
{
    namespace
    {
        // ------------------------------------------------------------------ Medidas
        const float EYE_W = 36;
        const float EYE_H = 36;
        const float EYE_R = 8;
        const float GAP = 10;           // Espacio entre los dos ojos
        const float MAX_GAZE_X = 20;    // Hasta donde pueden mirar sin salirse de la pantalla
        const float MAX_GAZE_Y = 11;

        // ------------------------------------------------------------------ Guion
        enum Mood : uint8_t
        {
            MOOD_WAKE,      // Abren poco a poco
            MOOD_IDLE,      // Miran alrededor
            MOOD_CURIOUS,   // Miran lejos a un lado y al otro
            MOOD_HAPPY,     // Parpados de abajo en arco y se rien (rebotan)
            MOOD_SURPRISED, // Se abren enormes
            MOOD_WINK,      // Guino
            MOOD_LOVE,      // Ojos de corazon que laten
            MOOD_ANGRY,     // Cejas hacia adentro
            MOOD_CONFUSED,  // Un ojo chico, sacuden la cabeza y aparece un "?"
            MOOD_SLEEPY,    // Se les cierran los parpados y se duermen
        };
        struct Part
        {
            Mood mood;
            unsigned long ms;
        };
        // Los humores del medio se barajan cada vez; empiezan despertando y terminan dormidos
        const Part MIDDLE[] = {
            {MOOD_HAPPY, 3600},
            {MOOD_SURPRISED, 2600},
            {MOOD_WINK, 2400},
            {MOOD_LOVE, 4000},
            {MOOD_ANGRY, 3000},
            {MOOD_CONFUSED, 3400},
        };
        const uint8_t MIDDLE_COUNT = sizeof(MIDDLE) / sizeof(MIDDLE[0]);
        const uint8_t MAX_PARTS = MIDDLE_COUNT * 2 + 4;

        Part script[MAX_PARTS];
        uint8_t partCount = 0;
        uint8_t part = 0;
        unsigned long partStart = 0;

        // ------------------------------------------------------------------ Estado
        // Lo que se dibuja (cur) se acerca suavemente a lo que pide el humor (goal)
        struct Face
        {
            float gazeX, gazeY;
            float leftW, leftH, rightW, rightH;
            float radius;
            float tired, angry, happy; // Parpados: 0 = nada, 1 = mucho
        };
        Face cur;
        Face goal;
        unsigned long lastT = 0;
        unsigned long nextGlance = 0; // Cuando cambian de direccion al mirar alrededor
        unsigned long nextBlink = 0;
        unsigned long blinkStart = 0;
        bool doubleBlink = false;

        // Se acerca a 'target' con una rapidez que no depende de los cuadros por segundo
        void approach(float &value, float target, float dt, float tauMs)
        {
            value += (target - value) * (1.0f - expf(-dt / tauMs));
        }

        float randomFloat(float lo, float hi)
        {
            return lo + (hi - lo) * (float)random(1001) / 1000.0f;
        }

        // Cuanto de abiertos estan por el parpadeo automatico (1 = abiertos)
        float blinkOpen(unsigned long t)
        {
            if (t >= nextBlink)
            {
                blinkStart = nextBlink;
                // Uno de cada cuatro es doble
                doubleBlink = random(4) == 0;
                nextBlink = t + random(2200, 5200);
            }
            unsigned long e = t - blinkStart;
            if (doubleBlink && e >= 260 && e < 520)
            {
                e -= 260;
            }
            if (e < 70)
            {
                return 1.0f - e / 70.0f;
            }
            if (e < 110)
            {
                return 0.0f;
            }
            if (e < 210)
            {
                return (e - 110) / 100.0f;
            }
            return 1.0f;
        }

        void setSize(float w, float h)
        {
            goal.leftW = goal.rightW = w;
            goal.leftH = goal.rightH = h;
        }

        // Mira alrededor: cada cierto tiempo elige otra direccion (a veces vuelve al centro)
        void glance(unsigned long t, unsigned long minMs, unsigned long maxMs)
        {
            if (t < nextGlance)
            {
                return;
            }
            nextGlance = t + random(minMs, maxMs);
            if (random(10) < 3)
            {
                goal.gazeX = 0;
                goal.gazeY = 0;
                return;
            }
            // Una de las ocho direcciones, con un poco de variacion
            static const int8_t DIRS[8][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}, {-1, -1}, {1, -1}, {-1, 1}, {1, 1}};
            const int8_t *d = DIRS[random(8)];
            goal.gazeX = d[0] * randomFloat(0.6f, 1.0f) * MAX_GAZE_X;
            goal.gazeY = d[1] * randomFloat(0.6f, 1.0f) * MAX_GAZE_Y;
        }

        // ------------------------------------------------------------------ Dibujo
        // Un ojo con sus parpados. open = 0 cerrado ... 1 abierto
        void drawEye(float cx, float cy, float w, float h, float open, bool isLeft)
        {
            int ww = (int)(w + 0.5f);
            int hh = (int)(h * open + 0.5f);
            if (hh < 2)
            {
                hh = 2; // Cerrado: una rayita
            }
            int x = (int)(cx - ww / 2.0f + 0.5f);
            int y = (int)(cy - hh / 2.0f + 0.5f);
            int r = (int)(cur.radius + 0.5f);
            int maxR = (ww < hh ? ww : hh) / 2 - 1;
            if (r > maxR)
            {
                r = maxR;
            }
            if (r >= 1)
            {
                DrawRoundBox(x, y, ww, hh, r);
            }
            else
            {
                DrawBox(x, y, ww, hh);
            }

            // Parpados en negro, columna por columna:
            //  cansado: el de arriba cae del lado de afuera; enojado: cae del lado de adentro;
            //  feliz: el de abajo sube en arco, asi el ojo queda como un arco (^)
            for (int i = 0; i < ww; i++)
            {
                float u = ww > 1 ? (float)i / (ww - 1) : 0.5f; // 0 orilla izquierda, 1 derecha
                float outer = isLeft ? 1.0f - u : u;
                float top = hh * 0.62f * (cur.tired * outer + cur.angry * (1.0f - outer));
                if (top >= 1)
                {
                    ClearBox(x + i, y, 1, (int)top);
                }
                float d = 2.0f * u - 1.0f;
                float arc = 1.0f - d * d;
                float bottom = cur.happy * hh * 0.72f * (arc > 0 ? sqrtf(arc) : 0);
                if (bottom >= 1)
                {
                    int b = (int)bottom;
                    ClearBox(x + i, y + hh - b, 1, b);
                }
            }
        }

        // Ojo en forma de corazon (dos circulos y un triangulo)
        void drawHeartEye(float cx, float cy, float size)
        {
            int r = (int)(size / 4 + 0.5f);
            int top = (int)(cy - r / 2.0f);
            DrawDisc((int)(cx - r), top, r);
            DrawDisc((int)(cx + r), top, r);
            int bottom = (int)(cy + size * 0.55f);
            for (int y = top; y <= bottom; y++)
            {
                int half = (2 * r) * (bottom - y) / (bottom - top);
                DrawBox((int)cx - half, y, 2 * half + 1, 1);
            }
        }

        void drawSmallHeart(int x, int y)
        {
            DrawBox(x, y, 2, 1);
            DrawBox(x + 3, y, 2, 1);
            DrawBox(x - 1, y + 1, 7, 2);
            DrawBox(x, y + 3, 5, 1);
            DrawBox(x + 1, y + 4, 3, 1);
            DrawPixel(x + 2, y + 5);
        }

        // ------------------------------------------------------------------ Humores
        // Al empezar un humor la mirada vuelve al centro (al mirar alrededor se va moviendo sola)
        void startMood(unsigned long lt)
        {
            nextGlance = lt;
            goal.gazeX = goal.gazeY = 0;
        }

        // Ajusta los objetivos segun el humor y regresa cuanto de abiertos van los ojos
        // (izquierdo y derecho) mas un desplazamiento extra para rebotar o sacudirse
        void updateMood(Mood mood, unsigned long t, unsigned long lt, float &openL, float &openR, float &dx, float &dy)
        {
            // Lo normal; cada humor cambia lo que necesita
            goal.tired = goal.angry = goal.happy = 0;
            goal.radius = EYE_R;
            setSize(EYE_W, EYE_H);
            float blink = blinkOpen(t);
            openL = openR = blink;
            dx = dy = 0;
            switch (mood)
            {
            case MOOD_WAKE:
            {
                // Cerrados, se abren despacio y parpadean dos veces
                float o = lt < 500 ? 0.0f : (lt < 1500 ? (lt - 500) / 1000.0f : 1.0f);
                o = o * o * (3 - 2 * o);
                if ((lt > 1700 && lt < 1850) || (lt > 2000 && lt < 2150))
                {
                    o = 0.05f;
                }
                openL = openR = o;
                nextBlink = t + 2500;
                break;
            }
            case MOOD_IDLE:
                glance(lt, 700, 1700);
                break;
            case MOOD_CURIOUS:
            {
                // Lejos a la izquierda, lejos a la derecha, arriba y de regreso
                if (lt < 1100)
                {
                    goal.gazeX = -MAX_GAZE_X;
                    goal.gazeY = 2;
                }
                else if (lt < 2200)
                {
                    goal.gazeX = MAX_GAZE_X;
                    goal.gazeY = 2;
                }
                else if (lt < 3000)
                {
                    goal.gazeX = 6;
                    goal.gazeY = -MAX_GAZE_Y;
                }
                else
                {
                    goal.gazeX = goal.gazeY = 0;
                }
                break;
            }
            case MOOD_HAPPY:
                goal.happy = 1;
                goal.gazeY = -3;
                // Se rien: rebotan rapido
                if (lt > 400 && lt < 1900)
                {
                    dy = -3.0f * fabsf(sinf((lt - 400) * 3.14159f / 170.0f));
                }
                break;
            case MOOD_SURPRISED:
                if (lt < 1700)
                {
                    goal.radius = 14;
                    setSize(44, 46);
                    goal.gazeY = -1;
                    if (lt < 260)
                    {
                        dy = -2;
                    }
                    openL = openR = 1; // No parpadean del susto
                }
                break;
            case MOOD_WINK:
                goal.happy = 0.45f;
                goal.gazeX = 4;
                goal.gazeY = -2;
                if (lt > 500 && lt < 1200)
                {
                    // El izquierdo se cierra (y se aplasta un poco)
                    float e = (lt - 500) / 700.0f;
                    float c = e < 0.2f ? e / 0.2f : (e > 0.7f ? (1 - e) / 0.3f : 1.0f);
                    openL = 1.0f - 0.95f * c;
                    openR = 1;
                }
                break;
            case MOOD_LOVE:
                openL = openR = 1;
                goal.gazeY = -1;
                break;
            case MOOD_ANGRY:
                goal.angry = 0.85f;
                setSize(EYE_W, 30);
                goal.gazeY = 3;
                if (lt < 450)
                {
                    dx = ((lt / 50) % 2) ? 2 : -2;
                }
                break;
            case MOOD_CONFUSED:
                goal.leftW = 30;
                goal.leftH = 26;
                goal.gazeX = -3;
                goal.gazeY = -2;
                if (lt > 200 && lt < 1000)
                {
                    dx = 3.0f * sinf((lt - 200) * 3.14159f / 90.0f);
                }
                break;
            case MOOD_SLEEPY:
            {
                // Los parpados van cayendo; parpadeos lentos; al final se cierran
                goal.tired = lt < 3200 ? 0.25f + 0.55f * lt / 3200.0f : 0.8f;
                goal.gazeY = 4;
                float o = 1;
                if (lt > 1200 && lt < 2000)
                {
                    o = 0.25f + 0.75f * fabsf(cosf((lt - 1200) * 3.14159f / 800.0f));
                }
                else if (lt >= 2600)
                {
                    float e = (lt - 2600) / 1600.0f;
                    o = e >= 1 ? 0 : 1 - e * e;
                }
                openL = openR = o;
                nextBlink = t + 3000;
                break;
            }
            }
        }
    }

    unsigned long eyesStart()
    {
        // Baraja los humores del medio
        Part middle[MIDDLE_COUNT];
        for (uint8_t i = 0; i < MIDDLE_COUNT; i++)
        {
            middle[i] = MIDDLE[i];
        }
        for (uint8_t i = MIDDLE_COUNT - 1; i > 0; i--)
        {
            uint8_t j = random(i + 1);
            Part tmp = middle[i];
            middle[i] = middle[j];
            middle[j] = tmp;
        }
        partCount = 0;
        script[partCount++] = {MOOD_WAKE, 2600};
        script[partCount++] = {MOOD_IDLE, 6000};
        script[partCount++] = {MOOD_CURIOUS, 3600};
        for (uint8_t i = 0; i < MIDDLE_COUNT; i++)
        {
            script[partCount++] = middle[i];
            // Entre humor y humor miran alrededor un rato
            if (i % 2 == 1)
            {
                script[partCount++] = {MOOD_IDLE, 2500};
            }
        }
        script[partCount++] = {MOOD_SLEEPY, 6500};

        part = 0;
        partStart = 0;
        lastT = 0;
        nextBlink = 2600;
        blinkStart = 0;
        startMood(0);
        goal.tired = goal.angry = goal.happy = 0;
        goal.radius = EYE_R;
        setSize(EYE_W, EYE_H);
        cur = goal;

        unsigned long total = 0;
        for (uint8_t i = 0; i < partCount; i++)
        {
            total += script[i].ms;
        }
        return total;
    }

    void eyesDraw(unsigned long t)
    {
        while (part + 1 < partCount && t >= partStart + script[part].ms)
        {
            partStart += script[part].ms;
            part++;
            startMood(t - partStart);
        }
        unsigned long lt = t - partStart;
        Mood mood = script[part].mood;

        float dt = t > lastT ? (float)(t - lastT) : 0.0f;
        if (dt > 100)
        {
            dt = 100;
        }
        lastT = t;

        float openL, openR, dx, dy;
        updateMood(mood, t, lt, openL, openR, dx, dy);

        // Perspectiva: el ojo del lado al que miran se acerca (crece) y el otro se aleja
        float k = goal.gazeX / MAX_GAZE_X;
        float near = mood == MOOD_CURIOUS ? 7.0f : 3.0f;
        float lw = goal.leftW - k * near, lh = goal.leftH - k * near;
        float rw = goal.rightW + k * near, rh = goal.rightH + k * near;

        // Seguir a los objetivos: la mirada rapido, el tamano y los parpados un poco mas lento
        float beforeX = cur.gazeX;
        approach(cur.gazeX, goal.gazeX, dt, 70);
        approach(cur.gazeY, goal.gazeY, dt, 70);
        approach(cur.leftW, lw, dt, 110);
        approach(cur.leftH, lh, dt, 110);
        approach(cur.rightW, rw, dt, 110);
        approach(cur.rightH, rh, dt, 110);
        approach(cur.radius, goal.radius, dt, 110);
        approach(cur.tired, goal.tired, dt, 160);
        approach(cur.angry, goal.angry, dt, 120);
        approach(cur.happy, goal.happy, dt, 120);

        // Se estiran un poco al moverse rapido
        float speed = dt > 0 ? fabsf(cur.gazeX - beforeX) / dt : 0;
        float stretch = speed * 30.0f;
        if (stretch > 5)
        {
            stretch = 5;
        }

        // Flotan despacio
        float floatX = 3.0f * sinf(t * 6.2832f / 4100.0f);
        float floatY = 4.0f * sinf(t * 6.2832f / 2700.0f);

        float cx = ANCHO_PANTALLA / 2.0f + cur.gazeX + floatX + dx;
        float cy = ALTO_PANTALLA / 2.0f + cur.gazeY + floatY + dy;
        float leftW = cur.leftW + stretch;
        float rightW = cur.rightW + stretch;
        float leftH = cur.leftH - stretch / 2;
        float rightH = cur.rightH - stretch / 2;
        float leftCx = cx - (GAP / 2 + cur.leftW / 2);
        float rightCx = cx + (GAP / 2 + cur.rightW / 2);

        // Que nunca se salgan de la pantalla (al mirar lejos el ojo de ese lado crece)
        float overLeft = 1 - (leftCx - leftW / 2);
        float overRight = (rightCx + rightW / 2) - (ANCHO_PANTALLA - 1);
        if (overLeft > 0)
        {
            leftCx += overLeft;
            rightCx += overLeft;
        }
        else if (overRight > 0)
        {
            leftCx -= overRight;
            rightCx -= overRight;
        }
        float halfH = (leftH > rightH ? leftH : rightH) / 2;
        if (cy - halfH < 1)
        {
            cy = 1 + halfH;
        }
        else if (cy + halfH > ALTO_PANTALLA - 1)
        {
            cy = ALTO_PANTALLA - 1 - halfH;
        }

        if (mood == MOOD_LOVE)
        {
            // Laten como corazon: dos golpes y una pausa
            unsigned long beat = lt % 900;
            float pump = beat < 120 ? beat / 120.0f : (beat < 240 ? 1 - (beat - 120) / 120.0f : 0);
            if (beat >= 260 && beat < 500)
            {
                unsigned long b = beat - 260;
                pump = 0.6f * (b < 120 ? b / 120.0f : 1 - (b - 120) / 120.0f);
            }
            float size = 34 + 6 * pump;
            drawHeartEye(cx - (GAP / 2 + 18), cy - 2, size);
            drawHeartEye(cx + (GAP / 2 + 18), cy - 2, size);
            // Corazoncitos que suben entre los ojos
            for (uint8_t i = 0; i < 3; i++)
            {
                unsigned long age = (lt + i * 700) % 2100;
                int hy = ALTO_PANTALLA - 4 - (int)(age * 40 / 2100);
                int hx = ANCHO_PANTALLA / 2 - 3 + (int)(3 * sinf(age / 180.0f + i));
                drawSmallHeart(hx, hy);
            }
            return;
        }

        drawEye(leftCx, cy, leftW, leftH, openL, true);
        drawEye(rightCx, cy, rightW, rightH, openR, false);

        if (mood == MOOD_CONFUSED && lt > 600)
        {
            SetCustomFont(FONT_MEDIUM_ES);
            int qy = 16 - (int)((lt - 600) / 300 % 2);
            DrawTextUTF8(ANCHO_PANTALLA - 14, qy, "?");
        }
        if (mood == MOOD_SLEEPY && lt > 3400)
        {
            for (uint8_t i = 0; i < 3; i++)
            {
                unsigned long born = 3400 + i * 900UL;
                if (lt < born)
                {
                    continue;
                }
                unsigned long age = (lt - born) % 2700;
                int zx = ANCHO_PANTALLA - 30 + (int)(age / 160);
                int zy = 30 - (int)(age * 26 / 2700);
                SetCustomFont(age < 900 ? FONT_TINY_ES : FONT_SMALL_ES);
                DrawText(zx, zy, age < 900 ? "z" : "Z");
            }
        }
    }
}
