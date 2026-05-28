#include "raylib.h"
#include "board.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

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

bool cheatActive = false;
bool devMode = false;                     // <- новый флаг
char inputBuffer[12] = "";               // <- расширенный буфер

bool fullscreen = false;

int cellSize = 30;
int gridX = 50;
int gridY = 80;

float generatorCharge = 50.0f;
const float GENERATOR_CHARGE_RATE = 25.0f;
const float GENERATOR_DISCHARGE_RATE = 3.0f;
bool generatorWarning = false;

int phaseEnemyPhase = 1;
double phaseEnemyTimer = 0.0;
const double PHASE_TRANSITION_TIME = 15.0;
Rectangle phaseEnemyResetBtn;
bool phaseEnemyResetQueued = false;

bool qteActive = false;
int qteSequence[5] = { 0 };
int qteCurrentIndex = 0;
double qteTimeLeft = 0.0;
double qteCooldownTimer = 0.0;
double qteSpawnTimer = 0.0;
Rectangle qteWindowRect;
bool qteSpawnCooldown = false;

bool jumpscareActive = false;
int jumpscarePhase = 0;
double jumpscareTimer = 0.0;
double jumpscareCooldownTimer = 0.0;
double jumpscareSpawnTimer = 0.0;
bool jumpscareSpawnCooldown = false;
Rectangle jumpscareCubeRect = { 0, 0, 0, 0 };

bool spamActive = false;
int spamCountdown = 5;
int spamProgress = 0;
double spamTickTimer = 0.0;
double spamCooldownTimer = 0.0;
double spamSpawnTimer = 0.0;
bool spamSpawnCooldown = false;

bool alignActive = false;
int alignCountdown = 8;
double alignTimer = 0.0;
bool alignCubesDone[3] = { false, false, false };
Rectangle alignCubeRects[3];
Rectangle alignZoneRects[3];
Rectangle alignSubRects[3];
int alignDragging = -1;
double alignSpawnTimer = 0.0;
double alignCooldownTimer = 0.0;
bool alignSpawnCooldown = false;

int activeDynamicEnemies = 0;
#define MAX_DYNAMIC_ENEMIES 3

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

void DrawJumpscare(void);
void DrawSpam(void);
void DrawAlign(void);

void SaveData(void) {
    FILE *f = fopen("sweeper.dat", "w");
    if (f) {
        fprintf(f, "%d %d %d %d\n", bgColor.r, bgColor.g, bgColor.b, bgColor.a);
        fprintf(f, "%d %d %d %d\n", cellColor.r, cellColor.g, cellColor.b, cellColor.a);
        fprintf(f, "%d %d %d %d\n", cellHiddenLine.r, cellHiddenLine.g, cellHiddenLine.b, cellHiddenLine.a);
        fprintf(f, "%d %d %d\n", gamesPlayed, gamesWon, gamesLost);
        fclose(f);
    }
}

void LoadData(void) {
    FILE *f = fopen("sweeper.dat", "r");
    if (f) {
        fscanf(f, "%hhu %hhu %hhu %hhu", &bgColor.r, &bgColor.g, &bgColor.b, &bgColor.a);
        fscanf(f, "%hhu %hhu %hhu %hhu", &cellColor.r, &cellColor.g, &cellColor.b, &cellColor.a);
        fscanf(f, "%hhu %hhu %hhu %hhu", &cellHiddenLine.r, &cellHiddenLine.g, &cellHiddenLine.b, &cellHiddenLine.a);
        fscanf(f, "%d %d %d", &gamesPlayed, &gamesWon, &gamesLost);
        fclose(f);
    }
}

void RecalculateGrid(void) {
    int w = GetScreenWidth();
    int h = GetScreenHeight();
    int topMargin = 80;
    int bottomMargin = 20;
    int sideMargin = 20;
    int availW = w - 2 * sideMargin;
    int availH = h - topMargin - bottomMargin;
    int maxCellW = availW / gameBoard.cols;
    int maxCellH = availH / gameBoard.rows;
    cellSize = (maxCellW < maxCellH) ? maxCellW : maxCellH;
    if (cellSize < 10) cellSize = 10;
    if (cellSize > 50) cellSize = 50;
    gridX = (w - gameBoard.cols * cellSize) / 2;
    gridY = topMargin + (availH - gameBoard.rows * cellSize) / 2;
    if (gridY < topMargin) gridY = topMargin;
}

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
    generatorCharge = 50.0f;
    phaseEnemyPhase = 1;
    phaseEnemyTimer = 0.0;
    qteActive = false;
    qteSpawnCooldown = false;
    qteCooldownTimer = 0.0;
    qteSpawnTimer = 0.0;
    jumpscareActive = false;
    jumpscareSpawnCooldown = false;
    jumpscareCooldownTimer = 0.0;
    jumpscareSpawnTimer = 0.0;
    spamActive = false;
    spamSpawnCooldown = false;
    spamCooldownTimer = 0.0;
    spamSpawnTimer = 0.0;
    alignActive = false;
    alignSpawnCooldown = false;
    alignCooldownTimer = 0.0;
    alignSpawnTimer = 0.0;
    activeDynamicEnemies = 0;
    RecalculateGrid();
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

    phaseEnemyResetBtn = (Rectangle){ 10, GetScreenHeight() - 140, 60, 30 };
}

