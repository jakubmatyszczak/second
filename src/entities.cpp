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

	s32 add(Entity::Arch archetype, v3i pos)
	{
		for (u32 i = 1; i < nMax; i++)
		{
			if (active[i])
				continue;
			memset(&arr[i], 0, sizeof(arr[i]));
			arr[i].arch = archetype;
			arr[i].iPos = pos;
			arr[i].fPos = {(f32)pos.x * G.tileSize, (f32)pos.y * G.tileSize};
			arr[i].fVel = {};
			active[i]	= true;
			return i;
		}
		return -1;
	}
};

extern Entities	 ENTITIES;
extern Content	 C;
extern FrameData F;

struct Player
{
	EntityPtr  pEntity;
	TexturePtr pTexture;
	v2f		   direction;
	bool	   canClimb;
	bool	   canGoDown;

	static s32 add(v3i pos)
	{
		s32		pEntity = ENTITIES.add(Entity::DUDE, pos);
		Entity& e		= ENTITIES.arr[pEntity];
		Player& dude	= *new (e.data) Player();
		dude.pTexture	= C.TEX_TILESET;
		dude.pEntity	= pEntity;

		F.dudePos = pos;
		return pEntity;
	}
	static Player& get(EntityPtr pEntity) { return *(Player*)ENTITIES.arr[pEntity].data; };
	Entity&		   getEntity() { return ENTITIES.arr[pEntity]; }

	void input(bool up,
			   bool down,
			   bool left,
			   bool right,
			   bool hit,
			   bool go,
			   bool goUp,
			   bool goDown,
			   bool willFall,
			   bool canClimb,
			   bool canGoDown)
	{
		Entity& e		= ENTITIES.arr[pEntity];
		e.iMoveTarget	= e.iPos;
		this->canClimb	= canClimb;
		this->canGoDown = canGoDown;

		if (up | down | left | right)
		{
			// TODO: seems this could be done nicely with mouse and gamepad.
			direction += {(f32)(right - left), (f32)(down - up)};
			direction = direction.norm().round();
		}
		if (go)
			e.iMoveTarget = e.iPos + toV3i(direction);
		if (willFall || (canGoDown && go && direction.isZero()))
			e.iMoveTarget.z--;
		if (canClimb && goUp && direction.isZero())
			e.iMoveTarget.z++;
		if (hit)
			F.dudeHit = true;
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
	}
	void move()
	{
		Entity& e = ENTITIES.arr[pEntity];
		if (e.iMoveTarget == e.iPos || e.iMoveTarget == v3i())
			return;
		e.iPos = e.iMoveTarget;
	}
	void draw()
	{
		Entity& e = ENTITIES.arr[pEntity];
		DrawTexturePro(C.textures[pTexture],
					   {0, 16, 16, 16},
					   {e.fPos.x + 2, e.fPos.y + 2, 12.f, 12.f},
					   {0, 0},
					   0.f,
					   WHITE);
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
	void drawOverlay() { DrawText(TextFormat("%d, %d", canClimb, canGoDown), 10, 30, 18, RED); }
};
