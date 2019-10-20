#include "stdafx.h"
#include "wind_field_module.h"
#include "kernel_service\service\COM_Service.h"
#include "kernel_service\service\COM_Module.h"
#include "kernel_interface\json\json.h"

#include "WindFieldData.h"
#include "TileClipper.h"
#include "engine_interface/engine_interface.h"
#include "engine_interface/space_line.h"

begin_register(wind_field2)
	on_service(test, on_test);
	on_service(reset, on_reset);
	on_service(set_param, on_set_param);
end_register

begin_implement(wind_field2)
int on_test(CString& strJson)
{
	
	//WindFieldDataReader reader;
	///WindFieldData data;
	//reader.ReadData("d:/test/output.bin",data);
	static int id = 0;
	const WindFieldData& data = TileClipper2::GetInstance()->wind_field_data_;
	/*for (int i = 1;i!=data.row_num_;++i)
	{
		double lat1 = (i-1)*data.lat_step_ + data.start_lla_.y;
		double lat2 = (i)*data.lat_step_ + data.start_lla_.y;
		for (int m = 1;m!=data.col_num_;++m)
		{
			double lon1 = (m-1)*data.lon_step_ + data.start_lla_.x;
			double lon2 = (m)*data.lon_step_ + data.start_lla_.x;
			std::vector<Vector3d> llh_list;
			llh_list.push_back(Vector3d(lon1,lat1,0));
			llh_list.push_back(Vector3d(lon2,lat1,0));
			llh_list.push_back(Vector3d(lon2,lat2,0));
			llh_list.push_back(Vector3d(lon1,lat2,0));
			llh_list.push_back(Vector3d(lon1,lat1,0));
			std::string id_str = std::to_string((_Longlong)id);
			id++;
			engine_interface_rc::space_line::create(id_str.c_str(),&llh_list[0],5,0x60ffff00);
		}
	}*/

	return 1;
}

int on_reset(CString& strJson)
{
	Json::Value value;
	if(!_JSON_V(strJson,value))
		return 0;
	bool is_reset= false;
	_JSON_G(value["is_reset"],is_reset);
	TileClipper2::GetInstance()->is_reset_ = is_reset;

	return 1;
}

int on_set_param(CString& strJson)
{
	Json::Value value;
	if(!_JSON_V(strJson,value))
		return 0;
	int max_partical_count = 15000;
	_JSON_G(value["max_partical_count"],max_partical_count);
	TileClipper2::GetInstance()->particle_render_mgr_->SetMaxParCount(max_partical_count);
	CString file_name="output";
	_JSON_G(value["file_name"],file_name);
	TileClipper2::GetInstance()->ReadData(file_name);
	float ratio =1;
	_JSON_G(value["ratio"],ratio);
	TileClipper2::GetInstance()->particle_render_mgr_->SetRatio(ratio);
	int age = 30;
	_JSON_G(value["age"],age);
	TileClipper2::GetInstance()->particle_render_mgr_->SetAge(age);
	DWORD render_queue = 800;
	_JSON_G(value["render_queue"],render_queue);
	TileClipper2::GetInstance()->render_queue_ = render_queue;
	return 1;
}

end_implement