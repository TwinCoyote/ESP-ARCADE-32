#include "scenes.h"
#include "../../drivers/display/display.h"
#include <math.h>

// Video musical: cada frase de la letra entra con su propio efecto de texto al ritmo de la cancion
// (maquina de escribir, onda, eco, remolino, zoom, letras que se derriten, glitch...) y las partes
// sin letra son animaciones psicodelicas inspiradas en el video de "Feels Like We Only Go Backwards":
// cabezas que se repiten una dentro de otra alejandose, figuras que giran y se derriten y un tunel
// del tiempo. Como la pantalla es de un solo color, los "colores" son grises hechos de puntos.
//
// La cancion es una tabla de frases (abajo). Cualquier texto se acomoda solo: en uno o dos renglones
// y con la fuente mas grande que quepa. Una frase vacia se ve como parte instrumental.

namespace screensaver
{
    namespace
    {
        enum Fx : uint8_t
        {
            // Texto
            FX_TITLE,     // Las letras llegan volando y se acomodan (el 2o renglon es un subtitulo)
            FX_TYPE,      // Maquina de escribir con cursor
            FX_POP,       // Palabra por palabra, con anillos que se expanden
            FX_WAVE,      // Las letras ondulan entre lineas que fluyen
            FX_MELT,      // Las letras se derriten y escurren
            FX_ZOOM,      // Cada palabra crece de chica a enorme dentro de un tunel
            FX_SPIN,      // Las letras giran en circulo, como remolino, dejando estela
            FX_ECHO,      // La ultima palabra se repite debajo, cada vez mas chica y tenue
            FX_KALEIDO,   // Caleidoscopio que gira, con destellos en negativo en cada golpe
            FX_SCROLL,    // Cruza la pantalla en camara lenta, ondulando y dejando estela
            FX_SUN,       // Rayos que giran alrededor de la frase
            FX_GLITCH,    // Tiras del texto que se desfasan, como video danado
            FX_STACK,     // Palabras apiladas como cartel, una en cada golpe
            FX_SPOTLIGHT, // A oscuras: solo se ve lo que alumbra una luz que se mueve
            // Animaciones (la frase, si hay, va encima)
            FX_BATS,  // Luna llena y murcielagos
            FX_WARP,  // Tunel del tiempo en espiral
            FX_MORPH, // Figuras de colores que giran y se derriten unas en otras
            FX_HEADS, // Cabezas una dentro de otra que se van alejando
        };

        struct Cue
        {
            uint8_t beat;  // Tiempo (negra) en que empieza
            uint8_t beats; // Cuantos tiempos dura
            Fx fx;
            FontSize font; // Fuente preferida (si no cabe se usa una mas chica)
            const char *text;
        };

        struct Song
        {
            unsigned int beatMs; // Duracion de un tiempo en ms (60000 / golpes por minuto)
            const Cue *cues;
            uint8_t cueCount;
        };

        // ------------------------------------------------------------------ "Dracula" (Tame Impala)
        // La letra no viene incluida porque tiene derechos de autor. Escribe cada frase entre las
        // comillas y aparece con el efecto de su renglon. Un '|' parte la frase en dos renglones
        // (si no, se acomoda sola). Para alargar o acortar una frase cambia 'tiempos' y recorre los
        // que siguen. 115 golpes por minuto, como la cancion.
        const Cue DRACULA[] = {
            // tiempo, tiempos, efecto, fuente, frase
            {0, 8, FX_BATS, FONT_MEDIUM_ES, "DRACULA|TAME IMPALA"},
            {8, 8, FX_WARP, FONT_SMALL_ES, ""},          // Intro
            {16, 4, FX_TYPE, FONT_SMALL_ES, ""},         // Frase 1
            {20, 4, FX_POP, FONT_BUBBLE, ""},            // Frase 2
            {24, 4, FX_WAVE, FONT_SMALL_ES, ""},         // Frase 3
            {28, 4, FX_GLITCH, FONT_MEDIUM_ES, ""},      // Frase 4
            {32, 8, FX_HEADS, FONT_SMALL_ES, ""},        // Puente
            {40, 4, FX_STACK, FONT_MEDIUM_ES, ""},       // Frase 5
            {44, 4, FX_ZOOM, FONT_SMALL_ES, ""},         // Frase 6
            {48, 4, FX_SPOTLIGHT, FONT_BUBBLE, ""},      // Frase 7
            {52, 4, FX_ECHO, FONT_MEDIUM_ES, ""},        // Frase 8
            {56, 8, FX_MORPH, FONT_SMALL_ES, ""},        // Puente
            {64, 4, FX_SPIN, FONT_SMALL_ES, ""},         // Frase 9
            {68, 4, FX_KALEIDO, FONT_MEDIUM_ES, ""},     // Frase 10
            {72, 4, FX_MELT, FONT_MEDIUM_ES, ""},        // Frase 11
            {76, 4, FX_SCROLL, FONT_SMALL_ES, ""},       // Frase 12
            {80, 4, FX_SUN, FONT_MEDIUM_ES, ""},         // Frase 13
            {84, 8, FX_BATS, FONT_MEDIUM_ES, "DRACULA"}, // Final
        };

        const Song SONGS[] = {
            {522, DRACULA, sizeof(DRACULA) / sizeof(DRACULA[0])},
        };
        const uint8_t SONG_COUNT = sizeof(SONGS) / sizeof(SONGS[0]);

        // ------------------------------------------------------------------ Estado
        uint8_t songIndex = 0; // Cada vez que sale esta escena toca la siguiente cancion
        const Song *song = SONGS;
        uint8_t frame = 0; // Para mover el patron de las estelas en cada cuadro

        const float PI_F = 3.14159265f;
        const int MAX_TEXT_W = ANCHO_PANTALLA - 4;

        // ------------------------------------------------------------------ Texto letra por letra
        const uint8_t MAX_GLYPHS = 64;
        struct Line
        {
            uint8_t count;
            char glyph[MAX_GLYPHS][5];  // Cada letra como cadena UTF-8
            uint8_t width[MAX_GLYPHS];
            uint8_t offset[MAX_GLYPHS]; // Donde empieza cada letra dentro del texto
            int total;
        };

        // Separa 'len' bytes de un texto UTF-8 en letras, medidas con la fuente actual
        void splitLine(const char *text, int len, Line &line)
        {
            line.count = 0;
            line.total = 0;
            int i = 0;
            while (i < len && line.count < MAX_GLYPHS)
            {
                uint8_t c = text[i];
                int n = c < 0x80 ? 1 : ((c & 0xE0) == 0xC0 ? 2 : ((c & 0xF0) == 0xE0 ? 3 : 4));
                if (i + n > len)
                {
                    break;
                }
                char *g = line.glyph[line.count];
                memcpy(g, text + i, n);
                g[n] = '\0';
                line.width[line.count] = TextWidthUTF8(g);
                line.offset[line.count] = i;
                line.total += line.width[line.count];
                line.count++;
                i += n;
            }
        }

        bool isAscii(const char *text)
        {
            for (; *text; text++)
            {
                if ((uint8_t)*text >= 0x80)
                {
                    return false;
                }
            }
            return true;
        }

