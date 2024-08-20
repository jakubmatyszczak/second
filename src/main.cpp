#include "raylib.h"

int main(void)
{
	InitWindow(800, 600, "game");
	SetTargetFPS(60);
	while (1)
	{
		ClearBackground(RAYWHITE);
		BeginDrawing();
		DrawText("Hello Raylib!", 50, 50, 30, RED);
		EndDrawing();
	}
}
