#include "raylib.h"
#include "board.h"
#include <stdio.h>

// ---------------------------------------------------------------------------
typedef enum {
    SCREEN_MENU,
    SCREEN_DIFFICULTY,
    SCREEN_GAMEPLAY,
    SCREEN_MINIGAME,
    SCREEN_GAME_OVER,
    SCREEN_VICTORY,
    SCREEN_SETTINGS
} GameScreen;

typedef struct {
    Rectangle bounds;
    const char *text;
    Color color;
    Color hoverColor;
} Button;

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

int flagCount = 0;
int timer = 0;
double startTime = 0.0;
bool timerStarted = false;

// Кастомизация
Color bgColor = DARKGRAY;       // цвет фона игры
Color cellColor = LIGHTGRAY;    // цвет скрытых клеток
Color cellHiddenLine = GRAY;    // цвет линий скрытых клеток

// Кнопки меню
Button newGameBtn;
Button settingsBtn;
Button exitBtn;

// Кнопки сложности
Button easyBtn;
Button mediumBtn;
Button hardBtn;
Button backBtn;

// Кнопки настроек
Button backFromSettingsBtn;
Button bgColorBtns[4];
Button cellColorBtns[4];

Rectangle smileyRect;

// Предопределённые цвета для выбора
Color bgColorOptions[4] = {
    { 80, 80, 80, 255 },   // Dark Gray
    { 20, 20, 20, 255 },   // Almost Black
    { 10, 10, 50, 255 },   // Dark Navy
    { 10, 50, 10, 255 }    // Dark Green
};

Color cellColorOptions[4] = {
    { 200, 200, 200, 255 }, // Light Gray
    { 255, 255, 255, 255 }, // White
    { 180, 200, 255, 255 }, // Light Blue
    { 180, 255, 180, 255 }  // Light Green
};

#define CELL_SIZE  30
#define GRID_X     50
#define GRID_Y     80

// ---------------------------------------------------------------------------
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
void InitMenuButtons(void) {
    float btnWidth = 200, btnHeight = 50;
    float centerX = (800 - btnWidth) / 2.0f;

    newGameBtn.bounds = (Rectangle){ centerX, 200, btnWidth, btnHeight };
    newGameBtn.text = "New Game";
    newGameBtn.color = DARKGREEN;
    newGameBtn.hoverColor = GREEN;

    settingsBtn.bounds = (Rectangle){ centerX, 270, btnWidth, btnHeight };
    settingsBtn.text = "Settings";
    settingsBtn.color = DARKBLUE;
    settingsBtn.hoverColor = BLUE;

    exitBtn.bounds = (Rectangle){ centerX, 340, btnWidth, btnHeight };
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

    // Кнопка Back на экране настроек
    backFromSettingsBtn.bounds = (Rectangle){ centerX, 480, btnWidth, btnHeight };
    backFromSettingsBtn.text = "Back";
    backFromSettingsBtn.color = DARKGRAY;
    backFromSettingsBtn.hoverColor = GRAY;

    // Кнопки выбора цветов фона
    float smallBtnW = 100, smallBtnH = 40;
    float startX = 200;
    for (int i = 0; i < 4; i++) {
        bgColorBtns[i].bounds = (Rectangle){ startX + i * 110, 200, smallBtnW, smallBtnH };
        bgColorBtns[i].text = "";  // цвет покажет заливка
        bgColorBtns[i].color = bgColorOptions[i];
        bgColorBtns[i].hoverColor = bgColorOptions[i];  // без изменения
    }

    // Кнопки выбора цветов ячеек
    for (int i = 0; i < 4; i++) {
        cellColorBtns[i].bounds = (Rectangle){ startX + i * 110, 300, smallBtnW, smallBtnH };
        cellColorBtns[i].text = "";
        cellColorBtns[i].color = cellColorOptions[i];
        cellColorBtns[i].hoverColor = cellColorOptions[i];
    }
}

