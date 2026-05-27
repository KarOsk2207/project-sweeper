#include "raylib.h"

// ---------------------------------------------------------------------------
// Перечисление всех возможных экранов игры
typedef enum {
    SCREEN_MENU,
    SCREEN_DIFFICULTY,
    SCREEN_GAMEPLAY,
    SCREEN_MINIGAME,
    SCREEN_GAME_OVER,
    SCREEN_VICTORY
} GameScreen;

// ---------------------------------------------------------------------------
// Глобальная переменная: текущий экран
GameScreen currentScreen = SCREEN_MENU;

// ---------------------------------------------------------------------------
// Заглушки функций обновления и рисования для каждого экрана
// (позже мы заменим их полноценными реализациями)

void UpdateMenu(void) {
    // Временно: по нажатию Enter переходим к выбору сложности
    if (IsKeyPressed(KEY_ENTER)) currentScreen = SCREEN_DIFFICULTY;
}

void DrawMenu(void) {
    DrawText("MENU (press ENTER to start game)", 100, 100, 30, WHITE);
}

void UpdateDifficulty(void) {
    // Временно: по нажатию Enter переходим в игру
    if (IsKeyPressed(KEY_ENTER)) currentScreen = SCREEN_GAMEPLAY;
    if (IsKeyPressed(KEY_ESCAPE)) currentScreen = SCREEN_MENU;
}

void DrawDifficulty(void) {
    DrawText("DIFFICULTY SELECTION", 100, 100, 30, WHITE);
}

void UpdateGameplay(void) {
    // Временно: по нажатию Escape возвращаемся в меню
    if (IsKeyPressed(KEY_ESCAPE)) currentScreen = SCREEN_MENU;
}

void DrawGameplay(void) {
    DrawText("GAMEPLAY (press ESC to return)", 100, 100, 30, WHITE);
}

void UpdateMinigame(void) {
    if (IsKeyPressed(KEY_ESCAPE)) currentScreen = SCREEN_GAMEPLAY;
}

void DrawMinigame(void) {
    DrawText("MINIGAME", 100, 100, 30, WHITE);
}

void UpdateGameOver(void) {
    if (IsKeyPressed(KEY_ENTER)) currentScreen = SCREEN_MENU;
}

void DrawGameOver(void) {
    DrawText("GAME OVER (press ENTER to menu)", 100, 100, 30, RED);
}

void UpdateVictory(void) {
    if (IsKeyPressed(KEY_ENTER)) currentScreen = SCREEN_MENU;
}

void DrawVictory(void) {
    DrawText("VICTORY! (press ENTER to menu)", 100, 100, 30, GREEN);
}

// ---------------------------------------------------------------------------
// Главная функция
int main(void)
{
    InitWindow(800, 600, "Sweeper");

    while (!WindowShouldClose())
    {
        // --- Обновление логики текущего экрана ---
        switch (currentScreen) {
            case SCREEN_MENU:      UpdateMenu();      break;
            case SCREEN_DIFFICULTY:UpdateDifficulty(); break;
            case SCREEN_GAMEPLAY:  UpdateGameplay();   break;
            case SCREEN_MINIGAME:  UpdateMinigame();   break;
            case SCREEN_GAME_OVER: UpdateGameOver();   break;
            case SCREEN_VICTORY:   UpdateVictory();    break;
        }

        // --- Отрисовка ---
        BeginDrawing();
        ClearBackground(DARKGRAY);

        switch (currentScreen) {
            case SCREEN_MENU:      DrawMenu();      break;
            case SCREEN_DIFFICULTY:DrawDifficulty(); break;
            case SCREEN_GAMEPLAY:  DrawGameplay();   break;
            case SCREEN_MINIGAME:  DrawMinigame();   break;
            case SCREEN_GAME_OVER: DrawGameOver();   break;
            case SCREEN_VICTORY:   DrawVictory();    break;
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}