void RecalculateButtons(void) {
    int w = GetScreenWidth();
    float centerX = (w - 200) / 2.0f;
    newGameBtn.bounds.x = centerX;
    settingsBtn.bounds.x = centerX;
    statsBtn.bounds.x = centerX;
    exitBtn.bounds.x = centerX;

    centerX = (w - 220) / 2.0f;
    easyBtn.bounds.x = centerX;
    mediumBtn.bounds.x = centerX;
    hardBtn.bounds.x = centerX;
    backBtn.bounds.x = centerX;
    backFromSettingsBtn.bounds.x = centerX;
    backFromStatsBtn.bounds.x = centerX;

    float startX = (w - 4 * 110) / 2.0f;
    for (int i = 0; i < 4; i++) {
        bgColorBtns[i].bounds.x = startX + i * 110;
        cellColorBtns[i].bounds.x = startX + i * 110;
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

void UpdateCheatInput(void) {
    int key = GetKeyPressed();
    if (key >= 65 && key <= 90) {
        char c = (char)key;
        memmove(inputBuffer, inputBuffer + 1, 10);
        inputBuffer[10] = c;
        inputBuffer[11] = '\0';
        if (strcmp(inputBuffer, "IDDQD") == 0) {
            cheatActive = !cheatActive;
            inputBuffer[0] = '\0';
            if (cheatActive) {
                if (qteActive) { qteActive = false; activeDynamicEnemies--; }
                if (jumpscareActive) { jumpscareActive = false; activeDynamicEnemies--; }
                if (spamActive) { spamActive = false; activeDynamicEnemies--; }
                if (alignActive) { alignActive = false; activeDynamicEnemies--; }
            }
        }
        if (strcmp(inputBuffer, "DEVCOMMANDS") == 0 && currentScreen == SCREEN_MENU) {
            devMode = !devMode;
            inputBuffer[0] = '\0';
        }
    }
}

void UpdateGenerator(void) {
    if (cheatActive) {
        generatorCharge = 100.0f;
        generatorWarning = false;
        return;
    }

    float dt = GetFrameTime();
    if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) {
        generatorCharge += GENERATOR_CHARGE_RATE * dt;
        if (generatorCharge >= 100.0f) {
            generatorCharge = 100.0f;
            if (!gameLost) gameLost = true;
        }
    } else {
        generatorCharge -= GENERATOR_DISCHARGE_RATE * dt;
        if (generatorCharge <= 0.0f) {
            generatorCharge = 0.0f;
            if (!gameLost) gameLost = true;
        }
    }
    generatorWarning = (generatorCharge >= 90.0f);
}

void UpdatePhaseEnemy(void) {
    if (cheatActive) {
        phaseEnemyPhase = 1;
        phaseEnemyTimer = 0.0;
        return;
    }
    if (gameLost || gameWon) return;

    float dt = GetFrameTime();
    phaseEnemyTimer += dt;

    if (phaseEnemyResetQueued) {
        if (generatorCharge >= 20.0f) {
            generatorCharge -= 20.0f;
            phaseEnemyPhase = 1;
            phaseEnemyTimer = 0.0;
        }
        phaseEnemyResetQueued = false;
    }

    if (phaseEnemyTimer >= PHASE_TRANSITION_TIME && phaseEnemyPhase < 5) {
        phaseEnemyPhase++;
        phaseEnemyTimer = 0.0;
        if (phaseEnemyPhase >= 5) {
            if (!gameLost) gameLost = true;
        }
    }
}

void UpdateQTE(void) {
    if (cheatActive) {
        if (qteActive) { activeDynamicEnemies--; }
        qteActive = false;
        qteSpawnCooldown = false;
        qteCooldownTimer = 0.0;
        qteSpawnTimer = 0.0;
        return;
    }
    if (gameLost || gameWon) return;

    float dt = GetFrameTime();

    if (qteSpawnCooldown) {
        qteCooldownTimer -= dt;
        if (qteCooldownTimer <= 0.0) {
            qteSpawnCooldown = false;
            qteSpawnTimer = 0.0;
        }
        return;
    }

    if (!qteActive) {
        qteSpawnTimer += dt;
        if (qteSpawnTimer >= 7.0) {
            qteSpawnTimer = 0.0;
            if (activeDynamicEnemies < MAX_DYNAMIC_ENEMIES && (rand() % 100) < 20) {
                qteActive = true;
                qteCurrentIndex = 0;
                qteTimeLeft = 9.0;
                for (int i = 0; i < 5; i++) {
                    qteSequence[i] = rand() % 4;
                }
                qteWindowRect.x = (float)(rand() % (GetScreenWidth() - 200));
                qteWindowRect.y = (float)(rand() % (GetScreenHeight() - 100));
                qteWindowRect.width = 200;
                qteWindowRect.height = 80;
                activeDynamicEnemies++;
            }
        }
    } else {
        int key = GetKeyPressed();
        if (key == KEY_UP || key == KEY_DOWN || key == KEY_LEFT || key == KEY_RIGHT) {
            int expected = qteSequence[qteCurrentIndex];
            bool correct = false;
            if (key == KEY_UP && expected == 0) correct = true;
            else if (key == KEY_DOWN && expected == 1) correct = true;
            else if (key == KEY_LEFT && expected == 2) correct = true;
            else if (key == KEY_RIGHT && expected == 3) correct = true;

            if (correct) {
                qteCurrentIndex++;
                if (qteCurrentIndex >= 5) {
                    qteActive = false;
                    qteSpawnCooldown = true;
                    qteCooldownTimer = 10.0;
                    activeDynamicEnemies--;
                }
            }
        }

        qteTimeLeft -= dt;
        if (qteTimeLeft <= 0.0) {
            if (!gameLost) gameLost = true;
            qteActive = false;
            activeDynamicEnemies--;
        }
    }
}

void UpdateJumpscare(void) {
    if (cheatActive) {
        if (jumpscareActive) { activeDynamicEnemies--; }
        jumpscareActive = false;
        jumpscareSpawnCooldown = false;
        jumpscareCooldownTimer = 0.0;
        jumpscareSpawnTimer = 0.0;
        return;
    }
    if (gameLost || gameWon) return;

    float dt = GetFrameTime();

    if (jumpscareSpawnCooldown) {
        jumpscareCooldownTimer -= dt;
        if (jumpscareCooldownTimer <= 0.0) {
            jumpscareSpawnCooldown = false;
            jumpscareSpawnTimer = 0.0;
        }
        return;
    }

    if (!jumpscareActive) {
        jumpscareSpawnTimer += dt;
        if (jumpscareSpawnTimer >= 7.0) {
            jumpscareSpawnTimer = 0.0;
            if (activeDynamicEnemies < MAX_DYNAMIC_ENEMIES && (rand() % 100) < 20) {
                jumpscareActive = true;
                jumpscarePhase = 0;
                jumpscareTimer = 0.0;
                activeDynamicEnemies++;
            }
        }
    } else {
        jumpscareTimer += dt;
        switch (jumpscarePhase) {
            case 0:
                if (jumpscareTimer >= 1.0) {
                    jumpscarePhase = 1;
                    jumpscareTimer = 0.0;
                }
                break;
            case 1:
                if (jumpscareTimer >= 2.0) {
                    jumpscarePhase = 2;
                    jumpscareTimer = 0.0;
                }
                break;
            case 2:
                if (jumpscareTimer >= 2.0) {
                    if (!gameLost) gameLost = true;
                    jumpscareActive = false;
                    activeDynamicEnemies--;
                }
                break;
        }
    }
}

void UpdateSpam(void) {
    if (cheatActive) {
        if (spamActive) { activeDynamicEnemies--; }
        spamActive = false;
        spamSpawnCooldown = false;
        spamCooldownTimer = 0.0;
        spamSpawnTimer = 0.0;
        return;
    }
    if (gameLost || gameWon) return;

    float dt = GetFrameTime();

    if (spamSpawnCooldown) {
        spamCooldownTimer -= dt;
        if (spamCooldownTimer <= 0.0) {
            spamSpawnCooldown = false;
            spamSpawnTimer = 0.0;
        }
        return;
    }

    if (!spamActive) {
        spamSpawnTimer += dt;
        if (spamSpawnTimer >= 7.0) {
            spamSpawnTimer = 0.0;
            if (activeDynamicEnemies < MAX_DYNAMIC_ENEMIES && (rand() % 100) < 20) {
                spamActive = true;
                spamCountdown = 5;
                spamProgress = 0;
                spamTickTimer = 0.0;
                activeDynamicEnemies++;
            }
        }
    } else {
        if (IsKeyPressed(KEY_LEFT_ALT) || IsKeyPressed(KEY_RIGHT_ALT)) {
            spamProgress += 7;
            if (spamProgress > 100) spamProgress = 100;
            if (spamProgress >= 100) {
                spamActive = false;
                spamSpawnCooldown = true;
                spamCooldownTimer = 10.0;
                activeDynamicEnemies--;
                return;
            }
        }

        spamTickTimer += dt;
        if (spamTickTimer >= 1.0) {
            spamTickTimer -= 1.0;
            spamCountdown--;
            if (spamCountdown < 0) {
                if (!gameLost) gameLost = true;
                spamActive = false;
                activeDynamicEnemies--;
            }
        }
    }
}

void UpdateAlign(void) {
    if (cheatActive) {
        if (alignActive) { activeDynamicEnemies--; }
        alignActive = false;
        alignSpawnCooldown = false;
        alignCooldownTimer = 0.0;
        alignSpawnTimer = 0.0;
        return;
    }
    if (gameLost || gameWon) return;

    float dt = GetFrameTime();

    if (alignSpawnCooldown) {
        alignCooldownTimer -= dt;
        if (alignCooldownTimer <= 0.0) {
            alignSpawnCooldown = false;
            alignSpawnTimer = 0.0;
        }
        return;
    }

    if (!alignActive) {
        alignSpawnTimer += dt;
        if (alignSpawnTimer >= 7.0) {
            alignSpawnTimer = 0.0;
            if (activeDynamicEnemies < MAX_DYNAMIC_ENEMIES && (rand() % 100) < 20) {
                alignActive = true;
                alignCountdown = 8;
                alignTimer = 0.0;
                for (int i = 0; i < 3; i++) alignCubesDone[i] = false;
                alignDragging = -1;

                Rectangle mainRect = {
                    GetScreenWidth()/2 - 150,
                    GetScreenHeight()/2 - 110,
                    300, 220
                };
                float subH = 50;
                float padding = 10;
                float zoneW = 60;
                float zoneH = 20;
                float cubeSize = 20;

                for (int i = 0; i < 3; i++) {
                    alignSubRects[i] = (Rectangle){
                        mainRect.x + padding,
                        mainRect.y + 60 + i * (subH + 5),
                        mainRect.width - 2 * padding,
                        subH
                    };
                    float zoneY = alignSubRects[i].y + rand() % (int)(subH - zoneH);
                    alignZoneRects[i] = (Rectangle){
                        alignSubRects[i].x + alignSubRects[i].width - zoneW - 5,
                        zoneY,
                        zoneW, zoneH
                    };
                    float cubeY;
                    do {
                        cubeY = alignSubRects[i].y + rand() % (int)(subH - cubeSize);
                    } while (cubeY + cubeSize > alignZoneRects[i].y - 2 &&
                             cubeY < alignZoneRects[i].y + zoneH + 2);
                    alignCubeRects[i] = (Rectangle){
                        alignSubRects[i].x + 5,
                        cubeY,
                        cubeSize, cubeSize
                    };
                }
                activeDynamicEnemies++;
            }
        }
    } else {
        alignTimer += dt;
        if (alignTimer >= 1.0) {
            alignTimer -= 1.0;
            alignCountdown--;
            if (alignCountdown < 0) {
                if (!gameLost) gameLost = true;
                alignActive = false;
                activeDynamicEnemies--;
                return;
            }
        }

        Vector2 mouse = GetMousePosition();
        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
            if (alignDragging == -1) {
                for (int i = 0; i < 3; i++) {
                    if (!alignCubesDone[i] && CheckCollisionPointRec(mouse, alignCubeRects[i])) {
                        alignDragging = i;
                        break;
                    }
                }
            }
            if (alignDragging != -1) {
                Rectangle* cube = &alignCubeRects[alignDragging];
                cube->y = mouse.y - cube->height/2;
                if (cube->y < alignSubRects[alignDragging].y)
                    cube->y = alignSubRects[alignDragging].y;
                if (cube->y + cube->height > alignSubRects[alignDragging].y + alignSubRects[alignDragging].height)
                    cube->y = alignSubRects[alignDragging].y + alignSubRects[alignDragging].height - cube->height;

                if (CheckCollisionRecs(*cube, alignZoneRects[alignDragging])) {
                    alignCubesDone[alignDragging] = true;
                    alignDragging = -1;
                }
            }
        } else {
            alignDragging = -1;
        }

        if (alignCubesDone[0] && alignCubesDone[1] && alignCubesDone[2]) {
            alignActive = false;
            alignSpawnCooldown = true;
            alignCooldownTimer = 10.0;
            activeDynamicEnemies--;
        }
    }
}

