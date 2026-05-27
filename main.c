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
// Структура для кнопки (прямоугольник + текст)
typedef struct {
    Rectangle bounds;
    const char *text;
    Color color;
    Color hoverColor;
} Button;

// ---------------------------------------------------------------------------
// Глобальная переменная: текущий экран
GameScreen currentScreen = SCREEN_MENU;

// ---------------------------------------------------------------------------
// Кнопки главного меню
Button newGameBtn;
Button exitBtn;

// ---------------------------------------------------------------------------
// Инициализация кнопок меню (вызывается один раз при запуске)
void InitMenuButtons(void) {
    float btnWidth = 200;
    float btnHeight = 50;
    float centerX = (800 - btnWidth) / 2.0f;   // ширина окна 800

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
// Вспомогательная функция: проверка наведения мыши на кнопку
bool IsMouseOverButton(Button btn) {
    Vector2 mouse = GetMousePosition();
    return CheckCollisionPointRec(mouse, btn.bounds);
}

// ---------------------------------------------------------------------------
// Отрисовка одной кнопки
void DrawButton(Button btn) {
    Color currentColor = IsMouseOverButton(btn) ? btn.hoverColor : btn.color;
    DrawRectangleRec(btn.bounds, currentColor);
    DrawRectangleLinesEx(btn.bounds, 2, WHITE);

    // Центрируем текст внутри кнопки
    int textWidth = MeasureText(btn.text, 20);
    float textX = btn.bounds.x + (btn.bounds.width - textWidth) / 2.0f;
    float textY = btn.bounds.y + (btn.bounds.height - 20) / 2.0f;
    DrawText(btn.text, (int)textX, (int)textY, 20, WHITE);
}

// ---------------------------------------------------------------------------
// Обновление логики главного меню
void UpdateMenu(void) {
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        if (IsMouseOverButton(newGameBtn)) {
            currentScreen = SCREEN_DIFFICULTY;
        }
        if (IsMouseOverButton(exitBtn)) {
            // Закрываем окно – эквивалент нажатия на крестик
            CloseWindow();
            // Чтобы главный цикл остановился, выставим флаг
            // WindowShouldClose() вернёт true, поэтому просто выйдем
        }
    }
}

// ---------------------------------------------------------------------------
// Отрисовка главного меню
void DrawMenu(void) {
    DrawText("SWEEPER", 300, 100, 50, WHITE);
    DrawButton(newGameBtn);
    DrawButton(exitBtn);
}

// ---------------------------------------------------------------------------
// Заглушки для остальных экранов (без изменений)
void UpdateDifficulty(void) {
    if (IsKeyPressed(KEY_ENTER)) currentScreen = SCREEN_GAMEPLAY;
    if (IsKeyPressed(KEY_ESCAPE)) currentScreen = SCREEN_MENU;
}

void DrawDifficulty(void) {
    DrawText("DIFFICULTY SELECTION", 100, 100, 30, WHITE);
}

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

    // Инициализация кнопок меню
    InitMenuButtons();

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