#include "entities.cpp"
#include "globals.cpp"

struct Light
{
	RenderTexture2D tex;
	f32				scale;
	Rectangle		source;
	f32				darkness;

	void init(v2f resolution, f32 maskScale)
	{
		darkness = 0.f;
		scale	 = maskScale;
		source	 = {0, 0, resolution.x * scale, resolution.y * scale};
		tex		 = LoadRenderTexture(resolution.x * scale, resolution.y * scale);
	}

	void recomputeLights()
	{
		BeginTextureMode(tex);
		ClearBackground(BLACK);
		if (darkness < 0.4f)
		{
			EndTextureMode();
			return;
		}
		for (u32 i = 0; i < ENTITIES.nMax; i++)
		{
			if (!ENTITIES.active[i] || !ENTITIES.isLightSource[i])
				continue;
			Entity& e		   = ENTITIES.arr[i];
			v2f		drawPos	   = e.fPos + v2f(G.tileSize * 0.5f);
			Vector2 posLocal   = GetWorldToScreen2D(drawPos.toVector2(), G.camera);
			posLocal		   = {posLocal.x * scale, posLocal.y * scale};
			posLocal.y		   = tex.texture.height - posLocal.y;  // dunno why this is needed
			DrawCircleV(posLocal, e.lightRadius * scale, {255, 200, 200, 200});
			DrawCircleV(posLocal, e.lightRadius * scale * 1.75f, {255, 50, 50, 20});
		}
		EndTextureMode();
	}
	void drawLight(v2f targetSize)
	{
		static f32 t = 0;
		t += 0.016f;
		darkness   = (sinf(t*0.1f) * 0.5f + 0.5f);
		u8 darkClr = (u8)(darkness * 220.f);
		BeginBlendMode(BLEND_MULTIPLIED);
		DrawTexturePro(tex.texture,
					   source,
					   {0, 0, targetSize.x, targetSize.y},
					   {},
					   0.f,
					   {darkClr, darkClr, darkClr, darkClr});
		EndBlendMode();
	}
};
