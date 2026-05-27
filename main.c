#include "raylib.h"

// ---------------------------------------------------------------------------
// Перечисление экранов
typedef enum {
    SCREEN_MENU,
    SCREEN_DIFFICULTY,
    SCREEN_GAMEPLAY,
    SCREEN_MINIGAME,
    SCREEN_GAME_OVER,
    SCREEN_VICTORY
} GameScreen;

// ---------------------------------------------------------------------------
// Структура для кнопки
typedef struct {
    Rectangle bounds;
    const char *text;
    Color color;
    Color hoverColor;
} Button;

// ---------------------------------------------------------------------------
// Параметры игры (заполняются при выборе сложности)
typedef struct {
    int rows;
    int cols;
    int mines;
} GameConfig;

// ---------------------------------------------------------------------------
// Глобальные переменные
GameScreen currentScreen = SCREEN_MENU;
GameConfig gameConfig;                     // текущие настройки сложности

// Кнопки главного меню
Button newGameBtn;
Button exitBtn;

// Кнопки меню выбора сложности
Button easyBtn;
Button mediumBtn;
Button hardBtn;
Button backBtn;

// ---------------------------------------------------------------------------
// Инициализация кнопок меню
void InitMenuButtons(void) {
    float btnWidth = 200;
    float btnHeight = 50;
    float centerX = (800 - btnWidth) / 2.0f;

    newGameBtn.bounds = (Rectangle){ centerX, 250, btnWidth, btnHeight };
    newGameBtn.text = "New Game";
    newGameBtn.color = DARKGREEN;
    newGameBtn.hoverColor = GREEN;

    exitBtn.bounds = (Rectangle){ centerX, 330, btnWidth, btnHeight };
    exitBtn.text = "Exit";
    exitBtn.color = DARKGRAY;
    exitBtn.hoverColor = GRAY;
}

// ---------------------------------------------------------------------------
// Инициализация кнопок выбора сложности
void InitDifficultyButtons(void) {
    float btnWidth = 220;
    float btnHeight = 50;
    float centerX = (800 - btnWidth) / 2.0f;

    easyBtn.bounds = (Rectangle){ centerX, 200, btnWidth, btnHeight };
    easyBtn.text = "Easy (9x9, 10 mines)";
    easyBtn.color = DARKGREEN;
    easyBtn.hoverColor = GREEN;

    mediumBtn.bounds = (Rectangle){ centerX, 270, btnWidth, btnHeight };
    mediumBtn.text = "Medium (16x16, 40 mines)";
    mediumBtn.color = DARKBLUE;
    mediumBtn.hoverColor = BLUE;

    hardBtn.bounds = (Rectangle){ centerX, 340, btnWidth, btnHeight };
    hardBtn.text = "Hard (30x16, 99 mines)";
    hardBtn.color = DARKPURPLE;
    hardBtn.hoverColor = PURPLE;

    backBtn.bounds = (Rectangle){ centerX, 420, btnWidth, btnHeight };
    backBtn.text = "Back";
    backBtn.color = DARKGRAY;
    backBtn.hoverColor = GRAY;
}

// ---------------------------------------------------------------------------
// Вспомогательные функции для кнопок
bool IsMouseOverButton(Button btn) {
    Vector2 mouse = GetMousePosition();
    return CheckCollisionPointRec(mouse, btn.bounds);
}

void DrawButton(Button btn) {
    Color currentColor = IsMouseOverButton(btn) ? btn.hoverColor : btn.color;
    DrawRectangleRec(btn.bounds, currentColor);
    DrawRectangleLinesEx(btn.bounds, 2, WHITE);

    int textWidth = MeasureText(btn.text, 20);
    float textX = btn.bounds.x + (btn.bounds.width - textWidth) / 2.0f;
    float textY = btn.bounds.y + (btn.bounds.height - 20) / 2.0f;
    DrawText(btn.text, (int)textX, (int)textY, 20, WHITE);
}

// ---------------------------------------------------------------------------
// Обновление и отрисовка главного меню
void UpdateMenu(void) {
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        if (IsMouseOverButton(newGameBtn)) {
            currentScreen = SCREEN_DIFFICULTY;
        }
        if (IsMouseOverButton(exitBtn)) {
            CloseWindow();
        }
    }
}

void DrawMenu(void) {
    DrawText("SWEEPER", 300, 100, 50, WHITE);
    DrawButton(newGameBtn);
    DrawButton(exitBtn);
}

// ---------------------------------------------------------------------------
// Обновление и отрисовка меню выбора сложности
void UpdateDifficulty(void) {
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        if (IsMouseOverButton(easyBtn)) {
            gameConfig.rows = 9;
            gameConfig.cols = 9;
            gameConfig.mines = 10;
            currentScreen = SCREEN_GAMEPLAY;
        }
        if (IsMouseOverButton(mediumBtn)) {
            gameConfig.rows = 16;
            gameConfig.cols = 16;
            gameConfig.mines = 40;
            currentScreen = SCREEN_GAMEPLAY;
        }
        if (IsMouseOverButton(hardBtn)) {
            gameConfig.rows = 16;
            gameConfig.cols = 30;
            gameConfig.mines = 99;
            currentScreen = SCREEN_GAMEPLAY;
        }
        if (IsMouseOverButton(backBtn)) {
            currentScreen = SCREEN_MENU;
        }
    }
}

void DrawDifficulty(void) {
    DrawText("SELECT DIFFICULTY", 230, 100, 40, WHITE);
    DrawButton(easyBtn);
    DrawButton(mediumBtn);
    DrawButton(hardBtn);
    DrawButton(backBtn);
}

// ---------------------------------------------------------------------------
// Заглушки для остальных экранов
void UpdateGameplay(void) {
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

    InitMenuButtons();
    InitDifficultyButtons();

    while (!WindowShouldClose())
    {
        // --- Обновление ---
        switch (currentScreen) {
            case SCREEN_MENU:       UpdateMenu();       break;
            case SCREEN_DIFFICULTY: UpdateDifficulty(); break;
            case SCREEN_GAMEPLAY:   UpdateGameplay();   break;
            case SCREEN_MINIGAME:   UpdateMinigame();   break;
            case SCREEN_GAME_OVER:  UpdateGameOver();   break;
            case SCREEN_VICTORY:    UpdateVictory();    break;
        }

        // --- Отрисовка ---
        BeginDrawing();
        ClearBackground(DARKGRAY);

        switch (currentScreen) {
            case SCREEN_MENU:       DrawMenu();       break;
            case SCREEN_DIFFICULTY: DrawDifficulty(); break;
            case SCREEN_GAMEPLAY:   DrawGameplay();   break;
            case SCREEN_MINIGAME:   DrawMinigame();   break;
            case SCREEN_GAME_OVER:  DrawGameOver();   break;
            case SCREEN_VICTORY:    DrawVictory();    break;
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}