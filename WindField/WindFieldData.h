#pragma once

struct WindFieldData
{
	Vector3d start_lla_;
	double lon_step_;
	double lat_step_;
	int row_num_;
	int col_num_;
	std::vector<Vector2f> wind_info_list_;//x:风速；y：风向
	WindFieldData()
	{
		start_lla_ = Vector3d::s_zero;
		lon_step_ = 0;
		lat_step_ = 0;
		row_num_ = 0;
		col_num_ = 0;
		wind_info_list_.clear();
	}
	bool CalcWindSpeed(const Vector3d& lla_pos,Vector2f&speed) const;

	Vector2f GetSpeedByIndex(int x,int y) const;

};

class WindFieldDataReader 
{
public:
	bool ReadData(const CString& file_path,WindFieldData& data);
	bool ParceData(WindFieldData& data,CString& url_wv,CString& url_wd,bool is_local_data);
	bool WriteData(const CString& file_path,WindFieldData& data,Vector3d& start_lla,Vector3d& end_lla);

private:
	CString InternetReadFileFromTM(CString strUrl,bool convert  = true );
	std::string utf8_to_gbk(const char* utf8);
	void cut_range(WindFieldData& data,Vector3d& start_lla,Vector3d& end_lla);
	CString get_file_data(CString& strFilePath,bool convert = false);
};


