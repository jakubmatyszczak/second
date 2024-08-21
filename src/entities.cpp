#pragma once
#include <cassert>
#include "engine/basic.cpp"
#include "engine/draw.cpp"

InteractionData& getInteractionData(u32 entityPtr) { return entities.instances[entityPtr].iData; }
Entity&			 entity(u32 entityPtr) { return entities.instances[entityPtr]; }
struct Dude
{
	u32				   pEntity;
	inline static u32  pTexDude	   = 0;
	inline static u32  pTexShadow  = 0;
	inline static u32  pSoundJump  = 0;
	inline static bool initialized = 0;

	SpriteSheet		  ss;
	AnimJump		  aJump;
	AnimJumpShadow	  aJumpShadow;
	AnimBreathe		  aBreathe;
	PlayerInteraction activeHint = NONE;

	v2	 colliderOffset = {0, 2};
	bool busy			= false;

	i32 interactEntityPtr = -1;
	i32 itemRightHand	  = -1;

	constexpr static const char* interactHint[4] = {"INTERACT", "FLIP", "RESTORE", "PICK UP"};

	Entity&		 getEntity() { return entities.instances[pEntity]; }
	static Dude& getRef(u32 pEntity) { return *(Dude*)entities.instances[pEntity].data; }

	void setCollision(u32 group, bool enable)
	{
		Entity* e = &entity(pEntity);
		if (group == 0)
			entities.collidesTerrain[e->instancePtr] = enable;
		if (group == 1)
			entities.collidesGroup1[e->instancePtr] = enable;
		if (group == 2)
			entities.collidesGroup2[e->instancePtr] = enable;
	}

