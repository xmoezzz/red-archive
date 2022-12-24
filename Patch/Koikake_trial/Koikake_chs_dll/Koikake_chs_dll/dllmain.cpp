#pragma comment(linker, "/SECTION:.X'moe,ERW /MERGE:.text=.X'moe")

#include "Koikake_chs_dll.h"
#include "FileManager.h"

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(hModule);
		if (FileManager::Create() == NULL)
		{
			MessageBoxW(NULL, L"ÓÎÏ·Æô¶¯Ê§°Ü", L"X'moe-CoreLib", MB_OK);
			ExitProcess(-1);
		}
		if (FileManager::GetFileManager()->Init() == false)
		{
			//ExitProcess(-1);
		}
		InitHook();
		break;

	case DLL_PROCESS_DETACH:
		UnInitHook();
		break;
	}
	return TRUE;
}

