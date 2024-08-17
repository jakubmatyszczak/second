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
struct InteractionProperties
{
	PlayerInteraction interaction;
	bool			  shouldPlayerBeBusy = false;
	BoundingCircle	  boundingCircle;
	Entity*			  targetEntity;
};
InteractionProperties* getInteractionProperties(u32 entityPtr)
{
	return ((InteractionProperties*)entities.instances[entityPtr].properties);
}
struct Dude
{
	Entity*				  e;
	SpriteSheet			  ss;
	Texture2D			  tShadow;
	Sound*				  sJump;
	AnimJump			  aJump;
	AnimJumpShadow		  aJumpShadow;
	AnimBreathe			  aBreathe;
	PlayerInteraction	  activeHint = NONE;
	InteractionProperties ip;

	v2 colliderOffset = {0, 2};

	bool busy				   = false;
	i32	 interactEntityPtr	   = -1;
	i32	 collidesWithGroup1[4] = {};

	constexpr static const char* interactHint[4] = {"INTERACT", "FLIP", "RESTORE", "TEST"};

	void setCollision(u32 group, bool enable)
	{
		if (group == 0)
			entities.collidesTerrain[e->instancePtr] = enable;
		if (group == 1)
			entities.collidesGroup1[e->instancePtr] = enable;
	}

	bool init(Texture& texDude, Texture& texShadow, Sound& soundJump, v2 pos)
	{
		int iPtr = entities.add(
			Entity::Id::PLAYER, this, &ip, pos, 0.f, update, draw, false, false, true, true);
		if (iPtr < 0)
			return false;
		e  = &entities.instances[iPtr];
		ip = {.boundingCircle = {e->pos + colliderOffset, 4}};
		ss.init(texDude, {8, 8}, 2, 10, true);
		tShadow = texShadow;
		sJump	= &soundJump;
		aBreathe.activate(5.f);
		return true;
	}
	void kill() { entities.remove(e->instancePtr); }
	void input(bool up, bool down, bool left, bool right, bool interact, bool jump)
	{
		if (busy)
		{
			e->vel = v2();
			busy   = getInteractionProperties(interactEntityPtr)->shouldPlayerBeBusy;
			if (busy)
				return;
			interactEntityPtr = -1;
		}
		e->vel.x = right - left;
		e->vel.y = up - down;
		if (e->vel.isZero())
			aBreathe.anim.period = 5.f;
		else
			aBreathe.anim.period = 1.f;
		if (jump && !aJump.anim.active)
		{
			PlaySound(*sJump);
			aJump.activate(0.4f);
			aJumpShadow.activate(0.4f);
		}
		if (aJump.anim.active)
			setCollision(1, false);
		else
			setCollision(1, true);

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
		if (entities.select(closestEntityPtr))
			activeHint = ((InteractionProperties*)entities.instances[closestEntityPtr].properties)
							 ->interaction;
		if (interact)
		{
			if (!entities.interact(closestEntityPtr))
				return;
			interactEntityPtr = closestEntityPtr;
			busy = ((InteractionProperties*)entities.instances[closestEntityPtr].properties)
					   ->shouldPlayerBeBusy;
		}
	}
	static void update(void* dudePtr, f32 dt)
	{
		Dude& dude	  = *(Dude*)dudePtr;
		bool  landing = dude.aJump.update(dt);
		dude.aJumpShadow.update(dt);
		dude.aBreathe.update(dt);

		if (landing)
		{
			dude.setCollision(1, true);
			for (u32 i = 0; i < entities.maxEntities; i++)
				if (entities.active[i] && entities.instances[i].id == Entity::Id::PORTAL)
				{
					Entity& hole = entities.instances[i];
					f32 rad = getInteractionProperties(dude.e->instancePtr)->boundingCircle.radius +
							  getInteractionProperties(hole.instancePtr)->boundingCircle.radius;
					if (dude.e->pos.distToSquared(hole.pos) < rad * rad)
					{
						dude.e->pos = getInteractionProperties(hole.instancePtr)->targetEntity->pos;
						break;
					}
				}
		}

		dude.e->pos += dude.e->vel;
		dude.ip.boundingCircle.position = dude.e->pos + dude.colliderOffset;
		dude.ss.update(dt);
	}
	static void draw(void* dudePtr)
	{
		Dude& dude		= *(Dude*)dudePtr;
		v2 shadowSize	= v2(dude.tShadow.width, dude.tShadow.height) * dude.aJumpShadow.getScale();
		v2 shadowOffset = v2(0, 1) / dude.aJumpShadow.getScale();
		DrawTextureEx(dude.tShadow,
					  (dude.e->pos - shadowSize * 0.5f + shadowOffset).toVector2(),
					  0.f,
					  dude.aJumpShadow.getScale(),
					  WHITE);

		v2	drawPos	  = dude.e->pos + dude.aJump.getPos();
		f32 drawScale = dude.aBreathe.getScale();
		if (dude.e->vel.getLengthSquared() < 0.1f)
			dude.ss.Draw(drawPos, WHITE, dude.e->rot, drawScale, 0, 0);
		else if (dude.e->vel.y < 0)
			dude.ss.Draw(drawPos, WHITE, dude.e->rot, drawScale, 3, -1);
		else
			dude.ss.Draw(drawPos, WHITE, dude.e->rot, drawScale, 2, -1);
		if (GLOBAL.drawDebugCollision)
		{
			BoundingCircle& bc = getInteractionProperties(dude.e->instancePtr)->boundingCircle;
			DrawCircleV(bc.position.toVector2(), bc.radius, RED_TRANSPARENT);
		}
	};
	void drawOverlay()
	{
		if (activeHint > 0)
			DrawText(
				TextFormat("Press [e] to %s", interactHint[activeHint]), 300, 400, 30, DARKBLUE);
	};
};
struct Hole
{
	Entity*				  e;
	SpriteSheet			  ss;
	InteractionProperties ip;

