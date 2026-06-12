#pragma once
// Inicializa la pantalla OLED (llama a begin y limpia)
void InitDisplay();
// Limpia el buffer sin llamar display.display()
void ClearDisplay();
void SetMenuFont();
void DrawText(int x, int y, const char *text);
// Dibuja un bitmap completo del tamaño indicado y hace display()
void DrawBitmap(const unsigned char *bitmap, int w, int h);
// Conveniencia: dibuja el logo inicial
void DrawLogo();
// Conveniencia: dibuja gráfico de menú principal
void DrawMenu();
// Funcion para esperar con millis
bool wait(unsigned long durationMs);
void ActDisplay();
