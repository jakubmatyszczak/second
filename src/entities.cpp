#pragma once
#include "globals.cpp"
#include "effects.cpp"

void updatePickaxe(void*, f32);
void drawPickaxe(void*);
void updateGoblin(void*, f32);
void drawGoblin(void*);

struct Entity
{
	enum class Meta
	{
		UNKNOWN = 0,
		PLAYER,
		ITEM,
		ENEMY,
	};
	enum Arch
	{
		UNKNOWN = 0,
		DUDE,
		PICKAXE,
		GOBLIN,
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
	f32		  digPower;
	Rectangle tilesetOffset;
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
	f32 meleeDmg;
	f32 digPower;
};

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
    f32 gotHit = 0;

	CreatueStats baseStats = {.maxHp = 10, .strikeCooldown = 1, .meleeDmg = 1, .digPower = 1};
	CreatueStats currentStats;
	f32			 hitpoints = baseStats.maxHp;

	static s32 add(v3i pos)
	{
		s32		pEntity	   = ENTITIES.add(Entity::Meta::PLAYER, Entity::DUDE, pos);
		Entity& e		   = ENTITIES.arr[pEntity];
		Player& dude	   = *new (e.data) Player();
		dude.pEntity	   = pEntity;
		dude.pTexture	   = C.TEX_TILESET;
		dude.pSfxHit	   = C.SFX_HIT;
		dude.pSfxStepGrass = C.SFX_STEP_GRASS;
		dude.pSfxStepRock  = C.SFX_STEP_ROCK;

		F.dudePos = pos;
		return pEntity;
	}
	static Player& get(EntityPtr pEntity) { return *(Player*)ENTITIES.arr[pEntity].data; };
	Entity&		   getEntity() { return ENTITIES.arr[pEntity]; }

	bool input(bool up,
			   bool down,
			   bool left,
			   bool right,
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

		if (up | down | left | right)
		{
			// TODO: seems this could be done nicely with mouse and gamepad.
			direction += {(f32)(right - left), (f32)(down - up)};
			direction = direction.norm().round();
		}
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
		}
		if (use || hit || go)
			return true;
		return false;
	}
	void update(f32 dt)
	{
		Entity& e = ENTITIES.arr[pEntity];
        gotHit -= dt;
		e.fVel	  = (toV2f(e.iPos * 16) - e.fPos) * 0.1f;
		e.fPos += e.fVel;
		v3i realPos	  = {(s32)((e.fPos.x + 8.f) / 16.f), (s32)((e.fPos.y + 8.f) / 16.f), e.iPos.z};
		F.dudePos	  = e.iPos;
		F.dudeAimTile = realPos + toV3i(direction);
		if (F.dudeAimTile == F.dudePos)
			F.dudeAimTile.z--;
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
			if (ENTITIES.arr[F.entUsed].arch == Entity::PICKAXE)
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
	}
	static bool tryHit(EntityPtr playerEntity, f32 dmg)
	{
		Player& p = Player::get(playerEntity);
		p.hitpoints -= dmg;
        p.gotHit = 0.6f;
		if (p.hitpoints <= 0)
		{
			exitWithMessage("XD zdech XD");
			ENTITIES.remove(playerEntity);
		}
		PlaySound(C.sounds[C.SFX_OOF]);
		return true;
	}
	void draw()
	{
		Entity& e = ENTITIES.arr[pEntity];
		DrawTexturePro(C.textures[pTexture],
					   {0, 16, 16, 16},
					   {e.fPos.x + 2, e.fPos.y + 2, 12.f, 12.f},
					   {0, 0},
					   0.f,
					   gotHit > 0 ? RED : WHITE);
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
		DrawText(TextFormat("Stats:\ndigPower: %.f", currentStats.digPower), 10, 40, 20, RED);
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
		DrawRectangle(55, 15, (s32)(100.f * (hitpoints / baseStats.maxHp)), 10, RED);
	}
};
void itemInit()
{
	static bool init = false;
	if (init)
		return;
	{
		strncpy(ITEMS.arr[1].name, "Basic Pickaxe", 15);
		ITEMS.arr[1].digPower	   = 1;
		ITEMS.arr[1].tilesetOffset = {0, 32, G.tileSize, G.tileSize};
	}
	init = true;
};
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

	if (F.dudeUse && F.dudePos == e.iPos && !pick.item.picked)
	{
		pick.item.picked = true;
		F.entUsed		 = pick.pEntity;
		F.itemUsed		 = pick.item.pItem;
	}

	if (!pick.item.picked)
		e.fPos = toV2f(e.iPos) * G.tileSize;
	else
		e.iPos = F.dudePos;
}
void drawPickaxe(void* data)
{
	Pickaxe&  pick	   = *(Pickaxe*)data;
	Entity&	  e		   = ENTITIES.arr[pick.pEntity];
	ItemData& itemData = ITEMS.arr[pick.item.pItem];
	if (F.dudePos.z != e.iPos.z)
		return;

	Texture&   tex	   = C.textures[ITEMS.tileset];
	v2f		   origin  = {4, 4};
	v2f		   drawPos = e.fPos + origin * 2;
	static f32 t	   = 0;
	t += 0.064f;
	f32 breath = ((sinf(t) + 1) * 0.5f) * 0.25f + 0.75f;
	if (!pick.item.picked)
	{
		DrawEllipse(drawPos.x, drawPos.y + 6, 4 * breath, 2 * breath, GRAY_CLEAR);
		DrawTexturePro(tex,
					   itemData.tilesetOffset,
					   {drawPos.x, drawPos.y - 4 * breath, 8.f, 8.f},
					   origin.toVector2(),
					   45.f,
					   WHITE);
	}
}

