#include "engine/commons.cpp"
#include "engine/fileio.cpp"
#include "engine/v2.cpp"
#include "entities.cpp"

struct FrameData
{
	v2f	 mousePosWorld;
	v2f	 mousePosWindow;
	v3i	 aimTile;
	void clear() { memset(this, 0, sizeof(FrameData)); }
};
struct Globals
{
	Camera2D camera;
};
struct Level
{
	static const u32 nTiles = 8;
	enum Tile
	{
		EMPTY,
		GRASS,
		DIRT,
	};
	v3i	 origin;
	Tile tiles[nTiles][nTiles];
};
void createLevelSurface(v3i origin, Level& level)
{
	level.origin = origin;
	for (u32 x = 0; x < level.nTiles; x++)
		for (u32 y = 0; y < level.nTiles; y++)
		{
			level.tiles[x][y] = Level::GRASS;
		}
}
void createLevelUnderground(v3i origin, Level& level)
{
	level.origin = origin;
	for (u32 x = 0; x < level.nTiles; x++)
		for (u32 y = 0; y < level.nTiles; y++)
		{
			level.tiles[x][y] = Level::DIRT;
		}
}
void drawLevel(Level levels[], u32 bottomLevel, u32 topLevel)
{
	for (u32 i = bottomLevel; i < topLevel + 1; i++)
		for (u32 x = 0; x < Level::nTiles; x++)
			for (u32 y = 0; y < Level::nTiles; y++)
			{
				Texture2D* texture = &CONTENT.textures[CONTENT.TEX_TILESET];
				v3i		   tilePos = levels[i].origin + v3i(x * 16, y * 16, 0);
				DrawTexturePro(*texture,
							   {levels[i].tiles[x][y] * 16.f, 0, 16, 16},
							   {(f32)tilePos.x, (f32)tilePos.y, 16, 16},
							   {0, 0},
							   0,
							   WHITE);
			}
}

FrameData FRAME;
Globals	  GLOBAL;
Entities  ENTITIES;
Content	  CONTENT;

struct WorldState
{
	Globals	 globals;
	Entities entities;
	Level	 levels[33];
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

	createLevelSurface({64, 64, 32}, worldState.levels[32]);
	createLevelUnderground({64 + 16, 64 + 16, 31}, worldState.levels[31]);
	createLevelUnderground({64 + 32, 64 + 31, 30}, worldState.levels[30]);

	GLOBAL.camera.zoom	 = 6.f;
	GLOBAL.camera.offset = {400, 300};
	u32		frame		 = 0;
	bool	done		 = false;
	Player& dude		 = Player::get(Player::add({10, 10, 0}));
	Entity& eDude		 = dude.getEntity();

	while (!done)
	{
		frame++;
		FRAME.clear();
		FRAME.mousePosWorld	 = GetScreenToWorld2D(GetMousePosition(), GLOBAL.camera);
		FRAME.mousePosWindow = GetMousePosition();

		dude.input(
			IsKeyPressed(KEY_W), IsKeyPressed(KEY_S), IsKeyPressed(KEY_A), IsKeyPressed(KEY_D));

		dude.update(0.016f);
		GLOBAL.camera.target =
			(v2f(GLOBAL.camera.target) + (dude.getEntity().fPos - v2f(GLOBAL.camera.target)) * 0.1f)
				.toVector2();

		BeginDrawing();
		{
			ClearBackground(SKYBLUE);
			BeginMode2D(GLOBAL.camera);
			{
				drawLevel(worldState.levels, 30, 32);
				dude.draw();
			}
			EndMode2D();
		}
		DrawText(TextFormat("%d, %d", eDude.iPos.x, eDude.iPos.y), 10, 10, 10, BLACK);
		EndDrawing();
		if (IsKeyPressed(KEY_Q))
			done = true;
	}
	return 0;
}
