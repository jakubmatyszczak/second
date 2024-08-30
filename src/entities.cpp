#include "globals.cpp"

struct Entity
{
	enum Arch
	{
		UNKNOWN,
		DUDE,
	};
	Arch arch;
	v3i	 iPos;
	v2f	 fPos;
	v2f	 fVel;
	u8	 data[1024];
};
struct Entities
{
	static constexpr u32 nMax		  = 128;
	Entity				 arr[nMax]	  = {};
	bool				 active[nMax] = {};

	s32 add(Entity::Arch archetype, v3i pos)
	{
		for (u32 i = 1; i < nMax; i++)
		{
			if (active[i])
				continue;
			memset(&arr[i], 0, sizeof(arr[i]));
			arr[i].arch = archetype;
			arr[i].iPos = pos;
			arr[i].fPos = {(f32)pos.x * GLOBAL.tileSize, (f32)pos.y * GLOBAL.tileSize};
			arr[i].fVel = {};
			active[i]	= true;
			return i;
		}
		return -1;
	}
};

extern Entities	 ENTITIES;
extern Content	 CONTENT;
extern FrameData FRAME;

struct Player
{
	EntityPtr  pEntity;
	TexturePtr pTexture;
	v2f		   direction;

	static s32 add(v3i pos)
	{
		s32		pEntity = ENTITIES.add(Entity::DUDE, pos);
		Entity& e		= ENTITIES.arr[pEntity];
		Player& dude	= *new (e.data) Player();
		dude.pTexture	= CONTENT.TEX_TILESET;
		dude.pEntity	= pEntity;
		return pEntity;
	}
	static Player& get(EntityPtr pEntity) { return *(Player*)ENTITIES.arr[pEntity].data; };
	Entity&		   getEntity() { return ENTITIES.arr[pEntity]; }

	void input(bool up, bool down, bool left, bool right, bool hit, bool go, bool standsOnEmpty)
	{
		Entity& e = ENTITIES.arr[pEntity];
		if (standsOnEmpty)
			e.iPos.z--;
		if (up | down | left | right)
		{
			direction += {(f32)(right - left), (f32)(down - up)};
			direction = direction.norm().round();
		}
		if (go)
		{
			e.iPos += toV3i(direction);
		}
		if (hit)
			FRAME.hit = true;
	}
	void update(f32 dt)
	{
		Entity& e = ENTITIES.arr[pEntity];
		e.fVel	  = (toV2f(e.iPos * 16) - e.fPos) * 0.1f;
		e.fPos += e.fVel;
		v3i realPos	  = {(s32)((e.fPos.x + 8.f) / 16.f), (s32)((e.fPos.y + 8.f) / 16.f), 0};
		FRAME.aimTile = realPos + toV3i(direction);
	}
	void draw()
	{
		Entity& e = ENTITIES.arr[pEntity];
		DrawTexturePro(CONTENT.textures[pTexture],
					   {0, 16, 16, 16},
					   {e.fPos.x, e.fPos.y, 16.f, 16.f},
					   {0, 0},
					   0.f,
					   WHITE);
		static f32 t = 0.f;
		t += 0.256f;
		f32 scaling = 8.f + ((sinf(t) * 0.5f + 0.5f) * 2.f);

		DrawCircle(e.fPos.x + 8 + roundf(direction.x) * scaling,
				   e.fPos.y + 8 + roundf(direction.y) * scaling,
				   2,
				   RED);
	}
};
