#pragma once
#include "engine/basic.cpp"
#include "engine/v2.cpp"
#include "entities.cpp"

v2 projectPointOntoLine(const v2& p, const v2& a, const v2& b)
{
	v2	  ab = b - a;					 // Vector from a to b
	v2	  ap = p - a;					 // Vector from a to p
	float t	 = ap.dot(ab) / ab.dot(ab);	 // Projection factor
	if (t > 1. || t < 0.f)
		return a;
	return a + ab * t;	// The projected point on the line
}
struct Level
{
	static const u32 maxTerrainVerts = 32;

	v2	terrainVerticies[maxTerrainVerts] = {};
	u32 nTerrainVerticies				  = 0;

	u32 terrainTexture = 0;
	v2	pos			   = v2();
	i32 pHole[4]	   = {-1, -1, -1, -1};
	u32 nHoles		   = 0;

	void init(v2 position)
	{
		terrainTexture = Content::TEX_LEVEL1;
		pos			   = position;
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
		collisionVector = v2();
		if (nTerrainVerticies < 2)
			return false;
		for (u32 i = 1; i <= nTerrainVerticies; i++)
		{
			v2 proj = projectPointOntoLine(
				c.position,
				terrainVerticies[i - 1],
				((i == nTerrainVerticies) ? terrainVerticies[0] : terrainVerticies[i]));
			if (proj.distToSquared(c.position) < c.radius * c.radius)
			{
				v2 correction = (proj - c.position).norm();
				correction *= c.radius;
				collisionVector += correction - (proj - c.position);
			}
		}
		if (collisionVector.isZero())
			return false;
		return true;
	}

	void draw()
	{
		DrawTextureEx(content.textures[terrainTexture], pos.toVector2(), 0.f, 1.f, WHITE);
		if (!GLOBAL.drawDebugCollision)
			return;
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
}
void LoadLevel1(Level& level)
{
	LoadLayout1(level);
	Table::add(level.pos + v2(30, 60));
	Table::add(level.pos + v2(60, 25));
	Table::add(level.pos + v2(50, 85));
	Table::add(level.pos + v2(90, 90));
	level.pHole[level.nHoles++] = Hole::add(level.pos + v2(50, 50));
}
void LoadLevel2(Level& level)
{
	LoadLayout1(level);
	Key::add(level.pos + v2(20, 50));
	Table::add(level.pos + v2(30, 60));
	level.pHole[level.nHoles++] = Hole::add(level.pos + v2(50, 50));
}
