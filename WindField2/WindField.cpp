// WindField.cpp : ���� DLL �ĳ�ʼ�����̡�
//

#include "stdafx.h"
#include "WindField.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

WindField* g_interface = NULL;
CDllInterface* CreateInterface()
{
	if ( NULL == g_interface )
	{
		g_interface = new WindField;
	}
	return (CDllInterface*)g_interface;
}

void ReleaseInterface()
{
	if ( g_interface )
		delete g_interface;
	g_interface = NULL;
}

