#include "info.h"
#include <WiFi.h>
#include "../../../drivers/display/display.h"
#include "../../../drivers/input/buttons.h"
#include "../../../core0/services/ota/OTA.h"
#include "../../../assets/images/info_images/info_pages.h"
#include "../../../assets/images/info_images/easter_egg.h"
#include "../../../config/debug_log.h"

namespace
{
    // Valores que regresa Input::realDirection()
    const uint8_t DIR_NONE = 0;
    const uint8_t DIR_UP = 1;
    const uint8_t DIR_DOWN = 2;
    const uint8_t DIR_RIGHT = 3;
    const uint8_t DIR_LEFT = 4;
    const uint8_t DIR_OK = 5;
    const uint8_t DIR_BACK = 6;

    // Easter egg: codigo Konami, con B = BACK y A = OK
    const uint8_t KONAMI[] = {DIR_UP, DIR_UP, DIR_DOWN, DIR_DOWN, DIR_LEFT, DIR_RIGHT, DIR_LEFT, DIR_RIGHT, DIR_BACK, DIR_OK};

    enum Page
    {
        PAGE_CONSOLE,
        PAGE_NETWORK,
        PAGE_CREDITS,
        PAGE_COUNT
    };
    const unsigned char *const PAGE_BITMAPS[PAGE_COUNT] = {InfoConsoleBitmap, InfoNetworkBitmap, InfoCreditsBitmap};

    // Linea base de cada fila y ancho maximo de su valor para no tapar la etiqueta
    const int ROW_BASELINE[] = {20, 29, 38, 47, 56};
    const int VALUE_RIGHT = 126;
    const int CONSOLE_MAX_W[] = {86, 101, 106, 76, 76};
    const int NETWORK_MAX_W[] = {91, 106, 111, 96, 106};

    const unsigned long DEBOUNCE_MS = 80;
    const unsigned long PAGE_REFRESH_MS = 500;
    const unsigned long SECRET_FRAME_MS = 100;
}

InfoMenu::InfoMenu()
    : page(PAGE_CONSOLE),
      secretVisible(false),
      leaveOnRelease(false),
      needsRedraw(true),
      lastInput(DIR_NONE),
      lastPressMs(0),
      lastDrawMs(0),
      secretStartMs(0)
{
    memset(history, 0, sizeof(history));
}

void InfoMenu::reset()
{
    page = PAGE_CONSOLE;
    secretVisible = false;
    leaveOnRelease = false;
    needsRedraw = true;
    lastInput = input.realDirection(); // el OK con el que se entro todavia esta presionado
    lastPressMs = 0;
    memset(history, 0, sizeof(history));
    version = OTAService::readVersion(); // se lee de la NVS una sola vez al entrar
}

bool InfoMenu::update()
{
    int dir = input.realDirection();

    // BACK actua al soltarlo, asi no cuenta otra vez como BACK en el menu de configuracion
    if (leaveOnRelease)
    {
        if (dir == DIR_BACK)
        {
            return false;
        }
        leaveOnRelease = false;
        lastInput = dir;
        if (secretVisible)
        {
            secretVisible = false; // del easter egg se regresa a Info
            needsRedraw = true;
            return false;
        }
        return true;
    }

    if (dir == lastInput)
    {
        return false;
    }
    lastInput = dir;

    unsigned long now = millis();
    if (dir == DIR_NONE || now - lastPressMs < DEBOUNCE_MS)
    {
        return false;
    }
    lastPressMs = now;

    if (secretVisible)
    {
        if (dir == DIR_BACK)
        {
            leaveOnRelease = true;
        }
        return false;
    }

    pushHistory(dir);

    switch (dir)
    {
    case DIR_LEFT:
        page = (page + PAGE_COUNT - 1) % PAGE_COUNT;
        needsRedraw = true;
        break;
    case DIR_RIGHT:
        page = (page + 1) % PAGE_COUNT;
        needsRedraw = true;
        break;
    case DIR_BACK:
        // Si es la B del codigo no sale de Info
        if (!codeTyped(sizeof(KONAMI) - 1))
        {
            leaveOnRelease = true;
        }
        break;
    case DIR_OK:
        if (codeTyped(sizeof(KONAMI)))
        {
            DEV_PRINTLN("[Info] Easter egg desbloqueado");
            secretVisible = true;
            secretStartMs = now;
            needsRedraw = true;
            memset(history, 0, sizeof(history));
        }
        break;
    default:
        break;
    }
    return false;
}

void InfoMenu::render()
{
    unsigned long now = millis();
    unsigned long interval = secretVisible ? SECRET_FRAME_MS : PAGE_REFRESH_MS;
    if (!needsRedraw && now - lastDrawMs < interval)
    {
        return;
    }
    needsRedraw = false;
    lastDrawMs = now;

    if (secretVisible)
    {
        renderSecret();
    }
    else
    {
        renderPage();
    }
}

