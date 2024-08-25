#include <unistd.h>
#include "engine/engine.cpp"
#include "engine/basic.cpp"
#include "engine/fileio.cpp"
#include "entities.cpp"
#include "levels.cpp"

struct WorldState
{
	EngineGlobals globals;
	Entities	  entities;
	Level		  levels[32];
};
void saveGame(WorldState& save, u32 pDudeInstance)
{
	save.globals  = GLOBAL;
	save.entities = entities;
	fileio::saveRawFile("1.save", &save, sizeof(save));
}
bool loadGame(WorldState& game)
{
	WorldState loaded;
	i32		  ret = fileio::loadRawFile("1.save", sizeof(loaded), &loaded);
	if (ret < 0)
		return false;
	else if (ret != sizeof(loaded))
		exitWithMessage("Failed to load save!");
	entities = loaded.entities;
	GLOBAL	 = loaded.globals;
	memcpy(game.levels, loaded.levels, sizeof(loaded.levels));
	return true;
}

int main(void)
{
	WorldState worldState;
	InitWindow(800, 600, "SECOND");
	SetTargetFPS(60.f);
	InitAudioDevice();
	bool done		 = false;
	bool levelEditor = false;
    loadContent(content);

	Dude::init();
	Table::init();
	Key::init();
	Hole::init();
    Baddie::init();
	worldState.levels[0].init(v2(), Content::TEX_LEVEL1);
	worldState.levels[1].init({200.0, 0.}, Content::TEX_LEVEL1);
	worldState.levels[2].init({0.f, 200.}, Content::TEX_LEVEL2);

	Dude& dude = Dude::getRef(dude.add({50, 50}));
	LoadLevel1(worldState.levels[0]);
	LoadLevel2(worldState.levels[1]);
	LoadLevel3(worldState.levels[2]);
	Hole::connect(worldState.levels[0].pHole[0], worldState.levels[1].pHole[0]);
	Hole::connect(worldState.levels[0].pHole[1], worldState.levels[2].pHole[0]);

	loadGame(worldState);

	GLOBAL.camera.zoom	 = 6.f;
	GLOBAL.camera.offset = {400, 300};
	u32 frame			 = 0;
	while (!done)
	{
		v2 mousePosWorld  = GetScreenToWorld2D(GetMousePosition(), GLOBAL.camera);
		v2 mousePosWindow = GetMousePosition();
		frame++;
		entities.refresh();

		f32 dt = GetFrameTime();
		dt	   = 0.016f;

		if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_E))
			levelEditor = !levelEditor;
		if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_W))
			GLOBAL.drawDebugCollision = !GLOBAL.drawDebugCollision;
		if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_S))
			saveGame(worldState, dude.pEntity);

		GLOBAL.camera.zoom = 6.f;
		if (levelEditor)
		{
			GLOBAL.camera.zoom = 2.f;
			if (mousePosWindow.x < GLOBAL.screen.width * 0.1)
				GLOBAL.camera.target.x -= 3;
			if (mousePosWindow.x > GLOBAL.screen.width * 0.9)
				GLOBAL.camera.target.x += 3;
			if (mousePosWindow.y < GLOBAL.screen.height * 0.1)
				GLOBAL.camera.target.y -= 3;
			if (mousePosWindow.y > GLOBAL.screen.height * 0.9)
				GLOBAL.camera.target.y += 3;
			if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
			{
				BoundingCircle mouseBc = {mousePosWorld, 1};
				v2			   dummy;
				for (u32 i = 0; i < 32; i++)
				{
					if (!worldState.levels[i].bc.computeCollision(mouseBc, dummy))
						continue;
					worldState.levels[i].appendVertexFromMouse(mouseBc.pos);
					break;
				}
			}
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
						for (u32 j = 0; j < 32; j++)
						{
							Level& l = worldState.levels[j];
							if (l.collidesWithTerrainBorder(e.iData.boundingCircle,
															collisionVector))
								e.vel -= collisionVector;
						}
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
							e1.vel -= collisionVector * e1.vel.getLength() * 1.25f;
					}
				// group 2
				Entity* group2[entities.maxEntities] = {};
				u32		nGroup2						 = 0;
				for (u32 i = 0; i < entities.maxEntities; i++)
					if (entities.active[i] && entities.collidesGroup2[i])
						group2[nGroup2++] = &entities.instances[i];
				for (u32 i = 0; i < nGroup2; i++)
					for (u32 j = 0; j < nGroup2; j++)
					{
						if (i == j)
							continue;
						Entity& e1				= *group2[i];
						Entity& e2				= *group2[j];
						v2		collisionVector = {};
						if (e1.iData.boundingCircle.computeCollision(e2.iData.boundingCircle,
																	 collisionVector))
							e1.vel -= collisionVector * e1.vel.getLength() * 1.25f;
					}
			}
			entities.updateAll(dt);
		}

		v2 dudePos = dude.getEntity().pos;
		BeginDrawing();
		{
			ClearBackground(SKYBLUE);
			if (!levelEditor)
				GLOBAL.camera.target =
					(v2(GLOBAL.camera.target) + ((dudePos - v2(GLOBAL.camera.target)) * 0.15f))
						.toVector2();
			BeginMode2D(GLOBAL.camera);
			{
				for (u32 i = 0; i < 32; i++)
					worldState.levels[i].draw();
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
