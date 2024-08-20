#pragma once
#include <cstring>
#include "v2.cpp"

extern void dudeUpdate(void* dudePtr, f32 dt);
extern void keyUpdate(void* keyPtr, f32 dt);
extern void tableUpdate(void* tablePtr, f32 dt);
extern void holeUpdate(void* holePtr, f32 dt);
extern void gateUpdate(void* gatePtr, f32 dt);

extern void dudeDraw(void* dudePtr);
extern void keyDraw(void* keyPtr);
extern void tableDraw(void* tablePtr);
extern void holeDraw(void* holePtr);
extern void gateDraw(void* gatePtr);

#define RED_TRANSPARENT \
	(Color) { 255, 0, 0, 128 }
#define GREEN_TRANSPARENT \
	(Color) { 0, 200, 0, 128 }
#define YELLOW_TRANSPARENT \
	(Color) { 200, 200, 0, 128 }

f32			lerp(f32 start, f32 end, f32 time) { return start + (end - start) * time; }
v2			lerp(const v2& start, const v2& end, f32 time) { return start + (end - start) * time; }
inline void swap(f32* x, f32* y)
{
	f32 temp = *x;
	*x		 = *y;
	*y		 = temp;
}
inline void swap(u64* x, u64* y)
{
	u64 temp = *x;
	*x		 = *y;
	*y		 = temp;
}
void bubble_sort(u64 sortArr[], f32 sortBy[], int n)
{
	bool swapped;
	for (i32 i = 0; i < n - 1; i++)
	{
		swapped = false;
		for (i32 j = 0; j < n - i - 1; j++)
		{
			if (sortBy[j] > sortBy[j + 1])
			{
				swap(&sortBy[j], &sortBy[j + 1]);
				swap(&sortArr[j], &sortArr[j + 1]);
				swapped = true;
			}
		}
		if (swapped == false)
			break;
	}
}
enum PlayerInteraction : i32
{
	NONE = -1,
	INTERACT,
	FLIP,
	RESTORE,
	PICKUP,
};
struct BoundingCircle
{
	v2	 pos;
	f32	 radius;
	bool computeCollision(const BoundingCircle& other, v2& collisionVector)
	{
		f32 rad = (radius + other.radius) / 2;
		if (pos.distToSquared(other.pos) > rad * rad)
			return false;
		collisionVector = (other.pos - pos).norm();
		return true;
	}
};
struct InteractionData
{
	inline static const v2 inventoryOffset = v2(10, -4);

	PlayerInteraction interaction;
	bool			  shouldPlayerBeBusy = false;
	BoundingCircle	  boundingCircle;
	i32				  targetEntityInstance = -1;
};
struct Entity
{
	static const u32 MAX_ENTITY_SIZE = 1024;
	enum class Id : u32
	{
		UNKNOWN,
		PLAYER,
		ITEM,
		OBJECT,
		PORTAL,
		// Add Types here
		ID_MAX	// Keep this last
	};
	enum class Arch : u32
	{
		UNKNOWN,
		DUDE,
		TABLE,
		HOLE,
		KEY,
		GATE,
	};
	Id				id					  = Id::UNKNOWN;
	Arch			arch				  = Arch::UNKNOWN;
	u32				instancePtr			  = 0;
	v2				pos					  = {};
	v2				vel					  = {};
	f32				rot					  = 0.f;
	f32				scale				  = 1.f;
	u8				data[MAX_ENTITY_SIZE] = {};
	InteractionData iData				  = {};
};
struct Entities
{
	static const u32 maxEntities = 128;
	struct flagInit
	{
		bool canSelect, canInteract, canDraw, canCollideTerrain, canCollideGroup1;
	};

	Entity instances[maxEntities] = {};

	u8 active[maxEntities]			= {};
	u8 selectable[maxEntities]		= {};
	u8 interactable[maxEntities]	= {};
	u8 drawable[maxEntities]		= {};
	u8 collidesTerrain[maxEntities] = {};
	u8 collidesGroup1[maxEntities]	= {};

	u32 nActive		= 0;
	i32 selectedPtr = -1;
	i32 interactPtr = -1;