        FontSize smallerFont(FontSize font)
        {
            switch (font)
            {
            case FONT_BUBBLE:
                return FONT_MEDIUM_ES;
            case FONT_MEDIUM_ES:
                return FONT_SMALL_ES;
            default:
                return FONT_TINY_ES;
            }
        }

        // Linea base para que el texto de la fuente actual quede centrado en 'cy'
        int baselineFor(int cy)
        {
            return cy + (TextAscent() + TextDescent()) / 2;
        }

        // Alto de un renglon con la fuente actual
        int rowHeight()
        {
            return TextAscent() - TextDescent() + 2;
        }

        // Texto acomodado en renglones con la fuente mas grande que quepa
        const uint8_t MAX_ROWS = 3;
        struct Layout
        {
            FontSize font;
            uint8_t rows;
            Line line[MAX_ROWS];
        };

        // Ancho de las letras [from, to) de un renglon
        int widthOf(const Line &line, int from, int to)
        {
            int w = 0;
            for (int i = from; i < to; i++)
            {
                w += line.width[i];
            }
            return w;
        }

        // Parte 'all' en 2 o 3 renglones por los espacios que dejen el renglon mas ancho lo mas corto
        // posible. Regresa ese ancho; cuts guarda en que letra (un espacio) se corta
        int bestCuts(const Line &all, uint8_t rows, int *cuts)
        {
            int best = 0x7FFF;
            for (int a = 1; a < all.count - 1; a++)
            {
                if (all.glyph[a][0] != ' ')
                {
                    continue;
                }
                if (rows == 2)
                {
                    int widest = max(widthOf(all, 0, a), widthOf(all, a + 1, all.count));
                    if (widest < best)
                    {
                        best = widest;
                        cuts[0] = a;
                    }
                    continue;
                }
                for (int b = a + 2; b < all.count - 1; b++)
                {
                    if (all.glyph[b][0] != ' ')
                    {
                        continue;
                    }
                    int widest = max(widthOf(all, 0, a), max(widthOf(all, a + 1, b), widthOf(all, b + 1, all.count)));
                    if (widest < best)
                    {
                        best = widest;
                        cuts[0] = a;
                        cuts[1] = b;
                    }
                }
            }
            return best;
        }

        // Acomoda el texto: primero con la fuente pedida en un renglon, luego en dos y luego en tres
        // (si caben en la pantalla); si no cabe, con la siguiente fuente mas chica. Un '|' corta a mano
        void layoutText(const char *text, FontSize preferred, Layout &out, uint8_t maxRows = MAX_ROWS)
        {
            FontSize font = preferred;
            if (font == FONT_BUBBLE && !isAscii(text))
            {
                font = FONT_MEDIUM_ES; // La de burbuja no trae acentos
            }
            int len = strlen(text);
            while (true)
            {
                SetCustomFont(font);
                out.font = font;
                uint8_t rowsThatFit = (ALTO_PANTALLA - 4) / rowHeight();
                uint8_t limit = maxRows < rowsThatFit ? maxRows : rowsThatFit;
                bool fits;
                if (strchr(text, '|') != nullptr)
                {
                    // Renglones a mano
                    out.rows = 0;
                    const char *p = text;
                    while (out.rows < MAX_ROWS)
                    {
                        const char *bar = strchr(p, '|');
                        int n = bar ? bar - p : strlen(p);
                        splitLine(p, n, out.line[out.rows++]);
                        if (bar == nullptr)
                        {
                            break;
                        }
                        p = bar + 1;
                    }
                    fits = out.rows <= limit;
                    for (uint8_t r = 0; r < out.rows; r++)
                    {
                        fits = fits && out.line[r].total <= MAX_TEXT_W;
                    }
                }
                else
                {
                    Line &all = out.line[0];
                    splitLine(text, len, all);
                    out.rows = 1;
                    fits = all.total <= MAX_TEXT_W;
                    int cuts[2] = {0, 0};
                    uint8_t rows = 0;
                    for (uint8_t tryRows = 2; !fits && tryRows <= limit; tryRows++)
                    {
                        if (bestCuts(all, tryRows, cuts) <= MAX_TEXT_W)
                        {
                            fits = true;
                            rows = tryRows;
                        }
                    }
                    if (rows > 1)
                    {
                        // Se arma cada renglon con su pedazo del texto (la ultima entrada es el final)
                        int ends[3] = {all.offset[cuts[0]], rows == 3 ? all.offset[cuts[1]] : len, len};
                        int starts[3] = {0, ends[0] + 1, rows == 3 ? ends[1] + 1 : len};
                        out.rows = rows;
                        for (int8_t r = rows - 1; r >= 0; r--)
                        {
                            splitLine(text + starts[r], ends[r] - starts[r], out.line[r]);
                        }
                    }
                }
                if (fits || font == FONT_TINY_ES)
                {
                    return;
                }
                font = smallerFont(font);
            }
        }

        // Palabras separadas por espacios (el '|' cuenta como espacio)
        uint8_t countWords(const char *text)
        {
            uint8_t n = 0;
            bool inWord = false;
            for (const char *p = text; *p; p++)
            {
                bool space = *p == ' ' || *p == '|';
                if (!space && !inWord)
                {
                    n++;
                }
                inWord = !space;
            }
            return n;
        }

        // Copia la palabra numero 'which' (empezando en 0) en 'out'
        void wordAt(const char *text, uint8_t which, char *out, int outSize)
        {
            uint8_t n = 0;
            const char *p = text;
            while (*p)
            {
                while (*p == ' ' || *p == '|')
                {
                    p++;
                }
                const char *start = p;
                while (*p && *p != ' ' && *p != '|')
                {
                    p++;
                }
                if (p > start && n++ == which)
                {
                    int len = p - start < outSize - 1 ? p - start : outSize - 1;
                    memcpy(out, start, len);
                    out[len] = '\0';
                    return;
                }
            }
            out[0] = '\0';
        }

        // Centro vertical de cada renglon para que el bloque quede centrado en 'cy'
        int rowCenter(const Layout &layout, uint8_t row, int cy)
        {
            int h = rowHeight();
            return cy - (layout.rows - 1) * h / 2 + row * h;
        }

        // Dibuja un renglon centrado en (cx, cy); halo = borra un recuadro detras de cada letra
        void drawLine(const Line &line, int cx, int cy, bool halo)
        {
            int x = cx - line.total / 2;
            int base = baselineFor(cy);
            int asc = TextAscent();
            for (uint8_t i = 0; i < line.count; i++)
            {
                if (halo)
                {
                    ClearBox(x - 1, base - asc - 1, line.width[i] + 2, asc - TextDescent() + 2);
                }
                DrawTextUTF8(x, base, line.glyph[i]);
                x += line.width[i];
            }
        }

        // Todos los renglones centrados en 'cy' (la fuente ya debe estar puesta)
        void drawLayout(const Layout &layout, int cx, int cy, bool halo)
        {
            for (uint8_t r = 0; r < layout.rows; r++)
            {
                drawLine(layout.line[r], cx, rowCenter(layout, r, cy), halo);
            }
        }

