#pragma once
#include "kernel_service/tool/com_utility.h"
#include "BaseInit.h"
#include "WindFieldData.h"

class GetData:private BaseInit
{
public:
	GetData();
	~GetData();
public:
	static GetData* GetInstance();
public:
	bool RefreshBinDataThread(vector<CString>& url_wv,vector<CString>& url_wd,Vector3d& start_lla,Vector3d& end_lla,vector<CString> &name,bool is_local_data);
	static DWORD RefreshBinData( LPVOID lpParam );//从网络读数据，存到bin
	bool ClearBinDataThread(float minute);
	static DWORD ClearBinData( LPVOID lpParam );
	void PutFilesIntoContainer(const CString& path, vector<CString>& container);

	//读取配置文件
	bool ReadConfigFile();
public:
	vector<CString> url_wv_;
	vector<CString> url_wd_;
	vector<CString> name_;
	vector<CString> bin_vec_;
	Vector3d start_lla_;
	Vector3d end_lla_;
	HANDLE thread_refresh_;
	HANDLE thread_clear_;
	WindFieldDataReader* reader_;
	float minute_clear_;
	map<float,float> speed_ratio_map_;
	bool is_local_data_;
};