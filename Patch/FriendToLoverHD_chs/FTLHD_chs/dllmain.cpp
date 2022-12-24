#include "Common.h"
#include "LocaleEmulator.h"
#include "LocaleEmulatorUser.h"
//#include "NtHook.h"

EXTERN_C
{
#include "UCIGraph.h" 
}

//Image Compressor
#pragma comment(lib, "UCIStatic.lib")
#pragma comment(lib, "ntdll.lib")
#pragma comment(lib, "detours.lib")
#pragma comment(lib, "lz4.lib")
#pragma comment(lib, "libavcodec.a")

//extern Settings settings;

BOOL APIENTRY DllMain(HMODULE hModule, DWORD Reason, LPVOID lpReserved)
{
	UNREFERENCED_PARAMETER(lpReserved);
	LPVOID p; 
	HANDLE thread;

	switch (Reason)
	{
	case DLL_PROCESS_ATTACH:
	{
		//AllocConsole();
		//DisableThreadLibraryCalls(hModule);
		UCIInitOrUninit(DLL_PROCESS_ATTACH);
		Init(hModule);
		//BeginLocalEmulator(936);
		//InitUnicodeLayer(hModule, 936, 0x411);
		//LoadLibraryW(L"NtLayer.dll");
	}
	break;

	case DLL_THREAD_DETACH:
		//ReleaseThreadLocalStorage();
		//p = TlsGetValue(settings.nTlsIndex);
		//if (p) FreeStringInternal(p);
		break;


	case DLL_PROCESS_DETACH:
	{
		UCIInitOrUninit(DLL_PROCESS_DETACH);
		UnInit(hModule);
	}
	break;
	}
	return TRUE;
}

