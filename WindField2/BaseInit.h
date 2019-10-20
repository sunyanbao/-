#pragma once

class BaseInit
{
public:
	BaseInit();
	~BaseInit();
protected:
	CKernelInterface* kernel_interface_;
	CDEA_Framework* framework_;
	CString resource_path_;
};
const int c_priority_count = 16;