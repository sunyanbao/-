#include "stdafx.h"
#include "TileClipper.h"
#include "VisibleTile.h"
#include "WindFieldData.h"
#include "engine_interface/engine_interface.h"
#include "Particle.h"
#include "ParticleRenderer.h"

static const int max_level = 17;
namespace 
{
	void CalcLonLatFromXY(int level, int x, int y, Vector2d& tl, Vector2d& tr, Vector2d& bl,Vector2d& br)
	{
		double total_x = std::pow(2.0, level+1);
		double total_y = std::pow(2.0, level);
		tl.x = 360.0*x / total_x - 180.0;
		tl.y = 180.0*(y + 1) / total_y - 90.0;
		tr.x = 360.0*(x + 1) / total_x - 180.0;
		tr.y = 180.0*(y + 1) / total_y - 90.0;
		bl.x = 360.0*x / total_x - 180.0;
		bl.y = 180.0*y / total_y - 90.0;
		br.x = 360.0*(x + 1) / total_x - 180.0;
		br.y = 180.0*y / total_y - 90.0;
	}

	bool IsPointInTile(const Vector2d& lonlat,const CDEA_GlobeTile& tile)
	{
		Vector2d tl,tr,bl,br;
		CalcLonLatFromXY(tile.m_level,tile.m_x,tile.m_y,tl,tr,bl,br);
		double cross_tl = Vector2Cross(lonlat-tl,lonlat-tr);
		double cross_tr = Vector2Cross(lonlat-tr,lonlat-br);
		double cross_br = Vector2Cross(lonlat-br,lonlat-bl);
		double cross_bl = Vector2Cross(lonlat-bl,lonlat-tl);
		return (cross_tl>0&&cross_tr>0&&cross_bl>0&&cross_br>0)
			||(cross_tl<0&&cross_tr<0&&cross_bl<0&&cross_br<0);
	}
}

////////////GlobalRoadObject/////////////////////////////////////////
DEEPEYE_IMPLEMENT_TYPE_INFO(TileClipper2,VisibleObject);
TileClipper2::TileClipper2()
	:resolution_(1)
	,render_view_idx_(0)
{
//	reader.ReadData("d:/test/output.bin",wind_field_data_);
	is_recalculate_ = false;
	//framework_->m_temporalObjectManager->AddTemporalObject(this);
	framework_->m_earthNode->AddMovableObject(this);
	particle_render_mgr_ = new ParticleRendererManager;
	is_reset_ = true;
	render_queue_ = 800;
}

TileClipper2::~TileClipper2()
{
	//RemoveTemporalObject();
}

bool TileClipper2::ReadData(const CString& name)
{
	CString path;
	path.Format("%splant\\WindField2\\bin\\%s.bin",resource_path_,name);
	MixedFileSystem::getSingleton().makeSureFileExistAndLatest(path.GetString());
	if (!reader_.ReadData(path,wind_field_data_))
	{
		return false;
	}
	return true;
}

TileClipper2* TileClipper2::GetInstance()
{
	static TileClipper2* s_clipper = NULL;
	if (s_clipper == NULL)
	{
		s_clipper = new TileClipper2;
	}
	return s_clipper;
}