	i32 add(Entity::Id idType, Entity::Arch archType, v2 pos, flagInit flags)
	{
		for (u32 i = 0; i < maxEntities; i++)
		{
			if (active[i])
				continue;
			Entity& e = instances[i];
			memset(&e, 0, sizeof(Entity));
			e				   = {.id = idType, .arch = archType, .instancePtr = i, .pos = pos};
			selectable[i]	   = flags.canSelect;
			interactable[i]	   = flags.canInteract;
			drawable[i]		   = flags.canDraw;
			collidesTerrain[i] = flags.canCollideTerrain;
			collidesGroup1[i]  = flags.canCollideGroup1;
			active[i]		   = true;
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
	bool select(u32 instancePtr)
	{
		if (instancePtr < maxEntities)
			if (active[instancePtr] && selectable[instancePtr])
				return selectedPtr = instancePtr;
		return false;
	}
	bool interact(u32 instancePtr)
	{
		if (instancePtr < maxEntities)
			if (active[instancePtr] && interactable[instancePtr])
				return interactPtr = instancePtr;
		return false;
	}
	void refresh()
	{
		selectedPtr = -1;
		interactPtr = -1;
	}
	void updateAll(f32 dt)
	{
		for (u32 i = 0; i < maxEntities; i++)
			if (active[i])
				switch (instances[i].arch)
				{
					case Entity::Arch::DUDE:
						dudeUpdate(&instances[i].data, dt);
						break;
					case Entity::Arch::TABLE:
						tableUpdate(&instances[i].data, dt);
						break;
					case Entity::Arch::HOLE:
						holeUpdate(&instances[i].data, dt);
						break;
					case Entity::Arch::KEY:
						keyUpdate(&instances[i].data, dt);
						break;
					case Entity::Arch::GATE:
						gateUpdate(&instances[i].data, dt);
						break;
					case Entity::Arch::UNKNOWN:
						break;
				}
	}
	void drawAll()
	{
		// This code sorts all entities draw calls, based on their Y position
		// so that in perspective view, objects closer (smaller Y) will be draw over
		// objects that are further
		Entity* sortedEntities[maxEntities] = {};
		f32		posValueY[maxEntities]		= {};
		u32		nSortedEntities				= 0;
		for (u32 i = 0; i < maxEntities; i++)
			if (active[i] && drawable[i])
			{
				sortedEntities[nSortedEntities] = &instances[i];
				posValueY[nSortedEntities]		= instances[i].pos.y;
				nSortedEntities++;
			}
		bubble_sort((u64*)sortedEntities, posValueY, (u64)nSortedEntities);
		for (i32 i = 0; i < nSortedEntities; i++)
			switch (sortedEntities[i]->arch)
			{
				case Entity::Arch::DUDE:
					dudeDraw(&sortedEntities[i]->data);
					break;
				case Entity::Arch::TABLE:
					tableDraw(&sortedEntities[i]->data);
					break;
				case Entity::Arch::HOLE:
					holeDraw(&sortedEntities[i]->data);
					break;
				case Entity::Arch::KEY:
					keyDraw(&sortedEntities[i]->data);
					break;
				case Entity::Arch::GATE:
					gateDraw(&sortedEntities[i]->data);
					break;
				case Entity::Arch::UNKNOWN:
					break;
			}
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
	bool looped	   = false;
	f32	 scale	   = 1.f;
	v2	 posOffset = {};
	f32	 rotOffset = {};

	f32 timer  = 0.f;
	f32 period = 1.f;
	i32 frame  = 0;

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
struct AnimFlip
{
	Animation		 anim;
	static const u32 nKeyFrames			   = 4;
	Keyframe		 keyFrames[nKeyFrames] = {
		{.pos = {0.f, 0.f}, .rot = 0.f, .duration = 0.3},
		{.pos = {0.f, -10.f}, .rot = 1.f, .duration = 0.3f},
		{.pos = {0.f, -10.f}, .rot = 3.14159f, .duration = 0.1f},
		{.pos = {0.f, 0.f}, .rot = 3.14159f, .duration = 0.3f},
	};

	void activate(f32 period) { anim.activate(nKeyFrames, period, false); }
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
		{.pos = {0.f, 0.f}, .rot = 0.f, .duration = 0.3},
		{.pos = {0.f, -7.f}, .rot = 0.f, .duration = 0.2f},
		{.pos = {0.f, -10.f}, .rot = 0.f, .duration = 0.2f},
		{.pos = {0.f, 0.f}, .rot = 0.f, .duration = 0.3f},
	};
	void activate(f32 period) { anim.activate(nKeyFrames, period, false); }
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
		{.scale = 1.f, .duration = 0.3f},
		{.scale = 0.6f, .duration = 0.2f},
		{.scale = 0.5f, .duration = 0.2f},
		{.scale = 1.f, .duration = 0.3f},
	};
	void activate(f32 period) { anim.activate(nKeyFrames, period, false); }
	// returns true if completed
	bool update(f32 dt) { return anim.update(dt, keyFrames, nKeyFrames); }
	f32	 getScale() { return anim.scale; }
};
struct AnimBreathe
{
	Animation		 anim;
	static const u32 nKeyFrames			   = 5;
	Keyframe		 keyFrames[nKeyFrames] = {
		{.scale = 0.f, .duration = 0.2f},
		{.scale = 0.05f, .duration = 0.2f},
		{.scale = 0.05f, .duration = 0.2f},
		{.scale = 0.f, .duration = 0.2f},
		{.scale = 0.f, .duration = 0.2f},
	};
	void activate(f32 period) { anim.activate(nKeyFrames, period, true); }
	// returns true if completed
	bool update(f32 dt) { return anim.update(dt, keyFrames, nKeyFrames); }
	f32	 getScale() { return anim.scale; }
};
struct AnimHoverFloat
{
	Animation		 anim;
	static const u32 nKeyFrames			   = 3;
	Keyframe		 keyFrames[nKeyFrames] = {
		{.pos = {0.f, -4.f}, .duration = 0.33f},
		{.pos = {0.f, -3.f}, .duration = 0.33f},
		{.pos = {0.f, -4.f}, .duration = 0.34f},
	};
	void activate(f32 period) { anim.activate(nKeyFrames, period, true); }
	// returns true if completed
	bool update(f32 dt) { return anim.update(dt, keyFrames, nKeyFrames); }
	f32	 getScale() { return anim.scale; }
	v2	 getPos() { return anim.posOffset; }
};
struct AnimHoverFloatShadow
{
	Animation		 anim;
	static const u32 nKeyFrames			   = 3;
	Keyframe		 keyFrames[nKeyFrames] = {
		{.scale = -0.3f, .duration = 0.33f},
		{.scale = -0.1f, .duration = 0.33f},
		{.scale = -0.3f, .duration = 0.34f},
	};
	void activate(f32 period) { anim.activate(nKeyFrames, period, true); }
	// returns true if completed
	bool update(f32 dt) { return anim.update(dt, keyFrames, nKeyFrames); }
	f32	 getScale() { return anim.scale; }
	v2	 getPos() { return anim.posOffset; }
};
