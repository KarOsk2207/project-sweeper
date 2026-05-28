#include "raylib.h"
#include "board.h"
#include <stdio.h>
#include <string.h>   // для strcmp

typedef enum {
    SCREEN_MENU,
    SCREEN_DIFFICULTY,
    SCREEN_GAMEPLAY,
    SCREEN_MINIGAME,
    SCREEN_GAME_OVER,
    SCREEN_VICTORY,
    SCREEN_SETTINGS,
    SCREEN_STATS
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

GameScreen currentScreen = SCREEN_MENU;
GameConfig gameConfig;
Board gameBoard;
bool gameLost = false;
bool gameWon  = false;

int flagCount = 0;
int timer = 0;
double startTime = 0.0;
bool timerStarted = false;

Color bgColor = DARKGRAY;
Color cellColor = LIGHTGRAY;
Color cellHiddenLine = GRAY;

int gamesPlayed = 0;
int gamesWon = 0;
int gamesLost = 0;
bool gameEndCounted = false;

// ит-мод
bool cheatActive = false;
char inputBuffer[6] = "";  // храним последние 5 символов + завершающий '\0'

Button newGameBtn;
Button settingsBtn;
Button statsBtn;
Button exitBtn;

Button easyBtn;
Button mediumBtn;
Button hardBtn;
Button backBtn;

Button backFromSettingsBtn;
Button bgColorBtns[4];
Button cellColorBtns[4];
Button backFromStatsBtn;

Rectangle smileyRect;

Color bgColorOptions[4] = {
    { 80, 80, 80, 255 },
    { 20, 20, 20, 255 },
    { 10, 10, 50, 255 },
    { 10, 50, 10, 255 }
};

Color cellColorOptions[4] = {
    { 200, 200, 200, 255 },
    { 255, 255, 255, 255 },
    { 180, 200, 255, 255 },
    { 180, 255, 180, 255 }
};

#define CELL_SIZE  30
#define GRID_X     50
#define GRID_Y     80

void ResetGame(void) {
    BoardInit(&gameBoard, gameConfig.rows, gameConfig.cols, gameConfig.mines);
    gameLost = false;
    gameWon = false;
    flagCount = 0;
    timer = 0;
    timerStarted = false;
    startTime = 0.0;
    gameEndCounted = false;
    gamesPlayed++;
}

void InitMenuButtons(void) {
    float btnWidth = 200, btnHeight = 50;
    float centerX = (800 - btnWidth) / 2.0f;

    newGameBtn.bounds = (Rectangle){ centerX, 160, btnWidth, btnHeight };
    newGameBtn.text = "New Game";
    newGameBtn.color = DARKGREEN;
    newGameBtn.hoverColor = GREEN;

    settingsBtn.bounds = (Rectangle){ centerX, 230, btnWidth, btnHeight };
    settingsBtn.text = "Settings";
    settingsBtn.color = DARKBLUE;
    settingsBtn.hoverColor = BLUE;

    statsBtn.bounds = (Rectangle){ centerX, 300, btnWidth, btnHeight };
    statsBtn.text = "Stats";
    statsBtn.color = DARKPURPLE;
    statsBtn.hoverColor = PURPLE;

    exitBtn.bounds = (Rectangle){ centerX, 370, btnWidth, btnHeight };
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

    backFromSettingsBtn.bounds = (Rectangle){ centerX, 480, btnWidth, btnHeight };
    backFromSettingsBtn.text = "Back";
    backFromSettingsBtn.color = DARKGRAY;
    backFromSettingsBtn.hoverColor = GRAY;

    backFromStatsBtn = backFromSettingsBtn;

    float smallBtnW = 100, smallBtnH = 40;
    float startX = 200;
    for (int i = 0; i < 4; i++) {
        bgColorBtns[i].bounds = (Rectangle){ startX + i * 110, 200, smallBtnW, smallBtnH };
        bgColorBtns[i].text = "";
        bgColorBtns[i].color = bgColorOptions[i];
        bgColorBtns[i].hoverColor = bgColorOptions[i];
    }

    for (int i = 0; i < 4; i++) {
        cellColorBtns[i].bounds = (Rectangle){ startX + i * 110, 300, smallBtnW, smallBtnH };
        cellColorBtns[i].text = "";
        cellColorBtns[i].color = cellColorOptions[i];
        cellColorBtns[i].hoverColor = cellColorOptions[i];
    }
}

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

// бновление буфера ввода для чит-кода
void UpdateCheatInput(void) {
    int key = GetKeyPressed();

    if (key >= 65 && key <= 90) {      // буквы A-Z
        char c = (char)key;
        // Сдвиг буфера влево на 1 символ
        memmove(inputBuffer, inputBuffer + 1, 4);
        inputBuffer[4] = c;
        inputBuffer[5] = '\0';

        if (strcmp(inputBuffer, "IDDQD") == 0) {
            cheatActive = !cheatActive;   // переключить
            inputBuffer[0] = '\0';        // сбросить буфер
        }
    }
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
        if (IsMouseOverButton(statsBtn)) {
            currentScreen = SCREEN_STATS;
        }
        if (IsMouseOverButton(exitBtn)) {
            CloseWindow();
        }
    }
}

