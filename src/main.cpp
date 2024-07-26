#include <unistd.h>
#include "engine/engine.cpp"
#include "engine/basic.cpp"
#include "entities.cpp"
int main(void)
{
	InitWindow(800, 600, "DUPA");
	bool	  done	   = false;
	Texture2D texture  = LoadTexture("art/dude_ss.png");
	Texture2D texture2 = LoadTexture("art/table_tex.png");
	Dude	  dude;
	Table	  table;
	dude.init(texture, {100, 100});
	table.init(texture2, {110, 110});
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
				   IsKeyPressed(KEY_E));
		dude.update();
		table.update(dt);

		BeginDrawing();
		{
			GLOBAL.camera.target = dude.e->pos.toVector2();
			BeginMode2D(GLOBAL.camera);
			ClearBackground(RAYWHITE);
			dude.draw();
			table.draw();
			EndMode2D();
		}
		EndDrawing();
		usleep(16000);
		if (IsKeyPressed(KEY_Q))
			done = true;
	}
	return 0;
}
