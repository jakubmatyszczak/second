#pragma once
#include "engine/commons.cpp"
#include "engine/v2.cpp"
#include "globals.cpp"
#include "animation.cpp"

typedef s32 EffectPtr;

extern bool isDead(EntityPtr);
extern bool tryHit(v3i, EntityPtr, f32);
void		updateStrike(void*, f32);
void		drawStrike(void*);
void		updateSwoosh(void*, f32);
void		drawSwoosh(void*);

struct Effect
{
	enum Arch
	{
		UNKNOWN,
		STRIKE,
		SWOOSH
	};
	enum class Type
	{
		UNKNOWN,
		CONTINOUS,
		TURNBASED
	};
	v3i	 iPos;
	v2f	 fPos;
	Type type;
	Arch arch;

	f32		  length;
	f32		  timer;
	EntityPtr source;
	EntityPtr target;
	u8		  data[256];
};
struct Effects
{
	static constexpr u32 nMax = 128;

	Effect	   arr[nMax];
	bool	   active[nMax];
	TexturePtr pTexture = Content::TEX_TILESET;

	EffectPtr add(v3i		   iPos,
				  v2f		   fPos,
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
			e		  = {iPos, fPos, type, arch, length, 0.f, source, target, {}};
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
				case Effect::SWOOSH:
					updateSwoosh(arr[i].data, dt);
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
				case Effect::SWOOSH:
					drawSwoosh(arr[i].data);
					break;
			}
		}
	}
};
Effects EFFECTS;

struct Strike
{
	EffectPtr		 pEffect;
	const Rectangle	 crosshairOffset = {48, 16, G.tileSize, G.tileSize};
	const Rectangle	 hitXOffset		 = {64, 16, G.tileSize, G.tileSize};
	f32				 crossRot		 = 0.f;
	f32				 crossScale		 = 1.f;
	f32				 dmg;
	AnimFadeout		 aFadeout;
	static EffectPtr add(v3i pos, f32 dmg, EntityPtr source, EntityPtr target)
	{
		EffectPtr effect =
			EFFECTS.add(pos, {}, Effect::Type::TURNBASED, Effect::STRIKE, 2, source, target);
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

	bool finished = s.aFadeout.update(dt);
	s.crossRot += dt * 128;
	if (F.progressLogic)
		e.timer++;
	if (e.timer == e.length)
	{
		if (!s.aFadeout.anim.active && !finished)
		{
			s.aFadeout.activate(0.2f);
			tryHit(e.iPos, e.target, s.dmg);
		}
	}
	if (e.timer > e.length || finished)
		EFFECTS.remove(s.pEffect);
    if(isDead(e.source))
        EFFECTS.remove(s.pEffect);
}
void drawStrike(void* data)
{
	Strike& s = *(Strike*)data;
	Effect& e = EFFECTS.arr[s.pEffect];

	s.crossScale  = 0.9f + sinf(math::degToRad(s.crossRot)) * 0.1f;
	f32 crossSize = G.tileSize * s.crossScale;
	v2f drawPos(e.iPos.x * G.tileSize + G.tileSize / 2.f, e.iPos.y * G.tileSize + G.tileSize / 2.f);

	if (e.timer < e.length)
		DrawTexturePro(C.textures[EFFECTS.pTexture],
					   s.crosshairOffset,
					   {drawPos.x, drawPos.y, crossSize, crossSize},
					   {crossSize / 2, crossSize / 2},
					   s.crossRot,
					   s.aFadeout.getColor());
	else
		DrawTexturePro(C.textures[EFFECTS.pTexture],
					   s.hitXOffset,
					   {drawPos.x, drawPos.y, G.tileSize, G.tileSize},
					   {G.tileSize / 2, G.tileSize / 2},
					   0.f,
					   s.aFadeout.getColor());
}
struct Swoosh
{
	EffectPtr		pEffect;
	const Rectangle swooshOffset = {80, 16, G.tileSize, G.tileSize};
	v2f				dir;
	f32				drawRot;
	AnimFadeout		aFadeOut;

	static EffectPtr add(v2f pos, f32 len, v2f dir)
	{
		EffectPtr pEffect =
			EFFECTS.add({}, pos, Effect::Type::CONTINOUS, Effect::SWOOSH, len, -1, -1);
		Swoosh& effect = *new (EFFECTS.arr[pEffect].data) Swoosh;
		effect.pEffect = pEffect;
		effect.dir	   = dir.norm();
		effect.drawRot = atan2f(dir.y, dir.x);

		return pEffect;
	}
};
void updateSwoosh(void* data, f32 dt)
{
	Swoosh& s = *(Swoosh*)data;
	Effect& e = EFFECTS.arr[s.pEffect];
	e.timer += dt;
	s.aFadeOut.update(dt);

	e.fPos += s.dir * (e.length - e.timer);
	if (e.timer > e.length * 0.5f && !s.aFadeOut.anim.active)
		s.aFadeOut.activate(e.length * 0.5f);
	if (e.timer > e.length)
		EFFECTS.remove(s.pEffect);
}
void drawSwoosh(void* data)
{
	Swoosh& s = *(Swoosh*)data;
	Effect& e = EFFECTS.arr[s.pEffect];
	DrawTexturePro(C.textures[EFFECTS.pTexture],
				   s.swooshOffset,
				   {e.fPos.x, e.fPos.y, G.tileSize, G.tileSize},
				   {G.tileSize * 0.5f, G.tileSize * 0.5f},
				   math::radToDeg(s.drawRot),
				   s.aFadeOut.getColor());
}
