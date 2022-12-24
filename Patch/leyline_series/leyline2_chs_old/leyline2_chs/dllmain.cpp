#include "Common.h"
#include "FileManager.h"

BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
	)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	{
		DisableThreadLibraryCalls(hModule);
		if (FileManager::Create() == nullptr)
		{
			MessageBoxW(GetForegroundWindow(), L"初始化失败-内部错误", L"X'moe-CoreLib", MB_OK);
			ExitProcess(-1);
		}
		FileManager::GetFileManager()->Init();
		Init(hModule);
	}
	break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		UnInit(hModule);
		break;
	}
	return TRUE;
}

