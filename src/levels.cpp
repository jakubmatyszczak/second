#pragma once
#include "engine/basic.cpp"
#include "engine/v2.cpp"
#include "entities.cpp"

enum class Tile : u8
{
	EMPTY,
	GRASS,
	HOLE,
	DIRT,
	ROCK,
};
struct Level
{
	static constexpr u32 tileSize = 8;
	static constexpr u32 gridSize = 16;

	Tile  tileset[gridSize][gridSize] = {};
	u32	  variant[gridSize][gridSize] = {};
	u8	  rot[gridSize][gridSize];
	Color tint[gridSize][gridSize] = {};

	i32 zLevel = 0;
	v2	origin;	 // top left corner
	//
	bool tileContainsPoint(u32 xTile, u32 yTile, Vector2& point)
	{
		v2 tileOrigin = origin + v2(xTile * tileSize, yTile * tileSize);
		v2 size		  = {tileSize, tileSize};
		if (point.x < tileOrigin.x || point.y < tileOrigin.y)
			return false;
		if (point.x > tileOrigin.x + size.x || point.y > tileOrigin.y + size.y)
			return false;
		return true;
	}
};

void LoadLevelSurface(Level& level, v2 origin)
{
	level.origin = origin;
	for (int x = 0; x < level.gridSize; x++)
		for (int y = 0; y < level.gridSize; y++)
		{
			level.tileset[x][y] = Tile::GRASS;
			level.variant[x][y] = Content::TEX_TILESET;
			level.rot[x][y]		= math::random(0, 3);
			level.tint[x][y]	= WHITE;
		}
	Pick::add({15, 15}, 0);
}
void LoadLevelUndergroud(Level& level, v2 origin, i32 zLevel)
{
	level.zLevel = zLevel;
	level.origin = origin;
	for (int x = 0; x < level.gridSize; x++)
		for (int y = 0; y < level.gridSize; y++)
		{
			level.tileset[x][y] = Tile::DIRT;
			level.variant[x][y] = Content::TEX_TILESET;
			level.rot[x][y]		= math::random(0, 3);
			level.tint[x][y]	= WHITE;
		}
}
void LoadLevelUndergroud2(Level& level, v2 origin, i32 zLevel)
{
	LoadLevelUndergroud(level, origin, zLevel);
	Key::add({100, 100}, zLevel);
}
void updateLevels(Level levels[], u32 startLevel, u32 endLevel)
{
	for (u32 z = startLevel; z < endLevel; z++)
	{
		if (FRAME.aimZLevel != z && FRAME.hitZLevel != z)
			continue;
		Level& level = levels[z];
		for (int x = 0; x < level.gridSize; x++)
			for (int y = 0; y < level.gridSize; y++)
			{
				level.tint[x][y] = WHITE;
				if (level.tileContainsPoint(x, y, FRAME.aimCoords))
					level.tint[x][y] = GREEN_CLEAR;
				if (level.tileContainsPoint(x, y, FRAME.hitCoords))
				{
					level.tileset[x][y] = Tile::HOLE;
					level.tint[x][y]	= BLACK_CLEAR;
				}
			}
	}
}

void drawLevels(Level levels[], u32 startLevel, u32 endLevel)
{
	for (u32 z = startLevel; z < endLevel; z++)
	{
		Level& level = levels[z];
		for (int x = 0; x < level.gridSize; x++)
			for (int y = 0; y < level.gridSize; y++)
			{
				v2		   tileOrigin = {level.tileSize / 2.f, level.tileSize / 2.f};
				v2		   tilePos	  = level.origin + v2(x * level.tileSize, y * level.tileSize);
				Texture2D* tileTex	  = &CONTENT.textures[CONTENT.TEX_TILESET];
				f32		   tileSetOffset = (u32)level.tileset[x][y] * level.tileSize * 2;
				DrawTexturePro(*tileTex,
							   {tileSetOffset, 0, 16, 16},
							   {tilePos.x + tileOrigin.x,
								tilePos.y + tileOrigin.y,
								(f32)level.tileSize,
								(f32)level.tileSize},
							   tileOrigin.toVector2(),
							   level.rot[x][y] * 90.f,
							   level.tint[x][y]);
			}
	}
}
