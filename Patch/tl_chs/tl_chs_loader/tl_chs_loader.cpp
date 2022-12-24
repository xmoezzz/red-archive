#include <my.h>

#pragma comment(linker, "/ENTRY:MainEntry")
#pragma comment(linker, "/SECTION:.text,ERW /MERGE:.rdata=.text /MERGE:.data=.text")
#pragma comment(linker, "/SECTION:.Xmoe,ERW /MERGE:.text=.Xmoe")

#pragma comment(lib, "MyLibrary_x86_static.lib")

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



INT CDECL MainEntry()
{
	WCHAR               DllPath[MAX_PATH];
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

	if (Nt_GetModuleHandle(L"LocaleEmulator.dll") || Nt_GetModuleHandle(L"ntleai.dll"))
	{
		MessageBoxW(NULL, L"Ո���_���D�^��ʽ...", L">_<", MB_OK | MB_ICONWARNING);
		Ps::ExitProcess(0);
	}

	if (Nt_CurrentPeb()->OSMajorVersion == 5)
	{
		MessageBoxW(NULL, L"��Ǹ������������֧��Windows XP\n�Լ�Windows 2003���ϵ�еĲ���ϵͳ", L">_<", MB_OK | MB_ICONWARNING);
		Ps::ExitProcess(0);
	}

	if (!IsWow64Process())
	{
		MessageBoxW(NULL, L"��Ǹ������������֧��32Bit�Ĳ���ϵͳ", L">_<", MB_OK | MB_ICONWARNING);
		Ps::ExitProcess(0);
	}

	StringCatW(DllPath, L"tl_chs.dll");

	Result = VMeCreateProcess(NULL, NULL, L"tl.exe", DllPath, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi, NULL);

	if (Result == FALSE)
	{
		MessageBoxW(NULL, L"��Ϸ����ʧ�ܣ�Ҳ���Ƿ�������������¡�\nͬʱ�����ļ������ԺͲ������Ŀ¼�Ƿ���ȷ��", L"��Ϸ����ʧ��", MB_OK | MB_ICONERROR);
	}
	Ps::ExitProcess(0);
	return Result;
}

