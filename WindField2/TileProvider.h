#pragma once

class VisibleTile;
class TileProvider
{
public:
	~TileProvider();
	void FreeUsedTiles(std::vector<VisibleTile*>& used_tiles);
	VisibleTile* GetTile(const CDEA_GlobeTile& tile,Uint32 frame);
private:
	std::vector<VisibleTile*> visible_tile_list_;
};
