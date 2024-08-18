#include <unistd.h>
#include "engine/engine.cpp"
#include "engine/basic.cpp"
#include "engine/fileio.cpp"
#include "entities.cpp"
#include "levels.cpp"

struct SaveState
{
	EngineGlobals globals;
	Entities	  entities;
	u32			  pDudeInstance;
	Level		  levels[32];
};
void saveGame(SaveState& save, u32 pDudeInstance)
{
	save.globals	   = GLOBAL;
	save.entities	   = entities;
	save.pDudeInstance = pDudeInstance;
	fileio::saveRawFile("1.save", &save, sizeof(save));
}
bool loadGame(SaveState& game)
{
	SaveState loaded;
	i32		  ret = fileio::loadRawFile("1.save", sizeof(loaded), &loaded);
	if (ret < 0)
		return false;
	else if (ret != sizeof(loaded))
		exitWithMessage("Failed to load save!");
	entities		   = loaded.entities;
	GLOBAL			   = loaded.globals;
	game.pDudeInstance = loaded.pDudeInstance;
	memcpy(game.levels, loaded.levels, sizeof(loaded.levels));
	return true;
}

int main(void)
{
	SaveState save;
	InitWindow(800, 600, "SECOND");
	SetTargetFPS(60.f);
	InitAudioDevice();
	bool done		 = false;
	bool levelEditor = false;
	content.loadTexture("res/art/dude_ss.png", Content::TEX_DUDE);
	content.loadTexture("res/art/table_tex.png", Content::TEX_TABLE);
	content.loadTexture("res/art/key.png", Content::TEX_KEY);
	content.loadTexture("res/art/shadow_tex.png", Content::TEX_SHADOW);
	content.loadTexture("res/art/level1.png", Content::TEX_LEVEL1);
	content.loadTexture("res/art/hole_ss.png", Content::TEX_HOLE);
	content.loadSound("res/sound/jump.wav", Content::SOUND_JUMP);
	content.loadSound("res/sound/punch.wav", Content::SOUND_WHAM);

	Dude::init();
	Table::init();
	Key::init();
	Hole::init();
	save.levels[0].init(v2());
	save.levels[1].init({500.0, 0.});

	Dude& dude = Dude::getRef(dude.add({50, 50}));
	LoadLevel1(save.levels[0]);
	LoadLevel2(save.levels[1]);
	Hole::connect(save.levels[0].pHole[0], save.levels[1].pHole[0]);

	loadGame(save);

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
		if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_W))
			GLOBAL.drawDebugCollision = !GLOBAL.drawDebugCollision;
		if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_S))
			saveGame(save, dude.pEntity);

		GLOBAL.camera.zoom = 6.f;
		if (levelEditor)
		{
			GLOBAL.camera.zoom = 2.f;
		}
		else
		{
			dude.input(IsKeyDown(KEY_K),
					   IsKeyDown(KEY_J),
					   IsKeyDown(KEY_H),
					   IsKeyDown(KEY_L),
					   IsKeyPressed(KEY_E),
					   IsKeyDown(KEY_W));

			// collisions
			{
				// terrain
				for (u32 i = 0; i < entities.maxEntities; i++)
					if (entities.active[i] && entities.collidesTerrain[i])
					{
						Entity& e = entities.instances[i];
						v2		collisionVector;
						Level&	l1 = save.levels[0];
						Level&	l2 = save.levels[0];
						if (l1.collidesWithTerrainBorder(e.iData.boundingCircle, collisionVector))
							e.vel -= collisionVector;
						if (l2.collidesWithTerrainBorder(e.iData.boundingCircle, collisionVector))
							e.vel -= collisionVector;
					}
				// group 1
				Entity* group1[entities.maxEntities] = {};
				u32		nGroup1						 = 0;
				for (u32 i = 0; i < entities.maxEntities; i++)
					if (entities.active[i] && entities.collidesGroup1[i])
						group1[nGroup1++] = &entities.instances[i];
				for (u32 i = 0; i < nGroup1; i++)
					for (u32 j = 0; j < nGroup1; j++)
					{
						if (i == j)
							continue;
						Entity& e1				= *group1[i];
						Entity& e2				= *group1[j];
						v2		collisionVector = {};
						if (e1.iData.boundingCircle.computeCollision(e2.iData.boundingCircle,
																	 collisionVector))
							e1.vel -= collisionVector / 2;
					}
			}
			entities.updateAll(dt);
		}

		v2 dudePos = dude.getEntity().pos;
		BeginDrawing();
		{
			ClearBackground(SKYBLUE);
			GLOBAL.camera.target =
				(v2(GLOBAL.camera.target) + ((dudePos - v2(GLOBAL.camera.target)) * 0.15f))
					.toVector2();
			BeginMode2D(GLOBAL.camera);
			{
				save.levels[0].draw();
				save.levels[1].draw();
				entities.drawAll();
			}
			EndMode2D();
		}
		dude.drawOverlay();
		DrawText(TextFormat("DUDE: %.f,%.f", dudePos.x, dudePos.y), 10, 10, 20, BLACK);
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
