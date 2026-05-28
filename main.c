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
bool devMode = false;
char inputBuffer[12] = "";

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

// ---------- Dev-спавн ----------
void SpawnQTE(void) {
    if (qteActive || activeDynamicEnemies >= MAX_DYNAMIC_ENEMIES) return;
    qteActive = true;
    qteCurrentIndex = 0;
    qteTimeLeft = 9.0;
    for (int i = 0; i < 5; i++) qteSequence[i] = rand() % 4;
    qteWindowRect.x = rand() % (GetScreenWidth() - 200);
    qteWindowRect.y = rand() % (GetScreenHeight() - 100);
    qteWindowRect.width = 200;
    qteWindowRect.height = 80;
    qteSpawnCooldown = false; // сброс кулдаунов, чтобы после завершения враг мог появиться естественно
    qteCooldownTimer = 0.0;
    qteSpawnTimer = 0.0;
    activeDynamicEnemies++;
}

void SpawnJumpscare(void) {
    if (jumpscareActive || activeDynamicEnemies >= MAX_DYNAMIC_ENEMIES) return;
    jumpscareActive = true;
    jumpscarePhase = 0;
    jumpscareTimer = 0.0;
    jumpscareSpawnCooldown = false;
    jumpscareCooldownTimer = 0.0;
    jumpscareSpawnTimer = 0.0;
    activeDynamicEnemies++;
}

void SpawnSpam(void) {
    if (spamActive || activeDynamicEnemies >= MAX_DYNAMIC_ENEMIES) return;
    spamActive = true;
    spamCountdown = 5;
    spamProgress = 0;
    spamTickTimer = 0.0;
    spamSpawnCooldown = false;
    spamCooldownTimer = 0.0;
    spamSpawnTimer = 0.0;
    activeDynamicEnemies++;
}

void SpawnAlign(void) {
    if (alignActive || activeDynamicEnemies >= MAX_DYNAMIC_ENEMIES) return;
    alignActive = true;
    alignCountdown = 8;
    alignTimer = 0.0;
    for (int i = 0; i < 3; i++) alignCubesDone[i] = false;
    alignDragging = -1;
    Rectangle mainRect = { GetScreenWidth()/2 - 150, GetScreenHeight()/2 - 110, 300, 220 };
    float subH = 50, pad = 10, zoneW = 60, zoneH = 20, cubeSz = 20;
    for (int i = 0; i < 3; i++) {
        alignSubRects[i] = (Rectangle){ mainRect.x+pad, mainRect.y+60+i*(subH+5), mainRect.width-2*pad, subH };
        float zY = alignSubRects[i].y + rand()%(int)(subH-zoneH);
        alignZoneRects[i] = (Rectangle){ alignSubRects[i].x+alignSubRects[i].width-zoneW-5, zY, zoneW, zoneH };
        float cY;
        do { cY = alignSubRects[i].y + rand()%(int)(subH-cubeSz); }
        while (cY+cubeSz > alignZoneRects[i].y-2 && cY < alignZoneRects[i].y+zoneH+2);
        alignCubeRects[i] = (Rectangle){ alignSubRects[i].x+5, cY, cubeSz, cubeSz };
    }
    alignSpawnCooldown = false;
    alignCooldownTimer = 0.0;
    alignSpawnTimer = 0.0;
    activeDynamicEnemies++;
}
// ---------------------------------

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
    int w = GetScreenWidth(), h = GetScreenHeight();
    int top = 80, bot = 20, side = 20;
    int availW = w - 2*side, availH = h - top - bot;
    int maxCw = availW / gameBoard.cols, maxCh = availH / gameBoard.rows;
    cellSize = (maxCw < maxCh) ? maxCw : maxCh;
    if (cellSize < 10) cellSize = 10;
    if (cellSize > 50) cellSize = 50;
    gridX = (w - gameBoard.cols*cellSize)/2;
    gridY = top + (availH - gameBoard.rows*cellSize)/2;
    if (gridY < top) gridY = top;
}

