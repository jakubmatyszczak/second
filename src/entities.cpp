#pragma once
#include <cassert>
#include "engine/basic.cpp"
#include "engine/draw.cpp"

InteractionData& getInteractionData(u32 entityPtr) { return entities.instances[entityPtr].iData; }
struct Dude
{
	Entity*			  e;
	SpriteSheet		  ss;
	Texture2D		  tShadow;
	Sound*			  sJump;
	AnimJump		  aJump;
	AnimJumpShadow	  aJumpShadow;
	AnimBreathe		  aBreathe;
	PlayerInteraction activeHint = NONE;

	v2	 colliderOffset = {0, 2};
	bool busy			= false;

	i32 interactEntityPtr = -1;
	i32 itemRightHand	  = -1;

	constexpr static const char* interactHint[4] = {"INTERACT", "FLIP", "RESTORE", "PICK UP"};

	void setCollision(u32 group, bool enable)
	{
		if (group == 0)
			entities.collidesTerrain[e->instancePtr] = enable;
		if (group == 1)
			entities.collidesGroup1[e->instancePtr] = enable;
	}

	static Dude& init(Texture& texDude, Texture& texShadow, Sound& soundJump, v2 pos)
	{
		int iPtr = entities.add(Entity::Id::PLAYER,
								Entity::Arch::DUDE,
								pos,
								{.canSelect			= false,
								 .canInteract		= false,
								 .canCollideTerrain = true,
								 .canCollideGroup1	= true},
								update,
								draw);
        assert(iPtr > -1);
		Dude& dude	  = *(new (entities.instances[iPtr].data) Dude);
		dude.e		  = &entities.instances[iPtr];
		dude.e->iData = {.boundingCircle = {dude.e->pos + dude.colliderOffset, 4}};
		dude.ss.init(texDude, {8, 8}, 2, 10, true);
		dude.tShadow = texShadow;
		dude.sJump	 = &soundJump;
		dude.aBreathe.activate(5.f);
		return dude;
	}
	void input(bool up, bool down, bool left, bool right, bool interact, bool jump)
	{
		if (busy)
		{
			e->vel = v2();
			busy   = getInteractionData(interactEntityPtr).shouldPlayerBeBusy;
			if (busy)
				return;
			interactEntityPtr = -1;
		}
		e->vel.x += right - left;
		e->vel.y += up - down;
		e->vel *= 0.5f;
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
		if (interact && closestEntityPtr >= 0)
		{
			if (!entities.interact(closestEntityPtr))
				return;
			InteractionData& iData = getInteractionData(closestEntityPtr);
			if (entities.instances[closestEntityPtr].id == Entity::Id::ITEM)
			{
				itemRightHand			   = closestEntityPtr;
				iData.targetEntityInstance = e->instancePtr;
			}
			interactEntityPtr = closestEntityPtr;
			busy			  = iData.shouldPlayerBeBusy;
		}
		else if (interact && closestEntityPtr < 0 && itemRightHand >= 0)
		{
			if (entities.interact(itemRightHand))
				itemRightHand = -1;
		}
		if (closestEntityPtr >= 0 && entities.select(closestEntityPtr))
			activeHint = getInteractionData(closestEntityPtr).interaction;
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
					f32		rad	 = getInteractionData(dude.e->instancePtr).boundingCircle.radius +
							  getInteractionData(hole.instancePtr).boundingCircle.radius;
					if (dude.e->pos.distToSquared(hole.pos) < rad * rad)
					{
						dude.e->pos = entities
										  .instances[getInteractionData(hole.instancePtr)
														 .targetEntityInstance]
										  .pos;
						break;
					}
				}
		}
		dude.e->pos += dude.e->vel;
		dude.e->iData.boundingCircle.position = dude.e->pos + dude.colliderOffset;
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
		f32 drawScale = dude.e->scale + dude.aBreathe.getScale();
		if (dude.e->vel.getLengthSquared() < 0.1f)
			dude.ss.Draw(drawPos, WHITE, dude.e->rot, drawScale, 0, 0);
		else if (dude.e->vel.y < -0.05f)  // Should be 0.f, but this looks better
			dude.ss.Draw(drawPos, WHITE, dude.e->rot, drawScale, 3, -1);
		else
			dude.ss.Draw(drawPos, WHITE, dude.e->rot, drawScale, 2, -1);
		if (GLOBAL.drawDebugCollision)
		{
			BoundingCircle& bc = getInteractionData(dude.e->instancePtr).boundingCircle;
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
	Entity*		e;
	SpriteSheet ss;

	static Hole& init(Texture2D& tHole, v2 pos)
	{
		i32	  iPtr = entities.add(Entity::Id::PORTAL,
								  Entity::Arch::HOLE,
								  pos,
								  {.canSelect		  = false,
								   .canInteract		  = false,
								   .canCollideTerrain = false,
								   .canCollideGroup1  = true},
								  update,
								  draw);
        assert(iPtr >= 0);
		Hole& hole = *(new (entities.instances[iPtr].data) Hole);
		hole.ss.init(tHole, v2(16, 8), 4, 16, true);
		hole.e		  = &entities.instances[iPtr];
		hole.e->iData = {.boundingCircle = {hole.e->pos, 5}};
		return hole;
	}
	void connect(Hole& target)
	{
		e->iData.targetEntityInstance		 = target.e->instancePtr;
		target.e->iData.targetEntityInstance = e->instancePtr;
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
			BoundingCircle& bc = getInteractionData(h.e->instancePtr).boundingCircle;
			DrawCircleV(bc.position.toVector2(), bc.radius, RED_TRANSPARENT);
		}
	};
};
struct Key
{
	Entity* e;

