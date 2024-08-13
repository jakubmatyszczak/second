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
	Texture2D textureLevel1 = LoadTexture("res/art/level1.png");
	Sound	  soundJump		= LoadSound("res/sound/jump.wav");
	Sound	  soundWham		= LoadSound("res/sound/punch.wav");
	Level	  l1;

	Dude  dude;
	Table table;
	Table table2;
	Table table3;
	Table table4;
	l1.init(textureLevel1, v2(), 1.f, 1.f);
	LoadLevel1(l1);
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

			// collisions
			{
				for (u32 i = 0; i < entities.maxEntities; i++)
					if (entities.active[i] && entities.collidesTerrain[i])
					{
						Entity&				   e  = entities.instances[i];
						InteractionProperties& ip = *(InteractionProperties*)e.properties;
						v2					   collisionVector;
						l1.collidesWithTerrainBorder(ip.boundingCircle, collisionVector);
                        e.pos -= collisionVector;
					}
			}
		}

		BeginDrawing();
		{
			ClearBackground(SKYBLUE);
			GLOBAL.camera.target = dude.e->pos.toVector2();
			BeginMode2D(GLOBAL.camera);
			{
				l1.draw();
				entities.drawAll();
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
