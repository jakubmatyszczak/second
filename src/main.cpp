#include "engine/commons.cpp"
#include "engine/fileio.cpp"
#include "engine/v2.cpp"
#include "globals.cpp"
#include "entities.cpp"

Entities ENTITIES;

struct Level
{
	static const u32 nTiles = 8;
	enum Tile
	{
		EMPTY,
		GRASS,
		DIRT,
	};
	v3i	 origin;
	Tile tiles[nTiles][nTiles];
	v3i	 posWorld[nTiles][nTiles];
	v3i	 toLocalCoords(v3i worldPos)
	{
		v3i local = worldPos - origin;
		return local;
	}
	bool containsXY(v3i worldPos)
	{
		v3i local = toLocalCoords(worldPos);
		if (local.x >= 0 && local.x < nTiles && local.y >= 0 && local.y < nTiles)
			return true;
		return false;
	}
	bool containsXYZ(v3i worldPos)
	{
		v3i local = toLocalCoords(worldPos);
		if (local.z == 0 && local.x >= 0 && local.x < nTiles && local.y >= 0 && local.y < nTiles)
			return true;
		return false;
	}
	Tile& getTileAt(v3i worldPos)
	{
		v3i local = toLocalCoords(worldPos);
		return tiles[local.x][local.y];
	}
};
void createLevelSurface(v3i origin, Level& level)
{
	level.origin = origin;
	for (u32 x = 0; x < level.nTiles; x++)
		for (u32 y = 0; y < level.nTiles; y++)
		{
			level.tiles[x][y]	 = Level::GRASS;
			level.posWorld[x][y] = origin + v3i(x, y, 0);
		}
}
void createLevelUnderground(v3i origin, Level& level)
{
	level.origin = origin;
	for (u32 x = 0; x < level.nTiles; x++)
		for (u32 y = 0; y < level.nTiles; y++)
		{
			level.tiles[x][y]	 = Level::DIRT;
			level.posWorld[x][y] = origin + v3i(x, y, 0);
		}
}
void updateLevel(Level& level)
{
	if (!FRAME.hit)
		return;
	for (u32 x = 0; x < Level::nTiles; x++)
		for (u32 y = 0; y < Level::nTiles; y++)
			if (level.posWorld[x][y] ==
				v3i(FRAME.aimTile.x, FRAME.aimTile.y, level.posWorld[x][y].z))
				return (void)(level.tiles[x][y] = Level::EMPTY);
}
void drawLevel(Level levels[], s32 bottomLevel, s32 topLevel)
{
	for (s32 z = bottomLevel; z <= topLevel; z++)
		for (s32 x = 0; x < Level::nTiles; x++)
			for (s32 y = 0; y < Level::nTiles; y++)
			{
				Color clr = WHITE;
				if (FRAME.aimTile ==
					v3i(x + levels[z].origin.x, y + levels[z].origin.y, FRAME.aimTile.z))
				{
					clr = WHITE_CLEAR;
				}
				Texture2D* texture = &CONTENT.textures[CONTENT.TEX_TILESET];
				v3i		   tilePos =
					(levels[z].origin * 16) + v3i(x * GLOBAL.tileSize, y * GLOBAL.tileSize, 0);
				DrawTexturePro(*texture,
							   {levels[z].tiles[x][y] * (f32)GLOBAL.tileSize,
								0,
								GLOBAL.tileSize,
								GLOBAL.tileSize},
							   {(f32)tilePos.x, (f32)tilePos.y, GLOBAL.tileSize, GLOBAL.tileSize},
							   {0, 0},
							   0,
							   clr);
			}
}
struct WorldState
{
	static constexpr u32 nLevels	 = 8;
	static constexpr u32 nLevelsDown = 6;
	static constexpr u32 nLevelsUp	 = nLevels - nLevelsDown - 1;
	Globals				 globals;
	Entities			 entities;
	Level				 levelArray[nLevels];
	Level*				 level = &levelArray[nLevelsDown];
};
WorldState WORLD;
bool	   isTileEmpty(v3i& pos)
{
	Level& l = WORLD.level[pos.z];
	if (l.containsXYZ(pos) && l.getTileAt(pos) == Level::EMPTY)
		return true;
	return false;
}

void saveGame(WorldState& save)
{
	save.globals  = GLOBAL;
	save.entities = ENTITIES;
	fileio::saveRawFile("1.save", &save, sizeof(save));
}
bool loadGame(WorldState& game)
{
	WorldState loaded;
	s32		   ret = fileio::loadRawFile("1.save", sizeof(loaded), &loaded);
	if (ret < 0)
		return false;
	else if (ret != sizeof(loaded))
		exitWithMessage("Failed to load save!");
	// entities = loaded.entities;
	// GLOBAL	 = loaded.globals;
	// memcpy(game.levels, loaded.levels, sizeof(loaded.levels));
	return true;
}

int main(void)
{
	InitWindow(800, 600, "SECOND");
	SetTargetFPS(60.f);
	InitAudioDevice();
	CONTENT.loadAll();

	loadGame(WORLD);

	createLevelSurface({4, 4, 0}, WORLD.level[0]);
	createLevelUnderground({5, 5, -1}, WORLD.level[-1]);
	createLevelUnderground({6, 6, -2}, WORLD.level[-2]);

	GLOBAL.camera.zoom	 = 6.f;
	GLOBAL.camera.offset = {400, 300};
	u32		frame		 = 0;
	bool	done		 = false;
	Player& dude		 = Player::get(Player::add({10, 10, 0}));
	Entity& eDude		 = dude.getEntity();

	while (!done)
	{
		frame++;
		FRAME.clear();
		FRAME.mousePosWorld	 = GetScreenToWorld2D(GetMousePosition(), GLOBAL.camera);
		FRAME.mousePosWindow = GetMousePosition();

		dude.input(IsKeyPressed(KEY_W),
				   IsKeyPressed(KEY_S),
				   IsKeyPressed(KEY_A),
				   IsKeyPressed(KEY_D),
				   IsKeyPressed(KEY_E),
				   isTileEmpty(eDude.iPos));

		dude.update(0.016f);
		GLOBAL.camera.target =
			(v2f(GLOBAL.camera.target) + (dude.getEntity().fPos - v2f(GLOBAL.camera.target)) * 0.1f)
				.toVector2();

		updateLevel(WORLD.level[eDude.iPos.z]);

		BeginDrawing();
		{
			ClearBackground(SKYBLUE);
			BeginMode2D(GLOBAL.camera);
			{
				drawLevel(WORLD.level, eDude.iPos.z - 1, eDude.iPos.z);
				dude.draw();
			}
			EndMode2D();
		}
		DrawText(TextFormat("%d, %d", eDude.iPos.x, eDude.iPos.y), 10, 10, 10, BLACK);
		EndDrawing();
		if (IsKeyPressed(KEY_Q))
			done = true;
	}
	return 0;
}
