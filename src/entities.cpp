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
	enum DIR
	{
		UP,
		DOWN,
		LEFT,
		RIGHT,
	};
	EntityPtr  pEntity;
	TexturePtr pTexture;
	DIR		   dir;

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

	void input(bool up, bool down, bool left, bool right, bool hit, bool standsOnEmpty)
	{
		Entity& e		  = ENTITIES.arr[pEntity];
        if(standsOnEmpty)
            e.iPos.z--;
		s32		upDown	  = down - up;
		s32		leftRight = right - left;
		e.iPos.y += upDown;
		e.iPos.x += leftRight;
		if (upDown > 0)
			dir = UP;
		if (upDown < 0)
			dir = DOWN;
		if (leftRight > 0)
			dir = RIGHT;
		if (leftRight < 0)
			dir = LEFT;
        if(hit)
            FRAME.hit = true;
	}
	void update(f32 dt)
	{
		Entity& e = ENTITIES.arr[pEntity];
		e.fVel	  = (toV2f(e.iPos * 16) - e.fPos) * 0.1f;
		e.fPos += e.fVel;
		v3i realPos = {(s32)((e.fPos.x + 8.f) / 16.f), (s32)((e.fPos.y + 8.f) / 16.f), 0};
		switch (dir)
		{
			case UP:
				FRAME.aimTile = realPos + v3i(0, 1, 0);
				break;
			case DOWN:
				FRAME.aimTile = realPos + v3i(0, -1, 0);
				break;
			case LEFT:
				FRAME.aimTile = realPos + v3i(-1, 0, 0);
				break;
			case RIGHT:
				FRAME.aimTile = realPos + v3i(1, 0, 0);
				break;
		}
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
	}
};
