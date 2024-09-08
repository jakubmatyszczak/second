#pragma once
#include "dialogs.cpp"
#include "globals.cpp"
#include "effects.cpp"
#include "animation.cpp"
#include "map.cpp"

void updateGoblin(void*, f32);
void drawGoblin(void*);
void updateArrow(void*, f32);
void drawArrow(void*);
void updateOldMan(void*, f32);
void drawOldMan(void*);
void updateItem(void*, f32);
void drawItem(void*);
void drawItemEquipped(EntityPtr pEntity, v2f carrierPos);
void drawItemThumbnail(EntityPtr pEntity, v2f pos);

v3i	 computeMoveToTarget(v3i start, v3i target, s32 speed);
void itemUse(EntityPtr user, EntityPtr pItem, bool hit, bool use, v2f dir);

struct Entity
{
	enum class Meta
	{
		UNKNOWN = 0,
		PLAYER,
		ITEM,
		ENEMY,
		PROJECTILE,
		NPC,
	};
	enum Arch
	{
		UNKNOWN = 0,
		DUDE,
		GOBLIN,
		ARROW,
		OLD_MAN,
		ITEM,
	};
	Arch arch;
	Meta meta;
	v3i	 iPos;
	v3i	 iMoveTarget;
	v2f	 fPos;
	v2f	 fVel;
	f32	 fScale;
	v2f	 fDrawPos;
	f32	 fDrawRot;
	u8	 data[1024];
};
struct Entities
{
	static constexpr u32 nMax		  = 128;
	Entity				 arr[nMax]	  = {};
	bool				 active[nMax] = {};

	s32 add(Entity::Meta meta, Entity::Arch archetype, v3i pos)
	{
		for (u32 i = 1; i < nMax; i++)
		{
			if (active[i])
				continue;
			memset(&arr[i], 0, sizeof(arr[i]));
			arr[i].arch	  = archetype;
			arr[i].meta	  = meta;
			arr[i].iPos	  = pos;
			arr[i].fPos	  = {(f32)pos.x * G.tileSize, (f32)pos.y * G.tileSize};
			arr[i].fVel	  = {};
			arr[i].fScale = 1.f;
			active[i]	  = true;
			return i;
		}
		return -1;
	}
	void remove(EntityPtr ePtr) { active[ePtr] = false; }
	void updateAll(f32 dt)
	{
		for (u32 i = 1; i < nMax; i++)
			if (active[i])
			{
				switch (arr[i].arch)
				{
					case Entity::UNKNOWN:
						break;
					case Entity::DUDE:
						break;
					case Entity::GOBLIN:
						updateGoblin(arr[i].data, dt);
						break;
					case Entity::ARROW:
						updateArrow(arr[i].data, dt);
						break;
					case Entity::OLD_MAN:
						updateOldMan(arr[i].data, dt);
						break;
					case Entity::ITEM:
						updateItem(arr[i].data, dt);
				}
			}
	}
	void drawAll()
	{
		for (u32 i = 1; i < nMax; i++)
			if (active[i])
			{
				switch (arr[i].arch)
				{
					case Entity::UNKNOWN:
						break;
					case Entity::DUDE:
						break;
					case Entity::GOBLIN:
						drawGoblin(arr[i].data);
						break;
					case Entity::ARROW:
						drawArrow(arr[i].data);
						break;
					case Entity::OLD_MAN:
						drawOldMan(arr[i].data);
						break;
					case Entity::ITEM:
						drawItem(arr[i].data);
						break;
				}
			}
	}
	bool tryMove(v3i pos)
	{
		for (u32 i = 0; i < nMax; i++)
			if (active[i] && arr[i].iPos == pos)
				return false;
		return true;
	}
};

extern Map		 MAP;
extern Entities	 ENTITIES;
extern Content	 C;
extern FrameData F;

struct Item
{
	enum Arch
	{
		PICKAXE,
		SWORD,
	};
	EntityPtr pEntity;
	Arch	  arch;
	char	  name[16];
	Rectangle tilesetOffset;

