#pragma once
#include "../../raylib/include/raylib.h"
#include "engine.cpp"
#include "v2.cpp"

struct SpriteSheet
{
	// Frames in sprite sheet texture must be same size and horizontal
	const Texture* tex;
	u32			   textureOffset = 0;
	v2			   cellSize;
	u32			   frames = 0;
	f32			   fps	  = 1.f;
	bool		   flipX = false, flipY = false;
	bool		   animate = true;

	f32	 timer = 0;
	u32	 frame = 0;
	void init(Texture& t, v2 _cellSize, u32 _frames, f32 _fps, bool _animate)
	{
		tex		 = &t;
		cellSize = _cellSize;
		frames	 = _frames;
		fps		 = _fps;
		animate	 = _animate;
	}
	void update(f32 deltaTime)
	{
		if (!animate)
			return;
		timer += deltaTime;
		if (timer > 1.f / fps)
		{
			frame++;
			timer = 0;
		}
		if (frame >= frames)
			frame = 0;
	}
	void Draw(v2		   pos,
			  const Color& tint		   = WHITE,
			  f32		   rot		   = 0.0f,
			  f32		   scale	   = 1.0f,
			  int		   row		   = 0,
			  int		   customFrame = -1)
	{
		u32 tmpFrame = frame;
		if (customFrame > -1 && customFrame < frames)
			tmpFrame = customFrame;
		rot = math::radToDeg(rot);
		DrawTexturePro(*tex,
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

