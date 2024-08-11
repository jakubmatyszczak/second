#pragma once
#include "engine/basic.cpp"
#include "engine/v2.cpp"

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

	bool appendVertex(v2 pos)
	{
		if (nTerrainVerticies >= maxTerrainVerts)
			return false;
		terrainVerticies[nTerrainVerticies++] = pos;
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
