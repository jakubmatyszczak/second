#pragma once
#include "engine/basic.cpp"
#include "engine/v2.cpp"
#include "entities.cpp"

struct Level
{
	static const u32 maxTerrainVerts = 32;
	BoundingCircle	 bc;

	v2	terrainVerticies[maxTerrainVerts] = {};
	u32 nTerrainVerticies				  = 0;

	u32	 terrainTexture = 0;
	v2	 pos			= v2();
	i32	 pHole[4]		= {-1, -1, -1, -1};
	u32	 nHoles			= 0;
	bool active			= false;

	bool dbgCollidedThisFrame = false;

	void init(v2 position, u32 pTexTerrain)
	{
		terrainTexture = pTexTerrain;
		pos			   = position;
		bc			   = {position + v2(64, 64), 90};  // 90 = sqrt[2]*64 - square png diagonal
	}

	bool appendVertexFromMouse(v2 mousePos)
	{
		if (nTerrainVerticies >= maxTerrainVerts)
		{
			printf("Too many verticies!\n");
			return false;
		}
		terrainVerticies[nTerrainVerticies++] = mousePos;

		v2 dv = mousePos - this->pos;
		printf("v2(%.0f, %0.f)\n", dv.x, dv.y);
		return true;
	}
	bool appendVertex(v2 pos)
	{
		if (nTerrainVerticies >= maxTerrainVerts)
			return false;
		terrainVerticies[nTerrainVerticies++] = this->pos + pos;
		return true;
	}

	bool collidesWithTerrainBorder(BoundingCircle& c, v2& collisionVector)
	{
		dbgCollidedThisFrame = false;
		if (!active)
			return false;
		if (nTerrainVerticies < 2)
			return false;
		if (!bc.computeCollision(c, collisionVector))  // We dont care about result here
		{
			dbgCollidedThisFrame = true;
			return false;
		}
		collisionVector = v2();
		for (u32 i = 1; i <= nTerrainVerticies; i++)
		{
			v2 proj = math::projectPointOntoLine(
				c.pos,
				terrainVerticies[i - 1],
				((i == nTerrainVerticies) ? terrainVerticies[0] : terrainVerticies[i]));
			if (proj.distToSquared(c.pos) < c.radius * c.radius)
			{
				v2 correction = (proj - c.pos).norm();
				correction *= c.radius;
				collisionVector += correction - (proj - c.pos);
			}
		}
		if (collisionVector.isZero())
			return false;
		return true;
	}

	void draw()
	{
		if (!active)
			return;
		DrawTextureEx(content.textures[terrainTexture], pos.toVector2(), 0.f, 1.f, WHITE);
		if (!GLOBAL.drawDebugCollision)
			return;
		DrawCircleV(bc.pos.toVector2(), bc.radius, dbgCollidedThisFrame ? RED_CLEAR : YELLOW_CLEAR);
		if (nTerrainVerticies < 2)
			return;
		for (u32 i = 0; i < nTerrainVerticies; i++)
			DrawCircleV(terrainVerticies[i].toVector2(), 4, RED);
		for (u32 i = 1; i < nTerrainVerticies; i++)
			DrawLineV(terrainVerticies[i - 1].toVector2(), terrainVerticies[i].toVector2(), GREEN);
		DrawLineV(terrainVerticies[nTerrainVerticies - 1].toVector2(),
				  terrainVerticies[0].toVector2(),
				  GREEN);
	}
};

void LoadLayout1(Level& level)
{
	level.appendVertex(v2(64, 105));
	level.appendVertex(v2(107, 106));
	level.appendVertex(v2(120, 97));
	level.appendVertex(v2(120, 76));
	level.appendVertex(v2(118, 55));
	level.appendVertex(v2(101, 45));
	level.appendVertex(v2(93, 34));
	level.appendVertex(v2(110, 32));
	level.appendVertex(v2(118, 21));
	level.appendVertex(v2(113, 11));
	level.appendVertex(v2(97, 6));
	level.appendVertex(v2(69, 15));
	level.appendVertex(v2(52, 20));
	level.appendVertex(v2(20, 19));
	level.appendVertex(v2(13, 38));
	level.appendVertex(v2(11, 58));
	level.appendVertex(v2(8, 84));
	level.appendVertex(v2(15, 97));
	level.appendVertex(v2(40, 99));
	level.appendVertex(v2(50, 95));
	level.active = true;
}
void LoadLayout2(Level& level)
{
	level.appendVertex(v2(39, 48));
	level.appendVertex(v2(29, 63));
	level.appendVertex(v2(17, 66));
	level.appendVertex(v2(4, 83));
	level.appendVertex(v2(7, 103));
	level.appendVertex(v2(18, 121));
	level.appendVertex(v2(35, 126));
	level.appendVertex(v2(67, 126));
	level.appendVertex(v2(94, 122));
	level.appendVertex(v2(109, 120));
	level.appendVertex(v2(119, 109));
	level.appendVertex(v2(112, 89));
	level.appendVertex(v2(101, 81));
	level.appendVertex(v2(84, 85));
	level.appendVertex(v2(75, 97));
	level.appendVertex(v2(61, 92));
	level.appendVertex(v2(64, 78));
	level.appendVertex(v2(90, 63));
	level.appendVertex(v2(90, 38));
	level.appendVertex(v2(72, 20));
	level.appendVertex(v2(53, 12));
	level.appendVertex(v2(10, 9));
	level.appendVertex(v2(1, 22));
	level.appendVertex(v2(7, 35));
	level.appendVertex(v2(20, 43));
	level.appendVertex(v2(38, 40));
	level.active = true;
}

void LoadLevel1(Level& level)
{
	LoadLayout1(level);
	Table::add(level.pos + v2(30, 60));
	Table::add(level.pos + v2(60, 25));
	Table::add(level.pos + v2(50, 85));
	Table::add(level.pos + v2(90, 90));
	Baddie::add(level.pos + v2(75, 75));
	level.pHole[level.nHoles++] = Hole::add(level.pos + v2(100, 50));
	level.pHole[level.nHoles++] = Hole::add(level.pos + v2(50, 60));
}
void LoadLevel2(Level& level)
{
	LoadLayout1(level);
	Key::add(level.pos + v2(20, 50));
	Table::add(level.pos + v2(30, 60));
	level.pHole[level.nHoles++] = Hole::add(level.pos + v2(50, 50));
}
void LoadLevel3(Level& level)
{
	LoadLayout2(level);
	Table::add(level.pos + v2(50, 60));
	Table::add(level.pos + v2(60, 30));
	Gateway::add(level.pos + v2(30, 50), level.pos + v2(90, 50));
	level.pHole[level.nHoles++] = Hole::add(level.pos + v2(25, 20));
}
