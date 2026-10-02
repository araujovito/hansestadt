#include "raylib.h"

int main() {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    InitWindow(1280, 720, "Hansestadt");

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(Color{ 74, 110, 60, 255 });
        DrawText("Hansestadt", 20, 20, 30, RAYWHITE);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