        // Borra de x0 a x1 (sin incluir) en una fila; ClearBox con ancho negativo borraria de mas
        void clearSpan(int x0, int x1, int y)
        {
            x0 = x0 < 0 ? 0 : x0;
            x1 = x1 > ANCHO_PANTALLA ? ANCHO_PANTALLA : x1;
            if (x1 > x0)
            {
                ClearBox(x0, y, x1 - x0, 1);
            }
        }

        float ease(float p)
        {
            if (p <= 0)
            {
                return 0;
            }
            if (p >= 1)
            {
                return 1;
            }
            float q = 1 - p;
            return 1 - q * q * q;
        }

        // ------------------------------------------------------------------ Tablas para las animaciones
        // Cada celda de 2x2 pixeles guarda su angulo (0-255 = una vuelta) y su distancia al centro
        const int CELLS_X = ANCHO_PANTALLA / 2;
        const int CELLS_Y = ALTO_PANTALLA / 2;
        uint8_t cellAngle[CELLS_Y][CELLS_X];
        uint8_t cellRadius[CELLS_Y][CELLS_X];
        int8_t sine[256]; // Seno de 0-255 (una vuelta) en -127..127
        bool tablesReady = false;

        void prepareTables()
        {
            if (tablesReady)
            {
                return;
            }
            for (int cy = 0; cy < CELLS_Y; cy++)
            {
                for (int cx = 0; cx < CELLS_X; cx++)
                {
                    float dx = cx * 2 + 1 - ANCHO_PANTALLA / 2;
                    float dy = cy * 2 + 1 - ALTO_PANTALLA / 2;
                    float a = atan2f(dy, dx);
                    cellAngle[cy][cx] = (uint8_t)((int)(a * 256 / (2 * PI_F) + 256) & 255);
                    cellRadius[cy][cx] = (uint8_t)sqrtf(dx * dx + dy * dy);
                }
            }
            for (int i = 0; i < 256; i++)
            {
                sine[i] = (int8_t)(127 * sinf(i * 2 * PI_F / 256));
            }
            tablesReady = true;
        }

        // ------------------------------------------------------------------ Fondos sencillos
        // Lineas que fluyen y se abren alrededor de un punto, como corrientes de aire
        void drawFlow(unsigned long t, float cy)
        {
            float cx = ANCHO_PANTALLA / 2 + 22 * sinf(t / 2300.0f);
            for (int i = 0; i < 9; i++)
            {
                float base = 3 + i * 7.25f;
                float side = base < cy ? -1.0f : 1.0f;
                float dist = fabsf(base - cy);
                int prevX = 0;
                int prevY = 0;
                for (int x = 0; x <= ANCHO_PANTALLA; x += 4)
                {
                    float dx = (x - cx) / 24.0f;
                    float bump = 10.0f * expf(-dx * dx) * expf(-dist / 20.0f);
                    int y = (int)(base + side * bump + 1.6f * sinf(x / 13.0f + t / 480.0f + i));
                    if (x > 0)
                    {
                        DrawLine(prevX, prevY, x, y);
                    }
                    prevX = x;
                    prevY = y;
                }
            }
        }

        // Anillos que salen del centro con cada golpe
        void drawRings(unsigned long age)
        {
            for (uint8_t k = 0; k < 2; k++)
            {
                long a = (long)age - k * 160;
                if (a < 0)
                {
                    continue;
                }
                int r = 6 + (int)(a * 0.12f);
                if (r < 75)
                {
                    DrawCircle(ANCHO_PANTALLA / 2, ALTO_PANTALLA / 2, r);
                }
            }
        }

        // Tunel de rectangulos que se acercan
        void drawTunnel(unsigned long lt)
        {
            for (uint8_t k = 0; k < 5; k++)
            {
                float s = fmodf(lt / 900.0f + k / 5.0f, 1.0f);
                s = s * s;
                int w = 6 + (int)(s * 150);
                int h = 3 + (int)(s * 76);
                DrawFrame(ANCHO_PANTALLA / 2 - w / 2, ALTO_PANTALLA / 2 - h / 2, w, h);
            }
        }

        // Caleidoscopio: el mismo "petalo" repetido 8 veces alrededor del centro, la mitad en espejo
        void drawKaleido(unsigned long t)
        {
            const float cx = ANCHO_PANTALLA / 2;
            const float cy = ALTO_PANTALLA / 2;
            float rot = t / 1400.0f;
            float r1 = 9 + 4 * sinf(t / 380.0f);
            float r2 = 34 + 8 * sinf(t / 610.0f + 1);
            float r3 = 22 + 6 * sinf(t / 450.0f + 2);
            for (uint8_t k = 0; k < 8; k++)
            {
                float base = rot + k * PI_F / 4;
                float mirror = (k % 2) ? -1.0f : 1.0f;
                float a1 = base + mirror * 0.28f;
                float a2 = base - mirror * 0.18f;
                DrawLine((int)(cx + r1 * cosf(a1)), (int)(cy + r1 * sinf(a1)), (int)(cx + r2 * cosf(base)), (int)(cy + r2 * sinf(base)));
                DrawCircle((int)(cx + r3 * cosf(a2)), (int)(cy + r3 * sinf(a2)), 2);
                DrawPixel((int)(cx + (r2 + 6) * cosf(a1)), (int)(cy + (r2 + 6) * sinf(a1)));
            }
        }

        // ------------------------------------------------------------------ Animaciones psicodelicas
        // Tunel del tiempo: anillos que se acercan y franjas que giran en espiral
        void drawWarp(unsigned long t, unsigned int beatMs)
        {
            prepareTables();
            static const uint8_t LEVELS[4] = {0, 6, 16, 10};
            bool flash = (t % beatMs) < 70;
            int travel = t / 5;
            int spin = t / 18;
            for (int cy = 0; cy < CELLS_Y; cy++)
            {
                for (int cx = 0; cx < CELLS_X; cx++)
                {
                    int r = cellRadius[cy][cx];
                    int depth = 1600 / (r + 5);
                    int z = depth + travel;
                    int v = cellAngle[cy][cx] + depth * 2 + spin;
                    uint8_t level = LEVELS[((z >> 4) & 1) * 2 + ((v >> 5) & 1)];
                    // Al fondo (el centro) se oscurece
                    if (r < 22)
                    {
                        level = level * r / 22;
                    }
                    if (flash)
                    {
                        level = 16 - level;
                    }
                    FillDither(cx * 2, cy * 2, 2, 2, level);
                }
            }
        }

        // Anillos con forma de flor que cambian de numero de petalos, giran y se van abriendo
        void drawMorph(unsigned long t, unsigned int beatMs)
        {
            prepareTables();
            static const uint8_t LEVELS[5] = {16, 3, 11, 0, 7};
            // Forma: de 3 a 6 petalos, pasando suavemente de una a la otra
            unsigned long cycle = t % 12000;
            uint8_t lobes = 3 + cycle / 3000;
            float blend = (cycle % 3000) / 3000.0f;
            blend = blend < 0.7f ? 0 : (blend - 0.7f) / 0.3f;
            float depth = 0.22f + 0.12f * sinf(t / 900.0f);
            uint8_t rot = t / 30;
            // Late con el ritmo
            unsigned long beat = t % beatMs;
            float pulse = beat < 160 ? 1.0f - beat / 160.0f : 0;
            float ringSize = 9.0f - 2.0f * pulse;
            float flow = t / 260.0f;
            for (int cy = 0; cy < CELLS_Y; cy++)
            {
                for (int cx = 0; cx < CELLS_X; cx++)
                {
                    uint8_t a = cellAngle[cy][cx] + rot;
                    float s1 = sine[(uint8_t)(a * lobes)];
                    float s2 = sine[(uint8_t)(a * (lobes == 6 ? 3 : lobes + 1))];
                    float shape = 1.0f + depth * ((1 - blend) * s1 + blend * s2) / 127.0f;
                    int ring = (int)(cellRadius[cy][cx] / (shape * ringSize) - flow);
                    FillDither(cx * 2, cy * 2, 2, 2, LEVELS[((ring % 5) + 5) % 5]);
                }
            }
        }