void ResetGame(void) {
    BoardInit(&gameBoard, gameConfig.rows, gameConfig.cols, gameConfig.mines);
    gameLost = false; gameWon = false;
    flagCount = 0; timer = 0; timerStarted = false; startTime = 0.0;
    gameEndCounted = false; gamesPlayed++;
    generatorCharge = 50.0f;
    phaseEnemyPhase = 1; phaseEnemyTimer = 0.0;
    qteActive = false; qteSpawnCooldown = false; qteCooldownTimer = 0.0; qteSpawnTimer = 0.0;
    jumpscareActive = false; jumpscareSpawnCooldown = false; jumpscareCooldownTimer = 0.0; jumpscareSpawnTimer = 0.0;
    spamActive = false; spamSpawnCooldown = false; spamCooldownTimer = 0.0; spamSpawnTimer = 0.0;
    alignActive = false; alignSpawnCooldown = false; alignCooldownTimer = 0.0; alignSpawnTimer = 0.0;
    activeDynamicEnemies = 0;
    RecalculateGrid();
}

void InitMenuButtons(void) {
    float bw = 200, bh = 50, cx = (800 - bw)/2;
    newGameBtn.bounds = (Rectangle){ cx, 160, bw, bh }; newGameBtn.text = "New Game"; newGameBtn.color = DARKGREEN; newGameBtn.hoverColor = GREEN;
    settingsBtn.bounds = (Rectangle){ cx, 230, bw, bh }; settingsBtn.text = "Settings"; settingsBtn.color = DARKBLUE; settingsBtn.hoverColor = BLUE;
    statsBtn.bounds = (Rectangle){ cx, 300, bw, bh }; statsBtn.text = "Stats"; statsBtn.color = DARKPURPLE; statsBtn.hoverColor = PURPLE;
    exitBtn.bounds = (Rectangle){ cx, 370, bw, bh }; exitBtn.text = "Exit"; exitBtn.color = DARKGRAY; exitBtn.hoverColor = GRAY;
}

void InitDifficultyButtons(void) {
    float bw = 220, bh = 50, cx = (800 - bw)/2;
    easyBtn.bounds = (Rectangle){ cx, 200, bw, bh }; easyBtn.text = "Easy (9x9, 10 mines)"; easyBtn.color = DARKGREEN; easyBtn.hoverColor = GREEN;
    mediumBtn.bounds = (Rectangle){ cx, 270, bw, bh }; mediumBtn.text = "Medium (16x16, 40 mines)"; mediumBtn.color = DARKBLUE; mediumBtn.hoverColor = BLUE;
    hardBtn.bounds = (Rectangle){ cx, 340, bw, bh }; hardBtn.text = "Hard (30x16, 99 mines)"; hardBtn.color = DARKPURPLE; hardBtn.hoverColor = PURPLE;
    backBtn.bounds = (Rectangle){ cx, 420, bw, bh }; backBtn.text = "Back"; backBtn.color = DARKGRAY; backBtn.hoverColor = GRAY;
    backFromSettingsBtn.bounds = (Rectangle){ cx, 480, bw, bh }; backFromSettingsBtn.text = "Back"; backFromSettingsBtn.color = DARKGRAY; backFromSettingsBtn.hoverColor = GRAY;
    backFromStatsBtn = backFromSettingsBtn;
    float sw = 100, sh = 40, sx = 200;
    for (int i=0;i<4;i++) {
        bgColorBtns[i].bounds = (Rectangle){ sx + i*110, 200, sw, sh }; bgColorBtns[i].text = ""; bgColorBtns[i].color = bgColorOptions[i]; bgColorBtns[i].hoverColor = bgColorOptions[i];
        cellColorBtns[i].bounds = (Rectangle){ sx + i*110, 300, sw, sh }; cellColorBtns[i].text = ""; cellColorBtns[i].color = cellColorOptions[i]; cellColorBtns[i].hoverColor = cellColorOptions[i];
    }
    phaseEnemyResetBtn = (Rectangle){ 10, GetScreenHeight()-140, 60, 30 };
}