	EntityPtr carrier;
	v2f		  gripOffset;
	f32		  rotOffset;
	v2f		  direction;
	bool	  isPicked;

	bool		 swingFwd;
	AnimSwingFwd aSwingFwd;

	f32 digPower;
	f32 dmg;
	f32 swingLen;

	static s32 add(Arch type, v3i pos)
	{
		static_assert(sizeof(Item) <= sizeof(Entity::data), "ITEM does not fir into ENTITY");
		s32		pEntity = ENTITIES.add(Entity::Meta::ITEM, Entity::ITEM, pos);
		Entity& e		= ENTITIES.arr[pEntity];
		Item&	item	= *new (e.data) Item();
		item.pEntity	= pEntity;
		item.arch		= type;
		switch (type)
		{
			case Arch::PICKAXE:
				strcpy(item.name, "Pickaxe");
				e.fScale		   = 0.7f;
				item.tilesetOffset = {0, 32, G.tileSize, G.tileSize};
				item.gripOffset	   = {13, 13};
				item.rotOffset	   = -math::pi4;
				item.digPower	   = 1;
				item.swingLen	   = 0.6f;
				break;
			case Arch::SWORD:
				strcpy(item.name, "Sword");
				e.fScale		   = 0.7f;
				item.tilesetOffset = {32, 32, G.tileSize, G.tileSize};
				item.gripOffset	   = {14, 14};
				item.rotOffset	   = -math::pi4;
				item.dmg		   = 1;
				item.swingLen	   = 0.4;
				break;
		}
		return pEntity;
	}
};
struct CreatueStats
{
	f32 maxHp;
	s32 strikeCooldown;
	f32 dmg;
	f32 range;
	f32 digPower;
	s32 movements;
	f32 speed;
	f32 sightRange;
	s32 idleSightRange;
	s32 alarmedSightRange;
};
struct Unit
{
	enum Role
	{
		UNKNOWN,
		FIGHTER,
		ARCHER,
	};
	Role role;
	f32	 hitpoints;
	f32	 gotHit;
	s32	 strikeCooldown;

	AnimBonk aBonk;
	AnimBonk aDead;
};
// returns true if entity should be removed cuz dead
bool unitUpdate(Unit& u, f32 dt, bool progressLogic)
{
	if (progressLogic)
		u.strikeCooldown--;
	u.gotHit -= dt;
	u.aBonk.update(dt);
	if (u.aDead.update(dt))
		return true;
	return false;
}
// returns true if died
bool unitHit(Unit& u, f32 animDuration, f32 dmg)
{
	u.hitpoints -= dmg;
	if (u.hitpoints <= 0)
	{
		u.gotHit = animDuration;
		u.aDead.activate(animDuration);
		return true;
	}
	u.gotHit = animDuration;
	u.aBonk.activate(animDuration);
	return false;
}
v2f unitGetAnimPos(Unit& u) { return u.aDead.getPos() + u.aBonk.getPos(); }
f32 unitGetAnimRot(Unit& u) { return u.aDead.getRot() + u.aBonk.getRot(); }

struct Action
{
	enum Activity
	{
		NONE,
		MOVE,
		STRIKE,
		SHOOT,
	};
	Activity type;
	v3i		 target;
};
struct Ai
{
	enum State
	{
		IDLE,
		CHASE
	};
	State  state;
	Action action;
	bool   alarmed;

