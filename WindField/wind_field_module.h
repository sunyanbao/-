#include "kernel_service\service\COM_ServiceDefine.h"
def_module(wind_field)
	def_service(test,"");
	def_service(reset,"");
	/*�����١����Ӵ�С����������
	{
	"age" : 300,
	"file_name" : "Ruler20190703020000",
	"particle_length" : 1,
	"particle_width" : 2,
	"service_id" : "wind_field.set_param",
	}
	particle_length���ӳ��ȣ�particle_width���ӿ�ȣ�����ͨ�������ļ��ּ����ƣ�file_nameҪ���صĻ����ļ���age������������
	*/
	def_service(set_param,"");
	/*�����������ݣ��Լ��ָ�����
	{
	"end_lat" : 32,
	"end_lon" : 124,
	"service_id" : "wind_field.refresh_bin_data",
	"start_lat" : 28,
	"start_lon" : 121,
	"url_wd" : "http://10.138.176.172/res-service/file/res/ec/u10/Mis4WindD/19051520_018.dat?token=02caae80077c45aba997000ce7eae901",
	"url_wv" : "http://10.138.176.172/res-service/file/res/ec/u10/Mis4WindV/19051520_018.dat?token=02caae80077c45aba997000ce7eae901"
	}
	start_lat��start_lon��end_lon��end_lat���Ʋ��з�Χ��url_wd����url��url_wv����url
	*/
	def_service(refresh_bin_data,"");
	/*
	���������棬������ʱ�䳬��minute_clear�Ļ���,��λ����
	{
	"minute_clear" : 10,
	"service_id" : "wind_field.clear_bin_data"
	}
	*/
	def_service(clear_bin_data,"");
end_module