#include "engine/v2.cpp"

struct Entity
{
	enum Arch
	{
		UNKNOWN,
		DUDE,
	};
	Arch arch;
	v3i	 pos;
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
			arr[i].arch = archetype;
			arr[i].pos	= pos;
			active[i]	= true;
			return i;
		}
		return -1;
	}
};

extern Entities ENTITIES;
extern Content	CONTENT;

struct Player
{
	EntityPtr  pEntity;
	TexturePtr pTexture;

	static s32 add(v3i pos)
	{
		s32		pEntity = ENTITIES.add(Entity::DUDE, pos);
		Entity& e		= ENTITIES.arr[pEntity];
		Player& dude	= *new (e.data) Player();
		dude.pTexture	= CONTENT.TEX_TILESET;
		return pEntity;
	}
	static Player& get(EntityPtr pEntity) { return *(Player*)ENTITIES.arr[pEntity].data; };

	void input(bool up, bool down, bool left, bool right)
	{
		Entity& e = ENTITIES.arr[pEntity];
		e.pos.y += (s32)(down - up);
		e.pos.x += (s32)(right - left);
	}
	void update(f32 dt) {}
	void draw()
	{
		Entity& e = ENTITIES.arr[pEntity];
		DrawTexturePro(CONTENT.textures[pTexture],
					   {0, 16, 16, 16},
					   {(f32)e.pos.x * 16, (f32)e.pos.y * 16, 16.f, 16.f},
					   {0, 0},
					   0.f,
					   WHITE);
	}
};