// Отрисовка врагов
void DrawJumpscare(void) {
    if (!jumpscareActive) return;
    int w = GetScreenWidth(), h = GetScreenHeight();
    if (jumpscarePhase == 0) {
        DrawText("PREPARE", w/2 - MeasureText("PREPARE", 40)/2, h - 50, 40, WHITE);
    } else {
        float size = (jumpscarePhase == 1) ? (float)(jumpscareTimer/2.0) * w * 0.7f :
                     w * 0.7f + (float)(jumpscareTimer/2.0) * w * 0.2f;
        float x = w/2 - size/2, y = h/2 - size/2;
        DrawRectangle((int)x, (int)y, (int)size, (int)size,
                      (jumpscarePhase == 1) ? YELLOW : RED);
    }
}

void DrawSpam(void) {
    if (!spamActive) return;
    Rectangle spamRect = { GetScreenWidth()/2 - 150, GetScreenHeight()/2 - 60, 300, 120 };
    DrawRectangleRec(spamRect, Fade(BLACK, 0.85f));
    DrawRectangleLinesEx(spamRect, 2, RED);
    DrawText("SPAM ALT!", spamRect.x + spamRect.width/2 - MeasureText("SPAM ALT!",24)/2, spamRect.y+5, 24, WHITE);
    char c[8]; sprintf(c, "%d", spamCountdown);
    DrawText(c, spamRect.x + spamRect.width/2 - MeasureText(c,40)/2, spamRect.y+35, 40, spamCountdown<=2 ? RED : YELLOW);
    int barX = spamRect.x+15, barY = spamRect.y+85, barW = spamRect.width-30, barH = 10;
    DrawRectangle(barX, barY, barW, barH, DARKGRAY);
    DrawRectangle(barX, barY, barW * (spamProgress/100.0f), barH, GREEN);
    DrawRectangleLines(barX, barY, barW, barH, WHITE);
}