void RecalculateButtons(void) {
    int w = GetScreenWidth();
    float cx = (w-200)/2; newGameBtn.bounds.x = cx; settingsBtn.bounds.x = cx; statsBtn.bounds.x = cx; exitBtn.bounds.x = cx;
    cx = (w-220)/2; easyBtn.bounds.x = cx; mediumBtn.bounds.x = cx; hardBtn.bounds.x = cx; backBtn.bounds.x = cx;
    backFromSettingsBtn.bounds.x = cx; backFromStatsBtn.bounds.x = cx;
    float sx = (w-4*110)/2;
    for (int i=0;i<4;i++) { bgColorBtns[i].bounds.x = sx + i*110; cellColorBtns[i].bounds.x = sx + i*110; }
}

bool IsMouseOverButton(Button btn) { return CheckCollisionPointRec(GetMousePosition(), btn.bounds); }

void DrawButton(Button btn) {
    Color c = IsMouseOverButton(btn) ? btn.hoverColor : btn.color;
    DrawRectangleRec(btn.bounds, c); DrawRectangleLinesEx(btn.bounds, 2, WHITE);
    int tw = MeasureText(btn.text, 20);
    float tx = btn.bounds.x + (btn.bounds.width-tw)/2, ty = btn.bounds.y + (btn.bounds.height-20)/2;
    DrawText(btn.text, (int)tx, (int)ty, 20, WHITE);
}

void UpdateCheatInput(void) {
    int key = GetKeyPressed();
    if (key >= 65 && key <= 90) {
        char c = (char)key;
        memmove(inputBuffer, inputBuffer+1, 10);
        inputBuffer[10] = c; inputBuffer[11] = '\0';
        if (strcmp(inputBuffer, "IDDQD") == 0) {
            cheatActive = !cheatActive; inputBuffer[0] = '\0';
            if (cheatActive) {
                if (qteActive) { qteActive=false; activeDynamicEnemies--; }
                if (jumpscareActive) { jumpscareActive=false; activeDynamicEnemies--; }
                if (spamActive) { spamActive=false; activeDynamicEnemies--; }
                if (alignActive) { alignActive=false; activeDynamicEnemies--; }
            }
        }
        if (strcmp(inputBuffer, "DEVCOMMANDS") == 0 && currentScreen == SCREEN_MENU) {
            devMode = !devMode; inputBuffer[0] = '\0';
        }
    }
}

void UpdateGenerator(void) {
    if (cheatActive) { generatorCharge = 100.0f; generatorWarning = false; return; }
    float dt = GetFrameTime();
    if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) {
        generatorCharge += GENERATOR_CHARGE_RATE * dt;
        if (generatorCharge >= 100.0f) { generatorCharge = 100.0f; if (!gameLost) gameLost = true; }
    } else {
        generatorCharge -= GENERATOR_DISCHARGE_RATE * dt;
        if (generatorCharge <= 0.0f) { generatorCharge = 0.0f; if (!gameLost) gameLost = true; }
    }
    generatorWarning = (generatorCharge >= 90.0f);
}

void UpdatePhaseEnemy(void) {
    if (cheatActive) { phaseEnemyPhase = 1; phaseEnemyTimer = 0.0; return; }
    if (gameLost || gameWon) return;
    float dt = GetFrameTime(); phaseEnemyTimer += dt;
    if (phaseEnemyResetQueued) {
        if (generatorCharge >= 20.0f) { generatorCharge -= 20.0f; phaseEnemyPhase = 1; phaseEnemyTimer = 0.0; }
        phaseEnemyResetQueued = false;
    }
    if (phaseEnemyTimer >= PHASE_TRANSITION_TIME && phaseEnemyPhase < 5) {
        phaseEnemyPhase++; phaseEnemyTimer = 0.0;
        if (phaseEnemyPhase >= 5) { if (!gameLost) gameLost = true; }
    }
}

