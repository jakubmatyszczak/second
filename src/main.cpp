#include <unistd.h>
#include "engine/engine.cpp"
#include "engine/basic.cpp"
#include "entities.cpp"
#include "levels.cpp"

int main(void)
{
	InitWindow(800, 600, "SECOND");
	SetTargetFPS(60.f);
	InitAudioDevice();
	bool	  done			= false;
	bool	  levelEditor	= false;
	Texture2D texture		= LoadTexture("res/art/dude_ss.png");
	Texture2D texture2		= LoadTexture("res/art/table_tex.png");
	Texture2D textureShadow = LoadTexture("res/art/shadow_tex.png");
	Sound	  soundJump		= LoadSound("res/sound/jump.wav");
	Sound	  soundWham		= LoadSound("res/sound/punch.wav");
	Level	  l1;

	Dude  dude;
	Table table;
	Table table2;
	Table table3;
	Table table4;
    l1.appendVertex({0.f, 0.f});
    l1.appendVertex({140.f, 0.f});
    l1.appendVertex({120.f, 110.f});
    l1.appendVertex({0.f, 150.f});
	dude.init(texture, textureShadow, soundJump, {100, 100});
	table.init(texture2, textureShadow, soundWham, {110, 110});
	table2.init(texture2, textureShadow, soundWham, {130, 111});
	table3.init(texture2, textureShadow, soundWham, {50, 100});
	table4.init(texture2, textureShadow, soundWham, {90, 120});
	GLOBAL.camera.zoom	 = 6.f;
	GLOBAL.camera.offset = {400, 300};
	u32 frame			 = 0;
	while (!done)
	{
		v2 mousePosWorld = GetScreenToWorld2D(GetMousePosition(), GLOBAL.camera);
		frame++;
		entities.refresh();

		f32 dt = GetFrameTime();
		dt	   = 0.016f;

		if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_E))
			levelEditor = !levelEditor;

		GLOBAL.camera.zoom = 6.f;
		if (levelEditor)
		{
			GLOBAL.camera.zoom = 2.f;
			if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
				l1.appendVertex(mousePosWorld);
		}
		else
		{
			dude.input(IsKeyDown(KEY_K),
					   IsKeyDown(KEY_J),
					   IsKeyDown(KEY_H),
					   IsKeyDown(KEY_L),
					   IsKeyPressed(KEY_E),
					   IsKeyDown(KEY_W));
			entities.updateAll(dt);
		}

		BeginDrawing();
		{
			ClearBackground(RAYWHITE);
			GLOBAL.camera.target = dude.e->pos.toVector2();
			BeginMode2D(GLOBAL.camera);
			{
				l1.draw();
				entities.drawAll();
				v2			   cv;
				BoundingCircle c = {dude.e->pos + v2(0,2), 4};
				if (l1.collidesWithTerrainBorder(c, cv))
				{
					dude.e->pos = dude.e->pos - cv;
					DrawCircleV(c.position.toVector2(), 4, {255, 0, 0, 150});
					DrawCircleV((c.position + cv).toVector2(), 1, {0, 255, 0, 150});
					DrawLineV((c.position + cv).toVector2(), c.position.toVector2(), GREEN);
				}
			}
			EndMode2D();
		}
		dude.drawOverlay();
		if (levelEditor)
		{
			DrawText("EDITOR", 10, 10, 20, BLACK);
			DrawText("EDITOR", 12, 12, 20, RED);
		}
		EndDrawing();
		if (IsKeyPressed(KEY_Q))
			done = true;
	}
	return 0;
}