        // Cabeza de perfil (mirando a la derecha) en una caja de 100x100
        const int8_t HEAD_X[] = {40, 55, 66, 73, 76, 75, 77, 85, 86, 80, 80, 84, 80, 83, 78, 78, 74, 66, 60, 60, 32, 34, 24, 16, 12, 14, 22, 30};
        const int8_t HEAD_Y[] = {2, 3, 8, 16, 25, 31, 35, 45, 49, 51, 55, 58, 60, 63, 66, 70, 76, 79, 80, 95, 100, 84, 76, 64, 48, 30, 15, 7};
        const uint8_t HEAD_POINTS = sizeof(HEAD_X);
        static_assert(sizeof(HEAD_X) == sizeof(HEAD_Y), "la cabeza necesita el mismo numero de X que de Y");
        const float HEAD_FOCUS_X = 48; // Punto de la cabeza hacia el que se van metiendo las demas
        const float HEAD_FOCUS_Y = 42;

        // Rellena un poligono con un gris de puntos, renglon por renglon
        void fillPolygon(const int16_t *xs, const int16_t *ys, uint8_t n, uint8_t level)
        {
            int top = ys[0];
            int bottom = ys[0];
            for (uint8_t i = 1; i < n; i++)
            {
                top = ys[i] < top ? ys[i] : top;
                bottom = ys[i] > bottom ? ys[i] : bottom;
            }
            top = top < 0 ? 0 : top;
            bottom = bottom > ALTO_PANTALLA - 1 ? ALTO_PANTALLA - 1 : bottom;
            int16_t cuts[16];
            for (int y = top; y <= bottom; y++)
            {
                uint8_t count = 0;
                for (uint8_t i = 0; i < n && count < 16; i++)
                {
                    uint8_t j = (i + 1) % n;
                    int y0 = ys[i];
                    int y1 = ys[j];
                    if ((y0 <= y && y1 > y) || (y1 <= y && y0 > y))
                    {
                        cuts[count++] = xs[i] + (long)(y - y0) * (xs[j] - xs[i]) / (y1 - y0);
                    }
                }
                // Ordena los cruces y rellena por pares
                for (uint8_t i = 1; i < count; i++)
                {
                    int16_t v = cuts[i];
                    int k = i - 1;
                    while (k >= 0 && cuts[k] > v)
                    {
                        cuts[k + 1] = cuts[k];
                        k--;
                    }
                    cuts[k + 1] = v;
                }
                for (uint8_t i = 0; i + 1 < count; i += 2)
                {
                    FillDither(cuts[i], y, cuts[i + 1] - cuts[i] + 1, 1, level);
                }
            }
        }

        // Cabezas una dentro de otra que se van haciendo chiquitas hacia atras, sin fin.
        // Cada una tiene su propio gris y el contorno se ondula como plastilina
        void drawHeads(unsigned long t)
        {
            static const uint8_t LEVELS[6] = {16, 4, 10, 1, 13, 7};
            const float ratio = 0.42f;    // Tamano de cada cabeza respecto a la que la contiene
            const float periodMs = 1700;  // Lo que tarda una cabeza en encogerse al tamano de la de adentro
            const float size = 64;        // Alto en pixeles de una cabeza a escala 1
            const float screenX = ANCHO_PANTALLA / 2;
            const float screenY = 30;
            float z = t / periodMs;
            int first = (int)floorf(z - 3.5f);
            int last = (int)floorf(z + 1.3f);
            int16_t xs[HEAD_POINTS];
            int16_t ys[HEAD_POINTS];
            // De la mas grande (afuera) a la mas chica (adentro), cada una tapando a la anterior
            for (int j = last; j >= first; j--)
            {
                float s = powf(ratio, z - j) * size / 100.0f;
                for (uint8_t i = 0; i < HEAD_POINTS; i++)
                {
                    float hy = HEAD_Y[i];
                    float wobble = 1.6f * sinf(hy * 0.15f + t / 260.0f + j);
                    xs[i] = (int16_t)(screenX + (HEAD_X[i] - HEAD_FOCUS_X + wobble) * s);
                    ys[i] = (int16_t)(screenY + (hy - HEAD_FOCUS_Y) * s);
                }
                uint8_t level = LEVELS[((j % 6) + 6) % 6];
                fillPolygon(xs, ys, HEAD_POINTS, level);
                for (uint8_t i = 0; i < HEAD_POINTS; i++)
                {
                    uint8_t k = (i + 1) % HEAD_POINTS;
                    DrawLine(xs[i], ys[i], xs[k], ys[k]);
                }
                // El ojo
                if (s > 0.12f)
                {
                    int ex = (int)(screenX + (68 - HEAD_FOCUS_X) * s);
                    int ey = (int)(screenY + (31 - HEAD_FOCUS_Y) * s);
                    int er = (int)(2.5f * s) + 1;
                    if (level > 8)
                    {
                        ClearBox(ex - er, ey - er / 2, 2 * er + 1, er + 1);
                    }
                    else
                    {
                        DrawBox(ex - er, ey - er / 2, 2 * er + 1, er + 1);
                    }
                }
            }
        }

        // Murcielago aleteando: alas arriba, extendidas y abajo
        const char *const BAT_UP[] = {
            "##.........##",
            ".##.......##.",
            ".###.#.#.###.",
            "..#########..",
            "....#####....",
            ".....#.#.....",
        };
        const char *const BAT_MID[] = {
            ".............",
            ".....#.#.....",
            "#...#####...#",
            "#############",
            ".###.....###.",
            ".............",
        };
        const char *const BAT_DOWN[] = {
            ".............",
            ".....#.#.....",
            "....#####....",
            "..#########..",
            ".###.....###.",
            "##.........##",
        };
        const char *const *const BAT_FLAP[4] = {BAT_UP, BAT_MID, BAT_DOWN, BAT_MID};
        const int BAT_W = 13;

