#pragma once

#include "../../raylib/include/raylib.h"
#include <sys/time.h>
#include <cstdint>
#include <cstdlib>
#include <iostream>

typedef uint8_t	 u8;
typedef int8_t	 i8;
typedef uint8_t	 u16;
typedef int8_t	 i16;
typedef uint32_t u32;
typedef int32_t	 i32;
typedef uint64_t u64;
typedef int64_t	 i64;
typedef float	 f32;

struct Screen
{
	u32 width  = 800;
	u32 height = 600;
};
struct GameState
{
	u32 frame = 0;
	f32 time  = 0.f;
	f32 dTime = 0.016f;
};
struct Content
{
	enum TEX_ID
	{
		TEX_DUDE,
		TEX_TABLE,
		TEX_KEY,
		TEX_HOLE,
		TEX_SHADOW,
		TEX_LEVEL1,
		TEX_LEVEL2,
        TEX_ID_MAX
	};
	enum SOUND_ID
	{
		SOUND_JUMP,
		SOUND_WHAM,
        SOUND_ID_MAX
	};
	Texture2D textures[32];
	Sound	  sounds[32];

	void loadTexture(const char* filepath, TEX_ID id) { textures[id] = LoadTexture(filepath); }
	void loadSound(const char* filepath, SOUND_ID id) { sounds[id] = LoadSound(filepath); }
};
struct EngineGlobals
{
	Screen	  screen;
	GameState state;
	Camera2D  camera;

	bool drawDebugCollision = false;
};
EngineGlobals GLOBAL;
Content	  content;

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