void DrawMenu(void) {
    DrawText("SWEEPER", 300, 60, 50, WHITE);
    DrawButton(newGameBtn);
    DrawButton(settingsBtn);
    DrawButton(statsBtn);
    DrawButton(exitBtn);
}

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

void UpdateGameplay(void) {
    if (gameLost || gameWon) {
        if (!gameEndCounted) {
            if (gameLost) gamesLost++;
            else gamesWon++;
            gameEndCounted = true;
        }
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

    DrawRectangle(GRID_X - 5, GRID_Y - 5,
                  gameBoard.cols * CELL_SIZE + 10, gameBoard.rows * CELL_SIZE + 10,
                  bgColor);

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

    // аглушка чека
    if (cheatActive) {
        DrawText("CHEAT ACTIVE", 600, 10, 20, RED);
    }
}

void UpdateSettings(void) {
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        for (int i = 0; i < 4; i++) {
            if (IsMouseOverButton(bgColorBtns[i])) {
                bgColor = bgColorOptions[i];
            }
            if (IsMouseOverButton(cellColorBtns[i])) {
                cellColor = cellColorOptions[i];
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
        DrawRectangleLinesEx(bgColorBtns[i].bounds, 2, WHITE);
    }
    DrawText("Cell Color:", 200, 270, 20, LIGHTGRAY);
    for (int i = 0; i < 4; i++) {
        DrawRectangleRec(cellColorBtns[i].bounds, cellColorBtns[i].color);
        DrawRectangleLinesEx(cellColorBtns[i].bounds, 2, WHITE);
    }
    DrawButton(backFromSettingsBtn);
}

void UpdateStats(void) {
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        if (IsMouseOverButton(backFromStatsBtn)) {
            currentScreen = SCREEN_MENU;
        }
    }
}

void DrawStats(void) {
    DrawText("STATISTICS", 280, 60, 40, WHITE);

    char playStr[32];
    sprintf(playStr, "Games played: %d", gamesPlayed);
    DrawText(playStr, 250, 160, 24, WHITE);

    char winStr[32];
    sprintf(winStr, "Wins: %d", gamesWon);
    DrawText(winStr, 250, 210, 24, GREEN);

    char loseStr[32];
    sprintf(loseStr, "Losses: %d", gamesLost);
    DrawText(loseStr, 250, 260, 24, RED);

    if (gamesPlayed > 0) {
        float winRate = (float)gamesWon / gamesPlayed * 100.0f;
        char rateStr[32];
        sprintf(rateStr, "Win rate: %.1f%%", winRate);
        DrawText(rateStr, 250, 310, 24, YELLOW);
    } else {
        DrawText("Win rate: --", 250, 310, 24, LIGHTGRAY);
    }

    DrawButton(backFromStatsBtn);
}

void UpdateMinigame(void) { if (IsKeyPressed(KEY_ESCAPE)) currentScreen = SCREEN_GAMEPLAY; }
void DrawMinigame(void) { DrawText("MINIGAME", 100, 100, 30, WHITE); }
void UpdateGameOver(void) { if (IsKeyPressed(KEY_ENTER)) currentScreen = SCREEN_MENU; }
void DrawGameOver(void) { DrawText("GAME OVER (press ENTER to menu)", 100, 100, 30, RED); }
void UpdateVictory(void) { if (IsKeyPressed(KEY_ENTER)) currentScreen = SCREEN_MENU; }
void DrawVictory(void) { DrawText("VICTORY! (press ENTER to menu)", 100, 100, 30, GREEN); }

int main(void)
{
    InitWindow(800, 600, "Sweeper");
    InitMenuButtons();
    InitDifficultyButtons();

    while (!WindowShouldClose())
    {
        UpdateCheatInput();   // обработка чит-кода

        switch (currentScreen) {
            case SCREEN_MENU:       UpdateMenu();       break;
            case SCREEN_DIFFICULTY: UpdateDifficulty(); break;
            case SCREEN_GAMEPLAY:   UpdateGameplay();   break;
            case SCREEN_MINIGAME:   UpdateMinigame();   break;
            case SCREEN_GAME_OVER:  UpdateGameOver();   break;
            case SCREEN_VICTORY:    UpdateVictory();    break;
            case SCREEN_SETTINGS:   UpdateSettings();   break;
            case SCREEN_STATS:      UpdateStats();      break;
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
            case SCREEN_STATS:      DrawStats();      break;
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
