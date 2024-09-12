#include "engine/fileio.cpp"
#include "engine/input.cpp"
#include "entities.cpp"
#include "map.cpp"
#include "dialogs.cpp"
#include "lighting.cpp"

#include "raylib.h"

struct WorldState
{
};
Map		   MAP;
Entities   ENTITIES;
DialogBox  DIALOGBOX;
Narrative  NARRATIVE;
WorldState WORLD;

bool canClimb(v3i& pos)
{
	MapLayer& lcurrent = MAP.level[pos.z];
	MapLayer& labove   = MAP.level[pos.z + 1];
	if (lcurrent.containsXYZ(pos) && lcurrent.getTileAt(pos).type == MapLayer::Tile::EMPTY)
		if (labove.containsXY(pos))
		{
			MapLayer::Tile& tileAbove = labove.getTileAt(pos);
			if (tileAbove.type == MapLayer::Tile::EMPTY)
				return true;
		}
	return false;
}
bool canGoDown(v3i& pos)
{
	MapLayer& lcurrent = MAP.level[pos.z];
	MapLayer& lbelow   = MAP.level[pos.z - 1];
	if (lcurrent.containsXYZ(pos) && lcurrent.getTileAt(pos).type == MapLayer::Tile::EMPTY)
		if (lbelow.containsXY(pos))
		{
			MapLayer::Tile& tileBelow = lbelow.getTileAt(pos);
			if (tileBelow.type == MapLayer::Tile::EMPTY)
				return true;
		}
	return false;
}

void saveGame()
{
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
	MAP.reloadPtr();
	printf("GAME LOADED!\n");
	return true;
}

int main(void)
{
	G.screenX = 1320;
	G.screenY = 720;
	InitWindow(G.screenX, G.screenY, "SECOND");
	SetTargetFPS(60.f);
	InitAudioDevice();
	C.loadAll();

	createLevelSurface({0, 0, 0}, MAP.level[0]);
	createLevelUnderground({0, 0, -1}, MAP.level[-1]);
	createLevelUnderground({0, 0, -2}, MAP.level[-2]);
	createLevelDeepUnderground({0, 0, -3}, MAP.level[-3]);
	createLevelDeepUnderground({0, 0, -4}, MAP.level[-4]);
	createLevelDeepUnderground({0, 0, -5}, MAP.level[-5]);
	Item::add(Item::PICKAXE, {10, 52, 0});
	Item::add(Item::SWORD, {10, 53, 0});
	Goblin::add({24, 30, 0}, Unit::Role::FIGHTER);
	Goblin::add({25, 31, 0}, Unit::Role::ARCHER);
	Goblin::add({24, 32, 0}, Unit::Role::FIGHTER);
	Goblin::add({23, 31, 0}, Unit::Role::FIGHTER);

	Goblin::add({38, 32, 0}, Unit::Role::FIGHTER);
	Goblin::add({40, 32, 0}, Unit::Role::ARCHER);
	Goblin::add({39, 41, 0}, Unit::Role::FIGHTER);
	Goblin::add({39, 33, 0}, Unit::Role::FIGHTER);

	Goblin::add({35, 41, 0}, Unit::Role::FIGHTER);
	Goblin::add({30, 41, 0}, Unit::Role::FIGHTER);

	Goblin::add({43, 21, 0}, Unit::Role::ARCHER);

	G.camera.zoom	= 3.f;
	G.camera.offset = {660, 360};
	bool done		= false;
	G.entDude		= Player::add({8, 53, 0});
	// G.entDude	  = Player::add({26, 26, 0});
	Player& dude  = Player::get(G.entDude);
	Entity& eDude = dude.getEntity();
	NARRATIVE.init();
	Input input;
	// NARRATIVE.start(0);

	Light l;
    l.init({1320, 720}, 0.5f);

	while (!done)
	{
		if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_S))
			saveGame();
		if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_L))
			loadGame();
		G.debugDrawSightRange = IsKeyDown(KEY_LEFT_CONTROL);

		G.frame++;
		FD.clear();
		FD.mousePosWorld  = GetScreenToWorld2D(GetMousePosition(), G.camera);
		FD.mousePosWindow = GetMousePosition();

		if (!DIALOGBOX.input(input.getAction()))
		{
			FD.progressLogic =
				dude.input(input.getHeading(eDude.fPos + v2f(G.tileSize / 2.f, G.tileSize / 2.f)),
						   input.getHit(),
						   input.getMove(),
						   IsKeyPressed(KEY_X),
						   input.getAction(),
						   input.getSwap(),
						   canClimb(eDude.iPos),
						   canGoDown(eDude.iPos));
			dude.update(0.016f);
			ENTITIES.updateAll(0.016f);
			dude.interact();
			updateLevels(MAP.level);
			dude.move();
			EFFECTS.updateAll(0.016f);
		}
		NARRATIVE.update();

		G.camera.target = follow(eDude.fPos, v2f(G.camera.target), 0.2f).toVector2();

		l.recomputeLights();

		BeginDrawing();
		{
			ClearBackground(SKYBLUE);
			BeginMode2D(G.camera);
			{
				drawLevel(MAP.level, eDude.iPos.z - 2, eDude.iPos.z);
				ENTITIES.drawAll();
				dude.draw();
				EFFECTS.drawAll();
			}
			EndMode2D();
		}
		l.drawLight({1320, 720});
		DrawText(
			TextFormat("%d, %d, %d", eDude.iPos.x, eDude.iPos.y, eDude.iPos.z), 10, 10, 20, YELLOW);
		DrawText(TextFormat("%d, %d, %d", FD.dudeAimTile.x, FD.dudeAimTile.y, FD.dudeAimTile.z),
				 10,
				 25,
				 20,
				 YELLOW);
		dude.drawOverlay();
		DIALOGBOX.draw();

		EndDrawing();
	}
	return 0;
}