struct Goblin
{
	EntityPtr pEntity;
	Rectangle tilesetOffset;

	SoundPtr pGrawlShort;

	CreatueStats baseStats = {.maxHp = 3, .strikeCooldown = 2, .meleeDmg = 1, .digPower = 0};
	CreatueStats currentStats;
	f32			 hitpoints		= baseStats.maxHp;
	s32			 strikeCooldown = baseStats.strikeCooldown;

	static s32 add(v3i pos)
	{
		EntityPtr pEntity = ENTITIES.add(Entity::Meta::ENEMY, Entity::Arch::GOBLIN, pos);
		Entity&	  e		  = ENTITIES.arr[pEntity];
		Goblin&	  g		  = *new (e.data) Goblin;
		g.pEntity		  = pEntity;
		g.tilesetOffset	  = {32, 16, G.tileSize, G.tileSize};
		g.pGrawlShort	  = C.SFX_GOBLIN_SHORT;
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
			g.hitpoints -= 1;
			F.dudeHit = false;
			SetSoundPitch(C.sounds[g.pGrawlShort], math::randomf(1.2f, 1.8f));
			SetSoundVolume(C.sounds[g.pGrawlShort], math::randomf(0.7f, 1.f));
			PlaySound(C.sounds[g.pGrawlShort]);
		}
		if (g.hitpoints <= 0)
		{
			SetSoundPitch(C.sounds[g.pGrawlShort], math::randomf(0.7f, 1.f));
			PlaySound(C.sounds[g.pGrawlShort]);
			ENTITIES.remove(g.pEntity);
		}
		v2f toDude = (toV2f(F.dudePos - e.iPos));
		if (toDude.getLength() > 1.f)
			e.iPos += toV3i(toDude.norm().round());
		else if (g.strikeCooldown-- <= 0)
		{
			Strike::add(F.dudePos, g.currentStats.meleeDmg, g.pEntity, G.entDude);
			g.strikeCooldown = g.currentStats.strikeCooldown;
		}
	}
	e.fVel = (toV2f(e.iPos * G.tileSize) - e.fPos) * 0.1f;
	e.fPos += e.fVel;
	v3i realPos = {(s32)((e.fPos.x + 8.f) / 16.f), (s32)((e.fPos.y + 8.f) / 16.f), e.iPos.z};
};
void drawGoblin(void* data)
{
	Goblin& g = *(Goblin*)data;
	Entity& e = ENTITIES.arr[g.pEntity];

	if (F.dudePos.z != e.iPos.z)
		return;
	v2f drawPos = e.fPos;
	DrawTexturePro(C.textures[C.TEX_TILESET],
				   g.tilesetOffset,
				   {drawPos.x + 2.5f, drawPos.y + 2.5f, 10, 10},
				   {},
				   0.f,
				   WHITE);
};

bool tryHit(v3i pos, EntityPtr target, f32 dmg)
{
	Entity& e = ENTITIES.arr[target];
	if (e.iPos == pos)
		if (e.meta == Entity::Meta::PLAYER)
			return Player::tryHit(target, dmg);
	return false;
}
