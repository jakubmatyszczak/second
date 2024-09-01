#pragma once
#include "engine/commons.cpp"
#include "engine/v2.cpp"

f32 lerp(f32 start, f32 end, f32 time) { return start + (end - start) * time; }
v2f lerp(const v2f& start, const v2f& end, f32 time) { return start + (end - start) * time; }

struct Keyframe
{
	v2f pos		 = {};
	f32 rot		 = 0.f;
	f32 scale	 = 1.f;
	f32 duration = 1.f;
};
struct Animation
{
	bool reversed  = false;
	bool active	   = false;
	bool looped	   = false;
	f32	 scale	   = 1.f;
	v2f	 posOffset = {};
	f32	 rotOffset = {};

	f32 timer  = 0.f;
	f32 period = 1.f;
	s32 frame  = 0;

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
		scale		 = lerp(previous.scale, current.scale, timeNorm);

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
