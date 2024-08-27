#pragma once
#include "../../raylib/include/raylib.h"
#include "engine.cpp"
#include "v2.cpp"

struct SpriteSheet
{
	// Frames in sprite sheet texture must be same size and horizontal
	u32	 pTexture;
	u32	 textureOffset = 0;
	v2	 cellSize;
	u32	 frames = 0;
	f32	 fps	= 1.f;
	bool flipX = false, flipY = false;
	bool animate = true;

	f32	 timer = 0;
	u32	 frame = 0;
	void init(u32 t, v2 _cellSize, u32 _frames, f32 _fps, bool _animate)
	{
		pTexture = t;
		cellSize = _cellSize;
		frames	 = _frames;
		fps		 = _fps;
		animate	 = _animate;
	}
	// returns true if frame changed
	bool update(f32 deltaTime)
	{
		if (!animate)
			return false;
		timer += deltaTime;
		if (timer > 1.f / fps)
		{
			frame++;
			timer = 0;
			return true;
		}
		if (frame >= frames)
			frame = 0;
		return false;
	}
	void Draw(v2		   pos,
			  const Color& tint		   = WHITE,
			  f32		   rot		   = 0.0f,
			  f32		   scale	   = 1.0f,
			  i32		   row		   = 0,
			  i32		   customFrame = -1)
	{
		i32 tmpFrame = frame;
		if (customFrame > -1 && customFrame < (i32)frames)
			tmpFrame = customFrame;
		rot = math::radToDeg(rot);
		DrawTexturePro(CONTENT.textures[pTexture],
					   {tmpFrame * cellSize.x,
						(f32)textureOffset + row * cellSize.y,
						flipX ? -cellSize.x : cellSize.x,
						flipY ? -cellSize.y : cellSize.y},
					   {pos.x, pos.y, scale * cellSize.x, scale * cellSize.y},
					   {cellSize.x / 2 * scale, cellSize.y / 2 * scale},
					   rot,
					   tint);
	}
};
