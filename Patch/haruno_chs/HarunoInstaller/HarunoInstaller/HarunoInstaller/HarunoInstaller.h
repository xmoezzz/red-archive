
// HarunoInstaller.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CHarunoInstallerApp: 
// �йش����ʵ�֣������ HarunoInstaller.cpp
//

class CHarunoInstallerApp : public CWinApp
{
public:
	CHarunoInstallerApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CHarunoInstallerApp theApp;