#pragma once
#include "engine/v2.cpp"
#include "globals.cpp"

struct MapLayer
{
	struct Tile
	{
		enum Type
		{
			GRASS = 0,
			DIRT,
			ROCK,
			TORCH,
			CAMPFIRE,
			WOODWALL,
			CRATE,
			BARREL,
			EMPTY
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
	v3i	 toLocalCoords(v3i worldPos) { return worldPos - origin; }
	v3i	 toWorldCoords(v3i localPos) { return localPos + origin; }

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
	bool tryMove(v3i pos) { return getTileAt(pos).crossable; }
	bool containsFPos(v2f fPosWorld, v3i& iWorldCoords)
	{
		v3i iPosWorld = toV3i(fPosWorld / G.tileSize);
		if (containsXY(iPosWorld))
		{
			iWorldCoords = iPosWorld;
			return true;
		}
		return false;
	}
};

struct Map
{
	static constexpr u32 nLevels	 = 8;
	static constexpr u32 nLevelsDown = 6;
	static constexpr u32 nLevelsUp	 = nLevels - nLevelsDown - 1;

	MapLayer  levelArray[nLevels];
	MapLayer* level = &levelArray[nLevelsDown];
	void	  reloadPtr() { level = &levelArray[nLevelsDown]; }
	bool	  tryMove(v3i pos) { return level[pos.z].tryMove(pos); }
};

void placeTile(MapLayer::Tile& tile, MapLayer::Tile::Type type, bool surface = false)
{
	switch (type)
	{
		case MapLayer::Tile::EMPTY:
			tile = {.hitPoints	= 0,
					.type		= type,
					.crossable	= true,
					.discovered = true,
					.floor		= false,
					.surface	= surface};
			return;
		case MapLayer::Tile::GRASS:
			tile = {.hitPoints	= 2,
					.type		= type,
					.crossable	= true,
					.discovered = true,
					.floor		= true,
					.surface	= true};
			return;
		case MapLayer::Tile::DIRT:
			tile = {.hitPoints	= 4,
					.type		= type,
					.crossable	= false,
					.discovered = surface,
					.floor		= true,
					.surface	= surface};
			return;
		case MapLayer::Tile::ROCK:
			tile = {.hitPoints	= 10,
					.type		= type,
					.crossable	= false,
					.discovered = surface,
					.floor		= true,
					.surface	= surface};
			return;
		case MapLayer::Tile::Type::TORCH:
			tile = {.hitPoints	= 3,
					.type		= type,
					.crossable	= false,
					.discovered = surface,
					.floor		= true,
					.surface	= surface};
			return;
		case MapLayer::Tile::CAMPFIRE:
			tile = {.hitPoints	= 999,
					.type		= type,
					.crossable	= false,
					.discovered = surface,
					.floor		= true,
					.surface	= surface};
			return;
		case MapLayer::Tile::WOODWALL:
			tile = {.hitPoints	= 40,
					.type		= type,
					.crossable	= false,
					.discovered = surface,
					.floor		= true,
					.surface	= surface};
			return;
		case MapLayer::Tile::CRATE:
			tile = {.hitPoints	= 10,
					.type		= type,
					.crossable	= false,
					.discovered = surface,
					.floor		= true,
					.surface	= surface};
			return;
		case MapLayer::Tile::BARREL:
			tile = {.hitPoints	= 8,
					.type		= type,
					.crossable	= false,
					.discovered = surface,
					.floor		= true,
					.surface	= surface};
			return;
	};
}
void loadLevelFromTexture(MapLayer& level, Content::TEX_LEVEL levelMap, v3i origin)
{
	level.origin	   = origin;
	Color* levelLayout = LoadImageColors(C.levelMap[levelMap]);
	for (u32 x = 0; x < level.nTiles; x++)
		for (u32 y = 0; y < level.nTiles; y++)
			placeTile(level.tile[x][y], (MapLayer::Tile::Type)(levelLayout[64 * y + x].r), true);
}
void createLevelSurface(v3i origin, MapLayer& level)
{
	loadLevelFromTexture(level, Content::TEX_LEVEL::LEVEL_CAMP, origin);
}
void createLevelUnderground(v3i origin, MapLayer& level)
{
	level.origin = origin;
	for (u32 x = 0; x < level.nTiles; x++)
		for (u32 y = 0; y < level.nTiles; y++)
		{
			level.tile[x][y].type	   = MapLayer::Tile::DIRT;
			level.tile[x][y].hitPoints = 3;
			level.tile[x][y].floor	   = true;
			level.posWorld[x][y]	   = origin + v3i(x, y, 0);
		}
}
void createLevelDeepUnderground(v3i origin, MapLayer& level)
{
	level.origin = origin;
	for (u32 x = 0; x < level.nTiles; x++)
		for (u32 y = 0; y < level.nTiles; y++)
		{
			level.tile[x][y].type	   = MapLayer::Tile::ROCK;
			level.tile[x][y].hitPoints = 10;
			level.tile[x][y].floor	   = true;
			level.posWorld[x][y]	   = origin + v3i(x, y, 0);
		}
}
void updateLevels(MapLayer levels[])
{
	if (!F.dudeHit)
		return;
	MapLayer& l		 = levels[F.dudeAimTile.z];
	MapLayer& lAbove = levels[F.dudeAimTile.z + 1];
	MapLayer& lBelow = levels[F.dudeAimTile.z - 1];

	// below checks if we are on the surface which is a special case (crossable, non-empty tiles)
	// todo: refactor
	if (F.dudePos.z >= 0 && levels[F.dudePos.z].containsXY(F.dudeAimTile))
		if (levels[F.dudePos.z].getTileAt(F.dudePos).type == MapLayer::Tile::GRASS)
			return (void)(levels[F.dudePos.z].getTileAt(F.dudePos).type = MapLayer::Tile::EMPTY);

	if (l.containsXYZ(F.dudeAimTile))
	{
		MapLayer::Tile& tile = l.getTileAt(F.dudeAimTile);
		tile.hitPoints -= 1;
		if (tile.hitPoints > 0)
			return;
		tile.type		= tile.EMPTY;
		tile.crossable	= true;
		tile.discovered = true;
		if (lAbove.containsXY(F.dudeAimTile) &&
			lAbove.getTileAt(F.dudeAimTile).type == MapLayer::Tile::EMPTY)
			lAbove.getTileAt(F.dudeAimTile).floor = false;
		for (s32 x = -1; x <= 1; x++)
			for (s32 y = -1; y <= 1; y++)
				if (l.containsXYZ(F.dudeAimTile + v3i(x, y, 0)))
				{
					MapLayer::Tile& tile = l.getTileAt(F.dudeAimTile + v3i(x, y, 0));
					tile.discovered		 = true;
				}
	}
}
void drawLevel(MapLayer levels[], s32 bottomLevel, s32 topLevel)
{
	for (s32 z = bottomLevel; z <= topLevel; z++)
		for (s32 x = 0; x < MapLayer::nTiles; x++)
			for (s32 y = 0; y < MapLayer::nTiles; y++)
			{
				MapLayer&		l		 = levels[z];
				MapLayer&		lAbove	 = levels[z + 1];
				MapLayer&		lBelow	 = levels[z - 1];
				MapLayer::Tile& tile	 = l.tile[x][y];
				Texture2D*		tTexture = &C.textures[C.TEX_TILESET];
				v3i tilePos = (l.origin * G.tileSize) + v3i(x * G.tileSize, y * G.tileSize, 0);
				if (tile.type == MapLayer::Tile::EMPTY)
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
