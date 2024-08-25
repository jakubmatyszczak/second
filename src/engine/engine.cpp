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
		TEX_GATE,
		TEX_BADDIE,
		TEX_CLOUD,
		TEX_LEVEL1,
		TEX_LEVEL2,
		TEX_ID_MAX
	};
	enum SOUND_ID
	{
		SOUND_JUMP,
		SOUND_WHAM,
        SOUND_LASER,
        SOUND_BADDIE_TARGET_FOUND1,
        SOUND_BADDIE_TARGET_FOUND2,
        SOUND_BADDIE_TARGET_FOUND3,
        SOUND_BADDIE_TARGET_FOUND4,
        SOUND_BADDIE_TARGET_FOUND5,
        SOUND_BADDIE_TARGET_LOST1,
        SOUND_BADDIE_TARGET_LOST2,
        SOUND_BADDIE_STOMP,
		SOUND_ID_MAX
	};
	Texture2D textures[32];
	Sound	  sounds[32];

	void loadTexture(const char* filepath, TEX_ID id) { textures[id] = LoadTexture(filepath); }
	void loadSound(const char* filepath, SOUND_ID id) { sounds[id] = LoadSound(filepath); }
};
struct EngineGlobals
{
	u32		  pDudeInstance;
	Screen	  screen;
	GameState state;
	Camera2D  camera;

	bool drawDebugCollision = false;
};
EngineGlobals GLOBAL;
Content		  content;
void loadContent(Content& content)
{
	content.loadTexture("res/art/dude_ss.png", Content::TEX_DUDE);
	content.loadTexture("res/art/table_tex.png", Content::TEX_TABLE);
	content.loadTexture("res/art/key.png", Content::TEX_KEY);
	content.loadTexture("res/art/shadow_tex.png", Content::TEX_SHADOW);
	content.loadTexture("res/art/level1.png", Content::TEX_LEVEL1);
	content.loadTexture("res/art/level2.png", Content::TEX_LEVEL2);
	content.loadTexture("res/art/hole_ss.png", Content::TEX_HOLE);
	content.loadTexture("res/art/gateway_ss.png", Content::TEX_GATE);
	content.loadTexture("res/art/baddie_ss.png", Content::TEX_BADDIE);
	content.loadTexture("res/art/cloud_tex.png", Content::TEX_CLOUD);
	content.loadSound("res/sound/jump.wav", Content::SOUND_JUMP);
	content.loadSound("res/sound/punch.wav", Content::SOUND_WHAM);
	content.loadSound("res/sound/laser.wav", Content::SOUND_LASER);
	content.loadSound("res/sound/baddie/GetOverHere.wav", Content::SOUND_BADDIE_TARGET_FOUND1);
	content.loadSound("res/sound/baddie/Initializing.wav", Content::SOUND_BADDIE_TARGET_FOUND2);
	content.loadSound("res/sound/baddie/MissionObjectiveSet.wav", Content::SOUND_BADDIE_TARGET_FOUND3);
	content.loadSound("res/sound/baddie/TargetAcquired.wav", Content::SOUND_BADDIE_TARGET_FOUND4);
	content.loadSound("res/sound/baddie/TargetFound.wav", Content::SOUND_BADDIE_TARGET_FOUND5);
	content.loadSound("res/sound/baddie/TargetLost.wav", Content::SOUND_BADDIE_TARGET_LOST1);
	content.loadSound("res/sound/baddie/TargetOutOfSight.wav", Content::SOUND_BADDIE_TARGET_LOST2);
	content.loadSound("res/sound/baddie/robotstep2.wav", Content::SOUND_BADDIE_STOMP);
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