	const Action& update(const v3i iPos, const CreatueStats& stats, const Unit& unit)
	{
		action.type = Action::NONE;
		if ((F.dudePos - iPos).getLength() < stats.sightRange)
		{
			alarmed = true;
			state	= State::CHASE;
		}
		else
			state = State::IDLE;
		switch (state)
		{
			case IDLE:
				action.type	  = Action::NONE;
				action.target = iPos;
				break;
			case CHASE:
			{
				v2f toDude = (toV2f(F.dudePos - iPos));
				if (toDude.getLength() > stats.range)
				{  // Move
					action.type	  = Action::MOVE;
					v3i targetPos = iPos + toV3i((toDude.norm() * stats.movements).round());
					if (ENTITIES.tryMove(targetPos) && MAP.tryMove(targetPos))
						action.target = targetPos;
					else  // cannot move on the shortest path
						action.target = computeMoveToTarget(iPos, F.dudePos, stats.movements);
				}
				else if (unit.strikeCooldown <= 0)
				{  // Attack!
					if (unit.role == Unit::FIGHTER)
						action.type = action.STRIKE;
					if (unit.role == Unit::ARCHER)
						action.type = action.SHOOT;
					if (F.dudeHit)
						action.target = F.dudePos;
					else
						action.target = {F.dudeAimTile.x, F.dudeAimTile.y, F.dudePos.z};
				}
			}
			break;
		};
		return action;
	}
};

struct Arrow
{
	EntityPtr pEntity;
	Rectangle tilesetOffset;

	f32 dmg;
	f32 range;

	static s32 add(v3i pos, v2f dir, f32 dmg, f32 speed, f32 range)
	{
		EntityPtr pEntity = ENTITIES.add(Entity::Meta::PROJECTILE, Entity::Arch::ARROW, pos);
		Entity&	  e		  = ENTITIES.arr[pEntity];
		Arrow&	  a		  = *new (e.data) Arrow;
		a = {.pEntity = pEntity, .tilesetOffset = {}, .dmg = dmg, .range = range * G.tileSize};
		a.tilesetOffset = {16, 32, G.tileSize, G.tileSize};
		e.fVel			= speed * dir.norm();
		return pEntity;
	}
};

struct Player
{
	EntityPtr  pEntity;
	TexturePtr pTexture;
	SoundPtr   pSfxHit;
	SoundPtr   pSfxStepGrass;
	SoundPtr   pSfxStepRock;

	static const u32 nItemsMax = 3;
	EntityPtr		 inventory[nItemsMax];
	EntityPtr		 itemHeld		  = -1;
	s32				 itemsInInventory = 0;

	v3i	 realPos;
	v2f	 direction;
	bool canClimb;
	bool canGoDown;
	Unit unit;

	CreatueStats baseStats = {.maxHp		  = 10,
							  .strikeCooldown = 1,
							  .dmg			  = 1,
							  .range		  = 0,
							  .digPower		  = 1,
							  .movements	  = 1,
							  .speed		  = 0.06f};
	CreatueStats currentStats;

	static s32 add(v3i pos)
	{
		static_assert(sizeof(Player) <= sizeof(Entity::data), "Player does not fir into ENTITY");
		s32		pEntity		= ENTITIES.add(Entity::Meta::PLAYER, Entity::DUDE, pos);
		Entity& e			= ENTITIES.arr[pEntity];
		Player& dude		= *new (e.data) Player();
		dude.pEntity		= pEntity;
		dude.pTexture		= C.TEX_TILESET;
		dude.pSfxHit		= C.SFX_HIT;
		dude.pSfxStepGrass	= C.SFX_STEP_GRASS;
		dude.pSfxStepRock	= C.SFX_STEP_ROCK;
		dude.unit.hitpoints = dude.baseStats.maxHp;

		F.dudePos = pos;
		return pEntity;
	}
	static Player& get(EntityPtr pEntity) { return *(Player*)ENTITIES.arr[pEntity].data; };
	Entity&		   getEntity() { return ENTITIES.arr[pEntity]; }

