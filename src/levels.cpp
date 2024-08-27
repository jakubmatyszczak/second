#pragma once
#include "engine/basic.cpp"
#include "engine/v2.cpp"
#include "entities.cpp"

enum class Tile : u8
{
	GRASS = 0,
	HOLE,
	DIRT,
	ROCK,
};
struct Level
{
	static constexpr u32 tileSize = 8;
	static constexpr u32 gridSize = 16;

	Tile tileset[gridSize][gridSize] = {};
	u32	 variant[gridSize][gridSize] = {};
	u8	 rot[gridSize][gridSize];
	v2	 origin;  // top left corner
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
		}
    Key::add({100, 100});
    Pick::add({150, 150});
}

void drawLevel(Level& level)
{
	for (int x = 0; x < level.gridSize; x++)
		for (int y = 0; y < level.gridSize; y++)
		{
			v2		   tilePos		 = level.origin + v2(x * level.tileSize, y * level.tileSize);
			Texture2D* tileTex		 = &content.textures[content.TEX_TILESET];
			f32		   tileSetOffset = (u32)level.tileset[x][y] * level.tileSize;
			DrawTexturePro(*tileTex,
						   {tileSetOffset, 0, 16, 16},
						   {tilePos.x, tilePos.y, (f32)level.tileSize, (f32)level.tileSize},
						   {level.tileSize / 2.f, level.tileSize / 2.f},
						   level.rot[x][y] * 90.f,
						   WHITE);
		}
}