void TileClipper2::Clip(RenderParameter& param, int intersection)
{
	if (!is_reset_)
	{
		return;
	}
	if (!RenderUsageIsScene(param.usage))
	{
		return;
	}
	const GCSTransform& cur_trans = framework_->m_camera->GetLonLatAltNode()->GetTransform();
	bool is_lonlatalt_change = (abs(old_trans_.m_lonLatAlt.m_lon - cur_trans.m_lonLatAlt.m_lon)>0 
		|| abs(old_trans_.m_lonLatAlt.m_lat - cur_trans.m_lonLatAlt.m_lat)>0)
		||abs(old_trans_.m_lonLatAlt.z - cur_trans.m_lonLatAlt.z) > 0;
	bool is_pose_change = abs(old_trans_.m_eularAngle.heading - cur_trans.m_eularAngle.heading)>0 
		|| abs(old_trans_.m_eularAngle.pitch - cur_trans.m_eularAngle.pitch)>0
		|| abs(old_trans_.m_eularAngle.roll - cur_trans.m_eularAngle.roll)>0;
	bool is_camera_change = is_lonlatalt_change || is_pose_change;
	old_trans_ = cur_trans;
	if (is_camera_change)
	{
		is_recalculate_ = true;
		particle_render_mgr_->Reset();
		return;
	}
	else
	{
		RenderQueue* render_queue = framework_->m_renderQueueManager->GetRenderQueue(render_queue_);
		render_queue->AddItem(particle_render_mgr_);
	}
	if (!is_recalculate_)
	{
		return;
	}
	render_view_idx_ = param.renderView->m_index;
	if (render_view_idx_ != 0)
	{
		return;
	}
	VisibleTileContainer& visible_tile_list = visible_labeling_tiles_[render_view_idx_];
	tile_provider_.FreeUsedTiles(visible_tile_list);
	for(int row = 0; row < 2; ++row)
	{
		for(int col = 3; col >= 0; --col)
		{
			CDEA_GlobeTile tile(1, col, row,1);
			ClipRecursive(param, tile, intersection);
		}
	}

	AxisAlignedBox3d box;
	box.SetEmpty();
	for (auto it = visible_tile_list.begin();it!=visible_tile_list.end();++it)
	{
		VisibleTile* label_tile = *it;
		const CDEA_GlobeTile& tile = label_tile->GetTile();
		Vector2d tl,tr,bl,br;
		CalcLonLatFromXY(tile.m_level,tile.m_x,tile.m_y,tl,tr,bl,br);
		box.Merge(Vector3d(tl.x,tl.y,0));
		box.Merge(Vector3d(tr.x,tr.y,0));
		box.Merge(Vector3d(bl.x,bl.y,0));
		box.Merge(Vector3d(br.x,br.y,0));
	}
	particle_render_mgr_->SetBox(box);
	particle_render_mgr_->GenerateParticleRender(&wind_field_data_);
	is_recalculate_ = false;
}

void TileClipper2::CalcBoundingBox()
{
	m_box.SetInfinite();
}

void TileClipper2::ClipRecursive(RenderParameter& param,const CDEA_GlobeTile& tile, int intersection)
{
	assert(0 < tile.m_level);
	LonLatAltBox llab;
	tile.GetLonLatRect(llab);
	llab.m_minAlt = 0;
	llab.m_maxAlt = 2;

	OrientedBox3f msBox;
	LonLatAltBoxToOBB(msBox, llab, deep_eye_earth_radius);
	Vector3f sceneOrigin = param.sceneManager->GetSceneOrigin();
	msBox.m_center -= sceneOrigin;

	if(frustum_intersection_intersecting == intersection)
	{
		intersection = param.worldSpaceViewFrustum.IntersectOBB(msBox);
		if(frustum_intersection_outside == intersection)
		{
			return;
		}
	}

	float slope = 1.0f;
	if(tile.m_level >= 3)
	{
		//接近水平视角，远处的块数量可以适当降低
		float dot = fabs(Vector3Dot(param.GetCameraDirection(), msBox.m_vAxis));
		slope = (dot + 0.05f)/1.05f;
	}
	float minDistanceSquare = GetMinDistanceSquare(msBox, param.GetWorldSpaceCameraPos());
	Vector2f diagonal(msBox.m_extent.x, msBox.m_extent.z);
	float criticalDistanceSquare = Vector2LengthSquare(diagonal) * 4 * slope*resolution_;//1==resolution
	if(max_level == tile.m_level || 
		(criticalDistanceSquare < minDistanceSquare ))
	{
		PresentRenderTile(param, tile);
		return;
	}
	CDEA_GlobeTile west_north_child;
	west_north_child.m_level = tile.m_level + 1;
	west_north_child.m_xlevel = west_north_child.m_level;
	west_north_child.m_x = tile.m_x * 2;
	west_north_child.m_y = tile.m_y * 2 + 1;
	ClipRecursive(param,west_north_child,intersection);

	CDEA_GlobeTile east_north_child;
	east_north_child.m_level = tile.m_level + 1;
	east_north_child.m_xlevel = east_north_child.m_level;
	east_north_child.m_x = tile.m_x * 2 + 1;
	east_north_child.m_y = tile.m_y * 2 + 1;
	ClipRecursive(param,east_north_child,intersection);

	CDEA_GlobeTile west_south_child;
	west_south_child.m_level = tile.m_level + 1;
	west_south_child.m_xlevel = west_south_child.m_level;
	west_south_child.m_x = tile.m_x * 2;
	west_south_child.m_y = tile.m_y * 2;
	ClipRecursive(param,west_south_child,intersection);

	CDEA_GlobeTile east_south_child;
	east_south_child.m_level = tile.m_level + 1;
	east_south_child.m_xlevel = east_south_child.m_level;
	east_south_child.m_x = tile.m_x * 2 + 1;
	east_south_child.m_y = tile.m_y * 2;
	ClipRecursive(param,east_south_child,intersection);
}