	bool input(v2f	heading,
			   bool hit,
			   bool go,
			   bool goUp,
			   bool use,
			   bool swap,
			   bool canClimb,
			   bool canGoDown)
	{
		Entity& e		= ENTITIES.arr[pEntity];
		e.iMoveTarget	= e.iPos;
		this->canClimb	= canClimb;
		this->canGoDown = canGoDown;
		currentStats	= baseStats;

		direction = heading;
		if (go)
		{
			e.iMoveTarget = e.iPos + toV3i(direction);
			if (!MAP.tryMove(e.iMoveTarget) && ENTITIES.tryMove(e.iMoveTarget))
			{
				e.iMoveTarget = e.iPos;
				go			  = false;
			}
		}
		if (canGoDown && go && direction.isZero())
			e.iMoveTarget.z--;
		if (canClimb && goUp && direction.isZero())
			e.iMoveTarget.z++;
		if (hit)
		{
			F.dudeHit = true;
			PlaySound(C.sounds[pSfxHit]);
		}
		if (use)
			F.dudeUse = true;
		if (itemHeld >= 0)
			itemUse(pEntity, inventory[itemHeld], hit, use, direction);
		if (swap)
		{
			itemHeld++;
			if (itemHeld >= itemsInInventory)
				itemHeld = 0;
		}

		for (u32 i = 0; i < nItemsMax; i++)
		{
			if (!inventory[i])
				continue;
			if (itemHeld == i)
			{
				// currentStats.digPower += ITEMS.arr[inventory[i]].digPower;
				// currentStats.dmg += ITEMS.arr[inventory[i]].dmg;
			}
		}
		if (use || hit || go)
			return true;
		return false;
	}
	void update(f32 dt)
	{
		Entity& e = ENTITIES.arr[pEntity];
		e.fVel	  = (toV2f(e.iPos * 16) - e.fPos) * currentStats.speed;
		e.fPos += e.fVel;
		realPos		  = {(s32)((e.fPos.x + 8.f) / 16.f), (s32)((e.fPos.y + 8.f) / 16.f), e.iPos.z};
		F.dudePos	  = e.iPos;
		F.dudeAimTile = realPos + toV3i(direction);
		if (F.dudeAimTile == F.dudePos)
			F.dudeAimTile.z--;
		if (unitUpdate(unit, dt, F.progressLogic))
		{
			exitWithMessage("XD zdech XD");
			ENTITIES.remove(pEntity);
		}
	}
	void move()
	{
		Entity& e = ENTITIES.arr[pEntity];
		if (e.iMoveTarget == e.iPos || e.iMoveTarget == v3i())
			return;
		e.iPos = e.iMoveTarget;
		PlaySound(C.sounds[pSfxStepGrass]);
	}
	void interact()
	{
		Entity& e = ENTITIES.arr[pEntity];
		if (F.entUsed)
		{
			for (u32 i = 0; i < nItemsMax; i++)
			{
				if (inventory[i])
					continue;
				inventory[i] = F.entUsed;
				itemsInInventory++;
				if (itemsInInventory == 1)
					itemHeld = 0;
				break;
			}
		}
	}
	static bool tryHit(EntityPtr playerEntity, f32 dmg)
	{
		Player& p = Player::get(playerEntity);
		unitHit(p.unit, 0.3f, dmg);
		PlaySound(C.sounds[C.SFX_OOF]);
		return true;
	}
	void draw()
	{
		Entity& e		= ENTITIES.arr[pEntity];
		f32		scale	= 0.8f;
		f32		size	= scale * G.tileSize;
		v2f		drawPos = e.fPos + v2f(G.tileSize / 2.f) + unitGetAnimPos(unit);
		DrawTexturePro(C.textures[pTexture],
					   {0, 16, G.tileSize, G.tileSize},
					   {drawPos.x, drawPos.y, size, size},
					   {size / 2, size / 2},
					   math::radToDeg(unitGetAnimRot(unit)),
					   unit.gotHit > 0 ? RED : WHITE);
		if (itemHeld >= 0)
			drawItemEquipped(inventory[itemHeld], e.fPos);

		static f32 t = 0.f;
		t += 0.128f;
		f32 scaling = 8.f + ((sinf(t) * 0.5f + 0.5f) * 2.f);
		if (!direction.isZero())
			DrawTexturePro(C.textures[pTexture],
						   {16, 16, 16, 16},
						   {e.fPos.x + 8 + roundf(direction.x) * scaling,
							e.fPos.y + 8 + roundf(direction.y) * scaling,
							12.f,
							12.f},
						   {6, 6},
						   math::radToDeg(atan2f(direction.y, direction.x)) + 90.f,
						   WHITE_CLEAR);
		if (direction.isZero())
			DrawCircle(e.fPos.x + 8, e.fPos.y + 8, 2 * scaling * 0.15, RED_CLEAR);
		if (canClimb)
			DrawTexturePro(C.textures[pTexture],
						   {16, 16, 16, 16},
						   {e.fPos.x + 8, e.fPos.y + 8, 8.f, 8.f},
						   Vector2({4, 4 + scaling * 0.5f}),
						   0.f,
						   GREEN_CLEAR);
		if (canGoDown)
			DrawTexturePro(C.textures[pTexture],
						   {16, 16, 16, 16},
						   {e.fPos.x + 8, e.fPos.y + 8, 8.f, 8.f},
						   Vector2({4, 4.f + scaling * 0.5f}),
						   180.f,
						   RED_CLEAR);
	}
	void drawOverlay()
	{
		DrawText(TextFormat(
					 "Stats:\ndigPower: %.f\nDamage: %.f", currentStats.digPower, currentStats.dmg),
				 10,
				 40,
				 20,
				 RED);
		for (u32 i = 0; i < nItemsMax; i++)
		{
			Vector2 slotPos = {10.f + 74 * i, 720 - 64 - 10};
			DrawRectangle(slotPos.x, slotPos.y, 64, 64, GRAY_CLEAR);
			if (!inventory[i])
				continue;
			if (i == itemHeld)
				DrawCircle(slotPos.x + 64 / 2.f, slotPos.y + 64 / 2.f, 24, RED_CLEAR);
			drawItemThumbnail(inventory[i], slotPos);
		}
		DrawRectangle(50, 10, 110, 20, BLACK);
		DrawRectangle(55, 15, (s32)(100.f * (unit.hitpoints / baseStats.maxHp)), 10, RED);
	}
};

