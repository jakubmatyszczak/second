#pragma once
#include "dialogs.cpp"
#include "globals.cpp"
#include "effects.cpp"
#include "animation.cpp"

void updatePickaxe(void*, f32);
void drawPickaxe(void*);
void updateGoblin(void*, f32);
void drawGoblin(void*);
void updateArrow(void*, f32);
void drawArrow(void*);
void updateSword(void*, f32);
void drawSword(void*);
void updateOldMan(void*, f32);
void drawOldMan(void*);

bool tryMove(v3i pos);
v3i	 computeMoveToTarget(v3i start, v3i target, s32 speed);

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
		PICKAXE,
		GOBLIN,
		ARROW,
		SWORD,
		OLD_MAN,
	};
	Arch arch;
	Meta meta;
	v3i	 iPos;
	v3i	 iMoveTarget;
	v2f	 fPos;
	v2f	 fVel;
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
			arr[i].arch = archetype;
			arr[i].meta = meta;
			arr[i].iPos = pos;
			arr[i].fPos = {(f32)pos.x * G.tileSize, (f32)pos.y * G.tileSize};
			arr[i].fVel = {};
			active[i]	= true;
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
					case Entity::PICKAXE:
						updatePickaxe(arr[i].data, dt);
						break;
					case Entity::GOBLIN:
						updateGoblin(arr[i].data, dt);
						break;
					case Entity::ARROW:
						updateArrow(arr[i].data, dt);
						break;
					case Entity::SWORD:
						updateSword(arr[i].data, dt);
						break;
					case Entity::OLD_MAN:
						updateOldMan(arr[i].data, dt);
						break;
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
					case Entity::PICKAXE:
						drawPickaxe(arr[i].data);
						break;
					case Entity::GOBLIN:
						drawGoblin(arr[i].data);
						break;
					case Entity::ARROW:
						drawArrow(arr[i].data);
						break;
					case Entity::SWORD:
						drawSword(arr[i].data);
						break;
					case Entity::OLD_MAN:
						drawOldMan(arr[i].data);
						break;
				}
			}
	}
};

extern Entities	 ENTITIES;
extern Content	 C;
extern FrameData F;

struct ItemData
{
	char	  name[16];
	Rectangle tilesetOffset;
	f32		  digPower;
	f32		  dmg;
};
struct Items
{
	TexturePtr tileset = C.TEX_TILESET;
	ItemData   arr[32] = {};
};
Items ITEMS;
struct Item
{
	ItemPtr pItem;
	bool	picked;
};
struct CreatueStats
{
	f32 maxHp;
	s32 strikeCooldown;
	f32 dmg;
	f32 range;
	f32 digPower;
	s32 speed;
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

struct Player
{
	EntityPtr  pEntity;
	TexturePtr pTexture;
	SoundPtr   pSfxHit;
	SoundPtr   pSfxStepGrass;
	SoundPtr   pSfxStepRock;

	static constexpr u32 nItemsMax = 6;
	ItemPtr				 inventory[nItemsMax];

	v2f	 direction;
	bool canClimb;
	bool canGoDown;
	Unit unit;

	CreatueStats baseStats = {
		.maxHp = 10, .strikeCooldown = 1, .dmg = 1, .range = 0, .digPower = 1, .speed = 1};
	CreatueStats currentStats;