void UpdateQTE(void) {
    if (cheatActive) { if (qteActive) { activeDynamicEnemies--; } qteActive=false; qteSpawnCooldown=false; qteCooldownTimer=0; qteSpawnTimer=0; return; }
    if (gameLost || gameWon) return;
    float dt = GetFrameTime();
    if (qteSpawnCooldown) { qteCooldownTimer-=dt; if (qteCooldownTimer<=0) { qteSpawnCooldown=false; qteSpawnTimer=0; } return; }
    if (!qteActive) {
        qteSpawnTimer += dt;
        if (qteSpawnTimer >= 7.0) { qteSpawnTimer=0; if (activeDynamicEnemies<MAX_DYNAMIC_ENEMIES && (rand()%100)<20) {
            qteActive=true; qteCurrentIndex=0; qteTimeLeft=9.0;
            for(int i=0;i<5;i++) qteSequence[i]=rand()%4;
            qteWindowRect.x=rand()%(GetScreenWidth()-200); qteWindowRect.y=rand()%(GetScreenHeight()-100);
            qteWindowRect.width=200; qteWindowRect.height=80; activeDynamicEnemies++; } }
    } else {
        int k = GetKeyPressed();
        if (k==KEY_UP||k==KEY_DOWN||k==KEY_LEFT||k==KEY_RIGHT) {
            int exp = qteSequence[qteCurrentIndex];
            if ((k==KEY_UP&&exp==0)||(k==KEY_DOWN&&exp==1)||(k==KEY_LEFT&&exp==2)||(k==KEY_RIGHT&&exp==3)) {
                qteCurrentIndex++;
                if (qteCurrentIndex>=5) { qteActive=false; qteSpawnCooldown=true; qteCooldownTimer=10.0; activeDynamicEnemies--; }
            }
        }
        qteTimeLeft-=dt; if (qteTimeLeft<=0) { if(!gameLost)gameLost=true; qteActive=false; activeDynamicEnemies--; }
    }
}

void UpdateJumpscare(void) {
    if (cheatActive) { if (jumpscareActive) { activeDynamicEnemies--; } jumpscareActive=false; jumpscareSpawnCooldown=false; jumpscareCooldownTimer=0; jumpscareSpawnTimer=0; return; }
    if (gameLost || gameWon) return;
    float dt = GetFrameTime();
    if (jumpscareSpawnCooldown) { jumpscareCooldownTimer-=dt; if (jumpscareCooldownTimer<=0) { jumpscareSpawnCooldown=false; jumpscareSpawnTimer=0; } return; }
    if (!jumpscareActive) {
        jumpscareSpawnTimer+=dt;
        if (jumpscareSpawnTimer>=7.0) { jumpscareSpawnTimer=0; if (activeDynamicEnemies<MAX_DYNAMIC_ENEMIES && (rand()%100)<20) {
            jumpscareActive=true; jumpscarePhase=0; jumpscareTimer=0; activeDynamicEnemies++; } }
    } else {
        jumpscareTimer+=dt;
        switch (jumpscarePhase) {
            case 0: if (jumpscareTimer>=1.0) { jumpscarePhase=1; jumpscareTimer=0; } break;
            case 1: if (jumpscareTimer>=2.0) { jumpscarePhase=2; jumpscareTimer=0; } break;
            case 2: if (jumpscareTimer>=2.0) { if(!gameLost)gameLost=true; jumpscareActive=false; activeDynamicEnemies--; } break;
        }
    }
}