void updateItem(void* data, f32 dt)
{
	Item&	i = *(Item*)data;
	Entity& e = ENTITIES.arr[i.pEntity];

	if (F.dudeUse && F.dudePos == e.iPos && !i.isPicked)
	{
        itemUse(G.entDude, i.pEntity, false, true, {});
		F.entUsed  = i.pEntity;
	}
	if (!i.isPicked)
		e.fPos = toV2f(e.iPos) * G.tileSize;
	else
	{
		Entity& eDude = ENTITIES.arr[G.entDude];
		e.fPos		  = eDude.fPos;
		e.iPos		  = eDude.iPos;
	}
	i.aSwingFwd.update(dt);
}
void drawItemThumbnail(EntityPtr pEntity, v2f pos)
{
	Entity&	 eItem = ENTITIES.arr[pEntity];
	Item&	 i	   = *(Item*)eItem.data;
	Texture& tex   = C.textures[Content::TEX_TILESET];
	DrawTexturePro(tex, i.tilesetOffset, {pos.x, pos.y, 64, 64}, {}, 0.f, WHITE);
}
v2f follow(v2f target, v2f current, f32 magnitude0to1)
{
	return current + (target - current) * magnitude0to1;
}
f32 follow(f32 target, f32 current, f32 magnitude0to1)
{
	return current + (target - current) * magnitude0to1;
}
void drawItemEquipped(EntityPtr pEntity, v2f carrierPos)
{
	Entity& e = ENTITIES.arr[pEntity];
	Item&	i = *(Item*)e.data;

	Texture& tex	 = C.textures[Content::TEX_TILESET];
	bool	 mirrorx = i.direction.x < 0;
	f32		 angle	 = i.direction.y * i.direction.x * math::pi4;
	if (i.direction.x == 0)
		angle = i.direction.y * math::pi2;
	e.fDrawPos = follow(
		carrierPos + v2f(G.tileSize * 0.5f) + i.aSwingFwd.getPos(mirrorx, angle), e.fDrawPos, 0.7f);
	v2f drawOrigin = i.gripOffset * e.fScale;
	e.fDrawRot	   = follow(i.rotOffset + i.aSwingFwd.getRot(mirrorx, angle), e.fDrawRot, 0.7f);
	DrawTexturePro(tex,
				   i.tilesetOffset,
				   {e.fDrawPos.x, e.fDrawPos.y, G.tileSize * e.fScale, G.tileSize * e.fScale},
				   drawOrigin.toVector2(),
				   math::radToDeg(e.fDrawRot),
				   WHITE);
}
void drawItem(void* data)
{
	Item&	 i	   = *(Item*)data;
	Entity&	 eItem = ENTITIES.arr[i.pEntity];
	Texture& tex   = C.textures[Content::TEX_TILESET];
	if (!i.isPicked)
	{
		v2f		   origin  = v2f(G.tileSize * 0.5f * 0.5f);
		v2f		   drawPos = eItem.fPos + origin * 2;
		static f32 t	   = 0;
		t += 0.064f;
		f32 breath = ((sinf(t) + 1) * 0.5f) * 0.25f + 0.75f;
		DrawEllipse(drawPos.x, drawPos.y + 5, 4 * breath, 2 * breath, GRAY_CLEAR);
		DrawTexturePro(tex,
					   i.tilesetOffset,
					   {drawPos.x, drawPos.y - 4 * breath, G.tileSize * .5f, G.tileSize * .5f},
					   origin.toVector2(),
					   0.f,
					   WHITE);
	}
}
void itemUse(EntityPtr user, EntityPtr pItem, bool hit, bool use, v2f dir)
{
	Entity& e = ENTITIES.arr[pItem];
	if (e.arch != Entity::ITEM)
		return;
	Item& i = *(Item*)e.data;
	if (use && !i.isPicked)
	{
		i.carrier  = user;
		i.isPicked = true;
	}
	i.direction = dir.norm();
	if (hit)
	{
		if (i.aSwingFwd.activate(i.swingLen))
		{
			switch (i.arch)
			{
				case Item::SWORD:
				{
					v2f swooshStart = e.fPos + v2f(G.tileSize * .5f) + i.direction * G.tileSize;
					Swoosh::add(swooshStart, i.swingLen, i.direction);
					break;
				}
				default:
					break;
			}
			i.swingFwd = !i.swingFwd;
		}
	}
}

