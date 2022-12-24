// dllmain.cpp : ���� DLL Ӧ�ó������ڵ㡣
#include "stdafx.h"
#include "xmoe_sabbat_chs2.h"
#include "DebugTool.h"
#include "FileSystem.h"
#include "NtProtect.h"
#include "DirectShowWorker.h"
#include "SetInternalError.h"
#include "ImageChecker.h"

#pragma comment(linker, "/MERGE:.text=.xmoe")

BOOL g_LoaderInited = FALSE;

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		//DisableThreadLibraryCalls(hModule);
#ifdef DebugMode
		AllocConsole();
#endif
		//MessageBoxW(NULL, L"Debug", 0, 0);
		if (InitImageCheck() != S_OK)
		{
			MessageBoxW(NULL, L"����ʧ��", L"X'moe-CoreLib", MB_OK);
			ExitProcess(-1);
		}
		SetFilter();
		InitDirectShowFilter();
		FileSystem::InitFileSystem();
		Init();
	}
	else if (ul_reason_for_call == DLL_THREAD_ATTACH)
	{
		ThreadWorker(hModule);
	}
	else if (ul_reason_for_call == DLL_PROCESS_DETACH)
	{
		UnInitDirectShowFilter();
	}
	return TRUE;
}