void UpdateSpam(void) {
    if (cheatActive) { if (spamActive) { activeDynamicEnemies--; } spamActive=false; spamSpawnCooldown=false; spamCooldownTimer=0; spamSpawnTimer=0; return; }
    if (gameLost || gameWon) return;
    float dt = GetFrameTime();
    if (spamSpawnCooldown) { spamCooldownTimer-=dt; if (spamCooldownTimer<=0) { spamSpawnCooldown=false; spamSpawnTimer=0; } return; }
    if (!spamActive) {
        spamSpawnTimer+=dt;
        if (spamSpawnTimer>=7.0) { spamSpawnTimer=0; if (activeDynamicEnemies<MAX_DYNAMIC_ENEMIES && (rand()%100)<20) {
            spamActive=true; spamCountdown=5; spamProgress=0; spamTickTimer=0; activeDynamicEnemies++; } }
    } else {
        if (IsKeyPressed(KEY_LEFT_ALT)||IsKeyPressed(KEY_RIGHT_ALT)) {
            spamProgress+=7; if (spamProgress>100) spamProgress=100;
            if (spamProgress>=100) { spamActive=false; spamSpawnCooldown=true; spamCooldownTimer=10.0; activeDynamicEnemies--; return; }
        }
        spamTickTimer+=dt;
        if (spamTickTimer>=1.0) { spamTickTimer-=1.0; spamCountdown--; if (spamCountdown<0) { if(!gameLost)gameLost=true; spamActive=false; activeDynamicEnemies--; } }
    }
}

void UpdateAlign(void) {
    if (cheatActive) { if (alignActive) { activeDynamicEnemies--; } alignActive=false; alignSpawnCooldown=false; alignCooldownTimer=0; alignSpawnTimer=0; return; }
    if (gameLost || gameWon) return;
    float dt = GetFrameTime();
    if (alignSpawnCooldown) { alignCooldownTimer-=dt; if (alignCooldownTimer<=0) { alignSpawnCooldown=false; alignSpawnTimer=0; } return; }
    if (!alignActive) {
        alignSpawnTimer+=dt;
        if (alignSpawnTimer>=7.0) { alignSpawnTimer=0; if (activeDynamicEnemies<MAX_DYNAMIC_ENEMIES && (rand()%100)<20) {
            alignActive=true; alignCountdown=8; alignTimer=0; for(int i=0;i<3;i++) alignCubesDone[i]=false; alignDragging=-1;
            Rectangle mr = { GetScreenWidth()/2-150, GetScreenHeight()/2-110, 300, 220 };
            float sh=50, pad=10, zw=60, zh=20, cs=20;
            for(int i=0;i<3;i++) {
                alignSubRects[i] = (Rectangle){ mr.x+pad, mr.y+60+i*(sh+5), mr.width-2*pad, sh };
                float zy = alignSubRects[i].y + rand()%(int)(sh-zh);
                alignZoneRects[i] = (Rectangle){ alignSubRects[i].x+alignSubRects[i].width-zw-5, zy, zw, zh };
                float cy;
                do { cy = alignSubRects[i].y + rand()%(int)(sh-cs); }
                while (cy+cs > alignZoneRects[i].y-2 && cy < alignZoneRects[i].y+zh+2);
                alignCubeRects[i] = (Rectangle){ alignSubRects[i].x+5, cy, cs, cs };
            }
            activeDynamicEnemies++; } }
    } else {
        alignTimer+=dt;
        if (alignTimer>=1.0) { alignTimer-=1.0; alignCountdown--; if (alignCountdown<0) { if(!gameLost)gameLost=true; alignActive=false; activeDynamicEnemies--; return; } }
        Vector2 m = GetMousePosition();
        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
            if (alignDragging==-1) { for(int i=0;i<3;i++) if(!alignCubesDone[i]&&CheckCollisionPointRec(m,alignCubeRects[i])) { alignDragging=i; break; } }
            if (alignDragging!=-1) {
                Rectangle* cu = &alignCubeRects[alignDragging];
                cu->y = m.y - cu->height/2;
                if (cu->y < alignSubRects[alignDragging].y) cu->y = alignSubRects[alignDragging].y;
                if (cu->y+cu->height > alignSubRects[alignDragging].y+alignSubRects[alignDragging].height) cu->y = alignSubRects[alignDragging].y+alignSubRects[alignDragging].height - cu->height;
                if (CheckCollisionRecs(*cu, alignZoneRects[alignDragging])) { alignCubesDone[alignDragging]=true; alignDragging=-1; }
            }
        } else alignDragging=-1;
        if (alignCubesDone[0]&&alignCubesDone[1]&&alignCubesDone[2]) { alignActive=false; alignSpawnCooldown=true; alignCooldownTimer=10.0; activeDynamicEnemies--; }
    }
}

