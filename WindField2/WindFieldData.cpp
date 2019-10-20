#include "stdafx.h"
#include "WindFieldData.h"

bool WindFieldData::CalcWindSpeed(const Vector3d& lla_pos,Vector2f&speed) const
{
	double lon_num = (lla_pos.x-start_lla_.x)/lon_step_;
	if (lon_num < 0 || lon_num >= col_num_-1)
	{
		return false;
	}
	double lat_num = (lla_pos.y-start_lla_.y)/lat_step_;
	if (lat_num < 0 || lat_num >= row_num_-1)
	{
		return false;
	}
	int left = floor(lon_num);
	int right = ceil(lon_num);
	int top = floor(lat_num);
	int bottom = ceil(lat_num);
	double lr_ratio = (lon_num-left)/(right-left);
	double tb_ratio = (lat_num-top)/(bottom-top);
	Vector2f speed_lt = GetSpeedByIndex(left,top);
	Vector2f speed_rt = GetSpeedByIndex(right,top);
	Vector2f speed_lb = GetSpeedByIndex(left,bottom);
	Vector2f speed_rb = GetSpeedByIndex(right,bottom);
	Vector2f speed_top = speed_lt + (speed_rt-speed_lt)*lr_ratio;
	Vector2f speed_bottom = speed_lb + (speed_rb-speed_lb)*lr_ratio;
	speed = speed_top + (speed_bottom-speed_top)*tb_ratio;
	return true;
}

DeepEye::Vector2f WindFieldData::GetSpeedByIndex(int x,int y) const
{
	const Vector2f& wind_info = wind_info_list_[y*col_num_+x];
	float rad = wind_info.y*deep_eye_degree_to_radian;
	Vector2f speed;
	speed.x = sin(rad)*wind_info.x;
	speed.y = cos(rad)*wind_info.x;
	return speed;
}

/////////////////////////////WindFieldDataReader////////////////////////////
bool WindFieldDataReader::ReadData(const CString& file_path,WindFieldData& data)
{
	std::ifstream ifs;
	ifs.open(file_path,std::ios::binary|std::ios::out);
	if (!ifs.good())
	{
		return false;
	}
	ifs.read((char*)&data.start_lla_,sizeof(data.start_lla_));
	ifs.read((char*)&data.col_num_,sizeof(data.col_num_));
	ifs.read((char*)&data.row_num_,sizeof(data.row_num_));
	ifs.read((char*)&data.lon_step_,sizeof(data.lon_step_));
	ifs.read((char*)&data.lat_step_,sizeof(data.lat_step_));
	int count = 0;
	ifs.read((char*)&count,sizeof(count));
	data.wind_info_list_.resize(count);
	ifs.read((char*)&data.wind_info_list_[0],sizeof(Vector2f)*count);
	ifs.close();
	//data.lon_step_ = data.lat_step_ = 0.0005;
	//data.start_lla_ =Vector3d(100,30,0);
	return true;
}


