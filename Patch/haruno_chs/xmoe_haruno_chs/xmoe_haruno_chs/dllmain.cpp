// dllmain.cpp : ���� DLL Ӧ�ó������ڵ㡣
#include "stdafx.h"
#include "xmoe_haruno_chs.h"

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		Install(hModule);
	case DLL_THREAD_ATTACH:
		Uni();
		//
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		//DeleteFile(L"data.xm3");
		break;
	}
	return TRUE;
}