	bool init(Texture2D& hole, v2 pos)
	{
		i32 iPtr = entities.add(
			Entity::Id::PORTAL, this, &ip, pos, 0.f, update, draw, false, false, false, true);
		ss.init(hole, v2(16, 8), 4, 16, true);
		e  = &entities.instances[iPtr];
		ip = {.boundingCircle = {e->pos, 5}};
		return true;
	}
	void connect(Hole& target)
	{
		ip.targetEntity		   = target.e;
		target.ip.targetEntity = e;
	}
	static void update(void* hole, f32 dt)
	{
		Hole& h = *(Hole*)hole;
		h.ss.update(dt);
	}
	static void draw(void* hole)
	{
		Hole& h = *(Hole*)hole;
		h.ss.Draw(h.e->pos);
		if (GLOBAL.drawDebugCollision)
		{
			BoundingCircle& bc = getInteractionProperties(h.e->instancePtr)->boundingCircle;
			DrawCircleV(bc.position.toVector2(), bc.radius, RED_TRANSPARENT);
		}
	};
};
struct Table
{
	Entity*				  e;
	Texture2D*			  tTable;
	Texture2D*			  tShadow;
	Sound*				  sWham;
	InteractionProperties props;

	Color		   tint = WHITE;
	AnimFlip	   aFlip;
	AnimJumpShadow aJumpShadow;
	bool		   flipped = true;

	bool init(Texture& table, Texture& shadow, Sound& wham, v2 pos)
	{
		int iPtr =
			entities.add(Entity::Id::OBJECT, this, &props, pos, 0.f, update, draw, true, true);
		if (iPtr < 0)
			return false;
		tTable			  = &table;
		tShadow			  = &shadow;
		sWham			  = &wham;
		e				  = &entities.instances[iPtr];
		props.interaction = FLIP;
		return true;
	}
	static void update(void* tablePtr, f32 dt)
	{
		Table& table = *(Table*)tablePtr;
		if (entities.selectedPtr == table.e->instancePtr)
			table.tint = RED;
		else
			table.tint = WHITE;
		if (!table.aFlip.anim.active && entities.interactPtr == table.e->instancePtr)
		{
			f32 flipPeriod = 0.3f;
			if (table.flipped)
				table.aFlip.anim.reversed = false;
			else
			{
				flipPeriod				  = flipPeriod * 3.f;
				table.aFlip.anim.reversed = true;
			}
			entities.selectable[table.e->instancePtr] = false;
			table.props.interaction					  = NONE;
			table.aJumpShadow.activate(flipPeriod);
			table.aFlip.activate(flipPeriod);
		}
		if (table.aFlip.update(dt))
		{
			entities.selectable[table.e->instancePtr] = true;
			if (table.flipped)
			{
				PlaySound(*table.sWham);
				table.props.shouldPlayerBeBusy = true;
				table.props.interaction		   = RESTORE;
			}
			else
			{
				table.props.shouldPlayerBeBusy = false;
				table.props.interaction		   = FLIP;
			}
			table.flipped = !table.flipped;
		}
		table.aJumpShadow.update(dt);
	}
	static void draw(void* tablePtr)
	{
		Table& table = *(Table*)tablePtr;
		v2	   shadowSize =
			v2(table.tShadow->width, table.tShadow->height) * table.aJumpShadow.getScale();
		v2 shadowOffset = v2(0, 1.f) / table.aJumpShadow.getScale();
		DrawTextureEx(*table.tShadow,
					  (table.e->pos - shadowSize * 0.5f + shadowOffset).toVector2(),
					  0.f,
					  table.aJumpShadow.getScale(),
					  WHITE);

		v2	flipPos = table.aFlip.getPos() * (table.aFlip.anim.reversed ? .25f : 1.f);
		v2	drawPos = table.e->pos + flipPos;
		f32 drawrot = math::radToDeg(table.e->rot + table.aFlip.getRot());
		DrawTexturePro(*table.tTable,
					   {0, 0, (f32)table.tTable->width, (f32)table.tTable->height},
					   {drawPos.x, drawPos.y, (f32)table.tTable->width, (f32)table.tTable->height},
					   {table.tTable->width / 2.f, 1.f + table.tTable->height / 2.f},
					   drawrot,
					   table.tint);
	};
};
