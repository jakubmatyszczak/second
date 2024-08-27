#pragma once
#include "engine/scene.cpp"

struct ParalaxAsset
{
	u32 pAsset = 0;
	u32 pTexture;

	static void add(u32 pTexture, v2 pos, v2 vel = v2(), f32 scale = 1.f, Color clr = WHITE)
	{
		u32 pAsset =
			scene::addAsset(scene::Asset::BG_PARALAX_TERRAIN, scene::Asset::BACKGROUND, pos);
		if (pAsset < 0)
			exitWithMessage("Could not create ParalaxAsset!");
		scene::Asset& asset	  = sceneAssets.instances[pAsset];
		ParalaxAsset& paralax = *(new (asset.data) ParalaxAsset);
		paralax.pAsset		  = pAsset;
		paralax.pTexture	  = pTexture;
        asset.vel = vel;
        asset.scale = scale;
        asset.color = clr;
	}
};
void updateAndDrawParalaxAsset(void* paralaxData)
{
	ParalaxAsset& paralax = *(ParalaxAsset*)paralaxData;
	scene::Asset& a		  = sceneAssets.instances[paralax.pAsset];
    // update
    a.pos += a.vel;
    
    // draw
    v2 drawPos = a.pos + (v2)GLOBAL.camera.target / GLOBAL.camera.zoom;
	DrawTextureEx(CONTENT.textures[paralax.pTexture], drawPos.toVector2(), 0.f, a.scale, a.color);
}
