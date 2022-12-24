#include "my.h"

#pragma comment(linker, "/ENTRY:MainEntry")
#pragma comment(linker, "/SECTION:.text,ERW /MERGE:.rdata=.text /MERGE:.data=.text")
#pragma comment(linker, "/SECTION:.Anzu,ERW /MERGE:.text=.Anzu")

#pragma comment(lib, "ntdll.lib")

typedef
BOOL
(WINAPI
*FuncCreateProcessInternalW)(
HANDLE                  hToken,
LPCWSTR                 lpApplicationName,
LPWSTR                  lpCommandLine,
LPSECURITY_ATTRIBUTES   lpProcessAttributes,
LPSECURITY_ATTRIBUTES   lpThreadAttributes,
BOOL                    bInheritHandles,
ULONG                   dwCreationFlags,
LPVOID                  lpEnvironment,
LPCWSTR                 lpCurrentDirectory,
LPSTARTUPINFOW          lpStartupInfo,
LPPROCESS_INFORMATION   lpProcessInformation,
PHANDLE                 phNewToken
);

BOOL
(WINAPI
*StubCreateProcessInternalW)(
HANDLE                  hToken,
LPCWSTR                 lpApplicationName,
LPWSTR                  lpCommandLine,
LPSECURITY_ATTRIBUTES   lpProcessAttributes,
LPSECURITY_ATTRIBUTES   lpThreadAttributes,
BOOL                    bInheritHandles,
ULONG                   dwCreationFlags,
LPVOID                  lpEnvironment,
LPCWSTR                 lpCurrentDirectory,
LPSTARTUPINFOW          lpStartupInfo,
LPPROCESS_INFORMATION   lpProcessInformation,
PHANDLE                 phNewToken
);

BOOL
WINAPI
VMeCreateProcess(
HANDLE                  hToken,
LPCWSTR                 lpApplicationName,
LPWSTR                  lpCommandLine,
LPCWSTR                 lpDllPath,
LPSECURITY_ATTRIBUTES   lpProcessAttributes,
LPSECURITY_ATTRIBUTES   lpThreadAttributes,
BOOL                    bInheritHandles,
ULONG                   dwCreationFlags,
LPVOID                  lpEnvironment,
LPCWSTR                 lpCurrentDirectory,
LPSTARTUPINFOW          lpStartupInfo,
LPPROCESS_INFORMATION   lpProcessInformation,
PHANDLE                 phNewToken
)
{
	BOOL             Result, IsSuspended;
	UNICODE_STRING   FullDllPath;

	RtlInitUnicodeString(&FullDllPath, lpDllPath);

	StubCreateProcessInternalW =
		(FuncCreateProcessInternalW)Nt_GetProcAddress(GetKernel32Handle(),
		"CreateProcessInternalW");

	IsSuspended = !!(dwCreationFlags & CREATE_SUSPENDED);
	dwCreationFlags |= CREATE_SUSPENDED;
	Result = StubCreateProcessInternalW(
		hToken,
		lpApplicationName,
		lpCommandLine,
		lpProcessAttributes,
		lpThreadAttributes,
		bInheritHandles,
		dwCreationFlags,
		lpEnvironment,
		lpCurrentDirectory,
		lpStartupInfo,
		lpProcessInformation,
		phNewToken);

	if (!Result)
		return Result;

	InjectDllToRemoteProcess(
		lpProcessInformation->hProcess,
		lpProcessInformation->hThread,
		&FullDllPath,
		IsSuspended
		);

	NtResumeThread(lpProcessInformation->hThread, NULL);

	return TRUE;
}

LPWSTR NTAPI StringCatW(LPWSTR lpString1, LPCWSTR lpString2)
{
	ULONG_PTR Length;

	Length = StrLengthW(lpString1);
	StrCopyW(&lpString1[Length], lpString2);
	return lpString1;
}


#define _TRIAL_VER_

INT CDECL MainEntry()
{
	WCHAR               DllPath[MAX_PATH];
	ULONG               CodePage;
	STARTUPINFOW        si;
	PROCESS_INFORMATION pi;
	BOOL                Result;

	RtlZeroMemory(&si, sizeof(si));
	RtlZeroMemory(&pi, sizeof(pi));
	si.cb = sizeof(si);

	RtlZeroMemory(DllPath, MAX_PATH * 2);
	Nt_SetExeDirectoryAsCurrent();
	Nt_GetCurrentDirectory(MAX_PATH, DllPath);
	StringCatW(DllPath, L"\\");


	StringCatW(DllPath, L"kokoiro_chs.dll");

	Result = VMeCreateProcess(NULL, NULL, L"nine_kokoiro.exe -Xmoe::CodePage::950", DllPath, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi, NULL);

	if (Result == FALSE)
	{
		MessageBoxW(NULL, L"遊戲啟動失敗，有可能是防毒軟體所致。\n同時請檢查文件完整性和補丁存放目錄是否正確。", L"遊戲啟動失敗", MB_OK | MB_ICONERROR);
	}
	Ps::ExitProcess(0);
	return Result;
}