	static void init()
	{
		static_assert(sizeof(Dude) < Entity::MAX_ENTITY_SIZE);
		Dude::pTexDude	 = Content::TEX_DUDE;
		Dude::pTexShadow = Content::TEX_SHADOW;
		Dude::pSoundJump = Content::SOUND_JUMP;
	}
	static u32 add(v2 pos)
	{
		int iPtr = entities.add(Entity::Id::PLAYER,
								Entity::Arch::DUDE,
								pos,
								{.canSelect			= false,
								 .canInteract		= false,
								 .canDraw			= true,
								 .canCollideTerrain = true,
								 .canCollideGroup1	= true,
								 .canCollideGroup2	= true});
		assert(iPtr > -1);
		Dude& dude	 = *(new (entities.instances[iPtr].data) Dude);
		dude.pEntity = iPtr;
		Entity& e	 = entities.instances[iPtr];
		e.iData		 = {.boundingCircle = {e.pos + dude.colliderOffset, 4}};
		dude.ss.init(dude.pTexDude, {8, 8}, 2, 10, true);
		dude.aBreathe.activate(5.f);
		return dude.pEntity;
	};
	void input(bool up, bool down, bool left, bool right, bool interact, bool jump)
	{
		Entity& e = entities.instances[pEntity];
		if (busy)
		{
			e.vel = v2();
			busy  = getInteractionData(interactEntityPtr).shouldPlayerBeBusy;
			if (busy)
				return;
			interactEntityPtr = -1;
		}
		e.vel.x += right - left;
		e.vel.y += up - down;
		e.vel *= 0.5f;
		if (e.vel.isZero())
			aBreathe.anim.period = 5.f;
		else
			aBreathe.anim.period = 1.f;
		if (jump && !aJump.anim.active)
		{
			PlaySound(content.sounds[pSoundJump]);
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
			if (entities.active[i] && i != e.instancePtr)
			{
				f32 distance = e.pos.distToSquared(entities.instances[i].pos);
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
				iData.targetEntityInstance = e.instancePtr;
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
	void drawOverlay()
	{
		if (activeHint > 0)
			DrawText(
				TextFormat("Press [e] to %s", interactHint[activeHint]), 300, 400, 30, DARKBLUE);
	};
};
void dudeUpdate(void* dudePtr, f32 dt)
{
	Dude&	dude	= *(Dude*)dudePtr;
	Entity& e		= entities.instances[dude.pEntity];
	bool	landing = dude.aJump.update(dt);
	dude.aJumpShadow.update(dt);
	dude.aBreathe.update(dt);

	if (landing)
	{
		dude.setCollision(1, true);
		for (u32 i = 0; i < entities.maxEntities; i++)
			if (entities.active[i] && entities.instances[i].id == Entity::Id::PORTAL)
			{
				Entity& hole = entities.instances[i];
				f32		rad	 = getInteractionData(e.instancePtr).boundingCircle.radius +
						  getInteractionData(hole.instancePtr).boundingCircle.radius;
				if (e.pos.distToSquared(hole.pos) < rad * rad)
				{
					e.pos =
						entities
							.instances[getInteractionData(hole.instancePtr).targetEntityInstance]
							.pos;
					break;
				}
			}
	}
	e.pos += e.vel;
	e.iData.boundingCircle.pos = e.pos + dude.colliderOffset;
	dude.ss.update(dt);
}
void dudeDraw(void* dudePtr)
{
	Dude&	   dude			= *(Dude*)dudePtr;
	Entity&	   e			= entities.instances[dude.pEntity];
	Texture2D& texShadow	= content.textures[dude.pTexShadow];
	v2		   shadowSize	= v2(texShadow.width, texShadow.height) * dude.aJumpShadow.getScale();
	v2		   shadowOffset = v2(0, 1) / dude.aJumpShadow.getScale();
	DrawTextureEx(texShadow,
				  (e.pos - shadowSize * 0.5f + shadowOffset).toVector2(),
				  0.f,
				  dude.aJumpShadow.getScale(),
				  WHITE);

	v2	drawPos	  = e.pos + dude.aJump.getPos();
	f32 drawScale = e.scale + dude.aBreathe.getScale();
	if (e.vel.getLengthSquared() < 0.1f)
		dude.ss.Draw(drawPos, WHITE, e.rot, drawScale, 0, 0);
	else if (e.vel.y < -0.05f)	// Should be 0.f, but this looks better
		dude.ss.Draw(drawPos, WHITE, e.rot, drawScale, 3, -1);
	else
		dude.ss.Draw(drawPos, WHITE, e.rot, drawScale, 2, -1);
	if (GLOBAL.drawDebugCollision)
	{
		BoundingCircle& bc = getInteractionData(e.instancePtr).boundingCircle;
		DrawCircleV(bc.pos.toVector2(), bc.radius, RED_TRANSPARENT);
	}
};

struct Hole
{
	u32 pEntity = 0;

	inline static u32  pSsTexture  = 0;
	inline static bool initialized = false;

	SpriteSheet ss;

	static void init()
	{
		static_assert(sizeof(Dude) < Entity::MAX_ENTITY_SIZE);
		pSsTexture	= Content::TEX_HOLE;
		initialized = true;
	}
	static u32 add(v2 pos)
	{
		if (!initialized)
			exitWithMessage("Hole not initialized!");
		i32 iPtr = entities.add(Entity::Id::PORTAL,
								Entity::Arch::HOLE,
								pos,
								{.canSelect			= false,
								 .canInteract		= false,
								 .canDraw			= true,
								 .canCollideTerrain = false,
								 .canCollideGroup1	= true});
		assert(iPtr >= 0);
		Hole&	hole = *(new (entities.instances[iPtr].data) Hole);
		Entity& e	 = entities.instances[iPtr];
		hole.pEntity = iPtr;
		hole.ss.init(pSsTexture, v2(16, 8), 4, 16, true);
		e.iData = {.boundingCircle = {e.pos, 5}};
		return iPtr;
	}
	static void connect(u32 h1InstancePtr, u32 h2InstancePtr)
	{
		entities.instances[h1InstancePtr].iData.targetEntityInstance = h2InstancePtr;
		entities.instances[h2InstancePtr].iData.targetEntityInstance = h1InstancePtr;
	}
};
void holeUpdate(void* hole, f32 dt)
{
	Hole& h = *(Hole*)hole;
	h.ss.update(dt);
}
void holeDraw(void* hole)
{
	Hole&	h = *(Hole*)hole;
	Entity& e = entities.instances[h.pEntity];
	h.ss.Draw(e.pos);
	if (GLOBAL.drawDebugCollision)
	{
		BoundingCircle& bc = getInteractionData(e.instancePtr).boundingCircle;
		DrawCircleV(bc.pos.toVector2(), bc.radius, RED_TRANSPARENT);
	}
}
struct Key
{
	u32 pEntity;

	inline static u32  pTexKey	   = 0;
	inline static u32  pTexShadow  = 0;
	inline static bool initialized = false;

	AnimHoverFloat		 aHover;
	AnimHoverFloatShadow aShadow;
	bool				 isPicked = false;
	Color				 tint	  = WHITE;

	static void init()
	{
		static_assert(sizeof(Dude) < Entity::MAX_ENTITY_SIZE);
		pTexShadow	= Content::TEX_SHADOW;
		pTexKey		= Content::TEX_KEY;
		initialized = true;
	}
	static u32 add(v2 pos)
	{
		if (!initialized)
			exitWithMessage("Key not initialized!");
		int iPtr = entities.add(Entity::Id::ITEM,
								Entity::Arch::KEY,
								pos,
								{.canSelect			= true,
								 .canInteract		= true,
								 .canDraw			= true,
								 .canCollideTerrain = false,
								 .canCollideGroup1	= false});
		assert(iPtr >= 0);
		Entity& e			= entities.instances[iPtr];
		Key&	key			= *(new (e.data) Key);
		key.pEntity			= iPtr;
		e.iData.interaction = PICKUP;

		key.aHover.anim.looped	= true;
		key.aShadow.anim.looped = true;
		key.aHover.activate(3.0f);
		key.aShadow.activate(3.0f);

		return iPtr;
	}
};
void keyUpdate(void* keyPtr, f32 dt)
{
	Key&	key = *(Key*)keyPtr;
	Entity& e	= entities.instances[key.pEntity];
	if (entities.selectedPtr == e.instancePtr)
		key.tint = RED;
	else
		key.tint = WHITE;
	if (entities.interactPtr == e.instancePtr)
	{
		key.isPicked = !key.isPicked;
		e.rot		 = 0.f;
		e.scale		 = 1.f;
		if (key.isPicked)
		{
			e.rot	= 1.f;
			e.scale = 0.7f;
		}
		else
			e.iData.targetEntityInstance = -1;
		entities.selectable[e.instancePtr] = !key.isPicked;
	}

	e.vel = v2();
	if (e.iData.targetEntityInstance >= 0)
	{
		Entity& target = entities.instances[e.iData.targetEntityInstance];
		e.vel		   = (target.pos + e.iData.inventoryOffset - e.pos) * 0.35f;
	}

	e.pos += e.vel;
	key.aHover.update(dt);
	key.aShadow.update(dt);
}
void keyDraw(void* keyPtr)
{
	Key&	   key		  = *(Key*)keyPtr;
	Entity&	   e		  = entities.instances[key.pEntity];
	Texture2D& texShadow  = content.textures[key.pTexShadow];
	Texture2D& texKey	  = content.textures[key.pTexKey];
	v2		   shadowSize = v2(texShadow.width, texShadow.height) * key.aShadow.getScale();
	v2		   keySize	  = v2(texKey.width / 2.f, texKey.height / 2.f) * e.scale;
	if (!key.isPicked)
		DrawTextureEx(texShadow,
					  (e.pos - keySize - shadowSize * 0.5).toVector2(),
					  0.f,
					  e.scale + key.aShadow.getScale(),
					  WHITE);
	DrawTextureEx(texKey,
				  (e.pos + key.aHover.getPos() - keySize).toVector2(),
				  math::radToDeg(e.rot),
				  e.scale,
				  key.tint);
}
struct Table
{
	u32				   pEntity;
	inline static u32  pTexTable   = 0;
	inline static u32  pTexShadow  = 0;
	inline static u32  pSoundWham  = 0;
	inline static bool initialized = false;

	Color		   tint = WHITE;
	AnimFlip	   aFlip;
	AnimJumpShadow aJumpShadow;
	bool		   flipped = true;

	static void init()
	{
		static_assert(sizeof(Dude) < Entity::MAX_ENTITY_SIZE);
		Table::pTexTable  = Content::TEX_TABLE;
		Table::pTexShadow = Content::TEX_SHADOW;
		Table::pSoundWham = Content::SOUND_WHAM;
		initialized		  = true;
	}
	static u32 add(v2 pos)
	{
		if (!initialized)
			exitWithMessage("Table not initialized!");
		int iPtr = entities.add(Entity::Id::OBJECT,
								Entity::Arch::TABLE,
								pos,
								{.canSelect			= true,
								 .canInteract		= true,
								 .canDraw			= true,
								 .canCollideTerrain = false,
								 .canCollideGroup1	= false});
		assert(iPtr >= 0);
		Entity& e			= entities.instances[iPtr];
		Table&	table		= *(new (e.data) Table);
		table.pEntity		= iPtr;
		e.iData.interaction = FLIP;
		return iPtr;
	}
};
void tableUpdate(void* tablePtr, f32 dt)
{
	Table&	table = *(Table*)tablePtr;
	Entity& e	  = entities.instances[table.pEntity];
	if (entities.selectedPtr == e.instancePtr)
		table.tint = RED;
	else
		table.tint = WHITE;
	if (!table.aFlip.anim.active && entities.interactPtr == e.instancePtr)
	{
		f32 flipPeriod = 0.3f;
		if (table.flipped)
			table.aFlip.anim.reversed = false;
		else
		{
			flipPeriod				  = flipPeriod * 3.f;
			table.aFlip.anim.reversed = true;
		}
		entities.selectable[e.instancePtr] = false;
		e.iData.interaction				   = NONE;
		table.aJumpShadow.activate(flipPeriod);
		table.aFlip.activate(flipPeriod);
	}
	if (table.aFlip.update(dt))
	{
		entities.selectable[e.instancePtr] = true;
		if (table.flipped)
		{
			PlaySound(content.sounds[table.pSoundWham]);
			e.iData.shouldPlayerBeBusy = true;
			e.iData.interaction		   = RESTORE;
		}
		else
		{
			e.iData.shouldPlayerBeBusy = false;
			e.iData.interaction		   = FLIP;
		}
		table.flipped = !table.flipped;
	}
	table.aJumpShadow.update(dt);
}
void tableDraw(void* tablePtr)
{
	Table&	   table		= *(Table*)tablePtr;
	Entity&	   e			= entities.instances[table.pEntity];
	Texture2D& texShadow	= content.textures[table.pTexShadow];
	v2		   shadowSize	= v2(texShadow.width, texShadow.height) * table.aJumpShadow.getScale();
	v2		   shadowOffset = v2(0, 1.f) / table.aJumpShadow.getScale();
	DrawTextureEx(texShadow,
				  (e.pos - shadowSize * 0.5f + shadowOffset).toVector2(),
				  0.f,
				  table.aJumpShadow.getScale(),
				  WHITE);

	Texture2D& texTable = content.textures[table.pTexTable];
	v2		   flipPos	= table.aFlip.getPos() * (table.aFlip.anim.reversed ? .25f : 1.f);
	v2		   drawPos	= e.pos + flipPos;
	f32		   drawrot	= math::radToDeg(e.rot + table.aFlip.getRot());
	DrawTexturePro(texTable,
				   {0, 0, (f32)texTable.width, (f32)texTable.height},
				   {drawPos.x, drawPos.y, (f32)texTable.width, (f32)texTable.height},
				   {texTable.width / 2.f, 1.f + texTable.height / 2.f},
				   drawrot,
				   table.tint);
};

struct Gateway
{
	u32		   pEntity;
	v2		   posEnd;
	static i32 add(v2 posStart, v2 posEnd)
	{
		int iPtr = entities.add(Entity::Id::OBJECT,
								Entity::Arch::GATE,
								posStart,
								{.canSelect			= false,
								 .canInteract		= false,
								 .canDraw			= true,
								 .canCollideTerrain = false,
								 .canCollideGroup1	= false,
								 .canCollideGroup2	= true});
		assert(iPtr >= 0);
		Entity&	 e	  = entities.instances[iPtr];
		Gateway& gate = *(new (e.data) Gateway);
		gate.pEntity  = iPtr;
		gate.posEnd	  = posEnd;
		return iPtr;
	}
};
void gateUpdate(void* gatePtr, f32 dt)
{
	Gateway& gate		   = *(Gateway*)gatePtr;
	Entity&	 e			   = entities.instances[gate.pEntity];
	Entity&	 dudeEntity	   = Dude::getRef(GLOBAL.pDudeInstance).getEntity();
	e.iData.boundingCircle = {.pos = math::projectPointOntoLine(dudeEntity.pos, e.pos, gate.posEnd),
							  .radius = 2};
}
void gateDraw(void* gatePtr)
{
	Gateway& gate = *(Gateway*)gatePtr;
	Entity&	 e	  = entities.instances[gate.pEntity];
	DrawLineEx(e.pos.toVector2(), gate.posEnd.toVector2(), 4, PINK);
	if (GLOBAL.drawDebugCollision)
		DrawCircleV(
			e.iData.boundingCircle.pos.toVector2(), e.iData.boundingCircle.radius, RED_TRANSPARENT);
}
