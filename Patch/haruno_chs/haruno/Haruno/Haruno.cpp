// Haruno.cpp : ����Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "Haruno.h"
#include "Application.h"
#include <string>
#include "Extend.h"
//#include "Movie2.h"

using std::wstring;

#define MAX_LOADSTRING 100

// ȫ�ֱ���: 
HINSTANCE hInst;								// ��ǰʵ��
TCHAR szTitle[MAX_LOADSTRING];					// �������ı�
TCHAR szWindowClass[MAX_LOADSTRING];			// ����������

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
				throw L"�޷���������Ӧ�ó���";
			}
			app->runApplication();
		}
		else
		{
			throw L"Ӧ�ó��򴴽�ʧ��";
		}
	}
	catch (wstring& e)
	{
		MessageBoxW(NULL, (L"�����Ĵ���" + e).c_str(), L"Maisakura min core", MB_OK);
		return -1;
	}
	catch (std::bad_alloc&)
	{
		MessageBoxW(NULL, wstring(L"�����Ĵ����ڴ�ռ䲻��").c_str(), L"Maisakura min core", MB_OK);
		return -1; 
	}
	catch (...)
	{
		MessageBoxW(NULL, wstring(L"δ֪����������").c_str(), L"Maisakura min core", MB_OK);
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


