#include "engine/fileio.cpp"
#include "engine/input.cpp"
#include "entities.cpp"
#include "map.cpp"
#include "dialogs.cpp"
#include "lighting.cpp"

#include "raylib.h"

struct WorldState
{
};
Map		   MAP;
Entities   ENTITIES;
DialogBox  DIALOGBOX;
Narrative  NARRATIVE;
WorldState WORLD;

struct MapLayerTemplate
{
	static constexpr u32 size = 64;
	MapLayer::Tile::Type terrain[size][size];
	Item::Arch			 items[size][size];
	Object::Arch		 objects[size][size];

	static bool load(MapLayer& layer, const char* filename)
	{
		MapLayerTemplate tmp = {};
		if (!fileio::loadRawFile(filename, sizeof(MapLayerTemplate), &tmp))
			return false;
		if (layer.nTiles != tmp.size)
			return false;
		bool isSurface = (layer.origin.z == 0);
		for (s32 x = 0; x < size; x++)
			for (s32 y = 0; y < size; y++)
			{
				layer.tile[x][y].placeTile(tmp.terrain[x][y], isSurface);
				v3i pos = layer.origin + v3i(x, y, 0);
				placeObject(tmp.objects[x][y], pos);
				placeItem(tmp.items[x][y], pos);
			}
		return true;
	}
	static bool save(MapLayer& layer, const char* filename)
	{
		MapLayerTemplate tmp = {};
		for (s32 x = 0; x < size; x++)
			for (s32 y = 0; y < size; y++)
			{
				v3i pos			  = layer.origin + v3i(x, y, 0);
				tmp.terrain[x][y] = layer.getTileAt(pos).type;
			}
		for (s32 i = 0; i < ENTITIES.nMax; i++)
		{
			Entity&		e	 = ENTITIES.arr[i];
			v3i			pos	 = layer.toLocalCoords(e.iPos);
			const Item* item = Item::getItemFromEntity(i);
			if (item)
				tmp.items[pos.x][pos.y] = item->arch;
			const Object* object = Object::getObjectFromEntity(i);
			if (object)
				tmp.objects[pos.x][pos.y] = object->arch;
		}
		return fileio::saveRawFile(filename, &tmp, sizeof(tmp));
	}
};

bool canClimb(v3i& pos)
{
	MapLayer& lcurrent = MAP.level[pos.z];
	MapLayer& labove   = MAP.level[pos.z + 1];
	if (lcurrent.containsXYZ(pos) && lcurrent.getTileAt(pos).type == MapLayer::Tile::EMPTY)
		if (labove.containsXY(pos))
		{
			MapLayer::Tile& tileAbove = labove.getTileAt(pos);
			if (tileAbove.type == MapLayer::Tile::EMPTY)
				return true;
		}
	return false;
}
bool canGoDown(v3i& pos)
{
	MapLayer& lcurrent = MAP.level[pos.z];
	MapLayer& lbelow   = MAP.level[pos.z - 1];
	if (lcurrent.containsXYZ(pos) && lcurrent.getTileAt(pos).type == MapLayer::Tile::EMPTY)
		if (lbelow.containsXY(pos))
		{
			MapLayer::Tile& tileBelow = lbelow.getTileAt(pos);
			if (tileBelow.type == MapLayer::Tile::EMPTY)
				return true;
		}
	return false;
}

void saveGame()
{
	if (fileio::saveRawFile("1.save", &WORLD, sizeof(WORLD)))
		printf("GAME SAVED!\n");
}
bool loadGame()
{
	WorldState loaded;
	s32		   ret = fileio::loadRawFile("1.save", sizeof(loaded), &loaded);
	if (ret < 0)
		return false;
	else if (ret != sizeof(loaded))
		exitWithMessage("Failed to load save!");
	memcpy(&WORLD, &loaded, sizeof(loaded));
	MAP.reloadPtr();
	printf("GAME LOADED!\n");
	return true;
}