	Texture2D*			 tKey;
	Texture2D*			 tShadow;
	Color				 tint = WHITE;
	AnimHoverFloat		 aHover;
	AnimHoverFloatShadow aShadow;
	bool				 isPicked = false;

	static bool init(Texture& tKey, Texture& shadow, v2 pos)
	{
		int iPtr = entities.add(Entity::Id::ITEM,
								Entity::Arch::KEY,
								pos,
								{.canSelect			= true,
								 .canInteract		= true,
								 .canCollideTerrain = false,
								 .canCollideGroup1	= false},
								update,
								draw);
		if (iPtr < 0)
			return false;
		Entity& entity			 = entities.instances[iPtr];
		Key&	key				 = *(new (entity.data) Key);
		key.tKey				 = &tKey;
		key.tShadow				 = &shadow;
		key.e					 = &entities.instances[iPtr];
		entity.iData.interaction = PICKUP;

		key.aHover.anim.looped	= true;
		key.aShadow.anim.looped = true;
		key.aHover.activate(3.0f);
		key.aShadow.activate(3.0f);

		return true;
	}
	static void update(void* keyPtr, f32 dt)
	{
		Key& key = *(Key*)keyPtr;
		if (entities.selectedPtr == key.e->instancePtr)
			key.tint = RED;
		else
			key.tint = WHITE;
		if (entities.interactPtr == key.e->instancePtr)
		{
			key.isPicked = !key.isPicked;
			key.e->rot	 = 0.f;
			key.e->scale = 1.f;
			if (key.isPicked)
			{
				key.e->rot	 = 1.f;
				key.e->scale = 0.7f;
			}
			else
				key.e->iData.targetEntityInstance = -1;
			entities.selectable[key.e->instancePtr] = !key.isPicked;
		}

		key.e->vel = v2();
		if (key.e->iData.targetEntityInstance >= 0)
		{
			Entity& target = entities.instances[key.e->iData.targetEntityInstance];
			key.e->vel	   = (target.pos + key.e->iData.inventoryOffset - key.e->pos) * 0.35f;
		}

		key.e->pos += key.e->vel;
		key.aHover.update(dt);
		key.aShadow.update(dt);
	}
	static void draw(void* keyPtr)
	{
		Key& key		= *(Key*)keyPtr;
		v2	 shadowSize = v2(key.tShadow->width, key.tShadow->height) * key.aShadow.getScale();
		v2	 keySize	= v2(key.tKey->width / 2.f, key.tKey->height / 2.f) * key.e->scale;
		if (!key.isPicked)
			DrawTextureEx(*key.tShadow,
						  (key.e->pos - keySize - shadowSize * 0.5).toVector2(),
						  0.f,
						  key.e->scale + key.aShadow.getScale(),
						  WHITE);
		DrawTextureEx(*key.tKey,
					  (key.e->pos + key.aHover.getPos() - keySize).toVector2(),
					  math::radToDeg(key.e->rot),
					  key.e->scale,
					  key.tint);
	}
};
struct Table
{
	Entity*	   e;
	Texture2D* tTable;
	Texture2D* tShadow;
	Sound*	   sWham;

	Color		   tint = WHITE;
	AnimFlip	   aFlip;
	AnimJumpShadow aJumpShadow;
	bool		   flipped = true;

	static bool init(Texture& tTable, Texture& shadow, Sound& wham, v2 pos)
	{
		int iPtr = entities.add(Entity::Id::OBJECT,
								Entity::Arch::TABLE,
								pos,
								{.canSelect			= true,
								 .canInteract		= true,
								 .canCollideTerrain = false,
								 .canCollideGroup1	= false},
								update,
								draw);
		if (iPtr < 0)
			return false;
		Entity& entity			 = entities.instances[iPtr];
		Table&	table			 = *(new (entity.data) Table);
		table.e					 = &entity;
		table.tTable			 = &tTable;
		table.tShadow			 = &shadow;
		table.sWham				 = &wham;
		entity.iData.interaction = FLIP;
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
			table.e->iData.interaction				  = NONE;
			table.aJumpShadow.activate(flipPeriod);
			table.aFlip.activate(flipPeriod);
		}
		if (table.aFlip.update(dt))
		{
			entities.selectable[table.e->instancePtr] = true;
			if (table.flipped)
			{
				PlaySound(*table.sWham);
				table.e->iData.shouldPlayerBeBusy = true;
				table.e->iData.interaction		  = RESTORE;
			}
			else
			{
				table.e->iData.shouldPlayerBeBusy = false;
				table.e->iData.interaction		  = FLIP;
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
