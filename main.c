#include "raylib.h"
#include "board.h"
#include <stdio.h>

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
// Параметры игры
typedef struct {
    int rows;
    int cols;
    int mines;
} GameConfig;

// ---------------------------------------------------------------------------
// Глобальные переменные
GameScreen currentScreen = SCREEN_MENU;
GameConfig gameConfig;
Board gameBoard;
bool gameLost = false;
bool gameWon  = false;

// Счётчик флагов и таймер
int flagCount = 0;          // количество установленных флагов
int timer = 0;              // прошедшее время в секундах
double startTime = 0.0;    // время начала игры (в секундах от GetTime())
bool timerStarted = false;

// Кнопки меню
Button newGameBtn;
Button exitBtn;

// Кнопки сложности
Button easyBtn;
Button mediumBtn;
Button hardBtn;
Button backBtn;

// Кнопка-смайлик (рестарт)
Rectangle smileyRect;

// ---------------------------------------------------------------------------
// Константы отрисовки
#define CELL_SIZE  30
#define GRID_X     50
#define GRID_Y     80   // сдвинули вниз, чтобы освободить место под таймер

// ---------------------------------------------------------------------------
// Сброс игры (остаёмся на той же сложности)
void ResetGame(void) {
    BoardInit(&gameBoard, gameConfig.rows, gameConfig.cols, gameConfig.mines);
    gameLost = false;
    gameWon = false;
    flagCount = 0;
    timer = 0;
    timerStarted = false;
    startTime = 0.0;
}

// ---------------------------------------------------------------------------
// Инициализация кнопок меню
void InitMenuButtons(void) {
    float btnWidth = 200, btnHeight = 50;
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

void InitDifficultyButtons(void) {
    float btnWidth = 220, btnHeight = 50;
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
// Вспомогательные функции кнопок
bool IsMouseOverButton(Button btn) {
    return CheckCollisionPointRec(GetMousePosition(), btn.bounds);
}

void DrawButton(Button btn) {
    Color color = IsMouseOverButton(btn) ? btn.hoverColor : btn.color;
    DrawRectangleRec(btn.bounds, color);
    DrawRectangleLinesEx(btn.bounds, 2, WHITE);

    int textWidth = MeasureText(btn.text, 20);
    float textX = btn.bounds.x + (btn.bounds.width - textWidth) / 2.0f;
    float textY = btn.bounds.y + (btn.bounds.height - 20) / 2.0f;
    DrawText(btn.text, (int)textX, (int)textY, 20, WHITE);
}

// ---------------------------------------------------------------------------
// Главное меню
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
// Меню выбора сложности
void UpdateDifficulty(void) {
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        if (IsMouseOverButton(easyBtn)) {
            gameConfig.rows = 9; gameConfig.cols = 9; gameConfig.mines = 10;
            ResetGame();
            currentScreen = SCREEN_GAMEPLAY;
        }
        if (IsMouseOverButton(mediumBtn)) {
            gameConfig.rows = 16; gameConfig.cols = 16; gameConfig.mines = 40;
            ResetGame();
            currentScreen = SCREEN_GAMEPLAY;
        }
        if (IsMouseOverButton(hardBtn)) {
            gameConfig.rows = 16; gameConfig.cols = 30; gameConfig.mines = 99;
            ResetGame();
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
// Игровой процесс (Сапёр)
void UpdateGameplay(void) {
    if (gameLost || gameWon) {
        if (IsKeyPressed(KEY_ENTER)) currentScreen = SCREEN_MENU;
        return;
    }

    if (IsKeyPressed(KEY_ESCAPE)) currentScreen = SCREEN_MENU;

    // Рестарт по клику на смайлик
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        Vector2 mouse = GetMousePosition();
        if (CheckCollisionPointRec(mouse, smileyRect)) {
            ResetGame();
            return;
        }
    }

    // Клики по клеткам
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) || IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) {
        Vector2 mouse = GetMousePosition();
        int col = (int)((mouse.x - GRID_X) / CELL_SIZE);
        int row = (int)((mouse.y - GRID_Y) / CELL_SIZE);

        if (row >= 0 && row < gameBoard.rows && col >= 0 && col < gameBoard.cols) {
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                // Первый клик запускает таймер и размещает мины
                if (!gameBoard.firstClickDone) {
                    BoardPlaceMines(&gameBoard, row, col);
                    BoardCalculateNumbers(&gameBoard);
                    timerStarted = true;
                    startTime = GetTime();
                }
                bool hitMine = BoardReveal(&gameBoard, row, col);
                if (hitMine) {
                    gameLost = true;
                } else {
                    gameWon = BoardCheckVictory(&gameBoard);
                }
            } else if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) {
                // Флаг: если уже запущен таймер
                if (!gameBoard.firstClickDone) return; // нельзя ставить флаг до первого клика? На самом деле можно, но мин ещё нет. Разрешим для удобства.
                // Переключение флага и обновление счётчика
                Cell* cell = &gameBoard.cells[row][col];
                if (cell->state == CELL_HIDDEN) {
                    cell->state = CELL_FLAGGED;
                    flagCount++;
                } else if (cell->state == CELL_FLAGGED) {
                    cell->state = CELL_HIDDEN;
                    flagCount--;
                }
            }
        }
    }

    // Обновление таймера
    if (timerStarted && !gameLost && !gameWon) {
        double elapsed = GetTime() - startTime;
        timer = (int)elapsed;
        // Ограничим 999 секундами, чтобы не ломать отображение
        if (timer > 999) timer = 999;
    }
}

