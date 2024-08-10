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
	AnimBreathe		  aBreathe;
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
		aBreathe.activate(true);
		return true;
	}
	void kill() { entities.remove(e->instancePtr); }
	void input(bool up, bool down, bool left, bool right, bool interact, bool jump)
	{
		vel.x = right - left;
		vel.y = up - down;
		if (vel.getLengthSquared() > 0.1f)
			aBreathe.anim.tempo = 10.f;
		else
			aBreathe.anim.tempo = 1.f;
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
	void update(f32 dt)
	{
		aJump.update(dt);
		aJumpShadow.update(dt);
		aBreathe.update(dt);
		e->pos += vel;
		ss.update(dt);
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

		v2	drawPos	  = e->pos + aJump.getPos();
		f32 drawScale = aBreathe.getScale();
		if (vel.getLengthSquared() < 1.f)
			return ss.Draw(drawPos, WHITE, e->rot, drawScale, 0, 0);
		if (vel.y < 0)
			return ss.Draw(drawPos, WHITE, e->rot, drawScale, 3, -1);
		ss.Draw(drawPos, WHITE, e->rot, drawScale, 2, -1);
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
	Texture2D*		 texShadow;
	EntityProperties props;

	Color		   tint = WHITE;
	AnimFlip	   aFlip;
	AnimJumpShadow aJumpShadow;
	bool		   flipped = true;

	bool init(Texture& tTable, Texture& tShadow, v2 pos)
	{
		int iPtr = entities.add(Entity::Id::OBJECT, this, &props, pos, 0.f, true, true, true);
		if (iPtr < 0)
			return false;
		tex				  = &tTable;
		texShadow		  = &tShadow;
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
		if (!aFlip.anim.active && entities.interactPtr == e->instancePtr)
		{
			if (flipped)
				aFlip.anim.reversed = false;
			else
				aFlip.anim.reversed = true;
			entities.selectable[e->instancePtr] = false;
			props.interaction					= NONE;
			aJumpShadow.activate();
			aFlip.activate();
		}
		if (aFlip.update(dt))
		{
			entities.selectable[e->instancePtr] = true;
			if (flipped)
				props.interaction = RESTORE;
			else
				props.interaction = FLIP;
			flipped = !flipped;
		}
		aJumpShadow.update(dt);
	}
	void draw()
	{
		v2 shadowSize	= v2(texShadow->width, texShadow->height) * aJumpShadow.getScale();
		v2 shadowOffset = v2(0, 1.f) / aJumpShadow.getScale();
		DrawTextureEx(*texShadow,
					  (e->pos - shadowSize * 0.5f + shadowOffset).toVector2(),
					  0.f,
					  aJumpShadow.getScale(),
					  WHITE);

		v2	flipPos = aFlip.getPos() * (aFlip.anim.reversed ? .25f : 1.f);
		v2	drawPos = e->pos + flipPos;
		f32 drawrot = math::radToDeg(e->rot + aFlip.getRot());
		DrawTexturePro(*tex,
					   {0, 0, (f32)tex->width, (f32)tex->height},
					   {drawPos.x, drawPos.y, (f32)tex->width, (f32)tex->height},
					   {tex->width / 2.f, 1.f + tex->height / 2.f},
					   drawrot,
					   tint);
	};
};
