#pragma once
#include "v2.cpp"
#include <cstring>

extern void updateAndDrawParalaxAsset(void* data);

namespace scene
{
	struct Asset
	{
		enum ArchId
		{
			BG_PARALAX_TERRAIN,
			// BG_CLOUD,
			// BG_BOT,
			// FG_CLOUD,
			// FG_BIRD,
		};
		enum Layer
		{
			BACKGROUND,
			FOREGROUND
		};
		ArchId arch;
		v2	   pos;
		v2	   vel;
		f32	   scale;
		Color  color;
		u8	   data[1024];
	};
	struct Assets
	{
		static constexpr u32 nMaxElements = 128;

		Asset		 instances[nMaxElements];
		bool		 active[nMaxElements];
		Asset::Layer layer[nMaxElements];

		void drawBackground()
		{
			Asset* bgAssets[nMaxElements] = {};
			u32	   nBgAssets			  = 0;
			for (u32 i = 0; i < nMaxElements; i++)
				if (active[i] && layer[i] == Asset::BACKGROUND)
					bgAssets[nBgAssets++] = &instances[i];
			for (u32 i = 0; i < nBgAssets; i++)
			{
				switch (bgAssets[i]->arch)
				{
					case Asset::BG_PARALAX_TERRAIN:
						updateAndDrawParalaxAsset(bgAssets[i]->data);
						break;
				}
			}
		}
		void drawForeground()
		{
			Asset* fgAssets[nMaxElements] = {};
			u32	   nfgAssets			  = 0;
			for (u32 i = 0; i < nMaxElements; i++)
				if (active[i] && layer[i] == Asset::FOREGROUND)
					fgAssets[nfgAssets++] = &instances[i];
			for (u32 i = 0; i < nfgAssets; i++)
			{
				switch (fgAssets[i]->arch)
				{
					// case Asset::fg_PARALAX_TERRAIN:
					// 	drawParalaxAsset(fgAssets[i]->data);
					// 	break;
				}
			}
		}
	};
}  // namespace scene
scene::Assets sceneAssets;
namespace scene
{
	i32 addAsset(Asset::ArchId archetype, Asset::Layer layer, v2 pos)
	{
		i32 pAsset = -1;
		for (u32 i = 0; i < sceneAssets.nMaxElements; i++)
			if (!sceneAssets.active[i])
			{
				pAsset = i;
				break;
			}
		if (pAsset < 0)
			return -1;
		Asset& asset = sceneAssets.instances[pAsset];
		memset(&asset, 0, sizeof(asset));
		asset.arch				   = archetype;
		asset.pos				   = pos;
		sceneAssets.active[pAsset] = true;
		sceneAssets.layer[pAsset]  = layer;

		return pAsset;
	}
}  // namespace scene