void DrawAlign(void) {
    if (!alignActive) return;
    Rectangle r = { GetScreenWidth()/2-150, GetScreenHeight()/2-110, 300, 220 };
    DrawRectangleRec(r, Fade(BLACK,0.85f)); DrawRectangleLinesEx(r,2,RED);
    DrawText("ALIGN", r.x+r.width/2-MeasureText("ALIGN",24)/2, r.y+5, 24, WHITE);
    char c[8]; sprintf(c, "%d", alignCountdown);
    DrawText(c, r.x+r.width/2-MeasureText(c,40)/2, r.y+30, 40, RED);
    for (int i=0;i<3;i++) {
        DrawRectangleRec(alignSubRects[i], Fade(DARKGRAY,0.5f));
        DrawRectangleLinesEx(alignSubRects[i],1,LIGHTGRAY);
        DrawRectangleRec(alignZoneRects[i], GREEN);
        DrawRectangleRec(alignCubeRects[i], alignCubesDone[i] ? GRAY : RED);
    }
}

void UpdateMenu(void) {
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        if (IsMouseOverButton(newGameBtn)) currentScreen = SCREEN_DIFFICULTY;
        if (IsMouseOverButton(settingsBtn)) currentScreen = SCREEN_SETTINGS;
        if (IsMouseOverButton(statsBtn)) currentScreen = SCREEN_STATS;
        if (IsMouseOverButton(exitBtn)) CloseWindow();
    }
}

