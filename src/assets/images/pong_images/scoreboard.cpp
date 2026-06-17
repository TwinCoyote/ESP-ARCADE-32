#include <Arduino.h>
#include "scoreboard.h"
// #include "../../drivers/display/display.h"
#include "drivers/display/display.h"

// Control variables for scoreboard display timing
static bool scoreboardActive = false;
static unsigned long scoreboardStartTime = 0;
static const unsigned long SCOREBOARD_DISPLAY_TIME = 2000; // Show for 2 seconds

/**
 * @brief Triggers the scoreboard display
 * Call this when a point is scored
 */
void trigger_scoreboard()
{
    scoreboardActive = true;
    scoreboardStartTime = millis();
}

/**
 * @brief Updates and handles the scoreboard display timing
 * Returns true if the scoreboard should be visible
 */
bool update_scoreboard_display()
{
    if (!scoreboardActive)
        return false;

    unsigned long elapsed = millis() - scoreboardStartTime;
    if (elapsed > SCOREBOARD_DISPLAY_TIME)
    {
        scoreboardActive = false;
        return false;
    }
    return true;
}

/**
 * @brief Draws a modal scoreboard popup centered in the screen
 * Format: "USER_SCORE - AI_SCORE"
 * Only draws if the scoreboard is currently active
 * @param user_points Points scored by the user/player
 * @param ai_points Points scored by the AI opponent
 */
void draw_scoreboard_modal(int user_points, int ai_points)
{
    if (!update_scoreboard_display())
        return;

    // Draw semi-transparent background box (actually opaque for visibility)
    // Box centered at screen center (128x64)
    // Center box: ~50 pixels wide, ~24 pixels tall
    const int box_width = 50;
    const int box_height = 24;
    const int box_x = (128 - box_width) / 2; // Center X
    const int box_y = (64 - box_height) / 2; // Center Y

    // Draw border box
    // DrawBox(box_x - 1, box_y - 1, box_width + 2, box_height + 2);

    // Convert scores to string format "X - Y"
    char score_str[10];
    sprintf(score_str, "%d - %d", user_points, ai_points);

    // Set larger font for visibility
    SetCustomFont(FONT_MEDIUM);

    // Draw the score in the center
    DrawText(box_x + 5, box_y + 16, score_str);
}
