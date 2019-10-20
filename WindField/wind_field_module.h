#include "kernel_service\service\COM_ServiceDefine.h"
def_module(wind_field)
	def_service(test,"");
	def_service(reset,"");
	/*调风速、粒子大小、生命周期
	{
	"age" : 300,
	"file_name" : "Ruler20190703020000",
	"particle_length" : 1,
	"particle_width" : 2,
	"service_id" : "wind_field.set_param",
	}
	particle_length粒子长度，particle_width粒子宽度，风速通过配置文件分级控制，file_name要加载的缓存文件，age粒子生命周期
	*/
	def_service(set_param,"");
	/*缓存网络数据，以及分割区块
	{
	"end_lat" : 32,
	"end_lon" : 124,
	"service_id" : "wind_field.refresh_bin_data",
	"start_lat" : 28,
	"start_lon" : 121,
	"url_wd" : "http://10.138.176.172/res-service/file/res/ec/u10/Mis4WindD/19051520_018.dat?token=02caae80077c45aba997000ce7eae901",
	"url_wv" : "http://10.138.176.172/res-service/file/res/ec/u10/Mis4WindV/19051520_018.dat?token=02caae80077c45aba997000ce7eae901"
	}
	start_lat，start_lon，end_lon，end_lat控制裁切范围，url_wd风向url，url_wv风力url
	*/
	def_service(refresh_bin_data,"");
	/*
	定期清理缓存，清理创建时间超过minute_clear的缓存,单位分钟
	{
	"minute_clear" : 10,
	"service_id" : "wind_field.clear_bin_data"
	}
	*/
	def_service(clear_bin_data,"");
end_module