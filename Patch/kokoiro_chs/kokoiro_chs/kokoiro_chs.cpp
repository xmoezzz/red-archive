#include "my.h"
#include "KaresekaHook.h"
#include "Se.h"

BOOL FASTCALL KaresekaInit(PVOID hModule)
{
	KaresekaHook*           Kareseka;
	THREAD_START_PARAMETER* StartParameter;

	ml::MlInitialize();
	Kareseka = GetKareseka();
	SetMainThread(Nt_CurrentTeb()->ClientId.UniqueThread);
	StartParameter = AllocateThreadParameter(NULL, (PVOID)((ULONG)NtAddAtom ^ (ULONG)Nt_CurrentPeb()));
	RtlPushFrame(StartParameter);
	return NT_SUCCESS(Kareseka->Init((HMODULE)hModule));
}


BOOL FASTCALL KaresekaUnInit(PVOID hModule)
{
	UNREFERENCED_PARAMETER(hModule);
	return TRUE;
}


OVERLOAD_CPP_NEW_WITH_HEAP(Nt_CurrentPeb()->ProcessHeap)

BOOL NTAPI DllMain(HMODULE hModule, DWORD Reason, LPVOID lpReserved)
{
	UNREFERENCED_PARAMETER(lpReserved);

	switch (Reason)
	{
	case DLL_PROCESS_ATTACH:
		return KaresekaInit(hModule);

	case DLL_PROCESS_DETACH:
		return KaresekaUnInit(hModule);
	}
	return TRUE;
}
