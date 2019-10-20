#pragma once
#include "BaseInit.h"
#include "DeepEyeAppExt/Common/DEA_GlobeTile.h"

class TileObjectRenderer;
class VisibleTile : private BaseInit
{
public:
	void Init(const CDEA_GlobeTile& tile,Uint32 frame);
	bool IsSameTile(const CDEA_GlobeTile& tile) const;
public:
	const CDEA_GlobeTile& GetTile() const;
private:
	CDEA_GlobeTile tile_;
	//std::vector<TileObjectRenderer*> tile_obj_renderers_;
};
