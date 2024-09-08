#pragma once
#include "engine/commons.cpp"
#include "engine/v2.cpp"

f32 lerp(f32 start, f32 end, f32 time) { return start + (end - start) * time; }
v2f lerp(const v2f& start, const v2f& end, f32 time) { return start + (end - start) * time; }

struct Keyframe
{
	v2f	  pos	   = {};
	f32	  rot	   = 0.f;
	f32	  scale	   = 1.f;
	Color clr	   = WHITE;
	f32	  duration = 1.f;
};
struct Animation
{
	bool  reversed	  = false;
	bool  active	  = false;
	bool  looped	  = false;
	f32	  scaleOffset = 1.f;
	v2f	  posOffset	  = {};
	f32	  rotOffset	  = {};
	Color color		  = {};

	f32 timer  = 0.f;
	f32 period = 1.f;
	s32 frame  = 0;

	bool init = false;

	void activate(u32 nKeyFrames, f32 animPeriod, bool loop = false)
	{
		active = true;
		looped = loop;
		period = animPeriod;
		timer  = 0.f;
		frame  = 1;
		if (reversed)
			frame = nKeyFrames - 2;
	}
	[[nodiscard]]
	bool update(f32 dt, Keyframe keyFrames[], u32 nKeyFrames)
	{
		if (!init)
		{
			posOffset	= keyFrames[0].pos;
			rotOffset	= keyFrames[0].rot;
			scaleOffset = keyFrames[0].scale;
			color		= keyFrames[0].clr;
			init		= true;
		}
		if (!active)
			return false;
		timer += dt;
		if (timer > keyFrames[frame].duration * period)
		{
			timer = 0.f;
			frame++;
			if (reversed)
				frame -= 2;
		}
		bool finished = false;
		if (frame >= (s32)nKeyFrames)
		{
			finished = true;
			frame	 = nKeyFrames - 1;
			timer	 = keyFrames[frame].duration;
		}
		if (frame < 0)	// reversed animation end
		{
			finished = true;
			frame	 = 0;
			timer	 = keyFrames[frame].duration;
		}

		u32 prevFrame = frame - 1;
		if (reversed)
			prevFrame = frame + 1;
		Keyframe& previous = keyFrames[prevFrame];
		Keyframe& current  = keyFrames[frame];

		f32 timeNorm = math::limit((timer / (current.duration * period)), 0.f, 1.f);
		posOffset	 = lerp(previous.pos, current.pos, timeNorm);
		rotOffset	 = lerp(previous.rot, current.rot, timeNorm);
		scaleOffset	 = lerp(previous.scale, current.scale, timeNorm);
		color		 = {
			   (u8)lerp(previous.clr.r, current.clr.r, timeNorm),
			   (u8)lerp(previous.clr.g, current.clr.g, timeNorm),
			   (u8)lerp(previous.clr.b, current.clr.b, timeNorm),
			   (u8)lerp(previous.clr.a, current.clr.a, timeNorm),
		   };

		if (finished)
		{
			active = false;
			if (looped)
			{
				activate(nKeyFrames, period, looped);
				return update(dt, keyFrames, nKeyFrames);
			}
			return true;
		}
		return false;
	}
};
struct AnimBonk
{
	Animation		 anim;
	static const u32 nKeyFrames			   = 4;
	Keyframe		 keyFrames[nKeyFrames] = {
		{.pos = {0.f, 0.f}, .rot = 0.f, .duration = 0.3},
		{.pos = {0.f, -4.f}, .rot = .2f, .duration = 0.3f},
		{.pos = {0.f, -4.f}, .rot = .4f, .duration = 0.1f},
		{.pos = {0.f, 0.f}, .rot = 0.f, .duration = 0.3f},
	};

	void activate(f32 period) { anim.activate(nKeyFrames, period, false); }
	// returns true if completed
	bool update(f32 dt) { return anim.update(dt, keyFrames, nKeyFrames); }
	v2f	 getPos() { return anim.posOffset; }
	f32	 getRot() { return anim.rotOffset; }
};
struct AnimSwingFwd
{
	static constexpr s32 steps			   = 2;
	static constexpr u32 nKeyFramesPerStep = 4;

	Animation anim[steps];
	s32		  currentStep						  = 0;
	Keyframe  keyFrames[steps][nKeyFramesPerStep] = {
		 {
			 {.pos = {0.f, -8.f}, .rot = 0.f, .duration = 0.05},
			 {.pos = {8.f, 0.f}, .rot = math::pi, .duration = 0.15f},
			 {.pos = {-1.f, 8.f}, .rot = math::pi * 2.1f, .duration = 0.6f},
			 {.pos = {0.f, 8.f}, .rot = math::tau, .duration = 0.2f},
		 },
		 {
			 {.pos = {0.f, 8.f}, .rot = 0, .duration = 0.05},
			 {.pos = {8.f, 0.f}, .rot = -math::pi, .duration = 0.15f},
			 {.pos = {-1.f, -8.f}, .rot = -math::pi * 2.1f, .duration = 0.6f},
			 {.pos = {0.f, -8.f}, .rot = -math::tau, .duration = 0.2f},
		 }};

	// returns true if activated, false if not (anim already running)
	bool activate(f32 period)
	{
		if (anim[0].active || anim[1].active)
			return false;
		if (++currentStep >= steps)
			currentStep = 0;
		anim[currentStep].activate(nKeyFramesPerStep, period, false);
		return true;
	}
	// returns true if completed
	bool update(f32 dt)
	{
		if (anim[currentStep].update(dt, keyFrames[currentStep], nKeyFramesPerStep))
		{
			return true;
		}
		return false;
	}
	v2f getPos(bool mirrorx, f32 angle)
	{
		v2f pos = anim[currentStep].posOffset;
		if (mirrorx)
			pos.x = -pos.x;
		return pos.rotate(angle);
	}
	f32 getRot(bool mirrorx, f32 angle)
	{
		if (mirrorx)
			return -anim[currentStep].rotOffset + math::pi + angle;
		return anim[currentStep].rotOffset + angle;
	}
};
struct AnimFadeout
{
	Animation		 anim;
	static const u32 nKeyFrames			   = 2;
	Keyframe		 keyFrames[nKeyFrames] = {
		{.clr = WHITE, .duration = 0.0},
		{.clr = {255, 255, 255, 0}, .duration = 1.f},
	};

	void activate(f32 period) { anim.activate(nKeyFrames, period, false); }
	// returns true if completed
	bool  update(f32 dt) { return anim.update(dt, keyFrames, nKeyFrames); }
	Color getColor() { return anim.color; }
};
