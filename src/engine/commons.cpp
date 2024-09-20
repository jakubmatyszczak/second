#pragma once
#include <sys/time.h>
#include <iostream>
#include <unistd.h>
#include <math.h>
#include <cstring>

#include "raylib.h"

typedef uint8_t	 u8;
typedef int8_t	 s8;
typedef uint8_t	 u16;
typedef int8_t	 s16;
typedef uint32_t u32;
typedef int32_t	 s32;
typedef uint64_t u64;
typedef int64_t	 s64;
typedef float	 f32;

typedef s32 EntityPtr;
typedef s32 TexturePtr;
typedef s32 SoundPtr;

struct Content
{
	enum TEX_ID
	{
		TEX_TILESET,
		TEX_PORTRAITS,
		TEX_TILESET_TERRAIN,
		TEX_ID_MAX
	};
	enum SFX_ID
	{
		SFX_HIT,
		SFX_STEP_GRASS,
		SFX_STEP_ROCK,
		SFX_GOBLIN_SHORT,
		SFX_OOF,
		SFX_ID_MAX
	};
	enum FONT_ID
	{
		FONT_ID_PIXEL,
	};
	enum TEX_LEVEL
	{
		LEVEL_CAMP,
	};
	Texture2D textures[32];
	Sound	  sounds[32];
	Font	  fonts[8];
	Image	  levelMap[32];

	void loadTexture(const char* filepath, TEX_ID id) { textures[id] = LoadTexture(filepath); }
	void loadLevelMap(const char* filepath, TEX_LEVEL id) { levelMap[id] = LoadImage(filepath); }
	void loadSfx(const char* filepath, SFX_ID id) { sounds[id] = LoadSound(filepath); }
	void loadFont(const char* filepath, FONT_ID id) { fonts[id] = LoadFont(filepath); }
	void loadAll()
	{
		loadTexture("res/art/tileset.png", TEX_TILESET);
		loadTexture("res/art/portraits.png", TEX_PORTRAITS);
		loadTexture("res/art/tileset_terrain.png", TEX_TILESET_TERRAIN);
		loadSfx("res/sound/playerSfx/hit.wav", SFX_HIT);
		loadSfx("res/sound/playerSfx/step_grass.wav", SFX_STEP_GRASS);
		loadSfx("res/sound/playerSfx/step_rock.wav", SFX_STEP_ROCK);
		loadSfx("res/sound/goblin.wav", SFX_GOBLIN_SHORT);
		loadSfx("res/sound/oof.wav", SFX_OOF);
		loadFont("res/fonts/pixelplay.png", FONT_ID_PIXEL);
		loadLevelMap("res/level/camp.png", LEVEL_CAMP);
	}
};

inline void exitWithMessage(const char* msg)
{
	const char* KNRM = "\x1B[0m";
	const char* KRED = "\x1B[31m";
	// const char* KGRN = "\x1B[32m";
	// const char* KYEL = "\x1B[33m";
	// const char* KBLU = "\x1B[34m";
	// const char* KMAG = "\x1B[35m";
	// const char* KCYN = "\x1B[36m";
	// const char* KWHT = "\x1B[37m";

	using namespace std;
	cerr << KRED << "WOWOWOWO HOLD UP!" << endl
		 << msg << endl
		 << "SO I AM QUITTING DAWG!" << KNRM << endl;
	exit(1);
}
inline u32 getTimestamp()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return 1000000 * tv.tv_sec + tv.tv_usec;
}

namespace math
{
	constexpr f32 pi  = M_PIf;
	constexpr f32 pi2 = pi / 2.f;
	constexpr f32 pi4 = pi / 4.f;
	constexpr f32 tau = 2.f * M_PIf;

	inline f32 radToDeg(f32 value) { return value * 180.f / pi; };
	inline f32 degToRad(f32 value) { return value / 180.f * pi; };
	inline f32 sq(f32 value) { return value * value; }
	inline s32 divCeil(s32 x, s32 y) { return (x + y - 1) / y; }

	s32 limit(s32 value, s32 min, s32 max)
	{
		if (value > max)
			return max;
		if (value < min)
			return min;
		return value;
	}
	f32 limit(float value, float min, float max)
	{
		if (value > max)
			return max;
		if (value < min)
			return min;
		return value;
	}
	float limit(float value, float minmax) { return limit(value, -minmax, minmax); }
	s32	  limit(s32 value, s32 minmax) { return limit(value, -minmax, minmax); }
	float randomf(float min, float max)
	{
		float range = fabsf(max - min);
		return (fmodf(rand(), range * 1000.f) / 1000.f + (min < max ? min : max));
	}
	s32 random(s32 min, s32 max)
	{
		if (max < min)
		{
			max = max + min;  // swap without 3rd variable
			min = max - min;
			max = max - min;
		}
		s32 range = max - min;
		return (rand() % range) + min;
	}
}  // namespace math
