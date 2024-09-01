#pragma once
#include "engine/commons.cpp"
#include "engine/v2.cpp"
#include "globals.cpp"

typedef s32 EffectPtr;

extern bool tryHit(v3i, EntityPtr, f32);
void		updateStrike(void*, f32);
void		drawStrike(void*);

struct Effect
{
	enum Arch
	{
		UNKNOWN,
		STRIKE
	};
	enum class Type
	{
		UNKNOWN,
		CONTINOUS,
		TURNBASED
	};
	v3i	 iPos;
	Type type;
	Arch arch;

	f32		  length;
	f32		  timer;
	EntityPtr source;
	EntityPtr target;
	f32		  fadeOutTimer;
	f32		  crossRot	 = 0.f;
	f32		  crossScale = 1.f;
	u8		  data[256];
};
struct Effects
{
	static constexpr u32 nMax = 128;

	Effect	   arr[nMax];
	bool	   active[nMax];
	TexturePtr pTexture		   = Content::TEX_TILESET;
	Rectangle  crosshairOffset = {48, 16, G.tileSize, G.tileSize};
	Rectangle  hitXOffset	   = {64, 16, G.tileSize, G.tileSize};
	f32		   fadeOutTime	   = 0.333f;

	EffectPtr add(v3i		   pos,
				  Effect::Type type,
				  Effect::Arch arch,
				  f32		   length,
				  EntityPtr	   source,
				  EntityPtr	   target)
	{
		for (u32 i = 1; i < 128; i++)
		{
			if (active[i])
				continue;
			Effect& e = arr[i];
			e		  = {pos, type, arch, length, 0.f, source, target, fadeOutTime, 0.f, 0.f, {}};
			if (type == Effect::Type::TURNBASED)
				e.length = roundf(length) < 1 ? 1 : roundf(length);
			else
				e.length = fabsf(length);
			active[i] = true;
			return i;
		}
		exitWithMessage("Too many effects created!");
		return -1;
	}
	void remove(EffectPtr pEffect) { active[pEffect] = false; }
	void updateAll(f32 dt)
	{
		for (u32 i = 1; i < nMax; i++)
		{
			if (!active[i])
				continue;
			switch (arr[i].arch)
			{
				case Effect::UNKNOWN:
					break;
				case Effect::STRIKE:
					updateStrike(arr[i].data, dt);
					break;
			}
		}
	}
	void drawAll()
	{
		for (u32 i = 1; i < nMax; i++)
		{
			if (!active[i])
				continue;
			switch (arr[i].arch)
			{
				case Effect::UNKNOWN:
					break;
				case Effect::STRIKE:
					drawStrike(arr[i].data);
					break;
			}
		}
	}
};
Effects EFFECTS;

struct Strike
{
	EffectPtr		 pEffect;
	f32				 dmg;
	static EffectPtr add(v3i pos, f32 dmg, EntityPtr source, EntityPtr target)
	{
		EffectPtr effect =
			EFFECTS.add(pos, Effect::Type::TURNBASED, Effect::STRIKE, 2, source, target);
		Strike& strike = *new (EFFECTS.arr[effect].data) Strike;
		strike.pEffect = effect;
		strike.dmg	   = dmg;
		return effect;
	}
};
void updateStrike(void* data, f32 dt)
{
	Strike& s = *(Strike*)data;
	Effect& e = EFFECTS.arr[s.pEffect];

	e.crossRot += dt * 128;
	if (F.progressLogic)
		e.timer++;
	if (e.timer < e.length)
		return;
	if (e.fadeOutTimer >= EFFECTS.fadeOutTime)
		tryHit(e.iPos, e.target, s.dmg);
	e.fadeOutTimer -= dt;
	if (e.fadeOutTimer <= 0.f)
		EFFECTS.remove(s.pEffect);
}
void drawStrike(void* data)
{
	Strike& s = *(Strike*)data;
	Effect& e = EFFECTS.arr[s.pEffect];

	e.crossScale  = 0.9f + sinf(math::degToRad(e.crossRot)) * 0.1f;
	f32 crossSize = G.tileSize * e.crossScale;
	v2f drawPos(e.iPos.x * G.tileSize + G.tileSize / 2.f, e.iPos.y * G.tileSize + G.tileSize / 2.f);

	f32 hitXScale = e.fadeOutTimer * 6.f;
	f32 hitXSize  = G.tileSize * hitXScale;

	if (e.fadeOutTimer >= EFFECTS.fadeOutTime)
		DrawTexturePro(C.textures[EFFECTS.pTexture],
					   EFFECTS.crosshairOffset,
					   {drawPos.x, drawPos.y, crossSize, crossSize},
					   {crossSize / 2, crossSize / 2},
					   e.crossRot,
					   WHITE);
	else
		DrawTexturePro(C.textures[EFFECTS.pTexture],
					   EFFECTS.hitXOffset,
					   {drawPos.x, drawPos.y, hitXSize, hitXSize},
					   {hitXSize / 2, hitXSize / 2},
					   0.f,
					   WHITE);
}
