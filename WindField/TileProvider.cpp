#include "stdafx.h"
#include "TileProvider.h"
#include "VisibleTile.h"

TileProvider::~TileProvider()
{
	for (size_t i = 0;i!=visible_tile_list_.size();++i)
	{
		SafeDeleteSetNull(visible_tile_list_[i]);
	}
	visible_tile_list_.clear();
}

void TileProvider::FreeUsedTiles(std::vector<VisibleTile *>& used_tiles)
{
	visible_tile_list_.insert(visible_tile_list_.end(),used_tiles.begin(),used_tiles.end());
	used_tiles.clear();
}

VisibleTile* TileProvider::GetTile(const CDEA_GlobeTile& tile,Uint32 frame)
{
 	VisibleTile* visible_tile = NULL;
 	auto it = std::find_if(visible_tile_list_.begin(), visible_tile_list_.end(),
 		bind2nd(mem_fun(&VisibleTile::IsSameTile),tile));
 	if (it!=visible_tile_list_.end())
 	{
 		visible_tile = *it;
 		visible_tile_list_.erase(it);
 	}
 	else
 	{
 		if(visible_tile_list_.empty())
 		{
 			visible_tile = new VisibleTile();
 		}
  		else
  		{
  			visible_tile = visible_tile_list_.back();
  			visible_tile_list_.pop_back();
  		}
 	}
 	
 	assert(visible_tile);
 	visible_tile->Init(tile,frame);
 	return visible_tile;
}