void DrawMenu(void) {
    DrawText("SWEEPER", GetScreenWidth()/2 - MeasureText("SWEEPER", 50)/2, 60, 50, WHITE);
    DrawButton(newGameBtn);
    DrawButton(settingsBtn);
    DrawButton(statsBtn);
    DrawButton(exitBtn);
    if (devMode) DrawText("DEV MODE ON", 10, GetScreenHeight()-30, 20, GREEN);
}

void UpdateDifficulty(void) {
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        if (IsMouseOverButton(easyBtn)) { gameConfig.rows=9;gameConfig.cols=9;gameConfig.mines=10; ResetGame(); currentScreen=SCREEN_GAMEPLAY; }
        if (IsMouseOverButton(mediumBtn)) { gameConfig.rows=16;gameConfig.cols=16;gameConfig.mines=40; ResetGame(); currentScreen=SCREEN_GAMEPLAY; }
        if (IsMouseOverButton(hardBtn)) { gameConfig.rows=16;gameConfig.cols=30;gameConfig.mines=99; ResetGame(); currentScreen=SCREEN_GAMEPLAY; }
        if (IsMouseOverButton(backBtn)) currentScreen=SCREEN_MENU;
    }
}

void DrawDifficulty(void) {
    DrawText("SELECT DIFFICULTY", GetScreenWidth()/2-MeasureText("SELECT DIFFICULTY",40)/2,100,40,WHITE);
    DrawButton(easyBtn); DrawButton(mediumBtn); DrawButton(hardBtn); DrawButton(backBtn);
}