void DrawJumpscare(void) {
    if (!jumpscareActive) return;
    int w=GetScreenWidth(), h=GetScreenHeight();
    if (jumpscarePhase==0) DrawText("PREPARE", w/2-MeasureText("PREPARE",40)/2, h-50, 40, WHITE);
    else {
        float sz = (jumpscarePhase==1) ? (jumpscareTimer/2.0)*w*0.7f : w*0.7f+(jumpscareTimer/2.0)*w*0.2f;
        float x=w/2-sz/2, y=h/2-sz/2;
        DrawRectangle((int)x,(int)y,(int)sz,(int)sz, jumpscarePhase==1?YELLOW:RED);
    }
}

void DrawSpam(void) {
    if (!spamActive) return;
    Rectangle r = { GetScreenWidth()/2-150, GetScreenHeight()/2-60, 300, 120 };
    DrawRectangleRec(r, Fade(BLACK,0.85f)); DrawRectangleLinesEx(r,2,RED);
    DrawText("SPAM ALT!", r.x+r.width/2-MeasureText("SPAM ALT!",24)/2, r.y+5, 24, WHITE);
    char buf[8]; sprintf(buf,"%d",spamCountdown);
    DrawText(buf, r.x+r.width/2-MeasureText(buf,40)/2, r.y+35, 40, spamCountdown<=2?RED:YELLOW);
    int bx=r.x+15, by=r.y+85, bw=r.width-30, bh=10;
    DrawRectangle(bx,by,bw,bh,DARKGRAY);
    DrawRectangle(bx,by,bw*(spamProgress/100.0f),bh,GREEN);
    DrawRectangleLines(bx,by,bw,bh,WHITE);
}

void DrawAlign(void) {
    if (!alignActive) return;
    Rectangle r = { GetScreenWidth()/2-150, GetScreenHeight()/2-110, 300, 220 };
    DrawRectangleRec(r, Fade(BLACK,0.85f)); DrawRectangleLinesEx(r,2,RED);
    DrawText("ALIGN", r.x+r.width/2-MeasureText("ALIGN",24)/2, r.y+5, 24, WHITE);
    char buf[8]; sprintf(buf,"%d",alignCountdown);
    DrawText(buf, r.x+r.width/2-MeasureText(buf,40)/2, r.y+30, 40, RED);
    for (int i=0;i<3;i++) {
        DrawRectangleRec(alignSubRects[i], Fade(DARKGRAY,0.5f)); DrawRectangleLinesEx(alignSubRects[i],1,LIGHTGRAY);
        DrawRectangleRec(alignZoneRects[i], GREEN);
        DrawRectangleRec(alignCubeRects[i], alignCubesDone[i]?GRAY:RED);
    }
}

void UpdateMenu(void) {
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        if (IsMouseOverButton(newGameBtn)) currentScreen=SCREEN_DIFFICULTY;
        if (IsMouseOverButton(settingsBtn)) currentScreen=SCREEN_SETTINGS;
        if (IsMouseOverButton(statsBtn)) currentScreen=SCREEN_STATS;
        if (IsMouseOverButton(exitBtn)) CloseWindow();
    }
}

