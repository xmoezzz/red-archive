
// Harmonia��������.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CHarmonia��������App: 
// �йش����ʵ�֣������ Harmonia��������.cpp
//

class CHarmonia��������App : public CWinApp
{
public:
	CHarmonia��������App();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CHarmonia��������App theApp;