void UpdateGameplay(void) {
    if (IsKeyPressed(KEY_F11)) {
        fullscreen = !fullscreen;
        if (fullscreen) {
            int monitor = GetCurrentMonitor();
            SetWindowSize(GetMonitorWidth(monitor), GetMonitorHeight(monitor));
            ToggleFullscreen();
        } else { ToggleFullscreen(); SetWindowSize(800,600); }
        RecalculateGrid(); RecalculateButtons();
    }

    if (!gameLost && !gameWon) {
        UpdateGenerator(); UpdatePhaseEnemy();
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            Vector2 mouse = GetMousePosition();
            if (CheckCollisionPointRec(mouse, phaseEnemyResetBtn)) phaseEnemyResetQueued = true;
        }
        UpdateQTE(); UpdateJumpscare(); UpdateSpam(); UpdateAlign();
        if (jumpscareActive && jumpscarePhase==2 && IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) {
            jumpscareActive=false; jumpscareSpawnCooldown=true; jumpscareCooldownTimer=10.0; activeDynamicEnemies--;
        }
    }

    if (gameLost || gameWon) {
        if (!gameEndCounted) { if (gameLost) gamesLost++; else gamesWon++; gameEndCounted=true; }
        if (IsKeyPressed(KEY_ENTER)) currentScreen=SCREEN_MENU;
        return;
    }
    if (IsKeyPressed(KEY_ESCAPE)) currentScreen=SCREEN_MENU;

    if (!alignActive) {
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            Vector2 mouse = GetMousePosition();
            if (CheckCollisionPointRec(mouse, smileyRect)) { ResetGame(); return; }
        }
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) || IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) {
            Vector2 mouse = GetMousePosition();
            int col = (int)((mouse.x - gridX) / cellSize), row = (int)((mouse.y - gridY) / cellSize);
            if (row>=0 && row<gameBoard.rows && col>=0 && col<gameBoard.cols) {
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    if (!gameBoard.firstClickDone) { BoardPlaceMines(&gameBoard,row,col); BoardCalculateNumbers(&gameBoard); timerStarted=true; startTime=GetTime(); }
                    bool hit = BoardReveal(&gameBoard,row,col);
                    if (hit) gameLost=true; else gameWon=BoardCheckVictory(&gameBoard);
                } else if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) {
                    if (!gameBoard.firstClickDone) return;
                    Cell* cell = &gameBoard.cells[row][col];
                    if (cell->state==CELL_HIDDEN) { cell->state=CELL_FLAGGED; flagCount++; }
                    else if (cell->state==CELL_FLAGGED) { cell->state=CELL_HIDDEN; flagCount--; }
                }
            }
        }
    }
    if (timerStarted && !gameLost && !gameWon) { timer = (int)(GetTime()-startTime); if (timer>999) timer=999; }
}

