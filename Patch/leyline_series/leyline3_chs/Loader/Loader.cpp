#include "my.h"

#pragma comment(linker, "/ENTRY:MainEntry")
#pragma comment(linker, "/SECTION:.text,ERW /MERGE:.rdata=.text /MERGE:.data=.text")
#pragma comment(linker, "/SECTION:.Xmoe,ERW /MERGE:.text=.Xmoe")

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

	CodePage = GetACP();
	RtlZeroMemory(&si, sizeof(si));
	RtlZeroMemory(&pi, sizeof(pi));
	si.cb = sizeof(si);

	RtlZeroMemory(DllPath, MAX_PATH * 2);
	Nt_SetExeDirectoryAsCurrent();
	Nt_GetCurrentDirectory(MAX_PATH, DllPath);
	StringCatW(DllPath, L"\\");

	if (Nt_GetModuleHandle(L"LocaleEmulator.dll") || Nt_GetModuleHandle(L"ntleai.dll"))
	{
		MessageBoxW(NULL, L"Ո���_���D�^��ʽ...", L">_<", MB_OK | MB_ICONWARNING);
		Ps::ExitProcess(0);
	}

	if (Nt_CurrentPeb()->OSMajorVersion == 5)
	{
		switch (CodePage)
		{
		case CP_GBK:
			MessageBoxW(NULL, L"��Ǹ������������֧��Windows XP\n�Լ�Windows 2003���ϵ�еĲ���ϵͳ", L">_<", MB_OK | MB_ICONWARNING);
			break;

		default:
			MessageBoxW(NULL, L"��Ǹ�����a������֧ԮWindows XP\n�Լ�Windows 2003���Pϵ�е����Iϵ�y", L">_<", MB_OK | MB_ICONWARNING);
			break;
		}
		return 0;
	}

	StringCatW(DllPath, L"leyline3_chs.dll");

	Result = VMeCreateProcess(NULL, NULL, L"LeyLine3.exe", DllPath, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi, NULL);

	if (Result == FALSE)
	{
		switch (CodePage)
		{
		case CP_GBK:
			MessageBoxW(NULL, L"���İ��Ļ�������ʧ��", L"���Ѱɣ����Ͷ���������", MB_OK | MB_ICONERROR);
			break;

		default:
			MessageBoxW(NULL, L"���Đ۵��S�͆���ʧ��", L"���Ѱɣ��S�Ͷ����ۊ���", MB_OK | MB_ICONERROR);
			break;
		}

	}
	Ps::ExitProcess(0);
	return Result;
}