        // Luna llena de puntos con murcielagos que la cruzan aleteando
        void drawBats(unsigned long t, unsigned long lt)
        {
            const int moonX = ANCHO_PANTALLA / 2;
            const int moonY = 30;
            const int moonR = 22;
            for (int dy = -moonR; dy <= moonR; dy++)
            {
                int half = (int)sqrtf((float)(moonR * moonR - dy * dy));
                FillDither(moonX - half, moonY + dy, 2 * half + 1, 1, 5);
            }
            DrawCircle(moonX, moonY, moonR);
            DrawCircle(moonX - 8, moonY - 6, 4);
            DrawCircle(moonX + 9, moonY + 7, 3);
            DrawCircle(moonX + 6, moonY - 11, 2);
            // Cada murcielago cruza en su propio tiempo, subiendo y bajando; los dos primeros
            // van mas cerca (se ven al doble de tamano) y los demas lejos
            for (uint8_t i = 0; i < 5; i++)
            {
                uint8_t scale = i < 2 ? 2 : 1;
                int w = BAT_W * scale;
                uint32_t h = hash32(i * 313 + 7);
                unsigned long period = (scale == 2 ? 2400 : 3400) + h % 1500;
                unsigned long phase = (lt + h % period) % period;
                int x = (int)((ANCHO_PANTALLA + 2 * w) * phase / period) - w;
                if (i % 2 == 0)
                {
                    x = ANCHO_PANTALLA - x - w;
                }
                int y = 3 + (int)((h >> 8) % (ALTO_PANTALLA - 12 - 6 * scale)) + (int)(4 * sinf(phase / 170.0f + i));
                const char *const *pose = BAT_FLAP[((t + i * 90) / 110) % 4];
                drawSprite(x, y, pose, 6, scale, i % 2 == 0);
            }
        }

        // Animacion que ocupa una frase: las de animacion son la suya; las de texto sin frase toman
        // una de las tres psicodelicas, distinta de la de antes y de la que viene despues
        Fx visualOf(const Cue *cues, uint8_t count, uint8_t index)
        {
            static const Fx CYCLE[3] = {FX_HEADS, FX_MORPH, FX_WARP};
            Fx previous = FX_TITLE;
            Fx visual = FX_TITLE;
            for (uint8_t k = 0; k <= index; k++)
            {
                const Cue &c = cues[k];
                bool isAnimation = c.fx == FX_BATS || c.fx == FX_WARP || c.fx == FX_MORPH || c.fx == FX_HEADS || c.fx == FX_KALEIDO;
                if (isAnimation || countWords(c.text) > 0)
                {
                    visual = c.fx;
                }
                else
                {
                    Fx next = k + 1 < count ? cues[k + 1].fx : FX_TITLE;
                    for (uint8_t step = 0; step < 3; step++)
                    {
                        visual = CYCLE[(k + step) % 3];
                        if (visual != previous && visual != next)
                        {
                            break;
                        }
                    }
                }
                previous = visual;
            }
            return visual;
        }

        void drawInstrumental(Fx visual, unsigned long t, unsigned int beatMs)
        {
            switch (visual)
            {
            case FX_HEADS:
                drawHeads(t);
                break;
            case FX_MORPH:
                drawMorph(t, beatMs);
                break;
            default:
                drawWarp(t, beatMs);
                break;
            }
        }

        // ------------------------------------------------------------------ Efectos de texto
        // Cada uno dibuja una frase: lt = ms desde que empezo la frase, dur = cuanto dura

        void fxTitle(const Cue &c, unsigned long lt, unsigned long dur)
        {
            const char *bar = strchr(c.text, '|');
            char main[64];
            int len = bar ? bar - c.text : strlen(c.text);
            len = len < (int)sizeof(main) - 1 ? len : sizeof(main) - 1;
            memcpy(main, c.text, len);
            main[len] = '\0';
            Layout layout;
            layoutText(main, c.font, layout, 1);
            const Line &line = layout.line[0];
            int x = (ANCHO_PANTALLA - line.total) / 2;
            int base = baselineFor(bar ? 26 : 32);
            int asc = TextAscent();
            float travel = dur * 0.5f;
            for (uint8_t i = 0; i < line.count; i++)
            {
                // Cada letra sale de un lugar distinto y llega un poquito despues que la anterior
                uint32_t h = hash32(i * 977 + 13);
                float sx = (float)(h % 220) - 46;
                float sy = (float)((h >> 9) % 130) - 33;
                float p = ease((lt - (float)i * 45) / travel);
                int gx = (int)(sx + (x - sx) * p);
                int gy = (int)(sy + (base - sy) * p);
                ClearBox(gx - 1, gy - asc - 1, line.width[i] + 2, asc + 3);
                DrawTextUTF8(gx, gy, line.glyph[i]);
                x += line.width[i];
            }
            // El subtitulo se escribe letra por letra cuando el titulo ya llego
            if (bar && lt > travel)
            {
                SetCustomFont(FONT_TINY_ES);
                const char *sub = bar + 1;
                size_t shown = (lt - (unsigned long)travel) / 45;
                char buf[64];
                size_t n = strlen(sub);
                n = n < sizeof(buf) - 1 ? n : sizeof(buf) - 1;
                n = shown < n ? shown : n;
                memcpy(buf, sub, n);
                buf[n] = '\0';
                int w = TextWidthUTF8(sub);
                ClearBox((ANCHO_PANTALLA - w) / 2 - 2, 41, w + 4, 9);
                DrawTextUTF8((ANCHO_PANTALLA - w) / 2, 48, buf);
            }
        }

        void fxType(const Cue &c, unsigned long lt, unsigned long dur)
        {
            Layout layout;
            layoutText(c.text, c.font, layout);
            int total = layout.line[0].count + (layout.rows > 1 ? layout.line[1].count : 0);
            unsigned long step = (unsigned long)(dur * 0.6f) / (total ? total : 1);
            int shown = step ? lt / step : total;
            int cursorX = 0;
            int cursorBase = 0;
            for (uint8_t r = 0; r < layout.rows; r++)
            {
                const Line &line = layout.line[r];
                int x = (ANCHO_PANTALLA - line.total) / 2;
                int base = baselineFor(rowCenter(layout, r, ALTO_PANTALLA / 2));
                if (r == 0 || shown > 0)
                {
                    cursorX = x;
                    cursorBase = base;
                }
                for (uint8_t i = 0; i < line.count && shown > 0; i++, shown--)
                {
                    DrawTextUTF8(x, base, line.glyph[i]);
                    x += line.width[i];
                    cursorX = x;
                }
            }
            // Cursor: fijo mientras escribe, despues parpadea
            if (lt < dur * 0.6f || (lt / 280) % 2 == 0)
            {
                DrawBox(cursorX + 1, cursorBase - TextAscent(), 2, TextAscent() + 1);
            }
        }

        void fxPop(const Cue &c, unsigned long lt, unsigned long dur)
        {
            uint8_t words = countWords(c.text);
            unsigned long each = dur / words;
            uint8_t k = lt / each;
            if (k >= words)
            {
                k = words - 1;
            }
            unsigned long age = lt - k * each;
            drawRings(age);
            char word[48];
            wordAt(c.text, k, word, sizeof(word));
            // Los primeros milisegundos sale chica: se ve como si brincara hacia la pantalla
            Layout layout;
            layoutText(word, age < 70 ? FONT_SMALL_ES : c.font, layout, 1);
            const Line &line = layout.line[0];
            int base = baselineFor(ALTO_PANTALLA / 2);
            ClearBox((ANCHO_PANTALLA - line.total) / 2 - 3, base - TextAscent() - 3, line.total + 6, TextAscent() - TextDescent() + 6);
            drawLine(line, ANCHO_PANTALLA / 2, ALTO_PANTALLA / 2, false);
        }

