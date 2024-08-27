#pragma once
#include <cassert>
#include "raylib.h"
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
	void input(bool up, bool down, bool left, bool right, bool interact, bool jump, bool use)
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
		f32 velScale = 0.3f;
		e.vel.x += (right - left) * velScale;
		e.vel.y += (up - down) * velScale;
		e.vel *= 0.8f;
		if (e.vel.isZero())
			aBreathe.anim.period = 5.f;
		else
			aBreathe.anim.period = 1.f;
		if (jump && !aJump.anim.active)
		{
			PlaySound(CONTENT.sounds[pSoundJump]);
			aJump.activate(0.4f);
			aJumpShadow.activate(0.4f);
		}
		if (aJump.anim.active)
			entities.setCollision(e.instancePtr, 1, false);
		else
			entities.setCollision(e.instancePtr, 1, true);

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
			InteractionData& closestiData = getInteractionData(closestEntityPtr);
			if (entities.instances[closestEntityPtr].id == Entity::Id::ITEM)
			{
				itemRightHand					  = closestEntityPtr;
				closestiData.targetEntityInstance = e.instancePtr;
			}
			interactEntityPtr = closestEntityPtr;
			busy			  = closestiData.shouldPlayerBeBusy;
		}
		else if (interact && closestEntityPtr < 0 && itemRightHand >= 0)
		{
			if (entities.interact(itemRightHand))
				itemRightHand = -1;
		}
		if (closestEntityPtr >= 0 && entities.select(closestEntityPtr))
			activeHint = getInteractionData(closestEntityPtr).interaction;
		if (use)
		{
			if (itemRightHand > 0 && getInteractionData(itemRightHand).hasAction)
				FRAME.useActionPtr = itemRightHand;
		}
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
		entities.setCollision(dude.pEntity, 1, true);
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
	Texture2D& texShadow	= CONTENT.textures[dude.pTexShadow];
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
		DrawCircleV(bc.pos.toVector2(), bc.radius, RED_CLEAR);
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
								 .canCollideGroup1	= true,
								 .canCollideGroup2	= false});
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
		DrawCircleV(bc.pos.toVector2(), bc.radius, RED_CLEAR);
	}
}
struct Item
{
	u32 pTexItem;
	u32 pTexShadow;

	AnimHoverFloat		 aHover;
	AnimHoverFloatShadow aShadow;
	bool				 isPicked = false;
	Color				 tint	  = WHITE;
};
u32 itemAdd(Entity::Arch arch, bool hasAction, v2 pos)
{
	i32 pEntity = entities.add(Entity::Id::ITEM,
							   arch,
							   pos,
							   {.canSelect		   = true,
								.canInteract	   = true,
								.canDraw		   = true,
								.canCollideTerrain = false,
								.canCollideGroup1  = false,
								.canCollideGroup2  = false});
	assert(pEntity >= 0);
	entities.instances[pEntity].iData.hasAction = hasAction;
	return pEntity;
}
void itemInit(Item& item, u32 pTexItem, u32 pTexShadow)
{
	item.pTexItem			 = pTexItem;
	item.pTexShadow			 = pTexShadow;
	item.aHover.anim.looped	 = true;
	item.aShadow.anim.looped = true;
	item.aHover.activate(3.0f);
	item.aShadow.activate(3.0f);
}

// returns true if should do "USE" action
bool itemUpdate(Item& item, u32 pEntity, f32 dt)
{
	Entity& e = entities.instances[pEntity];
	if (FRAME.selectedPtr == e.instancePtr)
		item.tint = RED;
	else
		item.tint = WHITE;
	if (FRAME.interactPtr == e.instancePtr)
	{
		item.isPicked = !item.isPicked;
		e.rot		  = 0.f;
		e.scale		  = .5f;
		if (item.isPicked)
		{
			e.rot	= 1.f;
			e.scale = 0.4f;
		}
		else
		{
			e.vel						 = entities.instances[e.iData.targetEntityInstance].vel;
			e.iData.targetEntityInstance = -1;
		}
		entities.selectable[e.instancePtr] = !item.isPicked;
	}
	e.vel = e.vel * 0.9f;
	if (e.iData.targetEntityInstance >= 0)
	{
		Entity& target = entities.instances[e.iData.targetEntityInstance];
		e.vel		   = (target.pos + e.iData.inventoryOffset - e.pos) * 0.35f;
	}
	e.pos += e.vel;
	item.aHover.update(dt);
	item.aShadow.update(dt);
	if (FRAME.useActionPtr == pEntity)
		return true;
	return false;
}
void itemDraw(Item& item, u32 pEntity)
{
	Entity&	   e		  = entities.instances[pEntity];
	Texture2D& texShadow  = CONTENT.textures[item.pTexShadow];
	Texture2D& texitem	  = CONTENT.textures[item.pTexItem];
	v2		   shadowSize = v2(texShadow.width, texShadow.height) * item.aShadow.getScale();
	v2		   itemSize	  = v2(texitem.width / 2.f, texitem.height / 2.f) * e.scale;
	if (!item.isPicked)
		DrawTextureEx(texShadow,
					  (e.pos - itemSize - shadowSize * 0.5).toVector2(),
					  0.f,
					  e.scale + item.aShadow.getScale(),
					  WHITE);
	DrawTextureEx(texitem,
				  (e.pos + item.aHover.getPos() - itemSize).toVector2(),
				  math::radToDeg(e.rot),
				  e.scale,
				  item.tint);
}
struct Key
{
	u32	 pEntity;
	Item item;

