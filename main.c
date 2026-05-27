#include "raylib.h"

int main(void)
{
    InitWindow(800, 600, "Sweeper");
    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(DARKGRAY);
        DrawText("Hello, Sweeper!", 300, 200, 30, WHITE);
        EndDrawing();
    }
    CloseWindow();
    return 0;
}