        void fxWave(const Cue &c, unsigned long lt, unsigned long t)
        {
            drawFlow(t, ALTO_PANTALLA / 2);
            Layout layout;
            layoutText(c.text, c.font, layout);
            float amp = lt < 400 ? lt / 100.0f : 4.0f;
            int asc = TextAscent();
            for (uint8_t r = 0; r < layout.rows; r++)
            {
                const Line &line = layout.line[r];
                int x = (ANCHO_PANTALLA - line.total) / 2;
                int base = baselineFor(rowCenter(layout, r, ALTO_PANTALLA / 2));
                for (uint8_t i = 0; i < line.count; i++)
                {
                    int y = base + (int)(amp * sinf(lt / 160.0f - i * 0.65f - r));
                    ClearBox(x - 1, y - asc - 1, line.width[i] + 2, asc + 4);
                    DrawTextUTF8(x, y, line.glyph[i]);
                    x += line.width[i];
                }
            }
        }

        void fxMelt(const Cue &c, unsigned long lt, unsigned long dur)
        {
            Layout layout;
            layoutText(c.text, c.font, layout);
            int asc = TextAscent();
            for (uint8_t r = 0; r < layout.rows; r++)
            {
                const Line &line = layout.line[r];
                bool melts = r == layout.rows - 1; // Con dos renglones, el de arriba solo ondula
                int x = (ANCHO_PANTALLA - line.total) / 2;
                int base = baselineFor(rowCenter(layout, r, layout.rows > 1 ? 30 : 26));
                for (uint8_t i = 0; i < line.count; i++)
                {
                    int drop = 0;
                    if (melts)
                    {
                        // Cada letra empieza a escurrir en un momento distinto y deja un hilo
                        unsigned long start = dur * 3 / 10 + hash32(i * 131 + 5) % (dur * 4 / 10);
                        if (lt > start)
                        {
                            float e = (float)(lt - start);
                            drop = (int)(0.00011f * e * e);
                        }
                        int cx = x + line.width[i] / 2;
                        if (drop > 0 && line.glyph[i][0] != ' ')
                        {
                            DrawLine(cx, base - asc / 2, cx, base + drop - asc);
                            DrawLine(cx - 2, base - 2, cx - 2, base - 2 + drop / 4);
                        }
                    }
                    else
                    {
                        drop = (int)(1.5f * sinf(lt / 200.0f - i * 0.5f));
                    }
                    if (base + drop - asc < ALTO_PANTALLA)
                    {
                        DrawTextUTF8(x, base + drop, line.glyph[i]);
                    }
                    x += line.width[i];
                }
            }
        }

        void fxZoom(const Cue &c, unsigned long lt, unsigned long dur, unsigned int beatMs)
        {
            drawTunnel(lt);
            uint8_t words = countWords(c.text);
            unsigned long each = dur / words;
            uint8_t k = lt / each;
            if (k >= words)
            {
                k = words - 1;
            }
            // Cada palabra pasa por tres tamanos: chica, mediana y la mas grande que quepa
            unsigned long age = lt - k * each;
            static const FontSize STEPS[] = {FONT_SMALL_ES, FONT_MEDIUM_ES, FONT_BUBBLE};
            uint8_t step = age * 3 / each;
            char word[48];
            wordAt(c.text, k, word, sizeof(word));
            Layout layout;
            layoutText(word, STEPS[step < 3 ? step : 2], layout, 1);
            const Line &line = layout.line[0];
            // La ultima palabra tiembla con cada golpe
            int shake = (k == words - 1 && (lt % beatMs) < 110) ? ((lt / 40) % 2 ? 2 : -2) : 0;
            int base = baselineFor(ALTO_PANTALLA / 2);
            ClearBox((ANCHO_PANTALLA - line.total) / 2 - 3 + shake, base - TextAscent() - 3, line.total + 6, TextAscent() - TextDescent() + 6);
            drawLine(line, ANCHO_PANTALLA / 2 + shake, ALTO_PANTALLA / 2, false);
        }

        void fxSpin(const Cue &c, unsigned long lt, unsigned int beatMs)
        {
            // La frase completa alrededor de un circulo; si es muy larga, con letra mas chica
            char ring[96];
            snprintf(ring, sizeof(ring), "%s · ", c.text);
            for (char *p = ring; *p; p++)
            {
                if (*p == '|')
                {
                    *p = ' ';
                }
            }
            SetCustomFont(c.font);
            Line line;
            splitLine(ring, strlen(ring), line);
            if (line.total > 180)
            {
                SetCustomFont(FONT_TINY_ES);
                splitLine(ring, strlen(ring), line);
            }
            // Empieza casi quieto (para que se lea), despues gira cada vez mas rapido y el circulo respira
            float secs = lt / 1000.0f;
            float angle = -0.35f * secs * secs - 0.2f * secs;
            float r = 25 + 3 * sinf(lt / 450.0f);
            int asc = TextAscent();
            float step = 2 * PI_F / (line.total > 0 ? line.total : 1);
            float along = 0;
            for (uint8_t i = 0; i < line.count; i++)
            {
                // Cada letra ocupa en el circulo lo que mide
                float a = angle + (along + line.width[i] / 2.0f) * step;
                along += line.width[i];
                int gx = (int)(ANCHO_PANTALLA / 2 + r * 1.35f * cosf(a)) - line.width[i] / 2;
                int gy = (int)(ALTO_PANTALLA / 2 + r * sinf(a)) + asc / 2;
                DrawTextUTF8(gx, gy, line.glyph[i]);
            }
            // Centro que late con el ritmo y un anillo de puntos girando al reves
            unsigned long beat = lt % beatMs;
            int pulse = beat < 150 ? 5 - (int)(beat / 50) : 2;
            DrawDisc(ANCHO_PANTALLA / 2, ALTO_PANTALLA / 2, pulse);
            for (uint8_t k = 0; k < 8; k++)
            {
                float a = -angle * 0.8f + k * PI_F / 4;
                DrawPixel((int)(ANCHO_PANTALLA / 2 + 12 * cosf(a)), (int)(ALTO_PANTALLA / 2 + 10 * sinf(a)));
            }
        }

