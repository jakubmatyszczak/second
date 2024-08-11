#include <unistd.h>
#include "engine/engine.cpp"
#include "engine/basic.cpp"
#include "entities.cpp"
int main(void)
{
	InitWindow(800, 600, "DUPA");
	SetTargetFPS(60.f);
	InitAudioDevice();
	bool	  done			= false;
	Texture2D texture		= LoadTexture("res/art/dude_ss.png");
	Texture2D texture2		= LoadTexture("res/art/table_tex.png");
	Texture2D textureShadow = LoadTexture("res/art/shadow_tex.png");
	Sound	  soundJump		= LoadSound("res/sound/jump.wav");
	Sound	  soundWham		= LoadSound("res/sound/punch.wav");
	Dude	  dude;
	Table	  table;
	Table	  table2;
	Table	  table3;
	Table	  table4;
	dude.init(texture, textureShadow, soundJump, {100, 100});
	table.init(texture2, textureShadow, soundWham, {110, 110});
	table2.init(texture2, textureShadow, soundWham, {130, 130});
	table3.init(texture2, textureShadow, soundWham, {50, 100});
	table4.init(texture2, textureShadow, soundWham, {90, 120});
	GLOBAL.camera.zoom	 = 6.f;
	GLOBAL.camera.offset = {400, 300};
	u32 frame			 = 0;
	while (!done)
	{
		frame++;
		entities.refresh();

		f32 dt = GetFrameTime();
		dt	   = 0.016f;
		dude.input(IsKeyDown(KEY_K),
				   IsKeyDown(KEY_J),
				   IsKeyDown(KEY_H),
				   IsKeyDown(KEY_L),
				   IsKeyPressed(KEY_E),
				   IsKeyDown(KEY_W));
		entities.updateAll(dt);

		BeginDrawing();
		{
			ClearBackground(RAYWHITE);
			GLOBAL.camera.target = dude.e->pos.toVector2();
			BeginMode2D(GLOBAL.camera);
			{
				entities.drawAll();
			}
			EndMode2D();
		}
		dude.drawOverlay();
		EndDrawing();
		usleep(16000);
		if (IsKeyPressed(KEY_Q))
			done = true;
	}
	return 0;
}