struct Goblin
{
	EntityPtr pEntity;
	Rectangle tilesetOffset;

	SoundPtr pGrawlShort;

	CreatueStats baseStats = {.maxHp			 = 3,
							  .strikeCooldown	 = 2,
							  .dmg				 = 1,
							  .range			 = 1.5f,
							  .digPower			 = 0,
							  .movements		 = 2,
							  .speed			 = 0.1f,
							  .sightRange		 = 4,
							  .idleSightRange	 = 4,
							  .alarmedSightRange = 10};
	CreatueStats currentStats;
	Unit		 unit;
	Ai			 ai;
	EntityPtr	 itemHeld = -1;

	static s32 add(v3i pos, Unit::Role role)
	{
		static_assert(sizeof(Goblin) < sizeof(Entity::data), "GOBLIN does not fit into ENTITY");
		EntityPtr pEntity = ENTITIES.add(Entity::Meta::ENEMY, Entity::Arch::GOBLIN, pos);
		Entity&	  e		  = ENTITIES.arr[pEntity];
		Goblin&	  g		  = *new (e.data) Goblin;
		g.pEntity		  = pEntity;
		g.tilesetOffset	  = {32, 16, G.tileSize, G.tileSize};
		g.pGrawlShort	  = C.SFX_GOBLIN_SHORT;
		g.unit.hitpoints  = g.baseStats.maxHp;
		g.unit.role		  = role;
		if (role == Unit::Role::FIGHTER)
		{
			g.itemHeld = Item::add(Item::Arch::SWORD, e.iPos);
			itemUse(g.pEntity, g.itemHeld, false, true, {});
		}
		if (role == Unit::Role::ARCHER)
		{
			g.baseStats.maxHp -= 1;
			g.baseStats.range		   = 5;
			g.baseStats.strikeCooldown = 5;
		}

		return pEntity;
	}
};
void updateGoblin(void* data, f32 dt)
{
	Goblin& g	   = *(Goblin*)data;
	Entity& e	   = ENTITIES.arr[g.pEntity];
	g.currentStats = g.baseStats;
	g.currentStats.sightRange =
		g.ai.alarmed ? g.baseStats.alarmedSightRange : g.baseStats.idleSightRange;

	if (F.progressLogic)
	{
		const Action& action = g.ai.update(e.iPos, g.currentStats, g.unit);
		if (action.type == action.MOVE)
			e.iPos = action.target;
		if (action.type == action.STRIKE)
		{
			Strike::add(action.target, g.currentStats.dmg, g.pEntity, G.entDude);
			g.unit.strikeCooldown = g.currentStats.strikeCooldown;
		}
		if (action.type == action.SHOOT)
		{
			g.unit.strikeCooldown = g.currentStats.strikeCooldown;
			Arrow::add(e.iPos, toV2f(action.target - e.iPos), g.currentStats.dmg, 1.5f, 4.f);
		}
		if (F.dudeHit && F.dudeAimTile == e.iPos)
		{
			unitHit(g.unit, 0.2f, Player::get(G.entDude).currentStats.dmg);
			SetSoundPitch(C.sounds[g.pGrawlShort], math::randomf(1.2f, 1.8f));
			SetSoundVolume(C.sounds[g.pGrawlShort], math::randomf(0.7f, 1.f));
			PlaySound(C.sounds[g.pGrawlShort]);
		}
	}
	e.fVel = (toV2f(e.iPos * G.tileSize) - e.fPos) * g.currentStats.speed;
	e.fPos += e.fVel;
	if (unitUpdate(g.unit, dt, F.progressLogic))
	{
		SetSoundPitch(C.sounds[g.pGrawlShort], math::randomf(0.7f, 1.f));
		PlaySound(C.sounds[g.pGrawlShort]);
		ENTITIES.remove(g.pEntity);
	}
};
void drawGoblin(void* data)
{
	Goblin& g = *(Goblin*)data;
	Entity& e = ENTITIES.arr[g.pEntity];

	if (F.dudePos.z != e.iPos.z)
		return;
	f32 scale	= 0.6f;
	f32 size	= G.tileSize * scale;
	v2f drawPos = e.fPos + v2f(G.tileSize / 2.f) + unitGetAnimPos(g.unit);
	DrawTexturePro(C.textures[C.TEX_TILESET],
				   g.tilesetOffset,
				   {drawPos.x, drawPos.y, size, size},
				   v2f(size / 2).toVector2(),
				   math::radToDeg(unitGetAnimRot(g.unit)),
				   g.unit.gotHit > 0 ? RED : WHITE);
    drawItemEquipped(g.itemHeld, e.fPos);
	if (G.debugDrawSightRange)
		DrawCircleV(drawPos.toVector2(), g.currentStats.sightRange * G.tileSize, RED_CLEAR);
};
struct OldMan
{
	EntityPtr pEntity;
	Rectangle tilesetOffset;

