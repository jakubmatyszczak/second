#include "engine/fileio.cpp"
#include "entities.cpp"
#include "level.cpp"

Entities   ENTITIES;
WorldState WORLD;

bool canClimb(v3i& pos)
{
	Level& lcurrent = WORLD.level[pos.z];
	Level& labove	= WORLD.level[pos.z + 1];
	if (lcurrent.containsXYZ(pos) && lcurrent.getTileAt(pos).type == Level::Tile::EMPTY)
		if (labove.containsXY(pos))
		{
			Level::Tile& tileAbove = labove.getTileAt(pos);
			if (tileAbove.type == Level::Tile::EMPTY)
				return true;
		}
	return false;
}
bool canGoDown(v3i& pos)
{
	Level& lcurrent = WORLD.level[pos.z];
	Level& lbelow	= WORLD.level[pos.z - 1];
	if (lcurrent.containsXYZ(pos) && lcurrent.getTileAt(pos).type == Level::Tile::EMPTY)
		if (lbelow.containsXY(pos))
		{
			Level::Tile& tileBelow = lbelow.getTileAt(pos);
			if (tileBelow.type == Level::Tile::EMPTY)
				return true;
		}
	return false;
}

void saveGame()
{
	WORLD.entities = ENTITIES;
	WORLD.globals  = G;
	if (fileio::saveRawFile("1.save", &WORLD, sizeof(WORLD)))
		printf("GAME SAVED!\n");
}
bool loadGame()
{
	WorldState loaded;
	s32		   ret = fileio::loadRawFile("1.save", sizeof(loaded), &loaded);
	if (ret < 0)
		return false;
	else if (ret != sizeof(loaded))
		exitWithMessage("Failed to load save!");
	memcpy(&WORLD, &loaded, sizeof(loaded));
	WORLD.reloadPtr();
	ENTITIES = loaded.entities;
	G		 = loaded.globals;
	printf("GAME LOADED!\n");
	return true;
}

int main(void)
{
	InitWindow(1320, 720, "SECOND");
	SetTargetFPS(60.f);
	InitAudioDevice();
	C.loadAll();

	createLevelSurface({6, 6, 0}, WORLD.level[0]);
	createLevelUnderground({6, 6, -1}, WORLD.level[-1]);
	createLevelUnderground({6, 6, -2}, WORLD.level[-2]);
	createLevelDeepUnderground({6, 6, -3}, WORLD.level[-3]);
	createLevelDeepUnderground({6, 6, -4}, WORLD.level[-4]);
	createLevelDeepUnderground({6, 6, -5}, WORLD.level[-5]);

	G.camera.zoom	= 6.f;
	G.camera.offset = {660, 360};
	u32		frame	= 0;
	bool	done	= false;
	Player& dude	= Player::get(Player::add({32, 32, 0}));
	Entity& eDude	= dude.getEntity();

	while (!done)
	{
		if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_S))
			saveGame();
		if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_L))
			loadGame();

		frame++;
		F.clear();
		F.mousePosWorld	 = GetScreenToWorld2D(GetMousePosition(), G.camera);
		F.mousePosWindow = GetMousePosition();

		dude.input(IsKeyPressed(KEY_W),
				   IsKeyPressed(KEY_S),
				   IsKeyPressed(KEY_A),
				   IsKeyPressed(KEY_D),
				   IsKeyPressed(KEY_E),
				   IsKeyPressed(KEY_G),
				   IsKeyPressed(KEY_X),
				   IsKeyPressed(KEY_Z),
				   canClimb(eDude.iPos),
				   canGoDown(eDude.iPos));

        ENTITIES.updateAll(0.016f);
		dude.update(0.016f);
		G.camera.target =
			(v2f(G.camera.target) + (dude.getEntity().fPos - v2f(G.camera.target)) * 0.1f)
				.toVector2();

		updateLevels(WORLD.level);

		collideLevel(WORLD.level[eDude.iPos.z], dude.pEntity);
		dude.move();

		BeginDrawing();
		{
			ClearBackground(SKYBLUE);
			BeginMode2D(G.camera);
			{
				drawLevel(WORLD.level, eDude.iPos.z - 2, eDude.iPos.z);
                ENTITIES.drawAll();
				dude.draw();
			}
			EndMode2D();
		}
		DrawText(
			TextFormat("%d, %d, %d", eDude.iPos.x, eDude.iPos.y, eDude.iPos.z), 10, 10, 20, YELLOW);
		DrawText(TextFormat("%d, %d, %d", F.dudeAimTile.x, F.dudeAimTile.y, F.dudeAimTile.z),
				 10,
				 25,
				 20,
				 YELLOW);
		dude.drawOverlay();
		EndDrawing();
		if (IsKeyPressed(KEY_Q))
			done = true;
	}
	return 0;
}