void DrawGameplay(void) {
    // --- Верхняя панель ---
    // Счётчик мин
    int minesLeft = gameConfig.mines - flagCount;
    if (minesLeft < 0) minesLeft = 0;
    char mineStr[16];
    sprintf(mineStr, "Mines: %d", minesLeft);
    DrawText(mineStr, 20, 10, 24, WHITE);

    // Таймер
    char timeStr[16];
    sprintf(timeStr, "Time: %d", timer);
    DrawText(timeStr, 150, 10, 24, WHITE);

    // Смайлик (кнопка рестарта)
    smileyRect = (Rectangle){ 350, 5, 40, 40 };
    Color smileyColor = gameLost ? RED : gameWon ? GREEN : YELLOW;
    DrawRectangleRec(smileyRect, smileyColor);
    DrawRectangleLinesEx(smileyRect, 2, WHITE);
    DrawText(":)", (int)(smileyRect.x + 8), (int)(smileyRect.y + 6), 24, BLACK);

    // --- Игровое поле ---
    for (int r = 0; r < gameBoard.rows; r++) {
        for (int c = 0; c < gameBoard.cols; c++) {
            Rectangle cellRect = {
                GRID_X + c * CELL_SIZE,
                GRID_Y + r * CELL_SIZE,
                CELL_SIZE, CELL_SIZE
            };

            Cell cell = gameBoard.cells[r][c];

            if (cell.state == CELL_HIDDEN) {
                DrawRectangleRec(cellRect, LIGHTGRAY);
                DrawRectangleLines((int)cellRect.x, (int)cellRect.y, CELL_SIZE, CELL_SIZE, GRAY);
            } else if (cell.state == CELL_REVEALED) {
                DrawRectangleRec(cellRect, WHITE);
                DrawRectangleLines((int)cellRect.x, (int)cellRect.y, CELL_SIZE, CELL_SIZE, GRAY);
                if (cell.hasMine) {
                    DrawText("*", (int)(cellRect.x + 8), (int)(cellRect.y + 5), 24, RED);
                } else if (cell.adjacentMines > 0) {
                    char num[4];
                    sprintf(num, "%d", cell.adjacentMines);
                    DrawText(num, (int)(cellRect.x + 8), (int)(cellRect.y + 5), 24, DARKBLUE);
                }
            } else if (cell.state == CELL_FLAGGED) {
                DrawRectangleRec(cellRect, LIGHTGRAY);
                DrawRectangleLines((int)cellRect.x, (int)cellRect.y, CELL_SIZE, CELL_SIZE, GRAY);
                DrawText("F", (int)(cellRect.x + 8), (int)(cellRect.y + 5), 24, RED);
            }
        }
    }

    // Сообщения о победе/поражении
    if (gameLost) {
        DrawText("YOU LOST! (ENTER to menu)", 200, 45, 30, RED);
    } else if (gameWon) {
        DrawText("YOU WIN! (ENTER to menu)", 200, 45, 30, GREEN);
    }
}

// ---------------------------------------------------------------------------
// Заглушки для мини-игр и финалов
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
        // Обновление текущего экрана
        switch (currentScreen) {
            case SCREEN_MENU:       UpdateMenu();       break;
            case SCREEN_DIFFICULTY: UpdateDifficulty(); break;
            case SCREEN_GAMEPLAY:   UpdateGameplay();   break;
            case SCREEN_MINIGAME:   UpdateMinigame();   break;
            case SCREEN_GAME_OVER:  UpdateGameOver();   break;
            case SCREEN_VICTORY:    UpdateVictory();    break;
        }

        // Отрисовка
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