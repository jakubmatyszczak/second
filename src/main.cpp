#include "engine/commons.cpp"
#include "engine/fileio.cpp"
#include "engine/v2.cpp"
#include "globals.cpp"
#include "entities.cpp"

Entities ENTITIES;

struct Level
{
	struct Tile
	{
		enum Type
		{
			EMPTY,
			GRASS,
			DIRT,
			ROCK,
			RAMP,
		};
		Type type;
		bool crossable;
		bool discovered;
		bool floor;
		bool surface;
	};
	static const u32 nTiles = 8;

	v3i	 origin					  = {};
	Tile tile[nTiles][nTiles]	  = {};
	v3i	 posWorld[nTiles][nTiles] = {};
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
		return tile[local.x][local.y];
	}
};
void createLevelSurface(v3i origin, Level& level)
{
	level.origin = origin;
	for (u32 x = 0; x < level.nTiles; x++)
		for (u32 y = 0; y < level.nTiles; y++)
		{
			level.tile[x][y].type		= Level::Tile::GRASS;
			level.tile[x][y].floor		= true;
			level.tile[x][y].crossable	= true;
			level.tile[x][y].surface	= true;
			level.tile[x][y].discovered = true;
			level.posWorld[x][y]		= origin + v3i(x, y, 0);
		}
}
void createLevelUnderground(v3i origin, Level& level)
{
	level.origin = origin;
	for (u32 x = 0; x < level.nTiles; x++)
		for (u32 y = 0; y < level.nTiles; y++)
		{
			level.tile[x][y].type  = Level::Tile::DIRT;
			level.tile[x][y].floor = true;
			level.posWorld[x][y]   = origin + v3i(x, y, 0);
		}
}
void createLevelDeepUnderground(v3i origin, Level& level)
{
	level.origin = origin;
	for (u32 x = 0; x < level.nTiles; x++)
		for (u32 y = 0; y < level.nTiles; y++)
		{
			level.tile[x][y].type  = Level::Tile::ROCK;
			level.tile[x][y].floor = true;
			level.posWorld[x][y]   = origin + v3i(x, y, 0);
		}
}
void updateLevels(Level levels[])
{
	if (!F.dudeHit)
		return;
	Level& l	  = levels[F.dudeAimTile.z];
	Level& lAbove = levels[F.dudeAimTile.z + 1];
	Level& lBelow = levels[F.dudeAimTile.z - 1];

	// below checks if we are on the surface which is a special case (crossable, non-empty tiles)
	// todo: refactor
	if (F.dudePos.z >= 0 && levels[F.dudePos.z].containsXY(F.dudeAimTile))
		if (levels[F.dudePos.z].getTileAt(F.dudePos).type == Level::Tile::GRASS)
			return (void)(levels[F.dudePos.z].getTileAt(F.dudePos).type = Level::Tile::EMPTY);

	if (l.containsXYZ(F.dudeAimTile))
	{
		Level::Tile& tile = l.getTileAt(F.dudeAimTile);
		tile.type		  = tile.EMPTY;
		tile.crossable	  = true;
		tile.discovered	  = true;
		if (lAbove.containsXY(F.dudeAimTile) &&
			lAbove.getTileAt(F.dudeAimTile).type == Level::Tile::EMPTY)
			lAbove.getTileAt(F.dudeAimTile).floor = false;
		for (s32 x = -1; x <= 1; x++)
			for (s32 y = -1; y <= 1; y++)
				if (l.containsXYZ(F.dudeAimTile + v3i(x, y, 0)))
				{
					Level::Tile& tile = l.getTileAt(F.dudeAimTile + v3i(x, y, 0));
					tile.discovered	  = true;
				}
	}
}
void drawLevel(Level levels[], s32 bottomLevel, s32 topLevel)
{
	for (s32 z = bottomLevel; z <= topLevel; z++)
		for (s32 x = 0; x < Level::nTiles; x++)
			for (s32 y = 0; y < Level::nTiles; y++)
			{
				Level&		 l		  = levels[z];
				Level&		 lAbove	  = levels[z + 1];
				Level&		 lBelow	  = levels[z - 1];
				Level::Tile& tile	  = l.tile[x][y];
				Texture2D*	 tTexture = &C.textures[C.TEX_TILESET];
				v3i tilePos = (l.origin * G.tileSize) + v3i(x * G.tileSize, y * G.tileSize, 0);
				if (tile.type == Level::Tile::EMPTY)
				{
					if (tile.floor)
						DrawTexturePro(
							*tTexture,
							{(u32)tile.floor * (f32)G.tileSize, 0, G.tileSize, G.tileSize},
							{(f32)tilePos.x, (f32)tilePos.y, G.tileSize, G.tileSize},
							{0, 0},
							0,
							GRAY);
					else
						DrawRectangleV(
							tilePos.toVector2(), {G.tileSize, G.tileSize}, BLUESKY_CLEAR);
					if (!lAbove.tile[x][y].floor && !tile.surface && !lAbove.tile[x][y].surface)
						DrawCircleGradient(tilePos.x + 8, tilePos.y + 8, 8, YELLOW, {});
				}
				else if (tile.discovered)
					DrawTexturePro(*tTexture,
								   {(u32)tile.type * (f32)G.tileSize, 0, G.tileSize, G.tileSize},
								   {(f32)tilePos.x, (f32)tilePos.y, G.tileSize, G.tileSize},
								   {0, 0},
								   0,
								   z < topLevel ? GRAY : WHITE);
				else
					DrawRectangleV(tilePos.toVector2(), {G.tileSize, G.tileSize}, BLACK);
			}
}
void collideLevel(Level& level, EntityPtr pEntity)
{
	Entity& e = ENTITIES.arr[pEntity];
	if (level.containsXYZ(e.iMoveTarget))
		if (level.getTileAt(e.iMoveTarget).crossable == false)
			e.iMoveTarget = {};
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
bool	   willFall(v3i& pos)
{
	Level& lcurrent = WORLD.level[pos.z];
	Level& lbelow	= WORLD.level[pos.z - 1];
	if (lcurrent.containsXYZ(pos) && lcurrent.getTileAt(pos).type == Level::Tile::EMPTY)
		if (lbelow.containsXY(pos) && lbelow.getTileAt(pos).type == Level::Tile::EMPTY)
			return true;
	return false;
}
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

void saveGame(WorldState& save)
{
	save.globals  = G;
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
	C.loadAll();

	loadGame(WORLD);

	createLevelSurface({6, 6, 0}, WORLD.level[0]);
	createLevelUnderground({6, 6, -1}, WORLD.level[-1]);
	createLevelUnderground({6, 6, -2}, WORLD.level[-2]);
	createLevelDeepUnderground({6, 6, -3}, WORLD.level[-3]);
	createLevelDeepUnderground({6, 6, -4}, WORLD.level[-4]);
	createLevelDeepUnderground({6, 6, -5}, WORLD.level[-5]);

	G.camera.zoom	= 6.f;
	G.camera.offset = {400, 300};
	u32		frame	= 0;
	bool	done	= false;
	Player& dude	= Player::get(Player::add({10, 10, 0}));
	Entity& eDude	= dude.getEntity();

	while (!done)
	{
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
				   // willFall(eDude.iPos),
				   false,
				   canClimb(eDude.iPos),
				   canGoDown(eDude.iPos));

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
