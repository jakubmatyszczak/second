#include <unistd.h>
#include "engine/engine.cpp"
#include "engine/basic.cpp"
#include "entities.cpp"
int main(void)
{
	InitWindow(800, 600, "DUPA");
	bool	  done			= false;
	Texture2D texture		= LoadTexture("art/dude_ss.png");
	Texture2D texture2		= LoadTexture("art/table_tex.png");
	Texture2D textureShadow = LoadTexture("art/shadow_tex.png");
	Dude	  dude;
	Table	  table;
	Table	  table2;
	dude.init(texture, textureShadow, {100, 100});
	table.init(texture2, {110, 110});
	table2.init(texture2, {130, 130});
	GLOBAL.camera.zoom	 = 6.f;
	GLOBAL.camera.offset = {400, 300};
	while (!done)
	{
		entities.refresh();

		f32 dt = GetFrameTime();
		dude.input(IsKeyDown(KEY_K),
				   IsKeyDown(KEY_J),
				   IsKeyDown(KEY_H),
				   IsKeyDown(KEY_L),
				   IsKeyPressed(KEY_E),
				   IsKeyDown(KEY_W));
		dude.update();
		table.update(dt);
		table2.update(dt);

		BeginDrawing();
		{
			ClearBackground(RAYWHITE);
			GLOBAL.camera.target = dude.e->pos.toVector2();
			BeginMode2D(GLOBAL.camera);
			{
				dude.draw();
				table.draw();
				table2.draw();
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