void InfoMenu::pushHistory(uint8_t button)
{
    static_assert(sizeof(KONAMI) == sizeof(history), "history debe guardar el codigo completo");
    memmove(history, history + 1, sizeof(history) - 1);
    history[sizeof(history) - 1] = button;
}

// true si las ultimas 'count' teclas son el inicio del codigo Konami
bool InfoMenu::codeTyped(uint8_t count) const
{
    for (uint8_t i = 0; i < count; i++)
    {
        if (history[sizeof(history) - count + i] != KONAMI[i])
        {
            return false;
        }
    }
    return true;
}

void InfoMenu::drawValue(uint8_t row, const char *text, int maxWidth)
{
    char buf[32];
    strncpy(buf, text, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

    // Si no cabe (por ejemplo un SSID largo) se recorta y termina en ".."
    size_t len = strlen(buf);
    while (len > 2 && TextWidth(buf) > maxWidth)
    {
        len--;
        buf[len] = '\0';
        buf[len - 1] = '.';
        buf[len - 2] = '.';
    }
    DrawText(VALUE_RIGHT - TextWidth(buf), ROW_BASELINE[row], buf);
}

void InfoMenu::renderPage()
{
    char buf[32];
    ClearDisplay();
    DrawImage(0, 0, infoPageWidth, infoPageHeight, PAGE_BITMAPS[page]);
    SetCustomFont(FONT_TINY);

    if (page == PAGE_CONSOLE)
    {
        drawValue(0, version.length() > 0 ? version.c_str() : "-", CONSOLE_MAX_W[0]);
        drawValue(1, ESP.getChipModel(), CONSOLE_MAX_W[1]);
        snprintf(buf, sizeof(buf), "%u x %lu MHz", (unsigned)ESP.getChipCores(), (unsigned long)ESP.getCpuFreqMHz());
        drawValue(2, buf, CONSOLE_MAX_W[2]);
        snprintf(buf, sizeof(buf), "%lu KB", (unsigned long)(ESP.getFreeHeap() / 1024));
        drawValue(3, buf, CONSOLE_MAX_W[3]);
        unsigned long s = millis() / 1000;
        snprintf(buf, sizeof(buf), "%02lu:%02lu:%02lu", s / 3600, (s / 60) % 60, s % 60);
        drawValue(4, buf, CONSOLE_MAX_W[4]);
    }
    else if (page == PAGE_NETWORK)
    {
        bool connected = WiFi.status() == WL_CONNECTED;
        drawValue(0, connected ? "Conectado" : "Sin conexion", NETWORK_MAX_W[0]);
        if (connected)
        {
            drawValue(1, WiFi.SSID().c_str(), NETWORK_MAX_W[1]);
            drawValue(2, WiFi.localIP().toString().c_str(), NETWORK_MAX_W[2]);
            snprintf(buf, sizeof(buf), "%d dBm", (int)WiFi.RSSI());
            drawValue(3, buf, NETWORK_MAX_W[3]);
        }
        else
        {
            drawValue(1, "-", NETWORK_MAX_W[1]);
            drawValue(2, "-", NETWORK_MAX_W[2]);
            drawValue(3, "-", NETWORK_MAX_W[3]);
        }
        drawValue(4, WiFi.macAddress().c_str(), NETWORK_MAX_W[4]);
    }
    // La pagina de creditos es solo la imagen

    ActDisplay();
}

void InfoMenu::renderSecret()
{
    static const char HOWL[] = "AUUUU!";
    static const uint8_t TWINKLE[][2] = {{24, 7}, {104, 9}, {6, 14}, {121, 18}, {40, 12}, {88, 14}};
    unsigned long t = millis() - secretStartMs;

    ClearDisplay();
    DrawImage(0, 0, easterEggWidth, easterEggHeight, EasterEggBitmap);

    // El aullido crece letra por letra, se queda un momento y luego descansa
    unsigned long phase = t % 3200;
    size_t shown = 0;
    if (phase < 1800)
    {
        shown = phase / 300 + 1;
    }
    else if (phase < 2600)
    {
        shown = sizeof(HOWL) - 1;
    }
    char buf[sizeof(HOWL)];
    memcpy(buf, HOWL, shown);
    buf[shown] = '\0';
    SetCustomFont(FONT_TINY);
    DrawText((ANCHO_PANTALLA - TextWidth(HOWL)) / 2, 9, buf);

    // Estrellas que titilan
    for (uint8_t i = 0; i < sizeof(TWINKLE) / sizeof(TWINKLE[0]); i++)
    {
        if ((t / 350 + i) % 3 == 0)
        {
            DrawBox(TWINKLE[i][0], TWINKLE[i][1], 1, 1);
        }
    }

    ActDisplay();
}