// ---------------------------------------------------------------------------
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
void UpdateMenu(void) {
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        if (IsMouseOverButton(newGameBtn)) {
            currentScreen = SCREEN_DIFFICULTY;
        }
        if (IsMouseOverButton(settingsBtn)) {
            currentScreen = SCREEN_SETTINGS;
        }
        if (IsMouseOverButton(exitBtn)) {
            CloseWindow();
        }
    }
}

void DrawMenu(void) {
    DrawText("SWEEPER", 300, 100, 50, WHITE);
    DrawButton(newGameBtn);
    DrawButton(settingsBtn);
    DrawButton(exitBtn);
}

// ---------------------------------------------------------------------------
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
void UpdateGameplay(void) {
    if (gameLost || gameWon) {
        if (IsKeyPressed(KEY_ENTER)) currentScreen = SCREEN_MENU;
        return;
    }
    if (IsKeyPressed(KEY_ESCAPE)) currentScreen = SCREEN_MENU;

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        Vector2 mouse = GetMousePosition();
        if (CheckCollisionPointRec(mouse, smileyRect)) {
            ResetGame();
            return;
        }
    }

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) || IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) {
        Vector2 mouse = GetMousePosition();
        int col = (int)((mouse.x - GRID_X) / CELL_SIZE);
        int row = (int)((mouse.y - GRID_Y) / CELL_SIZE);

        if (row >= 0 && row < gameBoard.rows && col >= 0 && col < gameBoard.cols) {
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                if (!gameBoard.firstClickDone) {
                    BoardPlaceMines(&gameBoard, row, col);
                    BoardCalculateNumbers(&gameBoard);
                    timerStarted = true;
                    startTime = GetTime();
                }
                bool hitMine = BoardReveal(&gameBoard, row, col);
                if (hitMine) gameLost = true;
                else gameWon = BoardCheckVictory(&gameBoard);
            } else if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) {
                if (!gameBoard.firstClickDone) return;
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

    if (timerStarted && !gameLost && !gameWon) {
        double elapsed = GetTime() - startTime;
        timer = (int)elapsed;
        if (timer > 999) timer = 999;
    }
}

void DrawGameplay(void) {
    // Верхняя панель
    int minesLeft = gameConfig.mines - flagCount;
    if (minesLeft < 0) minesLeft = 0;
    char mineStr[16];
    sprintf(mineStr, "Mines: %d", minesLeft);
    DrawText(mineStr, 20, 10, 24, WHITE);

    char timeStr[16];
    sprintf(timeStr, "Time: %d", timer);
    DrawText(timeStr, 150, 10, 24, WHITE);

    smileyRect = (Rectangle){ 350, 5, 40, 40 };
    Color smileyColor = gameLost ? RED : gameWon ? GREEN : YELLOW;
    DrawRectangleRec(smileyRect, smileyColor);
    DrawRectangleLinesEx(smileyRect, 2, WHITE);
    DrawText(":)", (int)(smileyRect.x + 8), (int)(smileyRect.y + 6), 24, BLACK);

    // Фон игры (подложка под сетку) — используем кастомный bgColor
    DrawRectangle(GRID_X - 5, GRID_Y - 5,
                  gameBoard.cols * CELL_SIZE + 10, gameBoard.rows * CELL_SIZE + 10,
                  bgColor);

    // Сетка
    for (int r = 0; r < gameBoard.rows; r++) {
        for (int c = 0; c < gameBoard.cols; c++) {
            Rectangle cellRect = {
                GRID_X + c * CELL_SIZE,
                GRID_Y + r * CELL_SIZE,
                CELL_SIZE, CELL_SIZE
            };

            Cell cell = gameBoard.cells[r][c];

            if (cell.state == CELL_HIDDEN) {
                DrawRectangleRec(cellRect, cellColor);
                DrawRectangleLines((int)cellRect.x, (int)cellRect.y, CELL_SIZE, CELL_SIZE, cellHiddenLine);
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
                DrawRectangleRec(cellRect, cellColor);
                DrawRectangleLines((int)cellRect.x, (int)cellRect.y, CELL_SIZE, CELL_SIZE, cellHiddenLine);
                DrawText("F", (int)(cellRect.x + 8), (int)(cellRect.y + 5), 24, RED);
            }
        }
    }

    if (gameLost) {
        DrawText("YOU LOST! (ENTER to menu)", 200, 45, 30, RED);
    } else if (gameWon) {
        DrawText("YOU WIN! (ENTER to menu)", 200, 45, 30, GREEN);
    }
}