void DrawMenu(void) {
    DrawText("SWEEPER", GetScreenWidth()/2-MeasureText("SWEEPER",50)/2,60,50,WHITE);
    DrawButton(newGameBtn); DrawButton(settingsBtn); DrawButton(statsBtn); DrawButton(exitBtn);
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
        if (fullscreen) { int mon = GetCurrentMonitor(); SetWindowSize(GetMonitorWidth(mon), GetMonitorHeight(mon)); ToggleFullscreen(); }
        else { ToggleFullscreen(); SetWindowSize(800,600); }
        RecalculateGrid(); RecalculateButtons();
    }
    if (!gameLost && !gameWon) {
        UpdateGenerator(); UpdatePhaseEnemy();
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            Vector2 m = GetMousePosition();
            if (CheckCollisionPointRec(m, phaseEnemyResetBtn)) phaseEnemyResetQueued = true;
        }
        UpdateQTE(); UpdateJumpscare(); UpdateSpam(); UpdateAlign();
        if (jumpscareActive && jumpscarePhase==2 && IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) {
            jumpscareActive=false; jumpscareSpawnCooldown=true; jumpscareCooldownTimer=10.0; activeDynamicEnemies--;
        }
        // Dev-спавн
        if (devMode) {
            if (IsKeyPressed(KEY_F1)) SpawnQTE();
            else if (IsKeyPressed(KEY_F2)) SpawnJumpscare();
            else if (IsKeyPressed(KEY_F3)) SpawnSpam();
            else if (IsKeyPressed(KEY_F4)) SpawnAlign();
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
            Vector2 m = GetMousePosition();
            if (CheckCollisionPointRec(m, smileyRect)) { ResetGame(); return; }
        }
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) || IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) {
            Vector2 m = GetMousePosition();
            int col = (int)((m.x-gridX)/cellSize), row = (int)((m.y-gridY)/cellSize);
            if (row>=0 && row<gameBoard.rows && col>=0 && col<gameBoard.cols) {
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    if (!gameBoard.firstClickDone) { BoardPlaceMines(&gameBoard,row,col); BoardCalculateNumbers(&gameBoard); timerStarted=true; startTime=GetTime(); }
                    bool hit = BoardReveal(&gameBoard,row,col);
                    if (hit) gameLost=true; else gameWon=BoardCheckVictory(&gameBoard);
                } else if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) {
                    if (!gameBoard.firstClickDone) return;
                    Cell* c = &gameBoard.cells[row][col];
                    if (c->state==CELL_HIDDEN) { c->state=CELL_FLAGGED; flagCount++; }
                    else if (c->state==CELL_FLAGGED) { c->state=CELL_HIDDEN; flagCount--; }
                }
            }
        }
    }
    if (timerStarted && !gameLost && !gameWon) { timer=(int)(GetTime()-startTime); if (timer>999) timer=999; }
}