	static u32 add(v2 pos)
	{
		static_assert(sizeof(Key) < Entity::MAX_ENTITY_SIZE);
		i32		pEntity = itemAdd(Entity::Arch::KEY, false, pos);
		Entity& e		= entities.instances[pEntity];
		Key&	key		= *(new (e.data) Key);
		key.pEntity		= pEntity;
		e.scale			= 0.5f;	 // TODO: Temporary until we go to 16x16 px textures by defautl
		itemInit(key.item, CONTENT.TEX_KEY, CONTENT.TEX_SHADOW);

		e.iData.accessLevel = 2137;
		return pEntity;
	}
};
void keyUpdate(void* keyPtr, f32 dt)
{
	Key& key = *(Key*)keyPtr;
	itemUpdate(key.item, key.pEntity, dt);
}
void keyDraw(void* keyPtr)
{
	Key& key = *(Key*)keyPtr;
	itemDraw(key.item, key.pEntity);
}
struct Pick
{
	u32	 pEntity;
	Item item;

	// TODO: Potentialy create struct "Swing" with fields below?
	bool	  swings = false;
	AnimSwing aSwing;
	const v2  origin	 = {16, 16};
	const v2  bonkOffset = {-18, 8};

	static u32 add(v2 pos)
	{
		static_assert(sizeof(Pick) < Entity::MAX_ENTITY_SIZE);
		u32		pEntity = itemAdd(Entity::Arch::PICK, true, pos);
		Entity& e		= entities.instances[pEntity];
		Pick&	pick	= *(new (e.data) Pick);
		itemInit(pick.item, CONTENT.TEX_PICK, CONTENT.TEX_SHADOW);
		pick.pEntity = pEntity;
		e.scale		 = 0.5f;  // TODO: Temporary until we go to 16x16 px textures by defautl
		return pEntity;
	}
	v2 getHitCoords() { return entity(pEntity).pos + bonkOffset; }
};
void pickUpdate(void* pickPtr, f32 dt)
{
	Pick& pick = *(Pick*)pickPtr;
	if (itemUpdate(pick.item, pick.pEntity, dt))
	{
		pick.swings = true;
		pick.aSwing.activate(0.3f);
	}
	if (pick.swings)
	{
		if (pick.aSwing.getHitThisFrame())
			FRAME.hitCoords = pick.getHitCoords().toVector2();
		if (pick.aSwing.update(dt))
			pick.swings = false;
	}
}
void pickDraw(void* pickPtr)
{
	Pick&	pick = *(Pick*)pickPtr;
	Entity& e	 = entities.instances[pick.pEntity];
	DrawCircleV(e.pos.toVector2(), 1, RED);
	if (pick.aSwing.getHitThisFrame())
		DrawCircleV((e.pos + pick.bonkOffset).toVector2(), 5, YELLOW);
	itemDraw(pick.item, pick.pEntity);
	if (!pick.swings)
		return;
	v2	drawPos = e.pos + pick.aSwing.getPos();
	f32 drawRot = e.rot + pick.aSwing.getRot();
	DrawTexturePro(CONTENT.textures[pick.item.pTexItem],
				   {0, 0, 16, 16},
				   {drawPos.x, drawPos.y, 8, 8},
				   {8, 8},
				   math::radToDeg(drawRot),
				   pick.item.tint);
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
								 .canCollideGroup1	= false,
								 .canCollideGroup2	= false});
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
	if (FRAME.selectedPtr == e.instancePtr)
		table.tint = RED;
	else
		table.tint = WHITE;
	if (!table.aFlip.anim.active && FRAME.interactPtr == e.instancePtr)
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
			PlaySound(CONTENT.sounds[table.pSoundWham]);
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
	Texture2D& texShadow	= CONTENT.textures[table.pTexShadow];
	v2		   shadowSize	= v2(texShadow.width, texShadow.height) * table.aJumpShadow.getScale();
	v2		   shadowOffset = v2(0, 1.f) / table.aJumpShadow.getScale();
	DrawTextureEx(texShadow,
				  (e.pos - shadowSize * 0.5f + shadowOffset).toVector2(),
				  0.f,
				  table.aJumpShadow.getScale(),
				  WHITE);

	Texture2D& texTable = CONTENT.textures[table.pTexTable];
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
	u32			pEntity;
	v2			posEnd;
	SpriteSheet ss;
	bool		open		= false;
	f32			rotVel		= 0.0f;
	f32			dRotVel		= 0.01f;
	f32			rotVelMax	= 0.3f;
	f32			paddleAngle = -0.7854f;	 // texture is at 45deg angle by default

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
		Entity&	 e		 = entities.instances[iPtr];
		Gateway& gate	 = *(new (e.data) Gateway);
		gate.pEntity	 = iPtr;
		gate.posEnd		 = posEnd;
		v2 gateDirection = posEnd - posStart;
		gate.paddleAngle += atan2f(gateDirection.y, gateDirection.x);
		gate.ss.init(Content::TEX_GATE, {8, 8}, 2, 0, false);
		e.iData.accessLevel = 2137;
		return iPtr;
	}
};
void gateUpdate(void* gatePtr, f32 dt)
{
	Gateway& gate		= *(Gateway*)gatePtr;
	Entity&	 e			= entities.instances[gate.pEntity];
	Dude&	 dude		= Dude::getRef(GLOBAL.pDudeInstance);
	Entity&	 dudeEntity = dude.getEntity();
	Entity&	 itemHeld	= entities.instances[dude.itemRightHand];

	gate.open = (e.iData.accessLevel == itemHeld.iData.accessLevel);
	if (gate.open)
	{
		gate.rotVel -= gate.dRotVel;
		entities.setCollision(e.instancePtr, 2, false);
	}
	else
	{
		gate.rotVel += gate.dRotVel;
		entities.setCollision(e.instancePtr, 2, true);
	}
	gate.rotVel = math::limit(gate.rotVel, 0., gate.rotVelMax);
	e.rot += gate.rotVel;

	e.iData.boundingCircle = {.pos = math::projectPointOntoLine(dudeEntity.pos, e.pos, gate.posEnd),
							  .radius = 2};
}
void gateDraw(void* gatePtr)
{
	Gateway& gate = *(Gateway*)gatePtr;
	Entity&	 e	  = entities.instances[gate.pEntity];

	if (!gate.open)
		DrawLineEx(e.pos.toVector2(), gate.posEnd.toVector2(), 2, PINK);

	gate.ss.Draw(e.pos, WHITE, e.rot, e.scale, 0, 0);
	gate.ss.Draw(gate.posEnd, WHITE, e.rot, e.scale, 0, 0);
	if (!gate.open && e.iData.boundingCircle.pos != e.pos)
		gate.ss.Draw(e.iData.boundingCircle.pos, WHITE, gate.paddleAngle, e.scale, 0, 1);
	if (GLOBAL.drawDebugCollision)
		DrawCircleV(
			e.iData.boundingCircle.pos.toVector2(), e.iData.boundingCircle.radius, RED_CLEAR);
}
struct Baddie
{
	static inline u32  pTexShadow			= 0;
	static inline u32  pSoundTargetFound[5] = {};
	static inline u32  pSoundTargetLost[2]	= {};
	static inline u32  pSoundLaser			= 0;
	static inline u32  pSoundStomp			= 0;
	static inline bool initialized			= 0;

