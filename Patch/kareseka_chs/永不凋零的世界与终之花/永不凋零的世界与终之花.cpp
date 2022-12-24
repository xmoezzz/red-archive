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

	StubCreateProcessInternalW = (FuncCreateProcessInternalW)EATLookupRoutineByHashPNoFix(GetKernel32Handle(), KERNEL32_CreateProcessInternalW);
	

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

NTSTATUS FASTCALL IsKrkrEngine(LPCWSTR lpFileName)
{
	NTSTATUS   Status;
	NtFileDisk File;
	ULONG_PTR  Size;
	PBYTE      Buffer;

	static WCHAR Pattern[] = L"TVP(KIRIKIRI) Z core / Scripting Platform for Win32";

	LOOP_ONCE
	{
		Status = File.Open(lpFileName);
		if (NT_FAILED(Status))
			break;

		Size   = File.GetSize32();
		Buffer = (PBYTE)AllocateMemoryP(Size);
		if (!Buffer)
			break;

		File.Read(Buffer, Size);
		if (KMP(Buffer, Size, Pattern, countof(Pattern) * sizeof(Pattern[0])) == NULL)
			Status = STATUS_NO_SUCH_FILE;
		else
			Status = STATUS_SUCCESS;

		FreeMemoryP(Buffer);
	}
	File.Close();
	return Status;
}

NTSTATUS FASTCALL CheckAndCreateProcess()
{
	NTSTATUS         Status;
	WIN32_FIND_DATAW Data;
	HANDLE           Handle;
	NtFileDisk       File;
	STARTUPINFOW        si;
	PROCESS_INFORMATION pi;
	WCHAR               DllPath[MAX_PATH];

	static WChar ProcessName[] = L"sweet_01kareseka.exe";

	RtlZeroMemory(&si, sizeof(si));
	RtlZeroMemory(&pi, sizeof(pi));
	si.cb = sizeof(si);

	RtlZeroMemory(DllPath, MAX_PATH * 2);
	Nt_GetCurrentDirectory(MAX_PATH, DllPath);
	StringCatW(DllPath, L"\\kareseka_chs.dll");

	if (NT_SUCCESS(File.Open(ProcessName)))
	{
		File.Close();
		return VMeCreateProcess(NULL, NULL, ProcessName, DllPath, NULL, NULL, FALSE, NULL, NULL, NULL, &si, &pi, NULL) ? STATUS_SUCCESS : STATUS_UNSUCCESSFUL;
	}

	RtlZeroMemory(&Data, sizeof(Data));

	LOOP_ONCE
	{
		Status = STATUS_NO_SUCH_FILE;

		Handle = Nt_FindFirstFile(L"*.exe", &Data);
		if (Handle == INVALID_HANDLE_VALUE)
			break;

		do
		{
			if (NT_SUCCESS(IsKrkrEngine(Data.cFileName)))
				return VMeCreateProcess(NULL, NULL, Data.cFileName, DllPath, NULL, NULL, FALSE, NULL, NULL, NULL, &si, &pi, NULL) ? STATUS_SUCCESS : STATUS_UNSUCCESSFUL;
		} while (Nt_FindNextFile(Handle, &Data));
	}
	return Status;
}


INT CDECL MainEntry()
{
	NTSTATUS    Status;

	ml::MlInitialize();
	Nt_SetExeDirectoryAsCurrent();

	if (Nt_GetModuleHandle(L"LocaleEmulator.dll") || Nt_GetModuleHandle(L"ntleai.dll"))
	{
		MessageBoxW(NULL, L"請勿開啟轉區程式...", L">_<", MB_OK | MB_ICONWARNING);
		Ps::ExitProcess(0);
	}

	if (Nt_CurrentPeb()->OSMajorVersion == 5)
	{
		MessageBoxW(NULL, L"抱歉，本补丁不再支持Windows XP\n以及Windows 2003相关系列的操作系统", L">_<", MB_OK | MB_ICONWARNING);
		Ps::ExitProcess(0);
	}

	Status = CheckAndCreateProcess();
	if (NT_FAILED(Status))
		MessageBoxW(NULL, L"你心爱的黄油启动失败", L"醒醒吧，黄油都不爱你了", MB_OK | MB_ICONERROR);

	Ps::ExitProcess(0);
	return Status;
}

