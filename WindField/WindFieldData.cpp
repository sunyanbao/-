#include "stdafx.h"
#include "WindFieldData.h"
#include <wininet.h>

#define MAXBLOCKSIZE 40960

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
	ifs.open(file_path,std::ios::binary|std::ios::in);
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
	return true;
}

CString WindFieldDataReader::InternetReadFileFromTM(CString strUrl,bool convert /* = true */)
{
	if (strUrl.IsEmpty())
		return false;
	HINTERNET hSession = InternetOpen("RookIE/1.0", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
	if (hSession == NULL)
		return false;
	HINTERNET handle2 = InternetOpenUrl(hSession, strUrl, NULL, 0, INTERNET_FLAG_DONT_CACHE, 0);

	if (!handle2)
	{
		handle2 = InternetOpenUrl(hSession, strUrl, NULL, 0, INTERNET_FLAG_DONT_CACHE, 0);
		if (!handle2)
		{
			DWORD error = GetLastError();
			char buff[1024] = {0};
			DWORD length = 0;
			InternetGetLastResponseInfo(&error, buff, &length);
			return false;
		}
	}

	char czBuffer[MAXBLOCKSIZE];
	memset(czBuffer, 0, MAXBLOCKSIZE);
	ULONG Number = 1;
	//FILE *stream;
	int iFlag = 0;
	CString str = "";
	while (Number > 0)
	{
		memset(czBuffer, 0, MAXBLOCKSIZE);
		if (!InternetReadFile(handle2, czBuffer, MAXBLOCKSIZE - 1, &Number))
		{
			InternetReadFile(handle2, czBuffer, MAXBLOCKSIZE - 1, &Number);
		}
		if (Number > 0 )
		{
			if (convert)
			{
				if (Number < MAXBLOCKSIZE)
				{
					czBuffer[Number] = '\0';
				}
				std::string temp = utf8_to_gbk(czBuffer);
				str += temp.c_str();
				str.Replace('\n', ' ');
			}
			else
			{
				str += czBuffer;
			}
		}
		str.Replace("\\u201c", "");
		str.Replace("\\u201d", "");
		str.Replace("'", "");
		//fwrite(czBuffer, sizeof(char), Number, stream);
		if (Number != 0)
		{
			iFlag = 1;
		}
	}

	InternetCloseHandle(handle2);
	handle2 = NULL;

	InternetCloseHandle(hSession);
	hSession = NULL;
	return str;
}

std::string WindFieldDataReader::utf8_to_gbk(const char* utf8)
{
	if (!utf8) return "";
	wchar_t* widechar = NULL;
	char* multibyte = NULL;
	int length = 0;
	length = ::MultiByteToWideChar(CP_UTF8, 0, utf8, -1, NULL, 0);
	if (length <= 0) return "";
	widechar = new wchar_t[length];
	::MultiByteToWideChar(CP_UTF8, 0, utf8, -1, widechar, length);
	length = ::WideCharToMultiByte(CP_ACP, 0, widechar, -1, NULL, 0, NULL, NULL);
	if (length <= 0) { delete[] widechar; return ""; }
	multibyte = new char[length];
	::WideCharToMultiByte(CP_ACP, 0, widechar, -1, multibyte, length, NULL, NULL);
	delete[] widechar;
	std::string gbk(multibyte);
	delete[] multibyte;
	return gbk;
}

CString WindFieldDataReader::get_file_data(CString& strFilePath,bool convert)
{
	ifstream in_file;
	in_file.open(strFilePath);
	if(!in_file.is_open())
		return "";
	std::string str(""),strTemp("");
	while(!in_file.eof())
	{
		std::getline(in_file,strTemp);
		if(strTemp=="")
			continue;
		str += strTemp;
	}
	in_file.close();
	if(convert)
	{
		str = utf8_to_gbk(str.c_str());
	}
	return str.c_str();
}

bool WindFieldDataReader::ParceData(WindFieldData& data,CString& url_wv,CString& url_wd,bool is_local_data)
{
	CString strDataWV ,strDataWD;
	if (is_local_data)
	{
		strDataWV = get_file_data(url_wv,1);
		strDataWD = get_file_data(url_wd,1);
	}
	else
	{
		strDataWV= InternetReadFileFromTM(url_wv,1);
		strDataWD = InternetReadFileFromTM(url_wd,1);
	}
	if(strDataWV.IsEmpty()||strDataWD.IsEmpty())
		return false;
	if(strDataWV.Find("diamond") == -1||strDataWD.Find("diamond") == -1)
		return false;
	vector<string> str_data_head,str_data_point_wv,str_data_point_wd;
	str_data_head.clear();
	str_data_point_wv.clear();
	str_data_point_wd.clear();
	istringstream stm1(strDataWV.GetBuffer()),stm2(strDataWD.GetBuffer());
	string strTemp;
	bool reserved = false;
	while (stm1 >> strTemp)
	{
		if (str_data_head.size()<22)
		{
			str_data_head.push_back(strTemp.c_str());
		}
		if(str_data_head.size()==22&&!reserved)
		{
			sscanf(str_data_head[9].c_str(),"%lf",&data.lon_step_);
			sscanf(str_data_head[10].c_str(),"%lf",&data.lat_step_);
			sscanf(str_data_head[11].c_str(),"%lf",&data.start_lla_.x);
			sscanf(str_data_head[13].c_str(),"%lf",&data.start_lla_.y);
			sscanf(str_data_head[15].c_str(),"%d",&data.col_num_);
			sscanf(str_data_head[16].c_str(),"%d",&data.row_num_);
			str_data_point_wv.reserve(data.row_num_*data.col_num_);
			reserved = true;
// 			data.start_lla_.x -= 7;
// 			data.start_lla_.y -= 6;
		}
		else if (str_data_head.size()==22&&reserved)
		{
			str_data_point_wv.push_back(strTemp.c_str());
		}
	}
	str_data_point_wd.reserve(data.row_num_*data.col_num_);
	int i = 0;
	while(stm2 >> strTemp)
	{
		if (i<22)
		{
			i++;
		}
		else
		{
			str_data_point_wd.push_back(strTemp.c_str());
		}
	}
	if (str_data_point_wv.size()==0||str_data_point_wd.size()!=str_data_point_wv.size())
	{
		return false;
	}
	
	Vector2f speed;
	for (int j=0;j<str_data_point_wv.size();j++)
	{
		sscanf(str_data_point_wv[j].c_str(),"%f",&speed.x);
		sscanf(str_data_point_wd[j].c_str(),"%f",&speed.y);
		data.wind_info_list_.push_back(speed);
	}
	return true;
}

bool WindFieldDataReader::WriteData(const CString& file_path,WindFieldData& data,Vector3d& start_lla,Vector3d& end_lla)
{
	cut_range(data,start_lla,end_lla);
	DeleteFile(file_path);
	std::ofstream ofs;
	ofs.open(file_path,std::ios::binary|std::ios::out|std::ios::app);
	ofs.write((char*)&data.start_lla_,sizeof(data.start_lla_));
	ofs.write((char*)&data.col_num_,sizeof(data.col_num_));
	ofs.write((char*)&data.row_num_,sizeof(data.row_num_));
	ofs.write((char*)&data.lon_step_,sizeof(data.lon_step_));
	ofs.write((char*)&data.lat_step_,sizeof(data.lat_step_));
	int count = data.col_num_*data.row_num_;
	ofs.write((char*)&count,sizeof(count));
	ofs.write((char*)&data.wind_info_list_[0],sizeof(Vector2f)*count);
	ofs.close();
	return true;
}

void WindFieldDataReader::cut_range(WindFieldData& data,Vector3d& start_lla,Vector3d& end_lla)
{
	if (start_lla.x<data.start_lla_.x||start_lla.y<data.start_lla_.y)
	{
		return ;
	}
	start_lla.x = start_lla.x>=data.start_lla_.x?start_lla.x:data.start_lla_.x;//·¶Î§ÏÞÖÆ
	start_lla.y = start_lla.y>=data.start_lla_.y?start_lla.y:data.start_lla_.y;
	float lon_end=0,lat_end=0;
	lon_end = data.start_lla_.x+data.lon_step_*(data.col_num_-1);
	lat_end = data.start_lla_.y+data.lat_step_*(data.row_num_-1);
	end_lla.x = end_lla.x<=lon_end?end_lla.x:lon_end;
	end_lla.y = end_lla.y<=lat_end?end_lla.y:lat_end;
	if (end_lla.x<start_lla.x||end_lla.y<start_lla.y)
	{
		return;
	}
	double lon_num = (start_lla.x-data.start_lla_.x)/data.lon_step_;
	if (lon_num < 0 || lon_num > data.col_num_-1)
	{
		return;
	}
	double lat_num = (start_lla.y-data.start_lla_.y)/data.lat_step_;
	if (lat_num < 0 || lat_num > data.row_num_-1)
	{
		return;
	}
	int left = floor(lon_num);
	int top = floor(lat_num);

	lon_num = (end_lla.x-data.start_lla_.x)/data.lon_step_;
	if (lon_num < 0 || lon_num > data.col_num_-1)
	{
		return;
	}
	lat_num = (end_lla.y-data.start_lla_.y)/data.lat_step_;
	if (lat_num < 0 || lat_num > data.row_num_-1)
	{
		return;
	}
	int right = floor(lon_num)+1;
	int bottom = floor(lat_num)+1;
	if (right<=left)
	{
		return;
	}
	if (bottom<=top)
	{
		return;
	}
	data.start_lla_.x = start_lla.x;
	data.start_lla_.y = start_lla.y;
	std::vector<Vector2f> wind_info_list;
	wind_info_list.clear();
	for (int row=top;row<bottom;row++)
	{
		for (int col=left;col<right;col++)
		{
			wind_info_list.push_back(data.wind_info_list_[row*data.col_num_+col]);
		}
	}
	if (wind_info_list.size()==0)
	{
		return;
	}
	data.col_num_ = right - left;
	data.row_num_ = bottom - top;
	data.wind_info_list_ = wind_info_list;
}