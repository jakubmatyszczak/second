#pragma once
#include "engine/commons.cpp"
#include "engine/v2.cpp"

constexpr Color WHITE_CLEAR = {255, 255, 255, 100};
constexpr Color BLACK_CLEAR = {0, 0, 0, 100};
constexpr Color BLUESKY_CLEAR = {0x87, 0xCE, 0xFF, 100};

struct FrameData
{
	v2f	 mousePosWorld;
	v2f	 mousePosWindow;
	v3i	 aimTile;
	bool hit;
	void clear() { memset(this, 0, sizeof(FrameData)); }
};
struct Globals
{
	static constexpr u32 tileSize = 16;
	Camera2D			 camera;
};

FrameData FRAME;
Globals	  GLOBAL;
Content	  CONTENT;
