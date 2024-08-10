#pragma once
#include "v2.cpp"

f32 lerp(f32 start, f32 end, f32 time) { return start + (end - start) * time; }
v2	lerp(const v2& start, const v2& end, f32 time) { return start + (end - start) * time; }

struct Entity
{
	enum class Id : u32
	{
		UNKNOWN,
		PLAYER,
		ITEM,
		OBJECT,
		// Add Types here
		ID_MAX	// Keep this last
	};
	v2	  pos		  = {};
	f32	  rot		  = 0.f;
	Id	  id		  = Id::UNKNOWN;
	u32	  instancePtr = 0;
	void* data		  = nullptr;
	void* properties  = nullptr;
};
struct Entities
{
	static const u32 maxEntities = 128;

	Entity instances[maxEntities]	 = {};
	u8	   active[maxEntities]		 = {};
	u8	   drawable[maxEntities]	 = {};
	u8	   selectable[maxEntities]	 = {};
	u8	   interactable[maxEntities] = {};

	u32 nActive		= 0;
	i32 selectedPtr = -1;
	i32 interactPtr = -1;

	i32 add(Entity::Id idType,
			void*	   entityData,
			void*	   entityProperties,
			v2		   position	   = {},
			f32		   rotation	   = 0.f,
			bool	   canDraw	   = false,
			bool	   canSelect   = false,
			bool	   canInteract = false)
	{
		for (int i = 0; i < maxEntities; i++)
		{
			if (active[i])
				continue;
			Entity& e		= instances[i];
			e.id			= idType;
			e.data			= entityData;
			e.properties	= entityProperties;
			e.pos			= position;
			e.rot			= rotation;
			e.instancePtr	= i;
			active[i]		= true;
			drawable[i]		= canDraw;
			selectable[i]	= canSelect;
			interactable[i] = canInteract;
			nActive++;
			return i;
		}
		return -1;
	}
	void remove(u32 instancePtr)
	{
		if (!active[instancePtr] && instancePtr < maxEntities)
			return;
		active[instancePtr] = false;
		nActive--;
	}
	void select(u32 instancePtr)
	{
		if (instancePtr < maxEntities)
			if (active[instancePtr] && selectable[instancePtr])
				selectedPtr = instancePtr;
	}
	void interact(u32 instancePtr)
	{
		if (instancePtr < maxEntities)
			if (active[instancePtr] && interactable[instancePtr])
				interactPtr = instancePtr;
	}
	void refresh()
	{
		selectedPtr = -1;
		interactPtr = -1;
	}
};
Entities entities = {};

struct Keyframe
{
	v2	pos		 = {};
	f32 rot		 = 0.f;
	f32 scale	 = 1.f;
	f32 duration = 1.f;
};
struct Animation
{
	bool reversed  = false;
	bool active	   = false;
    bool looped = false;
	f32	 scale	   = 1.f;
	v2	 posOffset = {};
	f32	 rotOffset = {};

	f32 timer = 0.f;
    f32 tempo = 1.f;
	i32 frame = 0;

	void activate(u32 nKeyFrames, bool loop = false)
	{
		active = true;
        looped = loop;
		timer  = 0.f;
		frame  = 1;
		if (reversed)
			frame = nKeyFrames - 2;
		posOffset = {};
		rotOffset = 0.f;
	}
	[[nodiscard]]
	bool update(f32 dt, Keyframe keyFrames[], u32 nKeyFrames)
	{
		if (!active)
			return false;
		timer += dt * tempo;
		if (timer > keyFrames[frame].duration)
		{
			timer = 0.f;
			frame++;
			if (reversed)
				frame -= 2;
		}
		bool finished = false;
		if (frame >= (i32)nKeyFrames)
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

		f32 timeNorm = math::limit((timer / current.duration), 0.f, 1.f);
		posOffset	 = lerp(previous.pos, current.pos, timeNorm);
		rotOffset	 = lerp(previous.rot, current.rot, timeNorm);
		scale		 = lerp(previous.scale, current.scale, timeNorm);

		if (finished)
		{
			active = false;
            if(looped)
                activate(nKeyFrames, looped);
			return true;
		}
		return false;
	}
};
struct AnimFlip
{
	Animation		 anim;
	static const u32 nKeyFrames			   = 4;
	Keyframe		 keyFrames[nKeyFrames] = {
		{.pos = {0.f, 0.f}, .rot = 0.f, .duration = 0.1},
		{.pos = {0.f, -10.f}, .rot = 1.f, .duration = 0.1f},
		{.pos = {0.f, -10.f}, .rot = 3.14159f, .duration = 0.05f},
		{.pos = {0.f, 0.f}, .rot = 3.14159f, .duration = 0.1f},
	};

	void activate() { anim.activate(nKeyFrames); }
	// returns true if completed
	bool update(f32 dt) { return anim.update(dt, keyFrames, nKeyFrames); }
	v2	 getPos() { return anim.posOffset; }
	f32	 getRot() { return anim.rotOffset; }
};
struct AnimJump
{
	Animation		 anim;
	static const u32 nKeyFrames			   = 4;
	Keyframe		 keyFrames[nKeyFrames] = {
		{.pos = {0.f, 0.f}, .rot = 0.f, .duration = 0.1},
		{.pos = {0.f, -7.f}, .rot = 0.f, .duration = 0.08f},
		{.pos = {0.f, -10.f}, .rot = 0.f, .duration = 0.08f},
		{.pos = {0.f, 0.f}, .rot = 0.f, .duration = 0.1f},
	};
	void activate() { anim.activate(nKeyFrames); }
	// returns true if completed
	bool update(f32 dt) { return anim.update(dt, keyFrames, nKeyFrames); }
	v2	 getPos() { return anim.posOffset; }
	f32	 getRot() { return anim.rotOffset; }
};
struct AnimJumpShadow
{
	Animation		 anim;
	static const u32 nKeyFrames			   = 4;
	Keyframe		 keyFrames[nKeyFrames] = {
		{.scale = 1.f, .duration = 0.1f},
		{.scale = 0.6f, .duration = 0.08f},
		{.scale = 0.5f, .duration = 0.08f},
		{.scale = 1.f, .duration = 0.1f},
	};
	void activate() { anim.activate(nKeyFrames); }
	// returns true if completed
	bool update(f32 dt) { return anim.update(dt, keyFrames, nKeyFrames); }
	f32	 getScale() { return anim.scale; }
};
struct AnimBreathe
{
	Animation		 anim;
	static const u32 nKeyFrames			   = 5;
	Keyframe		 keyFrames[nKeyFrames] = {
		{.scale = 1.f, .duration = 0.5f},
		{.scale = 1.05f, .duration = 0.5f},
		{.scale = 1.05f, .duration = 0.5f},
		{.scale = 1.f, .duration = 0.5f},
		{.scale = 1.f, .duration = 0.5f},
	};
	void activate(bool loop) { anim.activate(nKeyFrames, loop); }
	// returns true if completed
	bool update(f32 dt) { return anim.update(dt, keyFrames, nKeyFrames); }
	f32	 getScale() { return anim.scale; }
};