void TileClipper2::PresentRenderTile(RenderParameter& param,const CDEA_GlobeTile& tile)
{
	Uint32 render_view_index = GetRenderViewIndex(param.usage);
	VisibleTileContainer& visible_tile_list = visible_labeling_tiles_[render_view_index];
	VisibleTile* visible_tile = tile_provider_.GetTile(tile,param.frame);
	assert(visible_tile);
	visible_tile_list.push_back(visible_tile);
}

int TileClipper2::FindMinRowNum() const
{
	const VisibleTileContainer& visible_tile_list = visible_labeling_tiles_[render_view_idx_];
	const WindFieldData& wfd = wind_field_data_;
	int min_row_num = 0;
	for (;min_row_num<wfd.row_num_;++min_row_num)
	{
		Vector2d lonlat;
		lonlat.y = wfd.start_lla_.y + min_row_num*wfd.lat_step_;
		for (int i = 0;i!=wfd.col_num_;++i)
		{
			lonlat.x = wfd.start_lla_.x + i*wfd.lon_step_;
			for (auto it = visible_tile_list.begin();it!=visible_tile_list.end();++it)
			{
				VisibleTile* label_tile = *it;
				const CDEA_GlobeTile& tile = label_tile->GetTile();
				if (IsPointInTile(lonlat,tile))
				{
					return min_row_num;
				}
			}
		}
	}
	return -1;
}

int TileClipper2::FindMaxRowNum() const
{
	const VisibleTileContainer& visible_tile_list = visible_labeling_tiles_[render_view_idx_];
	const WindFieldData& wfd = wind_field_data_;
	int max_row_num = wfd.row_num_-2;
	for (;max_row_num > 0;--max_row_num)
	{
		Vector2d lonlat;
		lonlat.y = wfd.start_lla_.y + max_row_num*wfd.lat_step_;
		for (int i = 0;i!=wfd.col_num_;++i)
		{
			lonlat.x = wfd.start_lla_.x + i*wfd.lon_step_;
			for (auto it = visible_tile_list.begin();it!=visible_tile_list.end();++it)
			{
				VisibleTile* label_tile = *it;
				const CDEA_GlobeTile& tile = label_tile->GetTile();
				if (IsPointInTile(lonlat,tile))
				{
					return max_row_num;
				}
			}
		}
	}
	return -1;
}

