// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "stdafx.h"
#include "Common.h"
#include "tp_stub.h"

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		Init();
		break;
	case DLL_PROCESS_DETACH:
		UnInit();
		break;
	}
	return TRUE;
}

