#pragma once
#include "engine/basic.cpp"
#include "engine/draw.cpp"

enum PlayerInteraction : i32
{
	NONE = -1,
	INTERACT,
	FLIP,
	RESTORE
};
struct EntityProperties
{
	PlayerInteraction interaction;
};
struct Dude
{
	Entity*			  e;
	SpriteSheet		  ss;
	Jump			  aJump;
	PlayerInteraction activeHint = NONE;

	v2 vel;

	constexpr static const char* interactHint[4] = {"INTERACT",
													"FLIP",
													"RESTORE",
													"USRAJ"};

	bool init(Texture& t, v2 pos)
	{
		int iPtr = entities.add(Entity::Id::PLAYER, this, nullptr, pos, 0.f, true, true);
		if (iPtr < 0)
			return false;
		e = &entities.instances[iPtr];
		ss.init(t, {8, 8}, 2, 10, true);
		return true;
	}
	void kill() { entities.remove(e->instancePtr); }
	void input(bool up, bool down, bool left, bool right, bool interact, bool jump)
	{
		vel.x = right - left;
		vel.y = up - down;
		if (jump && !aJump.anim.active)
			aJump.activate();

		activeHint			 = NONE;
		i32 closestEntityPtr = -1;
		f32 closestDistance	 = MAXFLOAT;
		for (u32 i = 0; i < Entities::maxEntities; i++)
		{
			if (entities.active[i] && i != e->instancePtr)
			{
				f32 distance = e->pos.distToSquared(entities.instances[i].pos);
				if (distance < 100 && distance < closestDistance)
				{
					closestDistance	 = distance;
					closestEntityPtr = i;
				}
			}
		}
		if (closestEntityPtr == -1)
			return;
		entities.select(closestEntityPtr);
		activeHint =
			((EntityProperties*)entities.instances[closestEntityPtr].properties)->interaction;
		if (interact)
			entities.interact(closestEntityPtr);
	}
	void update()
	{
		aJump.update(0.016f);
		e->pos += vel;
		ss.update(0.016f);
	}
	void draw()
	{
		v2 drawPos = e->pos + aJump.getPos();
		if (vel.getLengthSquared() < 1.f)
			return ss.Draw(drawPos, WHITE, e->rot, 1.0, 0, 0);
		if (vel.y < 0)
			return ss.Draw(drawPos, WHITE, e->rot, 1.0, 3, -1);
		ss.Draw(drawPos, WHITE, e->rot, 1.0, 2, -1);
	};
	void drawOverlay()
	{
		if (activeHint > 0)
			DrawText(
				TextFormat("Press [e] to %s", interactHint[activeHint]), 300, 400, 30, DARKBLUE);
	};
};

struct Table
{
	Entity*			 e;
	Texture2D*		 tex;
	EntityProperties props;

	Color tint = WHITE;
	Flip  flip;
	bool  flipped = false;

	bool init(Texture& t, v2 pos)
	{
		int iPtr = entities.add(Entity::Id::OBJECT, this, &props, pos, 0.f, true, true, true);
		if (iPtr < 0)
			return false;
		tex				  = &t;
		e				  = &entities.instances[iPtr];
		props.interaction = FLIP;
		return true;
	}
	void update(f32 dt)
	{
		if (entities.selectedPtr == e->instancePtr)
			tint = RED;
		else
			tint = WHITE;
		if (entities.interactPtr == e->instancePtr && !flipped)
		{
            entities.selectable[e->instancePtr] = false;
            props.interaction = NONE;
			flipped = true;
			flip.activate();
		}
		flip.update(dt);
	}
	void draw()
	{
		v2	drawPos = e->pos + flip.getPos();
		f32 drawrot = math::radToDeg(e->rot + flip.getRot());
		DrawTexturePro(*tex,
					   {0, 0, (f32)tex->width, (f32)tex->height},
					   {drawPos.x, drawPos.y, (f32)tex->width, (f32)tex->height},
					   {(f32)tex->width / 2, (f32)tex->height / 2},
					   drawrot,
					   tint);
	};
};
