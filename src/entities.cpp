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
	Texture2D		  texShadow;
	AnimJump		  aJump;
	AnimJumpShadow	  aJumpShadow;
	PlayerInteraction activeHint = NONE;

	v2 vel;

	constexpr static const char* interactHint[4] = {"INTERACT", "FLIP", "RESTORE", "TEST"};

	bool init(Texture& tDude, Texture& tShadow, v2 pos)
	{
		int iPtr = entities.add(Entity::Id::PLAYER, this, nullptr, pos, 0.f, true, true);
		if (iPtr < 0)
			return false;
		e = &entities.instances[iPtr];
		ss.init(tDude, {8, 8}, 2, 10, true);
		texShadow = tShadow;
		return true;
	}
	void kill() { entities.remove(e->instancePtr); }
	void input(bool up, bool down, bool left, bool right, bool interact, bool jump)
	{
		vel.x = right - left;
		vel.y = up - down;
		if (jump && !aJump.anim.active)
		{
			aJump.activate();
			aJumpShadow.activate();
		}

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
		aJumpShadow.update(0.016f);
		e->pos += vel;
		ss.update(0.016f);
	}
	void draw()
	{
		v2 shadowSize	= v2(texShadow.width, texShadow.height) * aJumpShadow.getScale();
		v2 shadowOffset = v2(0, 1) / aJumpShadow.getScale();
		DrawTextureEx(texShadow,
					  (e->pos - shadowSize * 0.5f + shadowOffset).toVector2(),
					  0.f,
					  aJumpShadow.getScale(),
					  WHITE);

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

	Color	 tint = WHITE;
	AnimFlip flip;
	bool	 flipped = true;

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
		if (!flip.anim.active && entities.interactPtr == e->instancePtr)
		{
			if (flipped)
				flip.anim.reversed = false;
			else
				flip.anim.reversed = true;
			entities.selectable[e->instancePtr] = false;
			props.interaction					= NONE;
			flip.activate();
		}
		if (flip.update(dt))
		{
			entities.selectable[e->instancePtr] = true;
			if (flipped)
				props.interaction = RESTORE;
			else
				props.interaction = FLIP;
            flipped = !flipped;
		}
	}
	void draw()
	{
        v2 flipPos = flip.getPos() * (flip.anim.reversed ? .25f : 1.f);
		v2	drawPos = e->pos + flipPos;
		f32 drawrot = math::radToDeg(e->rot + flip.getRot());
		DrawTexturePro(*tex,
					   {0, 0, (f32)tex->width, (f32)tex->height},
					   {drawPos.x, drawPos.y, (f32)tex->width, (f32)tex->height},
					   {tex->width / 2.f, 1.f + tex->height / 2.f},
					   drawrot,
					   tint);
	};
};
