#include "stdafx.h"
#include "BaseInit.h"


BaseInit::BaseInit()
{
	kernel_interface_ = CKernelInterface::getSingletonPtr();
	resource_path_ = kernel_interface_->GetResourcePath();
	framework_ = kernel_interface_->m_pFramework;
}

BaseInit::~BaseInit()
{

}
