// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "stdafx.h"
#include "windows.h"
#include "Common.h"
#include "Patch.h"

FARPROC g_pOriFunc;
FARPROC g_pOriFunc_Title;
FARPROC g_pOriFunc_Font;
FARPROC g_pOriFunc_Mess;
FARPROC g_pOriFunc_CFile;

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		//0045C546  |. 68 23C54500    PUSH WhiteEte.0045C523                   ; /pModule = "KERNEL32.dll"
		//0045D0DC >768499BB  粰剉    kernel32.lstrcmpiA
		g_pOriFunc       = GetProcAddress(GetModuleHandleA("KERNEL32.dll"),"lstrcmpiA");
		g_pOriFunc_Title = GetProcAddress(GetModuleHandleA("user32.dll"),"CreateWindowExA");
		g_pOriFunc_Font  = GetProcAddress(GetModuleHandleA("Gdi32.dll"),"GetGlyphOutlineA");
		g_pOriFunc_Mess  = GetProcAddress(GetModuleHandleA("user32.dll"),"MessageBoxA");
		g_pOriFunc_CFile = GetProcAddress(GetModuleHandleA("KERNEL32.dll"),"CreateFileA");

		if(FALSE == StartHook("KERNEL32.dll", g_pOriFunc, (PROC)lstrcmpiEX))
			MessageBoxA(NULL, "Failed to Load xmoe_chs.dll[0x1]","WhiteEternityCHS", MB_OK);
		StartHook("user32.dll", g_pOriFunc_Mess, (PROC)MyMessageBoxA);
		StartHook("user32.dll", g_pOriFunc_Title, (PROC)MyCreateWindowExA);
		StartHook("KERNEL32.dll", g_pOriFunc_CFile, (PROC)MyCreateFileA);

		if(FALSE == InstallPatch())
			MessageBoxA(NULL, "Failed to Launch Application","WhiteEternityCHS", MB_OK);

	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}
