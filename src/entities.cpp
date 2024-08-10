#pragma once
#include "engine/basic.cpp"
#include "engine/draw.cpp"

struct Dude
{
	Entity*		e;
	SpriteSheet ss; 
    Jump aJump;

	v2 vel;

	bool init(Texture& t, v2 pos)
	{
		int iPtr = entities.add(Entity::Id::PLAYER, this, pos, 0.f, true, true);
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
        if(jump && !aJump.anim.active)
            aJump.activate();

		i32 closestEntity	= -1;
		f32 closestDistance = MAXFLOAT;
		for (u32 i = 0; i < Entities::maxEntities; i++)
		{
			if (entities.active[i] && i != e->instancePtr)
			{
				f32 distance = e->pos.distToSquared(entities.instances[i].pos);
				if (distance < 100 && distance < closestDistance)
				{
					closestDistance = distance;
					closestEntity	= i;
				}
			}
		}
        entities.select(closestEntity);
        if(interact)
            entities.interact(closestEntity);
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
};

struct Table
{
	Entity*	   e;
	Texture2D* tex;

    Color tint = WHITE;
    Flip flip; 

	bool init(Texture& t, v2 pos)
	{
		int iPtr = entities.add(Entity::Id::OBJECT, this, pos, 0.f, true, true);
		if (iPtr < 0)
			return false;
		tex = &t;
		e	= &entities.instances[iPtr];
		return true;
	}
	void update(f32 dt) 
    {
        if(entities.selectedPtr == e->instancePtr)
            tint = RED;
        else
            tint = WHITE;
        if(entities.interactPtr == e->instancePtr)
            flip.activate();
        flip.update(dt);
    }
	void draw()
	{
        v2 drawPos = e->pos + flip.getPos();
        f32 drawrot = math::radToDeg(e->rot + flip.getRot());
		DrawTexturePro(*tex,
					   {0, 0, (f32)tex->width, (f32)tex->height},
					   {drawPos.x, drawPos.y, (f32)tex->width, (f32)tex->height},
					   {(f32)tex->width / 2, (f32)tex->height / 2},
					   drawrot,
					   tint);
	};
};
