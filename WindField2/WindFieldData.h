#pragma once

struct WindFieldData
{
	Vector3d start_lla_;
	double lon_step_;
	double lat_step_;
	int row_num_;
	int col_num_;
	std::vector<Vector2f> wind_info_list_;//x:风速；y：风向
	bool CalcWindSpeed(const Vector3d& lla_pos,Vector2f&speed) const;

	Vector2f GetSpeedByIndex(int x,int y) const;

};

class WindFieldDataReader 
{
public:
	bool ReadData(const CString& file_path,WindFieldData& data);
};


