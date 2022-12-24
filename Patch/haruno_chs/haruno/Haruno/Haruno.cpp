// Haruno.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include "Haruno.h"
#include "Application.h"
#include <string>
#include "Extend.h"
//#include "Movie2.h"

using std::wstring;

#define MAX_LOADSTRING 100

// 全局变量: 
HINSTANCE hInst;								// 当前实例
TCHAR szTitle[MAX_LOADSTRING];					// 标题栏文本
TCHAR szWindowClass[MAX_LOADSTRING];			// 主窗口类名

static char *PlayList3[] =
{
	"mov\\fycho.mp4",
	"mov\\kaedeka.mp4",
	"mov\\zweib.mp4",
	"mov\\gr.mp4",
	"mov\\sammy.mp4",
	"mov\\Biscuits.mp4",
	NULL
};

int GetLength3()
{
	int ret = 0;
	while (PlayList3[ret])
	{
		ret++;
	}
	return ret;
}



int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{

	/*
	Application* app = NULL;
	try
	{
		app = new Application();
		if (app)
		{
			if (!app->initApplication())
			{
				throw L"无法正常启动应用程序";
			}
			app->runApplication();
		}
		else
		{
			throw L"应用程序创建失败";
		}
	}
	catch (wstring& e)
	{
		MessageBoxW(NULL, (L"致命的错误：" + e).c_str(), L"Maisakura min core", MB_OK);
		return -1;
	}
	catch (std::bad_alloc&)
	{
		MessageBoxW(NULL, wstring(L"致命的错误：内存空间不足").c_str(), L"Maisakura min core", MB_OK);
		return -1; 
	}
	catch (...)
	{
		MessageBoxW(NULL, wstring(L"未知的致命错误").c_str(), L"Maisakura min core", MB_OK);
		return -1;
	}
	*/

	StartMovie();
	
	/*
	srand((int)time(0));
	unsigned int seed = (unsigned int)rand();

	seed %= (GetLength3() - 1);
	if (PlayList3[seed])
	{
		return PlayMovie2(PlayList3[seed]);
	}
	else
		return 0;
	*/
	return 0;
}


