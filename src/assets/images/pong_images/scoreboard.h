#pragma once

/**
 * @brief Draws a modal scoreboard popup in the center of the screen
 * Shows format: "USER_SCORE - AI_SCORE"
 * @param user_points Points scored by the user/player
 * @param ai_points Points scored by the AI opponent
 */
void draw_scoreboard_modal(int user_points, int ai_points);

/**
 * @brief Updates and handles the scoreboard display timing
 * Call this every frame to manage when the scoreboard shows/hides
 * @return true if scoreboard should be visible, false otherwise
 */
bool update_scoreboard_display();

/**
 * @brief Triggers the scoreboard display (call when a point is scored)
 */
void trigger_scoreboard();
