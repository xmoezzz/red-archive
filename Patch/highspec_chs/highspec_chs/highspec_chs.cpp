#include "HighSpecHook.h"

#pragma comment(lib, "libMinHook.x86.lib")

BOOLEAN NTAPI Initialize(HMODULE hModule)
{
	auto Object = HighSpecHook::GetData();
	if (Object)
	{
		return NT_SUCCESS(HighSpecHook::GetData()->InitNativeHook());
	}
	return FALSE;
}

BOOLEAN NTAPI UnInitialize(HMODULE hModule)
{
	return TRUE;
}

BOOL NTAPI DllMain(HMODULE hModule, DWORD Reason, LPVOID lpReserved)
{
	switch (Reason)
	{
	case DLL_PROCESS_ATTACH:
		return  Initialize(hModule) || UnInitialize(hModule);
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		return UnInitialize(hModule);
	}
	return TRUE;
}
