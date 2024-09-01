#pragma once
#include "entities.cpp"

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
		s32	 hitPoints;
		Type type;
		bool crossable;
		bool discovered;
		bool floor;
		bool surface;
	};
	static const u32 nTiles = 64;

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
			level.tile[x][y].hitPoints	= 1;
			level.tile[x][y].crossable	= true;
			level.tile[x][y].surface	= true;
			level.tile[x][y].discovered = true;
			level.posWorld[x][y]		= origin + v3i(x, y, 0);
		}
	Pickaxe::add(v3i(30, 30, 0));
	Pickaxe::add(v3i(32, 30, 0));
	Pickaxe::add(v3i(34, 30, 0));
}
void createLevelUnderground(v3i origin, Level& level)
{
	level.origin = origin;
	for (u32 x = 0; x < level.nTiles; x++)
		for (u32 y = 0; y < level.nTiles; y++)
		{
			level.tile[x][y].type	   = Level::Tile::DIRT;
			level.tile[x][y].hitPoints = 3;
			level.tile[x][y].floor	   = true;
			level.posWorld[x][y]	   = origin + v3i(x, y, 0);
		}
}
void createLevelDeepUnderground(v3i origin, Level& level)
{
	level.origin = origin;
	for (u32 x = 0; x < level.nTiles; x++)
		for (u32 y = 0; y < level.nTiles; y++)
		{
			level.tile[x][y].type	   = Level::Tile::ROCK;
			level.tile[x][y].hitPoints = 10;
			level.tile[x][y].floor	   = true;
			level.posWorld[x][y]	   = origin + v3i(x, y, 0);
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
		Player&		 dude = Player::get(G.entDude);
		tile.hitPoints -= dude.current.digPower;
		if (tile.hitPoints > 0)
			return;
		tile.type		= tile.EMPTY;
		tile.crossable	= true;
		tile.discovered = true;
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
					if (z == F.dudePos.z && !lAbove.tile[x][y].floor && !tile.surface)
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
	void				 reloadPtr() { level = &levelArray[nLevelsDown]; }
};