int TileClipper2::FindMinColNum() const
{
	const VisibleTileContainer& visible_tile_list = visible_labeling_tiles_[render_view_idx_];
	const WindFieldData& wfd = wind_field_data_;
	int min_col_num = 0;
	for (;min_col_num<wfd.col_num_;++min_col_num)
	{
		Vector2d lonlat;
		lonlat.x = wfd.start_lla_.x + min_col_num*wfd.lon_step_;
		for (int i = 0;i!=wfd.row_num_;++i)
		{
			lonlat.y = wfd.start_lla_.y + i*wfd.lat_step_;
			for (auto it = visible_tile_list.begin();it!=visible_tile_list.end();++it)
			{
				VisibleTile* label_tile = *it;
				const CDEA_GlobeTile& tile = label_tile->GetTile();
				if (IsPointInTile(lonlat,tile))
				{
					return min_col_num;
				}
			}
		}
	}
	return -1;
}

int TileClipper2::FindMaxColNum() const
{
	const VisibleTileContainer& visible_tile_list = visible_labeling_tiles_[render_view_idx_];
	const WindFieldData& wfd = wind_field_data_;
	int max_col_num = wfd.col_num_-2;
	for (;max_col_num > 0;--max_col_num)
	{
		Vector2d lonlat;
		lonlat.x = wfd.start_lla_.x + max_col_num*wfd.lon_step_;
		for (int i = 0;i!=wfd.row_num_;++i)
		{
			lonlat.y = wfd.start_lla_.y + i*wfd.lat_step_;
			for (auto it = visible_tile_list.begin();it!=visible_tile_list.end();++it)
			{
				VisibleTile* label_tile = *it;
				const CDEA_GlobeTile& tile = label_tile->GetTile();
				if (IsPointInTile(lonlat,tile))
				{
					return max_col_num;
				}
			}
		}
	}
	return -1;
}

Vector3d TileClipper2::CalcNextPos(const Vector3d& cur_pos,Uint32 interval) const
{
	Vector3d next_pos = Vector3d::s_zero;
	const WindFieldData& wfd = wind_field_data_;
	double x_num = (cur_pos.x - wfd.start_lla_.x)/wfd.lon_step_;
	//assert(x_num >= 0 && x_num < wfd.col_num_);
	double y_num = (cur_pos.y - wfd.start_lla_.y)/wfd.lat_step_;
	//assert(y_num >= 0 && y_num < wfd.row_num_);
	int x_num1 = (int)x_num;
	int x_num2 = (int)x_num+1;
	int y_num1 = (int)y_num;
	int y_num2 = (int)y_num+1;
	int index11 = y_num1*wfd.row_num_ + x_num1;
	int index12 = y_num2*wfd.row_num_ + x_num1;
	int index21 = y_num1*wfd.row_num_ + x_num2;
	int index22 = y_num2*wfd.row_num_ + x_num2;
	if (index12 >= wfd.wind_info_list_.size() || index21 > wfd.wind_info_list_.size())
	{
		//assert(0);
		return Vector3d::s_zero;
	}
	float x_ratio = (x_num-x_num1)/(x_num2-x_num1);
	float y_ratio = (y_num-y_num1)/(y_num2-y_num1);
	float angle11 = wfd.wind_info_list_[index11].y;
	float angle12 = wfd.wind_info_list_[index12].y;
	float angle21 = wfd.wind_info_list_[index21].y;
	float angle22 = wfd.wind_info_list_[index22].y;
	float angle1 = angle11 + (angle21-angle11)*x_ratio;
	float angle2 = angle12 + (angle22-angle12)*x_ratio;
	float angle = angle1 + (angle2-angle1)*y_ratio;


	double rad = (double)angle*deep_eye_degree_to_radian;
	Vector3d dir = Vector3d(cos(rad),sin(rad),0);
	next_pos = cur_pos + dir*0.5*30/1000.0*1e-3;
	//double next_x_num = (next_pos.x - wfd.start_lla_.x)/wfd.lon_step_;
	//double next_y_num = (next_pos.y - wfd.start_lla_.y)/wfd.lat_step_;
	//assert(abs(next_x_num-x_num) <= 1.01 && abs(next_y_num-y_num)<=1.01);
	return next_pos;
}
