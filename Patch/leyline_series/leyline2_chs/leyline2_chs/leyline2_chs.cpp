#include <Windows.h>
#include "my.h"
#include "SenaHook.h"

#pragma comment(lib, "Psapi.lib")

NTSTATUS NTAPI Initialization(HMODULE hModule)
{
	//AllocConsole();
	SenaHook::GetSenaHook()->SetSelfModule(hModule);
	SenaHook::GetSenaHook()->Init();
	return STATUS_SUCCESS;
}

NTSTATUS NTAPI UnInitialization(HMODULE hModule)
{
	UNREFERENCED_PARAMETER(hModule);
	SenaHook::GetSenaHook()->UnInit();
	return STATUS_SUCCESS;
}

MY_DLL_EXPORT PWCHAR WINAPI XmoeLinkProc()
{
	return L"X'moe Sofpal Universal Patch V2.0";
}


BOOL NTAPI DllMain(HMODULE hModule, DWORD Reason, LPVOID lpReserved)
{
	switch (Reason)
	{
	case DLL_PROCESS_ATTACH:
		return IsStatusSuccess(Initialization(hModule)) ? TRUE : FALSE;

	case DLL_PROCESS_DETACH:
		return IsStatusSuccess(UnInitialization(hModule)) ? TRUE : FALSE;
	}
	return TRUE;
}