int main(void)
{
	G.screenX = 1320;
	G.screenY = 720;
	InitWindow(G.screenX, G.screenY, "SECOND");
	SetTargetFPS(60.f);
	InitAudioDevice();
	C.loadAll();

	createLevelSurface({0, 0, 0}, MAP.level[0]);
	createLevelUnderground({0, 0, -1}, MAP.level[-1]);
	createLevelUnderground({0, 0, -2}, MAP.level[-2]);
	createLevelDeepUnderground({0, 0, -3}, MAP.level[-3]);
	createLevelDeepUnderground({0, 0, -4}, MAP.level[-4]);
	createLevelDeepUnderground({0, 0, -5}, MAP.level[-5]);
	Item::add(Item::PICKAXE, {10, 52, 0});
	Item::add(Item::SWORD, {10, 53, 0});
	Goblin::add({24, 30, 0}, Unit::Role::FIGHTER);
	Goblin::add({25, 31, 0}, Unit::Role::ARCHER);
	Goblin::add({24, 32, 0}, Unit::Role::FIGHTER);
	Goblin::add({23, 31, 0}, Unit::Role::FIGHTER);

	Goblin::add({38, 32, 0}, Unit::Role::FIGHTER);
	Goblin::add({40, 32, 0}, Unit::Role::ARCHER);
	Goblin::add({39, 41, 0}, Unit::Role::FIGHTER);
	Goblin::add({39, 33, 0}, Unit::Role::FIGHTER);

	Goblin::add({35, 41, 0}, Unit::Role::FIGHTER);
	Goblin::add({30, 41, 0}, Unit::Role::FIGHTER);

	Goblin::add({43, 21, 0}, Unit::Role::ARCHER);
	addCampfire({7, 53, 0});
	addCampfire({20, 53, 0});

	addTorch({7, 40, 0});
	addTorch({17, 40, 0});
	addTorch({27, 40, 0});

	G.camera.zoom	= 3.f;
	G.camera.offset = {660, 360};
	bool done		= false;
	G.entDude		= Player::add({8, 53, 0});
	// G.entDude	  = Player::add({26, 26, 0});
	Player& dude  = Player::get(G.entDude);
	Entity& eDude = dude.getEntity();
	NARRATIVE.init();
	Input input;
	// NARRATIVE.start(0);

	Light l;
	l.init({1320, 720}, 0.5f);

	bool editor			= false;
	u32	 editorTileType = 0;

	while (!done)
	{
		if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_S))
			MapLayerTemplate::save(MAP.level[0], "surface.asdf");
		// saveGame();
		if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_L))
			MapLayerTemplate::load(MAP.level[0], "surface.asdf");
		// loadGame();
		if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_E))
			editor = !editor;
		G.debugDrawSightRange = IsKeyDown(KEY_LEFT_CONTROL);

		G.frame++;
		FD.clear();
		FD.mousePosWorld  = GetScreenToWorld2D(GetMousePosition(), G.camera);
		FD.mousePosWindow = GetMousePosition();

		if (!DIALOGBOX.input(input.getAction()) && !editor)
		{
			FD.progressLogic =
				dude.input(input.getHeading(eDude.fPos + v2f(G.tileSize / 2.f, G.tileSize / 2.f)),
						   input.getHit(),
						   input.getMove(),
						   IsKeyPressed(KEY_X),
						   input.getAction(),
						   input.getSwap(),
						   canClimb(eDude.iPos),
						   canGoDown(eDude.iPos));
			dude.update(0.016f);
			ENTITIES.updateAll(0.016f);
			dude.interact();
			updateLevels(MAP.level);
			dude.move();
			EFFECTS.updateAll(0.016f);

			G.camera.target = follow(eDude.fPos, v2f(G.camera.target), 0.2f).toVector2();
			l.recomputeLights();
		}
		NARRATIVE.update();
		if (editor)
		{
			for (int i = 0; i < 10; i++)
				if (IsKeyPressed(KEY_ZERO + i))
					editorTileType = i;
			if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
			{
				v3i mousePosTile = toV3i(FD.mousePosWorld / G.tileSize);
				if (MAP.level[FD.dudePos.z].containsXY(mousePosTile))
					MAP.level->getTileAt(mousePosTile)
						.placeTile((MapLayer::Tile::Type)editorTileType, true);
			}
		}

		BeginDrawing();
		{
			ClearBackground(SKYBLUE);
			BeginMode2D(G.camera);
			{
				drawLevel(MAP.level, eDude.iPos.z - 2, eDude.iPos.z);
				ENTITIES.drawAll();
				dude.draw();
				EFFECTS.drawAll();
			}
			EndMode2D();
		}
		l.drawLight({1320, 720}, editor);
		DrawText(
			TextFormat("%d, %d, %d", eDude.iPos.x, eDude.iPos.y, eDude.iPos.z), 10, 10, 20, YELLOW);
		DrawText(TextFormat("%d, %d, %d", FD.dudeAimTile.x, FD.dudeAimTile.y, FD.dudeAimTile.z),
				 10,
				 25,
				 20,
				 YELLOW);
		dude.drawOverlay();
		if (editor)
		{
			DrawText("EDITOR", 10, 10, 60, YELLOW);
			DrawTexturePro(C.textures[C.TEX_TILESET_TERRAIN],
						   {(f32)editorTileType * G.tileSize, 0, G.tileSize, G.tileSize},
						   {FD.mousePosWindow.x, FD.mousePosWindow.y, 64, 64},
						   {},
						   0.f,
						   WHITE);
		}
		DIALOGBOX.draw();

		EndDrawing();
	}
	return 0;
}
