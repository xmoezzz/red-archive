#include "my.h"
#include "MyHook.h"
#include "LocaleEmulator.h"

#pragma comment(linker, "/ENTRY:DllMainEntry")
#pragma comment(linker, "/SECTION:.text,ERW /MERGE:.rdata=.text /MERGE:.data=.text")
#pragma comment(linker, "/SECTION:.Xmoe,ERW /MERGE:.text=.Xmoe")

#pragma comment(lib, "psapi.lib")


API_POINTER(CreateFileW) StubCreateFileW = NULL;



PWChar GetPackageName(LPCWSTR FullFileName, LPWSTR FileName)
{
	ULONG TrimPos;

	for (TrimPos = StrLengthW(FullFileName) - 1; TrimPos >= 0; TrimPos--)
		if (FullFileName[TrimPos] == '\\')
			break;

	if (TrimPos != NULL)
		return StrCopyW(FileName, FullFileName + TrimPos + 1);

	for (TrimPos = StrLengthW(FullFileName) - 1; TrimPos >= 0; TrimPos--)
		if (FullFileName[TrimPos] == '/')
			break;

	if (TrimPos != NULL)
		return StrCopyW(FileName, FullFileName + TrimPos + 1);

	return StrCopyW(FileName, FullFileName);
}



HANDLE
WINAPI
HookCreateFileW(
_In_ LPCWSTR lpFileName,
_In_ DWORD dwDesiredAccess,
_In_ DWORD dwShareMode,
_In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes,
_In_ DWORD dwCreationDisposition,
_In_ DWORD dwFlagsAndAttributes,
_In_opt_ HANDLE hTemplateFile
)
{
	WCHAR PureFileName[MAX_PATH];
	RtlZeroMemory(PureFileName, countof(PureFileName) * sizeof(WCHAR));
	
	GetPackageName(lpFileName, PureFileName);

	if (!StrICompareW(PureFileName, L"script.dat", StrCmp_ToLower))
	{
		return StubCreateFileW(L"script_chs.dat", dwDesiredAccess, dwShareMode, lpSecurityAttributes,
			dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
	}
	else if (!StrICompareW(PureFileName, L"system.dat", StrCmp_ToLower))
	{
		return StubCreateFileW(L"system_chs.dat", dwDesiredAccess, dwShareMode, lpSecurityAttributes,
			dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
	}
	else
		return StubCreateFileW(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes,
		dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
}


HANDLE
WINAPI
HookCreateFileA(
_In_ LPCSTR lpFileName,
_In_ DWORD dwDesiredAccess,
_In_ DWORD dwShareMode,
_In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes,
_In_ DWORD dwCreationDisposition,
_In_ DWORD dwFlagsAndAttributes,
_In_opt_ HANDLE hTemplateFile
)
{
	ULONG  TransByte;
	WCHAR  FileName[MAX_PATH];
	RtlZeroMemory(FileName, countof(FileName) * sizeof(WCHAR));
	RtlMultiByteToUnicodeN(FileName, MAX_PATH * 2, &TransByte, lpFileName, StrLengthA(lpFileName));
	return HookCreateFileW(FileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes,
		dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
}

API_POINTER(NtCreateFile) StubNtCreateFile = NULL;


BOOL NTAPI Initialization(HMODULE hModule)
{
	NTSTATUS Status;

	INLINE_PATCH_DATA p[] = 
	{
		{ CreateFileA, HookCreateFileA, NULL                     },
		{ CreateFileW, HookCreateFileW, (PVOID*)&StubCreateFileW }
	};

	LOOP_ONCE
	{
		Status = InlinePatchMemory(p, countof(p));
		if (NT_FAILED(Status))
			break;

		Status = BeginLocalEmulator(936);
		if (NT_FAILED(Status))
			break;
	}

	return NT_SUCCESS(Status);
}


BOOL NTAPI UnInitialization(HMODULE hModule)
{
	UNREFERENCED_PARAMETER(hModule);
	return TRUE;
}


BOOL NTAPI DllMainEntry(HMODULE hModule, DWORD Reason, LPVOID lpReserved)
{
	switch (Reason)
	{
	case DLL_PROCESS_ATTACH:
		return Initialization(hModule);
	case DLL_PROCESS_DETACH:
		return UnInitialization(hModule);
	}
	return TRUE;
}