        // La frase y debajo su ultima palabra repetida, cada vez mas chica y mas tenue (una en cada golpe)
        void fxEcho(const Cue &c, unsigned long lt, unsigned int beatMs)
        {
            static const FontSize SMALLER[] = {FONT_MEDIUM_ES, FONT_SMALL_ES, FONT_TINY_ES, FONT_TINY_ES};
            Layout main;
            layoutText(c.text, c.font, main);
            int mainHeight = main.rows * rowHeight();
            int top = 4 + mainHeight / 2;
            uint8_t first = main.font == FONT_BUBBLE ? 0 : (main.font == FONT_MEDIUM_ES ? 1 : 2);
            uint8_t echoes = lt / beatMs;
            echoes = echoes > 3 ? 3 : echoes;
            char last[48];
            wordAt(c.text, countWords(c.text) - 1, last, sizeof(last));
            // Posicion de cada copia, una debajo de otra mientras quepan
            int ys[3];
            int y = top + mainHeight / 2 + 2;
            uint8_t fitting = 0;
            for (uint8_t k = 0; k < 3; k++)
            {
                SetCustomFont(SMALLER[first + k < 4 ? first + k : 3]);
                int h = rowHeight();
                if (y + h > ALTO_PANTALLA)
                {
                    break;
                }
                ys[k] = y + h / 2;
                y += h;
                fitting++;
            }
            echoes = echoes < fitting ? echoes : fitting;
            // Se dibujan de la mas lejana a la principal; despues de cada una se oscurece todo un poco
            for (int k = echoes; k >= 1; k--)
            {
                SetCustomFont(SMALLER[first + k - 1 < 4 ? first + k - 1 : 3]);
                Line line;
                splitLine(last, strlen(last), line);
                int sway = (int)(3 * sinf(lt / 260.0f + k));
                drawLine(line, ANCHO_PANTALLA / 2 + sway, ys[k - 1], false);
                DitherDisplay(3, k * 6);
            }
            SetCustomFont(main.font);
            drawLayout(main, ANCHO_PANTALLA / 2, top, true);
        }

        void fxKaleido(const Cue &c, unsigned long lt, unsigned long t, unsigned int beatMs)
        {
            drawKaleido(t);
            if (countWords(c.text) > 0)
            {
                Layout layout;
                layoutText(c.text, c.font, layout);
                drawLayout(layout, ANCHO_PANTALLA / 2, ALTO_PANTALLA / 2, true);
            }
            // Destello en negativo justo en cada golpe
            if ((lt % beatMs) < 90)
            {
                InvertBox(0, 0, ANCHO_PANTALLA, ALTO_PANTALLA);
            }
        }

        void fxScroll(const Cue &c, unsigned long lt, unsigned long dur)
        {
            SetCustomFont(c.font);
            Line line;
            char text[96];
            snprintf(text, sizeof(text), "%s", c.text);
            for (char *p = text; *p; p++)
            {
                if (*p == '|')
                {
                    *p = ' ';
                }
            }
            splitLine(text, strlen(text), line);
            // En "camara lenta": entra rapido por la derecha, cruza el centro muy despacio y sale
            // rapido por la izquierda (con la estela, lo rapido se ve barrido)
            float start = ANCHO_PANTALLA + 4;
            float end = -line.total - 4;
            float middle = (ANCHO_PANTALLA - line.total) / 2.0f;
            float slow = line.total > ANCHO_PANTALLA ? (float)(line.total - ANCHO_PANTALLA + 40) : 20.0f;
            float p = (float)lt / dur;
            float x0;
            if (p < 0.2f)
            {
                x0 = start + (middle + slow / 2 - start) * ease(p / 0.2f);
            }
            else if (p < 0.8f)
            {
                x0 = middle + slow / 2 - slow * (p - 0.2f) / 0.6f;
            }
            else
            {
                float q = (p - 0.8f) / 0.2f;
                x0 = middle - slow / 2 + (end - middle + slow / 2) * q * q;
            }
            int x = (int)x0;
            int base = baselineFor(ALTO_PANTALLA / 2 - 4);
            for (uint8_t i = 0; i < line.count; i++)
            {
                int y = base + (int)(3 * sinf(lt / 400.0f + i * 0.5f));
                if (x > -12 && x < ANCHO_PANTALLA)
                {
                    DrawTextUTF8(x, y, line.glyph[i]);
                }
                x += line.width[i];
            }
            // Una onda tranquila abajo, como osciloscopio
            int prevY = 0;
            for (int px = 0; px <= ANCHO_PANTALLA; px += 3)
            {
                int y = 54 + (int)(4 * sinf(px / 10.0f + lt / 250.0f) * sinf(px / 37.0f + lt / 900.0f));
                if (px > 0)
                {
                    DrawLine(px - 3, prevY, px, y);
                }
                prevY = y;
            }
        }

        void fxSun(const Cue &c, unsigned long lt, unsigned int beatMs)
        {
            const int cx = ANCHO_PANTALLA / 2;
            const int cy = 34;
            // Rayos que giran y laten con el ritmo
            unsigned long beat = lt % beatMs;
            float grow = beat < 200 ? 4 * (1 - beat / 200.0f) : 0;
            float spin = lt / 1800.0f;
            for (uint8_t k = 0; k < 16; k++)
            {
                float a = spin + k * PI_F / 8;
                float inner = (k % 2) ? 36 : 38;
                float outer = inner + ((k % 2) ? 5 : 9) + grow;
                DrawLine((int)(cx + inner * cosf(a)), (int)(cy + inner * 0.55f * sinf(a)), (int)(cx + outer * cosf(a)), (int)(cy + outer * 0.55f * sinf(a)));
            }
            Layout layout;
            layoutText(c.text, c.font, layout);
            drawLayout(layout, cx, cy, true);
        }

        // Tiras horizontales del texto que de repente se desfasan, como una cinta danada
        void fxGlitch(const Cue &c, unsigned long lt, unsigned int beatMs)
        {
            Layout layout;
            layoutText(c.text, c.font, layout);
            int height = layout.rows * rowHeight();
            int top = ALTO_PANTALLA / 2 - height / 2 - 1;
            // Falla en rafagas: fuerte justo en cada golpe y a ratos entre golpes
            unsigned long beat = lt % beatMs;
            uint32_t burst = hash32(lt / 90 + 7);
            bool glitching = beat < 140 || burst % 5 == 0;
            const uint8_t SLICES = 5;
            for (uint8_t k = 0; k < SLICES; k++)
            {
                int y0 = top + height * k / SLICES;
                int y1 = top + height * (k + 1) / SLICES;
                int dx = 0;
                if (glitching)
                {
                    uint32_t h = hash32((lt / 60) * 31 + k);
                    dx = (int)(h % 17) - 8;
                }
                SetClipWindow(0, y0, ANCHO_PANTALLA, y1);
                drawLayout(layout, ANCHO_PANTALLA / 2 + dx, ALTO_PANTALLA / 2, false);
            }
            ResetClipWindow();
            if (glitching)
            {
                // Rayas de estatica y una copia corrida del texto
                for (uint8_t k = 0; k < 4; k++)
                {
                    uint32_t h = hash32(lt / 45 + k * 97);
                    int y = h % ALTO_PANTALLA;
                    int x = (h >> 8) % ANCHO_PANTALLA;
                    DrawLine(x, y, x + 6 + (int)((h >> 16) % 30), y);
                }
                InvertBox(0, top + (int)(burst % (height > 0 ? height : 1)), ANCHO_PANTALLA, 2);
            }
        }