// ---------------------------------------------------------------------------
void UpdateSettings(void) {
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        for (int i = 0; i < 4; i++) {
            if (IsMouseOverButton(bgColorBtns[i])) {
                bgColor = bgColorOptions[i];
            }
            if (IsMouseOverButton(cellColorBtns[i])) {
                cellColor = cellColorOptions[i];
                // Автоматически подбираем линию чуть темнее
                cellHiddenLine.r = (unsigned char)(cellColor.r * 0.6f);
                cellHiddenLine.g = (unsigned char)(cellColor.g * 0.6f);
                cellHiddenLine.b = (unsigned char)(cellColor.b * 0.6f);
                cellHiddenLine.a = 255;
            }
        }
        if (IsMouseOverButton(backFromSettingsBtn)) {
            currentScreen = SCREEN_MENU;
        }
    }
}

void DrawSettings(void) {
    DrawText("SETTINGS", 300, 80, 40, WHITE);

    DrawText("Background Color:", 200, 170, 20, LIGHTGRAY);
    for (int i = 0; i < 4; i++) {
        DrawRectangleRec(bgColorBtns[i].bounds, bgColorBtns[i].color);
        DrawRectangleLinesEx(bgColorBtns[i].bounds, 2, (i == 0 && bgColor.r == bgColorOptions[0].r) ? RED : WHITE);
    }

    DrawText("Cell Color:", 200, 270, 20, LIGHTGRAY);
    for (int i = 0; i < 4; i++) {
        DrawRectangleRec(cellColorBtns[i].bounds, cellColorBtns[i].color);
        DrawRectangleLinesEx(cellColorBtns[i].bounds, 2, (i == 0 && cellColor.r == cellColorOptions[0].r) ? RED : WHITE);
    }

    DrawButton(backFromSettingsBtn);
}

// ---------------------------------------------------------------------------
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
int main(void)
{
    InitWindow(800, 600, "Sweeper");
    InitMenuButtons();
    InitDifficultyButtons();

    while (!WindowShouldClose())
    {
        switch (currentScreen) {
            case SCREEN_MENU:       UpdateMenu();       break;
            case SCREEN_DIFFICULTY: UpdateDifficulty(); break;
            case SCREEN_GAMEPLAY:   UpdateGameplay();   break;
            case SCREEN_MINIGAME:   UpdateMinigame();   break;
            case SCREEN_GAME_OVER:  UpdateGameOver();   break;
            case SCREEN_VICTORY:    UpdateVictory();    break;
            case SCREEN_SETTINGS:   UpdateSettings();   break;
        }

        BeginDrawing();
        ClearBackground(DARKGRAY);

        switch (currentScreen) {
            case SCREEN_MENU:       DrawMenu();       break;
            case SCREEN_DIFFICULTY: DrawDifficulty(); break;
            case SCREEN_GAMEPLAY:   DrawGameplay();   break;
            case SCREEN_MINIGAME:   DrawMinigame();   break;
            case SCREEN_GAME_OVER:  DrawGameOver();   break;
            case SCREEN_VICTORY:    DrawVictory();    break;
            case SCREEN_SETTINGS:   DrawSettings();   break;
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
