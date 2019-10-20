#include "stdafx.h"
#include "wind_field_module.h"
#include "kernel_service\service\COM_Service.h"
#include "kernel_service\service\COM_Module.h"
#include "kernel_interface\json\json.h"

#include "WindFieldData.h"
#include "TileClipper.h"
#include "engine_interface/engine_interface.h"
#include "engine_interface/space_line.h"
#include "GetData.h"

begin_register(wind_field)
	on_service(test, on_test);
	on_service(reset, on_reset);
	on_service(set_param,on_set_param);
	on_service(refresh_bin_data,on_refresh_bin_data);
	on_service(clear_bin_data,on_clear_bin_data);
end_register

begin_implement(wind_field)
int on_test(CString& strJson)
{
	static int id = 0;
	const WindFieldData& data = TileClipper::GetInstance()->wind_field_data_;
	Json::Value value;
	if(!_JSON_V(strJson,value))
		return 0;
	CString file_name="output";
	_JSON_G(value["file_name"],file_name);
	TileClipper::GetInstance()->ReadData(file_name);
	GetData::GetInstance()->ReadConfigFile();
	return 1;
}

int on_refresh_bin_data(CString& strJson)
{
	Json::Value value;
	if(!_JSON_V(strJson,value))
		return 0;
	if (!value.isMember("url_wv")||!value.isMember("url_wd"))
	{
		return 0;
	}
	Vector3d start_lla=Vector3d::s_zero,end_lla=Vector3d::s_zero;
	_JSON_G(value["start_lon"],start_lla.x);
	_JSON_G(value["start_lat"],start_lla.y);
	_JSON_G(value["end_lon"],end_lla.x);
	_JSON_G(value["end_lat"],end_lla.y);
	CString file_name="output";
	_JSON_G(value["file_name"],file_name);
	if (value["url_wv"].isArray()&&value["url_wd"].isArray()&&value["file_name"].isArray())
	{
		int count_wv = value["url_wv"].size();
		int count_wd = value["url_wd"].size();
		int count_name = value["file_name"].size();
		if (count_wd!=count_wv)
		{
			return 0;
		}
		vector<CString> url_wv,url_wd,file_name;
		file_name.resize(count_wv,"output");
		CString str_temp;
		for (int i=0;i<count_wv;i++)
		{
			if(_JSON_G(value["url_wv"][i],str_temp))
			{
				url_wv.push_back(str_temp);
			}
			if(_JSON_G(value["url_wd"][i],str_temp));
			{
				url_wd.push_back(str_temp);
			}
			if(_JSON_G(value["file_name"][i],str_temp));
			{
				file_name[i]=str_temp;
			}
		}
		bool is_local_data =false;
		_JSON_G(value["is_local_data"],is_local_data);
	//	GetData::GetInstance()->is_local_data_ = is_local_data;
		if (!GetData::GetInstance()->RefreshBinDataThread(url_wv,url_wd,start_lla,end_lla,file_name,is_local_data))
		{
			return 0;
		}
	}
	else
	{
		return 0;
	}
	return 1;
}


int on_reset(CString& strJson)
{
	Json::Value value;
	if(!_JSON_V(strJson,value))
		return 0;
	if(!value.isMember("show"))
		return 0;
	bool bShow = true;
	_JSON_G(value["show"],bShow);
	TileClipper::GetInstance()->is_reset = bShow;
	return 1;
}

int on_set_param(CString& strJson)
{
	Json::Value value;
	if(!_JSON_V(strJson,value))
		return 0;
	int age = 200;
	if(value.isMember("age"))
		_JSON_G(value["age"],age);
	TileClipper::GetInstance()->SetAge(age);
	float speed = 10;
	if(value.isMember("speed"))
		_JSON_G(value["speed"],speed);
	TileClipper::GetInstance()->SetSpeed(speed);
	float particle_width=1,particle_length=1;
	if(value.isMember("particle_width"))
		_JSON_G(value["particle_width"],particle_width);
	if(value.isMember("particle_length"))
		_JSON_G(value["particle_length"],particle_length);
	TileClipper::GetInstance()->SetParticleSize(particle_width,particle_length);
	if(value.isMember("max_patical_count"))
	{
		DWORD count=8000;
		_JSON_G(value["max_patical_count"],count);
		TileClipper::GetInstance()->particle_render_mgr_->SetMaxPaticalCount(count);
	}
	//TileClipper::GetInstance()->particle_render_mgr_->Reset();
	GetData::GetInstance()->ReadConfigFile();
	CString file_name="output";
	_JSON_G(value["file_name"],file_name);
	TileClipper::GetInstance()->ReadData(file_name);
	return 1;
}

int on_clear_bin_data(CString& strJson)
{
	Json::Value value;
	if(!_JSON_V(strJson,value))
		return 0;
	float minute_clear = 10;
	if(value.isMember("minute_clear"))
		_JSON_G(value["minute_clear"],minute_clear);
	if (!GetData::GetInstance()->ClearBinDataThread(minute_clear))
	{
		return 0;
	}
	return 1;
}
end_implement