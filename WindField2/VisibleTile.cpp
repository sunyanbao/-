#include "stdafx.h"
#include "VisibleTile.h"

void VisibleTile::Init(const CDEA_GlobeTile& tile,Uint32 frame)
{
	tile_ = tile;
}

bool VisibleTile::IsSameTile(const CDEA_GlobeTile& tile) const
{
	return tile_ == tile;
}

const CDEA_GlobeTile& VisibleTile::GetTile() const
{
	return tile_;
}