	static s32 add(v3i pos)
	{
		static_assert(sizeof(OldMan) < sizeof(Entity::data), "OLDMAN does not fit into ENTITY");
		EntityPtr pEntity = ENTITIES.add(Entity::Meta::NPC, Entity::Arch::OLD_MAN, pos);
		Entity&	  e		  = ENTITIES.arr[pEntity];
		OldMan&	  om	  = *new (e.data) OldMan;
		om.pEntity		  = pEntity;
		om.tilesetOffset  = {0, 16, G.tileSize, G.tileSize};

		return pEntity;
	}
};
void updateOldMan(void* data, f32 dt)
{
	OldMan& om = *(OldMan*)data;
	Entity& e  = ENTITIES.arr[om.pEntity];

	if (F.progressLogic)
		if (F.dudeUse && F.dudeAimTile == e.iPos)
			NARRATIVE.start(1);
}
void drawOldMan(void* data)
{
	OldMan& om = *(OldMan*)data;
	Entity& e  = ENTITIES.arr[om.pEntity];
	if (F.dudePos.z != e.iPos.z)
		return;
	f32 scale	= 0.6f;
	f32 size	= G.tileSize * scale;
	v2f drawPos = e.fPos + v2f(G.tileSize / 2.f);
	DrawTexturePro(C.textures[C.TEX_TILESET],
				   om.tilesetOffset,
				   {drawPos.x, drawPos.y, size, size},
				   v2f(size / 2).toVector2(),
				   0,
				   GRAY);
}

