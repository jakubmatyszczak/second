#include "engine/commons.cpp"
#include "engine/fileio.cpp"
#include "engine/v2.cpp"
#include "entities.cpp"

struct FrameData
{
	v2f	 mousePosWorld;
	v2f	 mousePosWindow;
	void clear() { memset(this, 0, sizeof(FrameData)); }
};
struct Globals
{
	Camera2D camera;
};
struct Level
{
};
FrameData FRAME;
Globals	  GLOBAL;
Entities  ENTITIES;
Content	  CONTENT;

struct WorldState
{
	Globals	 globals;
	Entities entities;
	Level	 levels[32];
};
void saveGame(WorldState& save)
{
	save.globals  = GLOBAL;
	save.entities = ENTITIES;
	fileio::saveRawFile("1.save", &save, sizeof(save));
}
bool loadGame(WorldState& game)
{
	WorldState loaded;
	s32		   ret = fileio::loadRawFile("1.save", sizeof(loaded), &loaded);
	if (ret < 0)
		return false;
	else if (ret != sizeof(loaded))
		exitWithMessage("Failed to load save!");
	// entities = loaded.entities;
	// GLOBAL	 = loaded.globals;
	// memcpy(game.levels, loaded.levels, sizeof(loaded.levels));
	return true;
}

int main(void)
{
	WorldState worldState;
	InitWindow(800, 600, "SECOND");
	SetTargetFPS(60.f);
	InitAudioDevice();
	CONTENT.loadAll();

	loadGame(worldState);

	GLOBAL.camera.zoom	 = 6.f;
	GLOBAL.camera.offset = {400, 300};
	u32		frame		 = 0;
	bool	done		 = false;
	Player& dude		 = Player::get(Player::add({1, 2, 3}));
	while (!done)
	{
		frame++;
		FRAME.clear();
		FRAME.mousePosWorld	 = GetScreenToWorld2D(GetMousePosition(), GLOBAL.camera);
		FRAME.mousePosWindow = GetMousePosition();

		dude.input(
			IsKeyPressed(KEY_W), IsKeyPressed(KEY_S), IsKeyPressed(KEY_A), IsKeyPressed(KEY_D));
		dude.update(0.016f);

		BeginDrawing();
		{
			ClearBackground(SKYBLUE);
			BeginMode2D(GLOBAL.camera);
			{
				dude.draw();
			}
			EndMode2D();
		}
		EndDrawing();
		if (IsKeyPressed(KEY_Q))
			done = true;
	}
	return 0;
}