	static s32 add(v3i pos)
	{
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

	bool input(v2f heading,
			   bool hit,
			   bool go,
			   bool goUp,
			   bool use,
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
			e.iMoveTarget = e.iPos + toV3i(direction);
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

		for (u32 i = 0; i < nItemsMax; i++)
		{
			if (!inventory[i])
				continue;
			currentStats.digPower += ITEMS.arr[inventory[i]].digPower;
			currentStats.dmg += ITEMS.arr[inventory[i]].dmg;
		}
		if (use || hit || go)
			return true;
		return false;
	}
	void update(f32 dt)
	{
		Entity& e = ENTITIES.arr[pEntity];
		e.fVel	  = (toV2f(e.iPos * 16) - e.fPos) * 0.1f;
		e.fPos += e.fVel;
		v3i realPos	  = {(s32)((e.fPos.x + 8.f) / 16.f), (s32)((e.fPos.y + 8.f) / 16.f), e.iPos.z};
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
		if (F.itemUsed)
		{
			for (u32 i = 0; i < nItemsMax; i++)
			{
				if (inventory[i])
					continue;
				inventory[i] = F.itemUsed;
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
		for (u32 i = 0; i < 6; i++)
		{
			Vector2 itemPos = {10.f + 74 * i, 720 - 64 - 10};
			DrawRectangle(itemPos.x, itemPos.y, 64, 64, GRAY_CLEAR);
			if (!inventory[i])
				continue;
			DrawTexturePro(C.textures[ITEMS.tileset],
						   ITEMS.arr[inventory[i]].tilesetOffset,
						   {itemPos.x, itemPos.y, 64, 64},
						   {0, 0},
						   0.f,
						   WHITE);
		}
		DrawRectangle(50, 10, 110, 20, BLACK);
		DrawRectangle(55, 15, (s32)(100.f * (unit.hitpoints / baseStats.maxHp)), 10, RED);
	}
};
void itemInit()
{
	static bool init = false;
	if (init)
		return;
	{
		ItemData& item = ITEMS.arr[1];
		strncpy(item.name, "Basic Pickaxe", 15);
		item.digPower	   = 1;
		item.tilesetOffset = {0, 32, G.tileSize, G.tileSize};
	}
	{
		ItemData& item = ITEMS.arr[2];
		strncpy(item.name, "Basic Sword", 15);
		item.tilesetOffset = {32, 32, G.tileSize, G.tileSize};
		item.dmg		   = 1;
	}
	init = true;
};
void updateItem(Item& item, Entity& eItem)
{
	if (F.dudeUse && F.dudePos == eItem.iPos && !item.picked)
	{
		item.picked = true;
		F.itemUsed	= item.pItem;
	}
	if (!item.picked)
		eItem.fPos = toV2f(eItem.iPos) * G.tileSize;
	else
		eItem.iPos = F.dudePos;
}
void drawItem(Item& item, Entity& eItem, ItemData& data)
{
	Texture&   tex	   = C.textures[ITEMS.tileset];
	v2f		   origin  = {4, 4};
	v2f		   drawPos = eItem.fPos + origin * 2;
	static f32 t	   = 0;
	t += 0.064f;
	f32 breath = ((sinf(t) + 1) * 0.5f) * 0.25f + 0.75f;
	if (!item.picked)
	{
		DrawEllipse(drawPos.x, drawPos.y + 6, 4 * breath, 2 * breath, GRAY_CLEAR);
		DrawTexturePro(tex,
					   data.tilesetOffset,
					   {drawPos.x, drawPos.y - 4 * breath, 8.f, 8.f},
					   origin.toVector2(),
					   45.f,
					   WHITE);
	}
}
struct Pickaxe
{
	EntityPtr pEntity;
	Item	  item;

	static s32 add(v3i pos)
	{
		itemInit();
		s32		 pEntity = ENTITIES.add(Entity::Meta::ITEM, Entity::PICKAXE, pos);
		Entity&	 e		 = ENTITIES.arr[pEntity];
		Pickaxe& item	 = *new (e.data) Pickaxe();
		item.pEntity	 = pEntity;
		item.item.pItem	 = 1;
		return pEntity;
	}
};
void updatePickaxe(void* data, f32 dt)
{
	Pickaxe& pick = *(Pickaxe*)data;
	Entity&	 e	  = ENTITIES.arr[pick.pEntity];
	updateItem(pick.item, e);
}
void drawPickaxe(void* data)
{
	Pickaxe&  pick	   = *(Pickaxe*)data;
	Entity&	  e		   = ENTITIES.arr[pick.pEntity];
	ItemData& itemData = ITEMS.arr[pick.item.pItem];
	if (F.dudePos.z != e.iPos.z)
		return;
	drawItem(pick.item, e, itemData);
}
struct Sword
{
	EntityPtr pEntity;
	Item	  item;

	static s32 add(v3i pos)
	{
		itemInit();
		s32		pEntity = ENTITIES.add(Entity::Meta::ITEM, Entity::SWORD, pos);
		Entity& e		= ENTITIES.arr[pEntity];
		Sword&	item	= *new (e.data) Sword();
		item.pEntity	= pEntity;
		item.item.pItem = 2;
		return pEntity;
	}
};
void updateSword(void* data, f32 dt)
{
	Sword&	item = *(Sword*)data;
	Entity& e	 = ENTITIES.arr[item.pEntity];
	updateItem(item.item, e);
}
void drawSword(void* data)
{
	Sword&	  item	   = *(Sword*)data;
	Entity&	  e		   = ENTITIES.arr[item.pEntity];
	ItemData& itemData = ITEMS.arr[item.item.pItem];
	if (F.dudePos.z != e.iPos.z)
		return;
	drawItem(item.item, e, itemData);
}

struct Goblin
{
	EntityPtr pEntity;
	Rectangle tilesetOffset;

	SoundPtr pGrawlShort;

	CreatueStats baseStats = {
		.maxHp = 3, .strikeCooldown = 2, .dmg = 1, .range = 1.5f, .digPower = 0, .speed = 2};
	CreatueStats currentStats;
	Unit		 unit;

	static s32 add(v3i pos, Unit::Role role)
	{
		EntityPtr pEntity = ENTITIES.add(Entity::Meta::ENEMY, Entity::Arch::GOBLIN, pos);
		Entity&	  e		  = ENTITIES.arr[pEntity];
		Goblin&	  g		  = *new (e.data) Goblin;
		g.pEntity		  = pEntity;
		g.tilesetOffset	  = {32, 16, G.tileSize, G.tileSize};
		g.pGrawlShort	  = C.SFX_GOBLIN_SHORT;
		g.unit.hitpoints  = g.baseStats.maxHp;
		g.unit.role		  = role;
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
	Goblin& g = *(Goblin*)data;
	Entity& e = ENTITIES.arr[g.pEntity];

	g.currentStats = g.baseStats;
	if (F.progressLogic)
	{
		if (F.dudeHit && F.dudeAimTile == e.iPos)
		{
			unitHit(g.unit, 0.2f, Player::get(G.entDude).currentStats.dmg);
			F.dudeHit = false;
			SetSoundPitch(C.sounds[g.pGrawlShort], math::randomf(1.2f, 1.8f));
			SetSoundVolume(C.sounds[g.pGrawlShort], math::randomf(0.7f, 1.f));
			PlaySound(C.sounds[g.pGrawlShort]);
		}
		v2f toDude = (toV2f(F.dudePos - e.iPos));
		if (toDude.getLength() > g.currentStats.range)
		{  // Move
			v3i targetPos = e.iPos + toV3i((toDude.norm() * g.currentStats.speed).round());
			if (tryMove(targetPos))
				e.iPos = targetPos;
			else  // cannot move on the shortest path
				e.iPos = computeMoveToTarget(e.iPos, F.dudePos, g.currentStats.speed);
		}
		else if (g.unit.strikeCooldown <= 0)
		{  // Attack!
			if (g.unit.role == Unit::FIGHTER)
				Strike::add(F.dudePos, g.currentStats.dmg, g.pEntity, G.entDude);
			if (g.unit.role == Unit::ARCHER)
				Arrow::add(e.iPos, toV2f(F.dudePos - e.iPos), g.currentStats.dmg, 1, 7);
			g.unit.strikeCooldown = g.currentStats.strikeCooldown;
		}
	}
	e.fVel = (toV2f(e.iPos * G.tileSize) - e.fPos) * 0.1f;
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
};
struct OldMan
{
	EntityPtr pEntity;
	Rectangle tilesetOffset;

	static s32 add(v3i pos)
	{
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
	if (e.fPos.distTo(eDude.fPos) < 5.f)
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
bool tryMove(v3i pos)
{
	for (u32 i = 0; i < ENTITIES.nMax; i++)
		if (ENTITIES.active[i] && ENTITIES.arr[i].iPos == pos)
			return false;
	return true;
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
		if (tryMove(bestPos[i]))
			return bestPos[i];
	return start;
}