void updateArrow(void* data, f32 dt)
{
	Arrow&	a = *(Arrow*)data;
	Entity& e = ENTITIES.arr[a.pEntity];
	e.fPos += e.fVel;
	a.range -= e.fVel.getLength();
	if (a.range < 0)
		e.fVel *= 0.95f;
	if (e.fVel.getLength() < 0.2f)
		ENTITIES.remove(a.pEntity);

	Entity& eDude = Player::get(G.entDude).getEntity();
	if (e.fPos.distTo(eDude.fPos) < G.tileSize * 0.4f)
	{
		Player::tryHit(G.entDude, a.dmg);
		e.fVel = 0.f;
	}
}
void drawArrow(void* data)
{
	Arrow&	a	  = *(Arrow*)data;
	Entity& e	  = ENTITIES.arr[a.pEntity];
	f32		rot	  = atan2f(e.fVel.y, e.fVel.x);
	f32		scale = 0.6f;
	f32		size  = G.tileSize * scale;
	v2f		pos	  = e.fPos + v2f(G.tileSize / 2.f);
	DrawTexturePro(C.textures[C.TEX_TILESET],
				   a.tilesetOffset,
				   {pos.x, pos.y, size, size},
				   {size / 2.f, size / 2.f},
				   math::radToDeg(rot) + 90.f,
				   WHITE);
}

bool tryHit(v3i pos, EntityPtr target, f32 dmg)
{
	Entity& e = ENTITIES.arr[target];
	if (e.iPos == pos)
		if (e.meta == Entity::Meta::PLAYER)
			return Player::tryHit(target, dmg);
	return false;
}
bool isDead(EntityPtr pEntity)
{
    return !ENTITIES.active[pEntity];
}

v3i computeMoveToTarget(v3i start, v3i target, s32 speed)
{
	const u32 nPoints = 6;
	v3i		  bestPos[nPoints];
	f32		  bestDist[nPoints];
	bestPos[0]	= start;
	bestDist[0] = (toV2f(target) - toV2f(bestPos[0])).getLengthSquared();
	for (u32 i = 1; i < nPoints; i++)
	{
		bestPos[i]	= bestPos[0];
		bestDist[i] = bestDist[0];
	}
	for (s32 x = -speed; x <= speed; x++)  // search nearest fields, choose the best and go
		for (s32 y = -speed; y <= speed; y++)
		{
			v3i newPos = start + v3i(x, y, 0);
			f32 dist   = (toV2f(F.dudePos) - toV2f(newPos)).getLengthSquared();
			for (s32 i = 0; i < nPoints; i++)
				if (dist < bestDist[i])
				{
					bestDist[i] = dist;
					bestPos[i]	= newPos;
					break;
				}
		}
	for (u32 i = 0; i < nPoints; i++)
		if (ENTITIES.tryMove(bestPos[i]))
			return bestPos[i];
	return start;
}
