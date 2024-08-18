#include <unistd.h>
#include "engine/engine.cpp"
#include "engine/basic.cpp"
#include "entities.cpp"
#include "levels.cpp"

struct SaveStae
{
	EngineGlobals globals;
	Entities	  entities;
	Dude		  dude;
};

int main(void)
{
	InitWindow(800, 600, "SECOND");
	SetTargetFPS(60.f);
	InitAudioDevice();
	bool	  done			= false;
	bool	  levelEditor	= false;
	Texture2D texture		= LoadTexture("res/art/dude_ss.png");
	Texture2D texture2		= LoadTexture("res/art/table_tex.png");
	Texture2D textureKey	= LoadTexture("res/art/key.png");
	Texture2D textureShadow = LoadTexture("res/art/shadow_tex.png");
	Texture2D textureLevel1 = LoadTexture("res/art/level1.png");
	Texture2D textureHole	= LoadTexture("res/art/hole_ss.png");
	Sound	  soundJump		= LoadSound("res/sound/jump.wav");
	Sound	  soundWham		= LoadSound("res/sound/punch.wav");
	Level	  l1;
	Level	  l2;

	l1.init(textureLevel1, v2());
	l2.init(textureLevel1, {500.0, 0.});
	LoadLevel1(l1);
	LoadLevel1(l2);
	Dude& dude = Dude::init(texture, textureShadow, soundJump, {80, 80});
	Table::init(texture2, textureShadow, soundWham, {30, 60});
	Table::init(texture2, textureShadow, soundWham, {60, 25});
	Table::init(texture2, textureShadow, soundWham, {50, 85});
	Table::init(texture2, textureShadow, soundWham, {90, 90});
	Hole& hole	= Hole::init(textureHole, {50, 50});
	Hole& hole2 = Hole::init(textureHole, {550, 50});

	hole.connect(hole2);

	Key::init(textureKey, textureShadow, {570, 80});

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

			// collisions
			{
				// terrain
				for (u32 i = 0; i < entities.maxEntities; i++)
					if (entities.active[i] && entities.collidesTerrain[i])
					{
						Entity&			 e	= entities.instances[i];
						v2				 collisionVector;
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
						Entity&			 e1				 = *group1[i];
						Entity&			 e2				 = *group1[j];
						v2				 collisionVector = {};
						if (e1.iData.boundingCircle.computeCollision(e2.iData.boundingCircle,
																 collisionVector))
							e1.vel -= collisionVector / 2;
					}
			}
			entities.updateAll(dt);
		}

		BeginDrawing();
		{
			ClearBackground(SKYBLUE);

			GLOBAL.camera.target =
				(v2(GLOBAL.camera.target) + ((dude.e->pos - v2(GLOBAL.camera.target)) * 0.15f))
					.toVector2();
			BeginMode2D(GLOBAL.camera);
			{
				l1.draw();
				l2.draw();
				entities.drawAll();
			}
			EndMode2D();
		}
		dude.drawOverlay();
		DrawText(TextFormat("DUDE: %.f,%.f", dude.e->pos.x, dude.e->pos.y), 10, 10, 20, BLACK);
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