void DrawGameplay(void) {
    int ml = gameConfig.mines - flagCount; if (ml<0) ml=0;
    char buf[32]; sprintf(buf,"Mines: %d",ml); DrawText(buf,20,10,24,WHITE);
    sprintf(buf,"Time: %d",timer); DrawText(buf,150,10,24,WHITE);
    smileyRect = (Rectangle){ GetScreenWidth()/2-20, 5, 40, 40 };
    DrawRectangleRec(smileyRect, gameLost?RED:gameWon?GREEN:YELLOW);
    DrawRectangleLinesEx(smileyRect,2,WHITE);
    DrawText(":)", smileyRect.x+8, smileyRect.y+6, 24, BLACK);
    DrawRectangle(gridX-5, gridY-5, gameBoard.cols*cellSize+10, gameBoard.rows*cellSize+10, bgColor);
    for (int r=0; r<gameBoard.rows; r++) for (int c=0; c<gameBoard.cols; c++) {
        Rectangle cr = { gridX+c*cellSize, gridY+r*cellSize, cellSize, cellSize };
        Cell cell = gameBoard.cells[r][c];
        if (cell.state==CELL_HIDDEN) {
            DrawRectangleRec(cr, cellColor); DrawRectangleLines(cr.x,cr.y,cellSize,cellSize,cellHiddenLine);
            if (cheatActive) DrawCircle(cr.x+cellSize/2, cr.y+cellSize/2, cellSize/6, cell.hasMine?RED:GREEN);
        } else if (cell.state==CELL_REVEALED) {
            DrawRectangleRec(cr, WHITE); DrawRectangleLines(cr.x,cr.y,cellSize,cellSize,GRAY);
            if (cell.hasMine) DrawText("*", cr.x+cellSize/4, cr.y+cellSize/4, cellSize, RED);
            else if (cell.adjacentMines>0) { char n[4]; sprintf(n,"%d",cell.adjacentMines); DrawText(n, cr.x+cellSize/4, cr.y+cellSize/4, cellSize/2, DARKBLUE); }
        } else if (cell.state==CELL_FLAGGED) {
            DrawRectangleRec(cr, cellColor); DrawRectangleLines(cr.x,cr.y,cellSize,cellSize,cellHiddenLine);
            DrawText("F", cr.x+cellSize/4, cr.y+cellSize/4, cellSize/2, RED);
        }
    }
    Rectangle gen = {10, GetScreenHeight()-70,150,60};
    DrawRectangleRec(gen, Fade(BLACK,0.7f)); DrawRectangleLinesEx(gen,2,WHITE);
    Color cc = generatorCharge>80?RED:generatorCharge>40?YELLOW:GREEN;
    if (generatorWarning && ((int)(GetTime()*4)%2)==0) DrawRectangleRec(gen, Fade(RED,0.3f));
    DrawText("GENERATOR", gen.x+10, gen.y+5, 14, LIGHTGRAY);
    sprintf(buf,"%.0f%%",generatorCharge); DrawText(buf, gen.x+gen.width/2-MeasureText(buf,20)/2, gen.y+25, 20, cc);
    Rectangle ph = {10, GetScreenHeight()-150,150,60};
    DrawRectangleRec(ph, Fade(BLACK,0.7f)); DrawRectangleLinesEx(ph,2,WHITE);
    DrawText("PHASE", ph.x+10, ph.y+5, 14, LIGHTGRAY);
    sprintf(buf,"%d/5",phaseEnemyPhase); DrawText(buf, ph.x+ph.width/2-MeasureText(buf,20)/2, ph.y+25, 20, WHITE);
    if (phaseEnemyPhase<5 && !gameLost && !gameWon) {
        float p = phaseEnemyTimer/PHASE_TRANSITION_TIME; int bw=ph.width-20, bx=ph.x+10, by=ph.y+48;
        DrawRectangle(bx,by,bw,4,DARKGRAY); DrawRectangle(bx,by,bw*p,4,RED);
    }
    DrawRectangleRec(phaseEnemyResetBtn, GRAY); DrawRectangleLinesEx(phaseEnemyResetBtn,1,WHITE);
    DrawText("<<", phaseEnemyResetBtn.x+12, phaseEnemyResetBtn.y+5, 18, WHITE);
    if (qteActive) {
        DrawRectangleRec(qteWindowRect, Fade(BLACK,0.85f)); DrawRectangleLinesEx(qteWindowRect,2,RED);
        float my = qteWindowRect.y+qteWindowRect.height/2;
        DrawLine(qteWindowRect.x,my, qteWindowRect.x+qteWindowRect.width,my,RED);
        char p[16]; sprintf(p,"%d/5",qteCurrentIndex);
        DrawText(p, qteWindowRect.x+qteWindowRect.width/2-MeasureText(p,24)/2, qteWindowRect.y+5, 24, YELLOW);
        const char* d[] = {"Press Up","Press Down","Press Left","Press Right"};
        int id = qteCurrentIndex<5 ? qteSequence[qteCurrentIndex] : 0;
        DrawText(d[id], qteWindowRect.x+qteWindowRect.width/2-MeasureText(d[id],16)/2, my+10, 16, WHITE);
        float fr = qteTimeLeft/9.0f; int by = qteWindowRect.y+qteWindowRect.height-6;
        DrawRectangle(qteWindowRect.x+2,by,qteWindowRect.width-4,4,DARKGRAY);
        DrawRectangle(qteWindowRect.x+2,by,(qteWindowRect.width-4)*fr,4,RED);
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