void DrawGameplay(void) {
    int minesLeft = gameConfig.mines - flagCount;
    if (minesLeft<0) minesLeft=0;
    char buf[32];
    sprintf(buf, "Mines: %d", minesLeft); DrawText(buf,20,10,24,WHITE);
    sprintf(buf, "Time: %d", timer); DrawText(buf,150,10,24,WHITE);
    smileyRect = (Rectangle){ GetScreenWidth()/2-20, 5, 40, 40 };
    DrawRectangleRec(smileyRect, gameLost?RED:gameWon?GREEN:YELLOW);
    DrawRectangleLinesEx(smileyRect,2,WHITE);
    DrawText(":)", smileyRect.x+8, smileyRect.y+6, 24, BLACK);
    DrawRectangle(gridX-5, gridY-5, gameBoard.cols*cellSize+10, gameBoard.rows*cellSize+10, bgColor);
    for (int r=0; r<gameBoard.rows; r++) for (int c=0; c<gameBoard.cols; c++) {
        Rectangle cr = { gridX+c*cellSize, gridY+r*cellSize, cellSize, cellSize };
        Cell cell = gameBoard.cells[r][c];
        if (cell.state==CELL_HIDDEN) {
            DrawRectangleRec(cr, cellColor);
            DrawRectangleLines(cr.x,cr.y,cellSize,cellSize,cellHiddenLine);
            if (cheatActive) DrawCircle(cr.x+cellSize/2, cr.y+cellSize/2, cellSize/6, cell.hasMine?RED:GREEN);
        } else if (cell.state==CELL_REVEALED) {
            DrawRectangleRec(cr, WHITE);
            DrawRectangleLines(cr.x,cr.y,cellSize,cellSize,GRAY);
            if (cell.hasMine) DrawText("*", cr.x+cellSize/4, cr.y+cellSize/4, cellSize, RED);
            else if (cell.adjacentMines>0) { char n[4]; sprintf(n,"%d",cell.adjacentMines); DrawText(n, cr.x+cellSize/4, cr.y+cellSize/4, cellSize/2, DARKBLUE); }
        } else if (cell.state==CELL_FLAGGED) {
            DrawRectangleRec(cr, cellColor);
            DrawRectangleLines(cr.x,cr.y,cellSize,cellSize,cellHiddenLine);
            DrawText("F", cr.x+cellSize/4, cr.y+cellSize/4, cellSize/2, RED);
        }
    }
    Rectangle genPanel = {10, GetScreenHeight()-70, 150, 60};
    DrawRectangleRec(genPanel, Fade(BLACK,0.7f)); DrawRectangleLinesEx(genPanel,2,WHITE);
    Color chargeColor = generatorCharge>80 ? RED : generatorCharge>40 ? YELLOW : GREEN;
    if (generatorWarning && ((int)(GetTime()*4)%2)==0) DrawRectangleRec(genPanel, Fade(RED,0.3f));
    DrawText("GENERATOR", genPanel.x+10, genPanel.y+5, 14, LIGHTGRAY);
    sprintf(buf, "%.0f%%", generatorCharge); DrawText(buf, genPanel.x+genPanel.width/2-MeasureText(buf,20)/2, genPanel.y+25, 20, chargeColor);
    Rectangle phasePanel = {10, GetScreenHeight()-150, 150, 60};
    DrawRectangleRec(phasePanel, Fade(BLACK,0.7f)); DrawRectangleLinesEx(phasePanel,2,WHITE);
    DrawText("PHASE", phasePanel.x+10, phasePanel.y+5, 14, LIGHTGRAY);
    sprintf(buf, "%d/5", phaseEnemyPhase); DrawText(buf, phasePanel.x+phasePanel.width/2-MeasureText(buf,20)/2, phasePanel.y+25, 20, WHITE);
    if (phaseEnemyPhase<5 && !gameLost && !gameWon) {
        float p = phaseEnemyTimer/PHASE_TRANSITION_TIME;
        int bw = phasePanel.width-20, bx = phasePanel.x+10, by = phasePanel.y+48;
        DrawRectangle(bx,by,bw,4,DARKGRAY); DrawRectangle(bx,by,bw*p,4,RED);
    }
    DrawRectangleRec(phaseEnemyResetBtn, GRAY); DrawRectangleLinesEx(phaseEnemyResetBtn,1,WHITE);
    DrawText("<<", phaseEnemyResetBtn.x+12, phaseEnemyResetBtn.y+5, 18, WHITE);
    if (qteActive) {
        DrawRectangleRec(qteWindowRect, Fade(BLACK,0.85f)); DrawRectangleLinesEx(qteWindowRect,2,RED);
        float midY = qteWindowRect.y+qteWindowRect.height/2;
        DrawLine(qteWindowRect.x,midY, qteWindowRect.x+qteWindowRect.width,midY,RED);
        char prog[16]; sprintf(prog, "%d/5", qteCurrentIndex);
        DrawText(prog, qteWindowRect.x+qteWindowRect.width/2-MeasureText(prog,24)/2, qteWindowRect.y+5, 24, YELLOW);
        const char* dirs[] = {"Press Up","Press Down","Press Left","Press Right"};
        int idx = qteCurrentIndex<5 ? qteSequence[qteCurrentIndex] : 0;
        DrawText(dirs[idx], qteWindowRect.x+qteWindowRect.width/2-MeasureText(dirs[idx],16)/2, midY+10, 16, WHITE);
        float frac = qteTimeLeft/9.0f;
        int by = qteWindowRect.y+qteWindowRect.height-6;
        DrawRectangle(qteWindowRect.x+2,by,qteWindowRect.width-4,4,DARKGRAY);
        DrawRectangle(qteWindowRect.x+2,by,(qteWindowRect.width-4)*frac,4,RED);
    }
    DrawJumpscare(); DrawSpam(); DrawAlign();
    if (gameLost) DrawText("YOU LOST! (ENTER to menu)", GetScreenWidth()/2-MeasureText("YOU LOST! (ENTER to menu)",30)/2,45,30,RED);
    else if (gameWon) DrawText("YOU WIN! (ENTER to menu)", GetScreenWidth()/2-MeasureText("YOU WIN! (ENTER to menu)",30)/2,45,30,GREEN);
}

void UpdateSettings(void) {
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        for (int i=0;i<4;i++) {
            if (IsMouseOverButton(bgColorBtns[i])) bgColor = bgColorOptions[i];
            if (IsMouseOverButton(cellColorBtns[i])) {
                cellColor = cellColorOptions[i];
                cellHiddenLine.r = cellColor.r*0.6f; cellHiddenLine.g = cellColor.g*0.6f; cellHiddenLine.b = cellColor.b*0.6f; cellHiddenLine.a=255;
            }
        }
        if (IsMouseOverButton(backFromSettingsBtn)) currentScreen=SCREEN_MENU;
    }
}