	u32			pEntity;
	SpriteSheet ss;

	f32	 sensorRange  = 50;
	bool targetLocked = false;

	static void init()
	{
		static_assert(sizeof(Baddie) < Entity::MAX_ENTITY_SIZE);
		Baddie::pTexShadow			 = Content::TEX_SHADOW;
		Baddie::pSoundLaser			 = Content::SOUND_LASER;
		Baddie::pSoundLaser			 = Content::SOUND_BADDIE_STOMP;
		Baddie::pSoundTargetFound[0] = Content::SOUND_BADDIE_TARGET_FOUND1;
		Baddie::pSoundTargetFound[1] = Content::SOUND_BADDIE_TARGET_FOUND2;
		Baddie::pSoundTargetFound[2] = Content::SOUND_BADDIE_TARGET_FOUND3;
		Baddie::pSoundTargetFound[3] = Content::SOUND_BADDIE_TARGET_FOUND4;
		Baddie::pSoundTargetFound[4] = Content::SOUND_BADDIE_TARGET_FOUND5;
		Baddie::pSoundTargetLost[0]	 = Content::SOUND_BADDIE_TARGET_LOST1;
		Baddie::pSoundTargetLost[1]	 = Content::SOUND_BADDIE_TARGET_LOST2;
		SetSoundVolume(CONTENT.sounds[Content::SOUND_LASER], 0.2f);
		SetSoundVolume(CONTENT.sounds[Content::SOUND_BADDIE_STOMP], 0.4f);
		initialized = true;
	}
	static i32 add(v2 pos)
	{
		if (!initialized)
			exitWithMessage("Baddie is not initialized!");
		int iPtr = entities.add(Entity::Id::ENEMY,
								Entity::Arch::BADDIE,
								pos,
								{.canSelect			= false,
								 .canInteract		= false,
								 .canDraw			= true,
								 .canCollideTerrain = true,
								 .canCollideGroup1	= false,
								 .canCollideGroup2	= true});
		assert(iPtr >= 0);
		Entity& e	   = entities.instances[iPtr];
		Baddie& baddie = *(new (e.data) Baddie);
		baddie.pEntity = iPtr;

		e.iData.boundingCircle = {.radius = 4};
		baddie.ss.init(Content::TEX_BADDIE, {8, 16}, 2, 2, true);
		return iPtr;
	}
};
void baddieUpdate(void* baddiePtr, f32 dt)
{
	Baddie& baddie	   = *(Baddie*)baddiePtr;
	Entity& e		   = entities.instances[baddie.pEntity];
	Dude&	dude	   = Dude::getRef(GLOBAL.pDudeInstance);
	Entity& dudeEntity = dude.getEntity();
	Entity& itemHeld   = entities.instances[dude.itemRightHand];

	if (e.pos.distTo(dudeEntity.pos) < baddie.sensorRange && itemHeld.arch == Entity::Arch::KEY)
	{
		e.vel = (dudeEntity.pos - e.pos).norm() * 0.3f;
		if (!baddie.targetLocked)
		{
			PlaySound(CONTENT.sounds[Content::SOUND_LASER]);
			PlaySound(CONTENT.sounds[Content::SOUND_BADDIE_TARGET_FOUND1 + math::random(0, 5)]);
		}
		baddie.targetLocked = true;
	}
	else
	{
		e.vel *= 0.8f;
		if (baddie.targetLocked)
			PlaySound(CONTENT.sounds[Content::SOUND_BADDIE_TARGET_LOST1 + math::random(0, 2)]);
		baddie.targetLocked = false;
	}

	e.pos += e.vel;
	e.iData.boundingCircle.pos = e.pos + v2(0, 4);
	if (baddie.ss.update(dt) && !e.vel.isZero())
	{
		SetSoundPitch(CONTENT.sounds[Content::SOUND_BADDIE_STOMP], math::randomf(.9f, 1.1f));
		PlaySound(CONTENT.sounds[Content::SOUND_BADDIE_STOMP]);
	}
}
void baddieDraw(void* baddiePtr)
{
	Baddie& baddie	   = *(Baddie*)baddiePtr;
	Entity& e		   = entities.instances[baddie.pEntity];
	Dude&	dude	   = Dude::getRef(GLOBAL.pDudeInstance);
	Entity& dudeEntity = dude.getEntity();

	Texture2D& texShadow	= CONTENT.textures[baddie.pTexShadow];
	v2		   shadowSize	= v2(texShadow.width, texShadow.height);
	v2		   shadowOffset = v2(0, 5.f);
	DrawTextureEx(
		texShadow, (e.pos - shadowSize * 0.5f + shadowOffset).toVector2(), 0.f, 1.f, WHITE);
	if (e.vel.isZero())
		baddie.ss.Draw(e.pos, WHITE, e.rot, e.scale, 0);
	else if (e.vel.y > 0)
	{
		baddie.ss.Draw(e.pos, WHITE, e.rot, e.scale, 1);
		if (baddie.targetLocked)
			DrawLineEx((e.pos + v2(0, -4)).toVector2(), dudeEntity.pos.toVector2(), 2, RED_CLEAR);
	}
	else
	{
		if (baddie.targetLocked)
			DrawLineEx((e.pos + v2(0, -4)).toVector2(), dudeEntity.pos.toVector2(), 2, RED_CLEAR);
		baddie.ss.Draw(e.pos, WHITE, e.rot, e.scale, 2);
	}
	if (GLOBAL.drawDebugCollision)
		DrawCircleV(
			e.iData.boundingCircle.pos.toVector2(), e.iData.boundingCircle.radius, RED_CLEAR);
}
