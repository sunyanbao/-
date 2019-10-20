#include "stdafx.h"
#include "GetData.h"
#include <io.h>


GetData::GetData()
{
	start_lla_ = end_lla_ = Vector3d::s_zero;
	url_wv_.clear();
	url_wd_.clear();
	bin_vec_.clear();
	name_.clear();
	thread_refresh_ = NULL;
	thread_clear_ = NULL;
	reader_ = new WindFieldDataReader;
	minute_clear_ = 10;
	speed_ratio_map_.clear();
	is_local_data_ = false;
}

GetData::~GetData()
{

}

GetData* GetData::GetInstance()
{
	static GetData* s_get_data = NULL;
	if (s_get_data==NULL)
	{
		s_get_data = new GetData;
	}
	return s_get_data;
}


bool GetData::RefreshBinDataThread(vector<CString>& url_wv,vector<CString>& url_wd,Vector3d& start_lla,Vector3d& end_lla,vector<CString> &name,bool is_local_data)
{
	is_local_data_ = is_local_data;
	url_wv_ = url_wv;
	url_wd_ = url_wd;
	start_lla_ = start_lla;
	end_lla_ = end_lla;
	name_ = name;
	DWORD dwThreadID=0;
	if (thread_refresh_==NULL)
	{
		thread_refresh_ = ::CreateThread(NULL, 0, RefreshBinData, (LPVOID)this, 0, &dwThreadID);
	}
	if(NULL == thread_refresh_)
	{
		DWORD dwErrorCode = GetLastError();
		TRACE("%d",dwErrorCode);
		return false;
	}
	return true;
}

DWORD GetData::RefreshBinData( LPVOID lpParam )
{
	GetData *get_data = (GetData *)lpParam;
	WindFieldData *wind_field_data = new WindFieldData;
	CString path;
	int count = get_data->url_wv_.size();
	for (int i=0;i<count;i++)
	{
		if (!get_data->reader_->ParceData(*wind_field_data,get_data->url_wv_[i],get_data->url_wd_[i],get_data->is_local_data_ ))
			continue;
		path.Format("%splant\\WindField\\bin\\%s.bin",get_data->resource_path_,get_data->name_[i]);
		if (!get_data->reader_->WriteData(path,*wind_field_data,get_data->start_lla_,get_data->end_lla_))
		{
			continue;
		}
	}
	delete wind_field_data;
	wind_field_data = NULL;
	get_data->thread_refresh_ = NULL;
	return 1;
}

bool GetData::ClearBinDataThread(float minute)
{
	minute_clear_ = minute;
	DWORD dwThreadID=0;
	if (thread_clear_==NULL)
	{
		thread_clear_ = ::CreateThread(NULL, 0, ClearBinData, (LPVOID)this, 0, &dwThreadID);
	}
	if(NULL == thread_clear_)
	{
		DWORD dwErrorCode = GetLastError();
		TRACE("%d",dwErrorCode);
		return false;
	}
	return true;
}

DWORD GetData::ClearBinData( LPVOID lpParam )
{
	GetData *get_data = (GetData *)lpParam;
	CString output_path = "";
	output_path.Format("%splant\\WindField\\bin",get_data->resource_path_);
// 	while(1)
// 	{
	get_data->bin_vec_.clear();
	get_data->PutFilesIntoContainer(output_path + "\\", get_data->bin_vec_);
	sort(get_data->bin_vec_.begin(), get_data->bin_vec_.end());
	SYSTEMTIME st;
	GetLocalTime(&st);
	vector<CString>::iterator it = get_data->bin_vec_.begin();
	float min_clear = 1.0;
	min_clear = get_data->minute_clear_;
	DWORD f_year=0,f_month=0,f_day=0,f_hour=0,f_min=0;
	FILETIME ftCreate,ftAccess,ftWrite;
	SYSTEMTIME stUTCCreate,stLocalCreate;
	for(;it!=get_data->bin_vec_.end();it++)
	{
		CString strTemp = *it;
		CString file_path = output_path + "\\" + *it;
		HANDLE hFile;
		hFile = CreateFile(file_path.GetBuffer(),0,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
		if(!GetFileTime(hFile, &ftCreate, &ftAccess, &ftWrite))
			continue;
		FileTimeToSystemTime(&ftCreate,&stUTCCreate);//UTC标准时间，和本地时间差8小时
		SystemTimeToTzSpecificLocalTime(NULL, &stUTCCreate, &stLocalCreate);
		f_year = stLocalCreate.wYear;
		f_month = stLocalCreate.wMonth;
		f_day = stLocalCreate.wDay;
		f_hour = stLocalCreate.wHour;
		f_min = stLocalCreate.wMinute;
		if(f_year!=st.wYear)//不能超过一年
		{
			DeleteFile(file_path);
		}
		float inteval = (st.wMonth-f_month)*30*24*60+(st.wDay-f_day)*24*60+(st.wHour-f_hour)*60+st.wMinute-f_min;
		if(inteval>=min_clear)
		{
			DeleteFile(file_path);
		}
	}
	get_data->thread_clear_ = NULL;
// 		Sleep(1000*60);
// 	}
	return 1;
}

 void GetData::PutFilesIntoContainer(const CString& path, vector<CString>& container)
 {
	 _finddata_t fdt;
	 intptr_t handle;
	 CString search_p = path + "*.*";
	 if ((handle = _findfirst(search_p, &fdt)) != -1)
	 {
		 do 
		 {
			 if (strcmp(fdt.name, ".")!=0 && strcmp(fdt.name, "..")!=0)
			 {
				 CString f_full_path = fdt.name;
				 container.push_back(f_full_path);
			 }
		 } while (_findnext(handle, &fdt) == 0);
		 _findclose(handle);
	 }
 }

 //读取配置文件，分级控制风速
 bool GetData::ReadConfigFile()
 {
	 CString cfgPath;
	 cfgPath.Format("%sPlant\\WindField\\Config.ini", CKernelInterface::getSingletonPtr()->GetResourcePath());
	 int count = 0;
	 char chBuf[1024];
	 memset(chBuf,0,sizeof(chBuf));
	 if(GetPrivateProfileString(_T("WindRatio"),_T("count"),NULL,chBuf,sizeof(chBuf),cfgPath))
	 {
		 count = atoi(chBuf);
	 }
	 else
		 return false;
	 float value=0,ratio=0;
	 CString str_value,str_ratio;
	 speed_ratio_map_.clear();
	 for (int i=1;i<=count;i++)
	 {
		 str_value.Format("value%d",i);
		 str_ratio.Format("ratio%d",i);
		 if(GetPrivateProfileString(_T("WindRatio"),_T(str_value),NULL,chBuf,sizeof(chBuf),cfgPath))
		 {
			 value = atof(chBuf);
		 }
		 else
			 return false;
		 if(GetPrivateProfileString(_T("WindRatio"),_T(str_ratio),NULL,chBuf,sizeof(chBuf),cfgPath))
		 {
			 ratio = atof(chBuf);
		 }
		 else
			 return false;
		 speed_ratio_map_.insert(make_pair(value,ratio));
	 }
	 return true;
 }