void DrawSettings(void) {
    DrawText("SETTINGS", GetScreenWidth()/2-MeasureText("SETTINGS",40)/2,80,40,WHITE);
    DrawText("Background Color:", GetScreenWidth()/2-MeasureText("Background Color:",20)/2,170,20,LIGHTGRAY);
    for (int i=0;i<4;i++) { DrawRectangleRec(bgColorBtns[i].bounds, bgColorBtns[i].color); DrawRectangleLinesEx(bgColorBtns[i].bounds,2,WHITE); }
    DrawText("Cell Color:", GetScreenWidth()/2-MeasureText("Cell Color:",20)/2,270,20,LIGHTGRAY);
    for (int i=0;i<4;i++) { DrawRectangleRec(cellColorBtns[i].bounds, cellColorBtns[i].color); DrawRectangleLinesEx(cellColorBtns[i].bounds,2,WHITE); }
    DrawButton(backFromSettingsBtn);
}

void UpdateStats(void) { if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && IsMouseOverButton(backFromStatsBtn)) currentScreen=SCREEN_MENU; }
void DrawStats(void) {
    DrawText("STATISTICS", GetScreenWidth()/2-MeasureText("STATISTICS",40)/2,60,40,WHITE);
    char s[64]; sprintf(s,"Games played: %d",gamesPlayed); DrawText(s, GetScreenWidth()/2-MeasureText(s,24)/2,160,24,WHITE);
    sprintf(s,"Wins: %d",gamesWon); DrawText(s, GetScreenWidth()/2-MeasureText(s,24)/2,210,24,GREEN);
    sprintf(s,"Losses: %d",gamesLost); DrawText(s, GetScreenWidth()/2-MeasureText(s,24)/2,260,24,RED);
    if (gamesPlayed>0) { sprintf(s,"Win rate: %.1f%%", (float)gamesWon/gamesPlayed*100); DrawText(s, GetScreenWidth()/2-MeasureText(s,24)/2,310,24,YELLOW); }
    else DrawText("Win rate: --", GetScreenWidth()/2-MeasureText("Win rate: --",24)/2,310,24,LIGHTGRAY);
    DrawButton(backFromStatsBtn);
}

void UpdateMinigame(void) { if (IsKeyPressed(KEY_ESCAPE)) currentScreen=SCREEN_GAMEPLAY; }
void DrawMinigame(void) { DrawText("MINIGAME",100,100,30,WHITE); }
void UpdateGameOver(void) { if (IsKeyPressed(KEY_ENTER)) currentScreen=SCREEN_MENU; }
void DrawGameOver(void) { DrawText("GAME OVER (press ENTER to menu)",100,100,30,RED); }
void UpdateVictory(void) { if (IsKeyPressed(KEY_ENTER)) currentScreen=SCREEN_MENU; }
void DrawVictory(void) { DrawText("VICTORY! (press ENTER to menu)",100,100,30,GREEN); }

int main(void) {
    InitWindow(800,600,"Sweeper"); SetWindowMinSize(640,480);
    InitMenuButtons(); InitDifficultyButtons(); LoadData(); srand((unsigned int)time(NULL));
    while (!WindowShouldClose()) {
        if (IsWindowResized() && !fullscreen) { RecalculateButtons(); if (currentScreen==SCREEN_GAMEPLAY) RecalculateGrid(); }
        switch (currentScreen) {
            case SCREEN_MENU: UpdateMenu(); break;
            case SCREEN_DIFFICULTY: UpdateDifficulty(); break;
            case SCREEN_GAMEPLAY: UpdateGameplay(); break;
            case SCREEN_MINIGAME: UpdateMinigame(); break;
            case SCREEN_GAME_OVER: UpdateGameOver(); break;
            case SCREEN_VICTORY: UpdateVictory(); break;
            case SCREEN_SETTINGS: UpdateSettings(); break;
            case SCREEN_STATS: UpdateStats(); break;
        }
        UpdateCheatInput();
        BeginDrawing(); ClearBackground(DARKGRAY);
        switch (currentScreen) {
            case SCREEN_MENU: DrawMenu(); break;
            case SCREEN_DIFFICULTY: DrawDifficulty(); break;
            case SCREEN_GAMEPLAY: DrawGameplay(); break;
            case SCREEN_MINIGAME: DrawMinigame(); break;
            case SCREEN_GAME_OVER: DrawGameOver(); break;
            case SCREEN_VICTORY: DrawVictory(); break;
            case SCREEN_SETTINGS: DrawSettings(); break;
            case SCREEN_STATS: DrawStats(); break;
        }
        EndDrawing();
    }
    SaveData(); CloseWindow(); return 0;
}
