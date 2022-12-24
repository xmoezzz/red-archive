#include "my.h"
#include "AstralHook.h"

#pragma comment(linker, "/SECTION:.text,ERW /MERGE:.rdata=.text /MERGE:.data=.text")
#pragma comment(linker, "/SECTION:.Xmoe,ERW /MERGE:.text=.Xmoe")

BOOL FASTCALL InitHook(PVOID DllModule)
{
	NTSTATUS Status;

	LOOP_ONCE
	{
		Status = ml::MlInitialize();
		if (NT_FAILED(Status))
		{
			MessageBoxW(NULL, L"Ü¢Ñ” ßî°£∫”õëõÛw≤ª◊„(1)", L"ﬂ[ëÚÜ¢Ñ” ßî°", MB_OK | MB_ICONERROR);
			break;
		}

		if (!AstralHook::GetHook(DllModule))
		{
			MessageBoxW(NULL, L"Ü¢Ñ” ßî°£∫”õëõÛw≤ª◊„(2)", L"ﬂ[ëÚÜ¢Ñ” ßî°", MB_OK | MB_ICONERROR);
			Status = STATUS_UNSUCCESSFUL;
			break;
		}

		Status = AstralHook::GetHook()->Init();
		if (NT_FAILED(Status))
		{
			MessageBoxW(NULL, L"Ü¢Ñ” ßî°£∫É»≤ø≥ı ºªØ ßî°", L"ﬂ[ëÚÜ¢Ñ” ßî°", MB_OK | MB_ICONERROR);
			break;
		}
	}
	return NT_SUCCESS(Status);
}

BOOL FASTCALL RestoreHook(PVOID DllModule)
{
	UNREFERENCED_PARAMETER(DllModule);
	return TRUE;
}

BOOL NTAPI DllMain(HMODULE hModule, DWORD Reason, LPVOID lpReserved)
{
	switch (Reason)
	{
	case DLL_PROCESS_ATTACH:
		return InitHook(hModule);

	case DLL_PROCESS_DETACH:
		return RestoreHook(hModule);
	}
	return TRUE;
}


