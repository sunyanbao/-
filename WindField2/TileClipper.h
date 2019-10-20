#pragma once
#include "BaseInit.h"
#include "TileProvider.h"
#include "WindFieldData.h"
#include "DeepEyeKernel/Scene/DeepEye_SpatialObject.h"
#include "DeepEyeKernel/Scene/DeepEye_TemporalObject.h"
#include "ParticleRendererManager.h"


class VisibleTile;
struct WindFieldData;
class ParticleRenderer;
class ParticleRendererManager;
class TileClipper2 : public SpatialObject /*, public TemporalObject*/,private BaseInit
{
	typedef std::vector<VisibleTile*> VisibleTileContainer;
public:
	DEEPEYE_DECLARE_TYPE_INFO(TileClipper2);
	TileClipper2();
	~TileClipper2();
public:
	static TileClipper2* GetInstance();
public:
	//virtual void FrameMove(Uint32 frame, Uint32 interval, Uint32 curTime) override;
	virtual void Clip(RenderParameter& renderParameter, int intersection) override;
	virtual void CalcBoundingBox() override;

	bool ReadData(const CString& name);
private:
	void ClipRecursive(RenderParameter& param,const CDEA_GlobeTile& tile, int intersection);
	void PresentRenderTile(RenderParameter& renderParameter, const CDEA_GlobeTile& tile);
	int FindMinRowNum() const;
	int FindMaxRowNum() const;
	int FindMinColNum() const;
	int FindMaxColNum() const;
	Vector3d CalcNextPos(const Vector3d& cur_pos,Uint32 interval) const;
private:
	VisibleTileContainer visible_labeling_tiles_[render_view_scene_count];/*!当前帧能够看到的块的列表<*/
	TileProvider tile_provider_;/*!<VisibleTile的缓存*/
public:
	float resolution_;
	WindFieldData wind_field_data_;
	int render_view_idx_;
	GCSTransform old_trans_;
private:
	bool is_recalculate_;
	WindFieldDataReader reader_;
public:
	//std::vector<ParticleRenderer*> particle_renderer_list_;
	ParticleRendererManager* particle_render_mgr_;
	bool is_reset_;
	DWORD render_queue_;
};
