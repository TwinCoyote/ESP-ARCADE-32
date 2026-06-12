#ifdef ARDUINO
#include <Arduino.h>
#else
#include <iostream>
#include <cstdio>
#endif
#include <unity.h>
#include "../src/drivers/display/display.h"
#include "../src/drivers/input/buttons.h"
#include "../src/ui/menu.cpp"

#ifdef ARDUINO
#else
struct DummySerial
{
    void begin(int) {}
    void println(const char *s) { std::cout << s << std::endl; }
    void println(int v) { std::cout << v << std::endl; }
    void print(const char *s) { std::cout << s; }
    void print(int v) { std::cout << v; }
} Serial;
#endif

#ifdef ARDUINO
void renderCurrentMenu()
{
    MenuRender();
    delay(200);
}
#else
void renderCurrentMenu() {}
#endif

static bool pressedRight = false;
static bool pressedLeft = false;

// Esta implementación se usa solo en el test para simular los botones.
bool isPressed(int pin)
{
    if (pin == BTN_RIGHT)
    {
        return pressedRight;
    }
    if (pin == BTN_LEFT)
    {
        return pressedLeft;
    }
    return false;
}

void test_MenuUpdate_increments_when_right_pressed()
{
    indexMenu = 0;
    pressedRight = true;
    MenuUpdate();
    renderCurrentMenu();
    TEST_ASSERT_EQUAL_INT(1, indexMenu);
}

void test_MenuUpdate_does_not_change_when_right_not_pressed()
{
    indexMenu = 0;
    pressedRight = false;
    MenuUpdate();
    renderCurrentMenu();
    TEST_ASSERT_EQUAL_INT(0, indexMenu);
}

void test_MenuUpdate_increments_six_times_with_six_presses()
{
    indexMenu = 0;
    pressedRight = true;
    Serial.println("Simulando 6 pulsaciones de BTN_RIGHT...");
    for (int i = 0; i < 6; i++)
    {
        MenuUpdate();
        renderCurrentMenu();
        Serial.print("Conteo tras pulsacion ");
        Serial.print(i + 1);
        Serial.print(": ");
        Serial.println(indexMenu);
    }
    TEST_ASSERT_EQUAL_INT(2, indexMenu);
}

void test_MenuUpdate_wraps_to_zero_after_last_option()
{
    indexMenu = totalOptions - 1;
    pressedRight = true;
    MenuUpdate();
    renderCurrentMenu();
    TEST_ASSERT_EQUAL_INT(0, indexMenu);
}

void test_MenuUpdate_decrements_six_times_with_six_presses()
{
    indexMenu = 0;
    pressedRight = false;
    pressedLeft = true;
    Serial.println("Simulando 6 pulsaciones de BTN_LEFT...");
    for (int i = 0; i < 6; i++)
    {
        MenuUpdate();
        renderCurrentMenu();
        Serial.print("Conteo tras pulsacion ");
        Serial.print(i + 1);
        Serial.print(": ");
        Serial.println(indexMenu);
    }
    TEST_ASSERT_EQUAL_INT(2, indexMenu);
}

#ifndef ARDUINO
void setUp(void) {}
void tearDown(void) {}

void SetMenuFont() {}
void DrawText(int x, int y, const char *text)
{
    (void)x;
    (void)y;
    (void)text;
}
void SendBuffer() {}

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    UNITY_BEGIN();
    RUN_TEST(test_MenuUpdate_increments_when_right_pressed);
    RUN_TEST(test_MenuUpdate_does_not_change_when_right_not_pressed);
    RUN_TEST(test_MenuUpdate_increments_six_times_with_six_presses);
    RUN_TEST(test_MenuUpdate_wraps_to_zero_after_last_option);
    RUN_TEST(test_MenuUpdate_decrements_six_times_with_six_presses);
    return UNITY_END();
}
#else
void setup()
{
    Serial.begin(115200);
    InitDisplay();
    UNITY_BEGIN();
    RUN_TEST(test_MenuUpdate_increments_when_right_pressed);
    RUN_TEST(test_MenuUpdate_does_not_change_when_right_not_pressed);
    RUN_TEST(test_MenuUpdate_increments_six_times_with_six_presses);
    RUN_TEST(test_MenuUpdate_wraps_to_zero_after_last_option);
    RUN_TEST(test_MenuUpdate_decrements_six_times_with_six_presses);
    UNITY_END();
}

void loop() {}
#endif