        // Las palabras se van apilando como cartel: cortas en grande, largas mas chicas,
        // alternando a la izquierda y a la derecha. Si ya no caben, la pila sube
        void fxStack(const Cue &c, unsigned long lt, unsigned long dur)
        {
            uint8_t words = countWords(c.text);
            unsigned long each = dur * 8 / 10 / words;
            uint8_t shown = 1 + lt / each;
            shown = shown > words ? words : shown;
            // Alto de cada palabra con la fuente que le toca
            int heights[16];
            int total = 0;
            for (uint8_t k = 0; k < shown && k < 16; k++)
            {
                char word[48];
                wordAt(c.text, k, word, sizeof(word));
                Layout layout;
                layoutText(word, strlen(word) <= 5 ? FONT_BUBBLE : c.font, layout, 1);
                heights[k] = rowHeight();
                total += heights[k];
            }
            int y = total > ALTO_PANTALLA ? ALTO_PANTALLA - total : (ALTO_PANTALLA - total) / 2;
            for (uint8_t k = 0; k < shown && k < 16; k++)
            {
                char word[48];
                wordAt(c.text, k, word, sizeof(word));
                Layout layout;
                layoutText(word, strlen(word) <= 5 ? FONT_BUBBLE : c.font, layout, 1);
                const Line &line = layout.line[0];
                int cy = y + heights[k] / 2;
                int cx;
                switch (k % 3)
                {
                case 0:
                    cx = 4 + line.total / 2;
                    break;
                case 1:
                    cx = ANCHO_PANTALLA - 4 - line.total / 2;
                    break;
                default:
                    cx = ANCHO_PANTALLA / 2;
                    break;
                }
                drawLine(line, cx, cy, false);
                // La palabra nueva entra con un destello
                if (k == shown - 1 && lt - k * each < 90)
                {
                    InvertBox(cx - line.total / 2 - 2, y, line.total + 4, heights[k]);
                }
                y += heights[k];
            }
        }

        // A oscuras: una luz redonda recorre la frase y solo se ve lo que alumbra
        void fxSpotlight(const Cue &c, unsigned long lt, unsigned int beatMs)
        {
            Layout layout;
            layoutText(c.text, c.font, layout);
            drawLayout(layout, ANCHO_PANTALLA / 2, ALTO_PANTALLA / 2, false);
            int width = layout.line[0].total;
            if (layout.rows > 1 && layout.line[1].total > width)
            {
                width = layout.line[1].total;
            }
            // La luz va y viene sobre el texto y titila un poco con el ritmo
            float swing = width / 2.0f + 6;
            int lx = ANCHO_PANTALLA / 2 + (int)(swing * sinf(lt / 520.0f));
            int ly = ALTO_PANTALLA / 2 + (int)(9 * sinf(lt / 330.0f));
            int r = 19 + (((lt % beatMs) < 80) ? 3 : 0);
            for (int y = 0; y < ALTO_PANTALLA; y++)
            {
                int dy = y - ly;
                if (dy * dy >= r * r)
                {
                    ClearBox(0, y, ANCHO_PANTALLA, 1);
                    continue;
                }
                int half = (int)sqrtf((float)(r * r - dy * dy));
                clearSpan(0, lx - half, y);
                clearSpan(lx + half + 1, ANCHO_PANTALLA, y);
            }
            DrawCircle(lx, ly, r + 1);
        }

        // Animacion con la frase encima (si hay)
        void textOver(const Cue &c)
        {
            if (countWords(c.text) == 0)
            {
                return;
            }
            Layout layout;
            layoutText(c.text, c.font, layout);
            int h = layout.rows * rowHeight();
            int w = layout.line[0].total;
            if (layout.rows > 1 && layout.line[1].total > w)
            {
                w = layout.line[1].total;
            }
            ClearBox(ANCHO_PANTALLA / 2 - w / 2 - 3, ALTO_PANTALLA / 2 - h / 2 - 2, w + 6, h + 3);
            drawLayout(layout, ANCHO_PANTALLA / 2, ALTO_PANTALLA / 2, false);
        }

        // Titulo o final con murcielagos: el primer renglon grande y el segundo como subtitulo
        void fxBats(const Cue &c, unsigned long lt, unsigned long dur, unsigned long t, unsigned int beatMs)
        {
            drawBats(t, lt);
            if (countWords(c.text) > 0)
            {
                fxTitle(c, lt, dur);
            }
            // Un relampago (dos destellos) al empezar
            if (lt < 70 || (lt >= 160 && lt < 210))
            {
                InvertBox(0, 0, ANCHO_PANTALLA, ALTO_PANTALLA);
            }
            (void)beatMs;
        }

        unsigned long songLength(const Song &s)
        {
            const Cue &last = s.cues[s.cueCount - 1];
            return (unsigned long)(last.beat + last.beats) * s.beatMs;
        }
    }

    unsigned long musicStart()
    {
        song = &SONGS[songIndex];
        songIndex = (songIndex + 1) % SONG_COUNT;
        prepareTables();
        return songLength(*song);
    }

    void musicDraw(unsigned long t)
    {
        // Frase que toca ahora
        unsigned long beatMs = song->beatMs;
        uint8_t i = 0;
        while (i + 1 < song->cueCount && t >= (unsigned long)song->cues[i + 1].beat * beatMs)
        {
            i++;
        }
        const Cue &c = song->cues[i];
        unsigned long start = (unsigned long)c.beat * beatMs;
        unsigned long dur = (unsigned long)c.beats * beatMs;
        unsigned long lt = t - start;
        bool empty = countWords(c.text) == 0;

        // El remolino y el texto que pasa dejan estela: el cuadro anterior se va apagando
        // en vez de borrarse de golpe
        frame++;
        if (!empty && c.fx == FX_SPIN)
        {
            DitherDisplay(9, frame * 7);
        }
        else if (!empty && c.fx == FX_SCROLL)
        {
            DitherDisplay(11, frame * 7);
        }
        else
        {
            ClearDisplay();
        }

        switch (c.fx)
        {
        case FX_BATS:
            fxBats(c, lt, dur, t, beatMs);
            return;
        case FX_WARP:
            drawWarp(t, beatMs);
            textOver(c);
            return;
        case FX_MORPH:
            drawMorph(t, beatMs);
            textOver(c);
            return;
        case FX_HEADS:
            drawHeads(t);
            textOver(c);
            return;
        case FX_KALEIDO:
            fxKaleido(c, lt, t, beatMs);
            return;
        default:
            break;
        }

        // Los efectos que son solo texto, sin frase, se ven como parte instrumental
        if (empty)
        {
            drawInstrumental(visualOf(song->cues, song->cueCount, i), t, beatMs);
            return;
        }

        switch (c.fx)
        {
        case FX_TITLE:
            fxTitle(c, lt, dur);
            break;
        case FX_TYPE:
            fxType(c, lt, dur);
            break;
        case FX_POP:
            fxPop(c, lt, dur);
            break;
        case FX_WAVE:
            fxWave(c, lt, t);
            break;
        case FX_MELT:
            fxMelt(c, lt, dur);
            break;
        case FX_ZOOM:
            fxZoom(c, lt, dur, beatMs);
            break;
        case FX_SPIN:
            fxSpin(c, lt, beatMs);
            break;
        case FX_ECHO:
            fxEcho(c, lt, beatMs);
            break;
        case FX_SCROLL:
            fxScroll(c, lt, dur);
            break;
        case FX_SUN:
            fxSun(c, lt, beatMs);
            break;
        case FX_GLITCH:
            fxGlitch(c, lt, beatMs);
            break;
        case FX_STACK:
            fxStack(c, lt, dur);
            break;
        case FX_SPOTLIGHT:
            fxSpotlight(c, lt, beatMs);
            break;
        default:
            break;
        }
    }
}
