
// Harmonia启动配置.h : PROJECT_NAME 应用程序的主头文件
//

#pragma once

#ifndef __AFXWIN_H__
	#error "在包含此文件之前包含“stdafx.h”以生成 PCH 文件"
#endif

#include "resource.h"		// 主符号


// CHarmonia启动配置App: 
// 有关此类的实现，请参阅 Harmonia启动配置.cpp
//

class CHarmonia启动配置App : public CWinApp
{
public:
	CHarmonia启动配置App();

// 重写
public:
	virtual BOOL InitInstance();

// 实现

	DECLARE_MESSAGE_MAP()
};

extern CHarmonia启动配置App theApp;