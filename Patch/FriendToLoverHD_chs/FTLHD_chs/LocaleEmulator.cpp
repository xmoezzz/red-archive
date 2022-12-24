#include "LocaleEmulator.h"
#include <ntstatus.h>
#include "MemoryPatch.h"
#include "Hook.h"

#pragma comment(lib, "undoc_k32.lib")
#pragma comment(lib, "comctl32.lib")

ATOM        g_PropAtom = 0;
//DWORD       g_TlsIndex = TLS_OUT_OF_INDEXES;
HANDLE      g_hHeap = 0;
// UINT        g_CodePage;
PUSHORT     g_pCodePageBase;
CPTABLEINFO g_CodePageTable, g_DefaultCodePageTable;

LOCALE_INFO g_LocaleInfo;

LOCALE_EMULATOR_THREAD_INFO* InitTlsData()
{
	//    DWORD   TlsIndex;
	HANDLE  hHeap;
	LOCALE_EMULATOR_THREAD_INFO *pInfo;

	//    TlsIndex = g_TlsIndex;
	hHeap = g_hHeap;
	pInfo = (LOCALE_EMULATOR_THREAD_INFO *)RtlAllocateHeap(hHeap, HEAP_ZERO_MEMORY, sizeof(*pInfo));
	if (pInfo == NULL)
		return NULL;

	pInfo->Context = LOCALE_EMULATOR_THREAD_INFO_MAGIC;
	RtlPushFrame(pInfo);
	//    if (!TlsSetValue(TlsIndex, pInfo))
	//    {
	//        RtlFreeHeap(hHeap, 0, pInfo);
	//        return NULL;
	//    }
	/*
	//    pInfo->bRecursion    = FALSE;
	pInfo->hHook         = NULL;
	pInfo->OldDlgProcA   = NULL;
	*/
	return pInfo;
}

LOCALE_EMULATOR_THREAD_INFO* GetTlsData(BOOL bAutoInit = TRUE)
{
	LOCALE_EMULATOR_THREAD_INFO *pInfo;

	pInfo = (LOCALE_EMULATOR_THREAD_INFO *)Nt_FindThreadFrameByContext(LOCALE_EMULATOR_THREAD_INFO_MAGIC);
	//    pInfo = (LOCALE_EMULATOR_THREAD_INFO *)TlsGetValue(g_TlsIndex);
	if (pInfo != NULL || !bAutoInit)
		return pInfo;

	return InitTlsData();
}

WINDOW_PROP_INFO* InitPropInfo(HWND hWnd)
{
	HANDLE hHeap;
	WINDOW_PROP_INFO *pInfo;

	hHeap = g_hHeap;
	pInfo = (WINDOW_PROP_INFO *)RtlAllocateHeap(hHeap, HEAP_ZERO_MEMORY, sizeof(*pInfo));
	if (pInfo == NULL)
		return NULL;

	if (!SetPropW(hWnd, (LPWSTR)g_PropAtom, (HANDLE)pInfo))
	{
		RtlFreeHeap(hHeap, 0, pInfo);
		return NULL;
	}

	return pInfo;
}

WINDOW_PROP_INFO* GetPropInfo(HWND hWnd, BOOL bAutoInit = FALSE)
{
	WINDOW_PROP_INFO *pInfo;

	pInfo = (WINDOW_PROP_INFO *)GetPropW(hWnd, (LPWSTR)g_PropAtom);
	if (pInfo != NULL || !bAutoInit)
		return pInfo;

	return InitPropInfo(hWnd);
}


#if 0
DWORD MByteToWChar(LPCSTR lpAnsiString, DWORD Length, LPWSTR lpUnicodeBuffer, DWORD BufferCount)
{
	NTSTATUS Status;

	if (lpAnsiString == NULL)
		return (DWORD)lpAnsiString;

	if (Length == -1)
		Length = StrLengthA((PCHAR)lpAnsiString);

	if (Length == 0)
	{
		if (lpUnicodeBuffer != NULL)
			lpUnicodeBuffer[0] = 0;

		return 0;
	}

	if (lpUnicodeBuffer == NULL || BufferCount == 0)
	{
		lpUnicodeBuffer = (LPWSTR)AllocStack((Length + 1) * sizeof(*lpUnicodeBuffer));
		BufferCount = Length;
	}

	Status = RtlCustomCPToUnicodeN(
		&g_CodePageTable,
		lpUnicodeBuffer,
		BufferCount * sizeof(*lpUnicodeBuffer),
		&Length,
		(PCHAR)lpAnsiString,
		Length);

	BaseSetLastNTError(Status);
	if (!NT_SUCCESS(Status) || Length == 0)
		return 0;

	Length /= sizeof(*lpUnicodeBuffer);
	if (Length < BufferCount)
		lpUnicodeBuffer[Length] = 0;

	return Length;
}

DWORD WCharToMByte(LPCWSTR lpUnicodeString, DWORD Length, LPSTR lpAnsiBuffer, DWORD BufferCount)
{
	NTSTATUS Status;

	if (lpUnicodeString == NULL)
		return (DWORD)lpUnicodeString;

	if (Length == -1)
		Length = StrLengthW(lpUnicodeString);

	if (Length == 0)
	{
		if (lpAnsiBuffer != NULL)
			lpAnsiBuffer[0] = 0;
		return 0;
	}

	if (lpAnsiBuffer == NULL || BufferCount == 0)
	{
		BufferCount = (Length + 1) * sizeof(*lpUnicodeString);
		lpAnsiBuffer = (LPSTR)AllocStack(BufferCount);
	}

	Length *= sizeof(*lpUnicodeString);
	Status = RtlUnicodeToCustomCPN(
		&g_CodePageTable,
		lpAnsiBuffer,
		BufferCount,
		&Length,
		(PWSTR)lpUnicodeString,
		Length);

	BaseSetLastNTError(Status);
	if (!NT_SUCCESS(Status) || Length == 0)
		return 0;

	if (Length < BufferCount)
		lpAnsiBuffer[Length] = 0;

	return Length;
}

#else


DWORD MByteToWChar(LPCSTR lpAnsiString, DWORD Length, LPWSTR lpUnicodeBuffer, DWORD BufferCount)
{
	NTSTATUS Status;

	if (lpAnsiString == NULL)
		return (DWORD)lpAnsiString;

	if (Length == -1)
		Length = StrLengthA((PCHAR)lpAnsiString);

	if (Length == 0)
	{
		if (lpUnicodeBuffer != NULL)
			lpUnicodeBuffer[0] = 0;

		return 0;
	}

	if (lpUnicodeBuffer == NULL || BufferCount == 0)
	{
		lpUnicodeBuffer = (LPWSTR)AllocStack((Length + 1) * sizeof(*lpUnicodeBuffer));
		BufferCount = Length;
	}

	RtlZeroMemory(lpUnicodeBuffer, BufferCount * sizeof(*lpUnicodeBuffer));

	MultiByteToWideChar(
		936,
		0,
		(PCHAR)lpAnsiString,
		Length,
		lpUnicodeBuffer,
		BufferCount * sizeof(*lpUnicodeBuffer));

	return Length;
}

DWORD WCharToMByte(LPCWSTR lpUnicodeString, DWORD Length, LPSTR lpAnsiBuffer, DWORD BufferCount)
{
	NTSTATUS Status;

	if (lpUnicodeString == NULL)
		return (DWORD)lpUnicodeString;

	if (Length == -1)
		Length = StrLengthW(lpUnicodeString);

	if (Length == 0)
	{
		if (lpAnsiBuffer != NULL)
			lpAnsiBuffer[0] = 0;
		return 0;
	}

	if (lpAnsiBuffer == NULL || BufferCount == 0)
	{
		BufferCount = (Length + 1) * sizeof(*lpUnicodeString);
		lpAnsiBuffer = (LPSTR)AllocStack(BufferCount);
	}

	Length *= sizeof(*lpUnicodeString);

	RtlZeroMemory(lpAnsiBuffer, BufferCount);
	
	Length = WideCharToMultiByte(
		936,
		0,
		(PWSTR)lpUnicodeString,
		Length,
		lpAnsiBuffer,
		BufferCount,
		0,
		0
		);

	if (Length < BufferCount)
		lpAnsiBuffer[Length] = 0;

	return Length;
}

#endif

ULONG GetEmuCharSet(ULONG CharSet)
{
	switch (CharSet)
	{
	case ANSI_CHARSET:
	case DEFAULT_CHARSET:
		return g_LocaleInfo.FontCharSet;
	}

	return CharSet;
}

ULONG GetEmuCodePage(ULONG CodePage)
{
	switch (CodePage)
	{
	case CP_ACP:
	case CP_OEMCP:
		return g_LocaleInfo.CodePage;
	}

	return CodePage;
}

/************************************************************************/
/* kernel32.dll                                                         */
/************************************************************************/

UINT WINAPI MyGetACP()
{
	return g_LocaleInfo.CodePage;
}

LANGID OldGetSystemDefaultLangID()
{
	ASM_DUMMY_AUTO();
}

LANGID MyGetSystemDefaultLangID()
{
	return g_LocaleInfo.DefaultSystemLangID == (LANGID)-1 ? OldGetSystemDefaultLangID() : g_LocaleInfo.DefaultSystemLangID;
}

ASM
int
WINAPI
OldWideCharToMultiByte(
UINT    CodePage,
DWORD   dwFlags,
LPCWSTR lpWideCharStr,
int     cchWideChar,
LPSTR   lpMultiByteStr,
int     cbMultiByte,
LPCSTR  lpDefaultChar,
LPBOOL  lpUsedDefaultChar
)
{
	UNREFERENCED_PARAMETER(CodePage);
	UNREFERENCED_PARAMETER(dwFlags);
	UNREFERENCED_PARAMETER(lpWideCharStr);
	UNREFERENCED_PARAMETER(cchWideChar);
	UNREFERENCED_PARAMETER(lpMultiByteStr);
	UNREFERENCED_PARAMETER(cbMultiByte);
	UNREFERENCED_PARAMETER(lpDefaultChar);
	UNREFERENCED_PARAMETER(lpUsedDefaultChar);
	ASM_DUMMY_AUTO();
}

int
WINAPI
MyWideCharToMultiByte(
UINT    CodePage,
DWORD   dwFlags,
LPCWSTR lpWideCharStr,
int     cchWideChar,
LPSTR   lpMultiByteStr,
int     cbMultiByte,
LPCSTR  lpDefaultChar,
LPBOOL  lpUsedDefaultChar
)
{
	return OldWideCharToMultiByte(
		GetEmuCodePage(CodePage),
		dwFlags,
		lpWideCharStr,
		cchWideChar,
		lpMultiByteStr,
		cbMultiByte,
		lpDefaultChar,
		lpUsedDefaultChar);
}

ASM
int
WINAPI
OldMultiByteToWideChar(
UINT    CodePage,
DWORD   dwFlags,
LPCSTR  lpMultiByteStr,
int     cbMultiByte,
LPWSTR  lpWideCharStr,
int     cchWideChar
)
{
	UNREFERENCED_PARAMETER(CodePage);
	UNREFERENCED_PARAMETER(dwFlags);
	UNREFERENCED_PARAMETER(lpMultiByteStr);
	UNREFERENCED_PARAMETER(cbMultiByte);
	UNREFERENCED_PARAMETER(lpWideCharStr);
	UNREFERENCED_PARAMETER(cchWideChar);
	ASM_DUMMY_AUTO();
}

int
WINAPI
MyMultiByteToWideChar(
UINT    CodePage,
DWORD   dwFlags,
LPCSTR  lpMultiByteStr,
int     cbMultiByte,
LPWSTR  lpWideCharStr,
int     cchWideChar
)
{
	return OldMultiByteToWideChar(
		GetEmuCodePage(CodePage),
		dwFlags,
		lpMultiByteStr,
		cbMultiByte,
		lpWideCharStr,
		cchWideChar);
}

ASM NTSTATUS NTAPI OldRtlUnicodeToMultiByteSize(PULONG pBytesInMultiByteString, PWSTR UnicodeString, ULONG BytesInUnicodeString)
{
	UNREFERENCED_PARAMETER(pBytesInMultiByteString);
	UNREFERENCED_PARAMETER(UnicodeString);
	UNREFERENCED_PARAMETER(BytesInUnicodeString);
	ASM_DUMMY_AUTO();
}

NTSTATUS
NTAPI
MyRtlUnicodeToMultiByteSize(
PULONG pBytesInMultiByteString,
PWSTR  UnicodeString,
ULONG  BytesInUnicodeString
)
{
	ULONG nChar;
	nChar = WCharToMByte(UnicodeString, BytesInUnicodeString / sizeof(*UnicodeString), NULL, 0);
	if (pBytesInMultiByteString != NULL)
		*pBytesInMultiByteString = nChar;
	return 0;
}

ASM NTSTATUS NTAPI OldRtlUnicodeToMultiByteN(PCHAR MultiByteString, ULONG MaxBytesInMultiByteString, PULONG BytesInMultiByteString, PWSTR UnicodeString, ULONG BytesInUnicodeString)
{
	UNREFERENCED_PARAMETER(MaxBytesInMultiByteString);
	UNREFERENCED_PARAMETER(MultiByteString);
	UNREFERENCED_PARAMETER(BytesInMultiByteString);
	UNREFERENCED_PARAMETER(UnicodeString);
	UNREFERENCED_PARAMETER(BytesInUnicodeString);
	ASM_DUMMY_AUTO();
}

NTSTATUS
NTAPI
MyRtlUnicodeToMultiByteN(
PCHAR   MultiByteString,
ULONG   MaxBytesInMultiByteString,
PULONG  pBytesInMultiByteString OPTIONAL,
PWSTR   UnicodeString,
ULONG   BytesInUnicodeString
)
{
	return RtlUnicodeToCustomCPN(&g_CodePageTable, MultiByteString, MaxBytesInMultiByteString, pBytesInMultiByteString, UnicodeString, BytesInUnicodeString);
}

ASM NTSTATUS NTAPI OldRtlMultiByteToUnicodeSize(PULONG pBytesInUnicodeString, PCHAR MultiByteString, ULONG BytesInMultiByteString)
{
	UNREFERENCED_PARAMETER(pBytesInUnicodeString);
	UNREFERENCED_PARAMETER(MultiByteString);
	UNREFERENCED_PARAMETER(BytesInMultiByteString);
	ASM_DUMMY_AUTO();
}

NTSTATUS
NTAPI
MyRtlMultiByteToUnicodeSize(
PULONG pBytesInUnicodeString,
PCHAR  MultiByteString,
ULONG  BytesInMultiByteString
)
{
	ULONG nChar;
	nChar = MByteToWChar(MultiByteString, BytesInMultiByteString, NULL, 0);
	if (pBytesInUnicodeString != NULL)
		*pBytesInUnicodeString = nChar * sizeof(WCHAR);
	return 0;
}

ASM NTSTATUS WINAPI OldRtlMultiByteToUnicodeN(PWSTR UnicodeString, ULONG MaxBytesInUnicodeString, PULONG BytesInUnicodeString, PCHAR MultiByteString, ULONG BytesInMultiByteString)
{
	UNREFERENCED_PARAMETER(UnicodeString);
	UNREFERENCED_PARAMETER(BytesInUnicodeString);
	UNREFERENCED_PARAMETER(MaxBytesInUnicodeString);
	UNREFERENCED_PARAMETER(BytesInMultiByteString);
	UNREFERENCED_PARAMETER(MultiByteString);
	ASM_DUMMY_AUTO();
}

NTSTATUS
WINAPI
MyRtlMultiByteToUnicodeN(
PWSTR  UnicodeString,
ULONG  MaxBytesInUnicodeString,
PULONG pBytesInUnicodeString OPTIONAL,
PCHAR  MultiByteString,
ULONG  BytesInMultiByteString
)
{
	return RtlCustomCPToUnicodeN(&g_CodePageTable, UnicodeString, MaxBytesInUnicodeString, pBytesInUnicodeString, MultiByteString, BytesInMultiByteString);
}

BOOL WINAPI MyIsDBCSLeadByte(BYTE TestChar)
{
	return g_CodePageTable.DBCSOffsets[TestChar] != 0;
	//    return IsDBCSLeadByteEx(g_CodePage, TestChar);
}

ASM PVOID LoadExternDll()
{
	INLINE_ASM
	{
		push eax;           // ret addr
		pushad;
		pushfd;
		call SELF_LOCALIZATION;
	SELF_LOCALIZATION:
		pop  esi;
		and  esi, 0xFFFF0000;
		lodsd;
		mov[esp + 0x24], eax;   // ret addr
		lodsd;
		xor  ecx, ecx;
		push ecx;
		push esp;               // pModuleHandle
		push esi;               // ModuleFileName
		push ecx;               // Flags
		push ecx;               // PathToFile
		call eax;               // LdrLoadDll
		pop  eax;               // pop ModuleHandle
		xchg eax, esi;
		and  eax, 0FFFF0000h;
		and  dword ptr[eax], 0;
		popfd;
		popad;
		ret;
	}
}

ASM VOID _LoadExternDllEnd() {}

typedef struct
{
	ULONG           ReturnAddr;
	PVOID           pfLdrLoadDll;
	UNICODE_STRING  ModuleFileName;
} INJECT_DLL_CURRENT_THREAD;

NTSTATUS InjectDllToRemoteProcess(HANDLE hProcess, HANDLE hThread, PUNICODE_STRING DllFullPath, BOOL IsSuspended)
{
	NTSTATUS    Status;
	PVOID       pvBuffer;
	ULONG       Length;
	WCHAR       szSelfPath[MAX_NTPATH];
	CONTEXT     ThreadContext;
	LARGE_INTEGER TimeOut;
	INJECT_DLL_CURRENT_THREAD inj;

	ThreadContext.ContextFlags = CONTEXT_CONTROL;
	Status = NtGetContextThread(hThread, &ThreadContext);
	if (!NT_SUCCESS(Status))
		return Status;

	CopyMemory(szSelfPath, DllFullPath->Buffer, DllFullPath->Length);
	Length += DllFullPath->Length / sizeof(WCHAR);
	szSelfPath[Length] = 0;

	pvBuffer = NULL;
	Status = Nt_AllocateMemory(hProcess, &pvBuffer, MEMORY_PAGE_SIZE);
	if (!NT_SUCCESS(Status))
		return Status;

	Length *= sizeof(WCHAR);
	inj.pfLdrLoadDll = LdrLoadDll;
	inj.ReturnAddr = ThreadContext.Eip;
	inj.ModuleFileName.Length = Length;
	inj.ModuleFileName.MaximumLength = Length + sizeof(WCHAR);
	inj.ModuleFileName.Buffer = (LPWSTR)((ULONG_PTR)pvBuffer + sizeof(inj));

	Status = STATUS_UNSUCCESSFUL;
	LOOP_ONCE
	{
		Status = Nt_WriteMemory(hProcess, pvBuffer, &inj, sizeof(inj));
		if (!NT_SUCCESS(Status))
			break;

		Length += sizeof(WCHAR);
		Status = Nt_WriteMemory(hProcess, (PVOID)((ULONG_PTR)pvBuffer + sizeof(inj)), szSelfPath, Length);
		if (!NT_SUCCESS(Status))
			break;

		ThreadContext.Eip = ROUND_UP((ULONG)(PBYTE)pvBuffer + sizeof(inj) + Length, 16);
		Status = Nt_WriteMemory(
			hProcess,
			(PVOID)ThreadContext.Eip,
			LoadExternDll,
			(ULONG_PTR)_LoadExternDllEnd - (ULONG_PTR)LoadExternDll
			);
		if (!NT_SUCCESS(Status))
			break;

		Status = NtSetContextThread(hThread, &ThreadContext);
		if (!NT_SUCCESS(Status))
			break;

		if (IsSuspended)
			return Status;

		Status = NtResumeThread(hThread, NULL);
		if (!NT_SUCCESS(Status))
			break;

		FormatTimeOut(&TimeOut, 500);
		for (ULONG TryTimes = 30; TryTimes; --TryTimes)
		{
			ULONG Val;
			Status = Nt_ReadMemory(hProcess, pvBuffer, &Val, sizeof(Val), NULL);
			if (!NT_SUCCESS(Status))
				break;

			if (Val != 0)
			{
				NtDelayExecution(FALSE, &TimeOut);
				continue;
			}

			break;
		}

		if (!NT_SUCCESS(Status))
			break;

		NtDelayExecution(FALSE, &TimeOut);
		Status = NtGetContextThread(hThread, &ThreadContext);
		if (!NT_SUCCESS(Status))
			break;

		if ((ULONG_PTR)ThreadContext.Eip < (ULONG_PTR)pvBuffer ||
			(ULONG_PTR)ThreadContext.Eip >(ULONG_PTR)pvBuffer + MEMORY_PAGE_SIZE)
		{
			Status = STATUS_SUCCESS;
		}
		else
		{
			Status = STATUS_UNSUCCESSFUL;
		}
	}

	Nt_FreeMemory(hProcess, pvBuffer);

	return Status;
}

BOOL InjectSelfToRemoteProcess(HANDLE hProcess, HANDLE hThread, BOOL IsSuspended)
{
	NTSTATUS    Status;
	LDR_MODULE *LdrModule;

	LdrModule = Nt_FindLdrModuleByHandle(&__ImageBase);

	Status = InjectDllToRemoteProcess(
		hProcess,
		hThread,
		&LdrModule->FullDllName,
		IsSuspended
		);

	return NT_SUCCESS(Status);
}

ASM
BOOL
WINAPI
OldCreateProcessInternalW(
HANDLE                  hToken,
LPCWSTR                 lpApplicationName,
LPWSTR                  lpCommandLine,
LPSECURITY_ATTRIBUTES   lpProcessAttributes,
LPSECURITY_ATTRIBUTES   lpThreadAttributes,
BOOL                    bInheritHandles,
DWORD                   dwCreationFlags,
LPVOID                  lpEnvironment,
LPCWSTR                 lpCurrentDirectory,
LPSTARTUPINFOW          lpStartupInfo,
LPPROCESS_INFORMATION   lpProcessInformation,
PHANDLE                 phNewToken
)
{
	UNREFERENCED_PARAMETER(hToken);
	UNREFERENCED_PARAMETER(lpApplicationName);
	UNREFERENCED_PARAMETER(lpCommandLine);
	UNREFERENCED_PARAMETER(lpProcessAttributes);
	UNREFERENCED_PARAMETER(lpThreadAttributes);
	UNREFERENCED_PARAMETER(bInheritHandles);
	UNREFERENCED_PARAMETER(dwCreationFlags);
	UNREFERENCED_PARAMETER(lpEnvironment);
	UNREFERENCED_PARAMETER(lpCurrentDirectory);
	UNREFERENCED_PARAMETER(lpStartupInfo);
	UNREFERENCED_PARAMETER(lpProcessInformation);
	UNREFERENCED_PARAMETER(phNewToken);
	ASM_DUMMY_AUTO();
}

BOOL
WINAPI
MyCreateProcessInternalW(
HANDLE                  hToken,
LPCWSTR                 lpApplicationName,
LPWSTR                  lpCommandLine,
LPSECURITY_ATTRIBUTES   lpProcessAttributes,
LPSECURITY_ATTRIBUTES   lpThreadAttributes,
BOOL                    bInheritHandles,
DWORD                   dwCreationFlags,
LPVOID                  lpEnvironment,
LPCWSTR                 lpCurrentDirectory,
LPSTARTUPINFOW          lpStartupInfo,
LPPROCESS_INFORMATION   lpProcessInformation,
PHANDLE                 phNewToken
)
{
	BOOL Result, IsSuspended;

	IsSuspended = dwCreationFlags & CREATE_SUSPENDED;
	dwCreationFlags |= CREATE_SUSPENDED;
	Result = OldCreateProcessInternalW(
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

	InjectSelfToRemoteProcess(lpProcessInformation->hProcess, lpProcessInformation->hThread, IsSuspended);

	return TRUE;
}

BOOL
WINAPI
MyCreateProcessInternalA(
HANDLE                  hToken,
LPCSTR                  lpApplicationName,
LPSTR                   lpCommandLine,
LPSECURITY_ATTRIBUTES   lpProcessAttributes,
LPSECURITY_ATTRIBUTES   lpThreadAttributes,
BOOL                    bInheritHandles,
DWORD                   dwCreationFlags,
LPVOID                  lpEnvironment,
LPCSTR                  lpCurrentDirectory,
LPSTARTUPINFOA          lpStartupInfo,
LPPROCESS_INFORMATION   lpProcessInformation,
PHANDLE                 phNewToken
)
{
	DWORD  Length;
	LPSTR  lpEnvA;
	LPWSTR lpAppNameW, lpCmdLineW, lpEnvW, lpCurDirW;
	STARTUPINFOW StartupInfoW;

	MByteToWCharStack(lpApplicationName, -1, lpAppNameW, NULL);
	MByteToWCharStack(lpCommandLine, -1, lpCmdLineW, NULL);
	MByteToWCharStack(lpCurrentDirectory, -1, lpCurDirW, NULL);

	if (lpEnvironment != NULL)
	{
		lpEnvA = (LPSTR)lpEnvironment;
		Length = 0;
		do
		{
			DWORD BlockLength;
			BlockLength = StrLengthA(lpEnvA) + 1;
			lpEnvA += BlockLength;
			Length += BlockLength;
		} while (*lpEnvA != 0);

		MByteToWCharStack(lpEnvA, Length, lpEnvW, NULL);
	}
	else
	{
		lpEnvW = NULL;
	}

	CopyMemory(&StartupInfoW, lpStartupInfo, lpStartupInfo->cb);
	MByteToWCharStack(lpStartupInfo->lpReserved, -1, StartupInfoW.lpReserved, NULL);
	MByteToWCharStack(lpStartupInfo->lpDesktop, -1, StartupInfoW.lpDesktop, NULL);
	MByteToWCharStack(lpStartupInfo->lpTitle, -1, StartupInfoW.lpTitle, NULL);

	return MyCreateProcessInternalW(
		hToken,
		lpAppNameW,
		lpCmdLineW,
		lpProcessAttributes,
		lpThreadAttributes,
		bInheritHandles,
		dwCreationFlags,
		lpEnvW,
		lpCurDirW,
		&StartupInfoW,
		lpProcessInformation,
		phNewToken);
}

/************************************************************************/
/* GDI32.dll                                                            */
/************************************************************************/

typedef struct
{
	BOOL          bExVersion;
	FONTENUMPROCA lpEnumFontFamProcA;
	LPARAM        lParam;
} ENUM_FONT_FAMILIES_PARAM;

UINT WINAPI MyGdiGetCodePage(HDC hDC)
{
	UNREFERENCED_PARAMETER(hDC);
	return g_LocaleInfo.CodePage;
}

INT
WINAPI
EnumFontFamExProcW(
ENUMLOGFONTEXW           *lpelfe,
NEWTEXTMETRICEXW         *lpntme,
DWORD                     FontType,
ENUM_FONT_FAMILIES_PARAM *pParam
)
{
	ENUMLOGFONTEXA   EnumLogFontA;
	NEWTEXTMETRICEXA NewTextMetricA;

	EnumLogFontA.elfLogFont = *(LPLOGFONTA)&lpelfe->elfLogFont;
	WCharToMByte(lpelfe->elfLogFont.lfFaceName, countof(lpelfe->elfLogFont.lfFaceName), EnumLogFontA.elfLogFont.lfFaceName, sizeof(EnumLogFontA.elfLogFont.lfFaceName));
	WCharToMByte(lpelfe->elfFullName, countof(lpelfe->elfFullName), (LPSTR)EnumLogFontA.elfFullName, countof(EnumLogFontA.elfFullName));
	WCharToMByte(lpelfe->elfStyle, countof(lpelfe->elfStyle), (LPSTR)EnumLogFontA.elfStyle, countof(EnumLogFontA.elfStyle));
	if (pParam->bExVersion)
		WCharToMByte(lpelfe->elfScript, countof(lpelfe->elfScript), (LPSTR)EnumLogFontA.elfScript, countof(EnumLogFontA.elfScript));

	RtlCopyMemory(&NewTextMetricA, &lpntme->ntmTm, GetStructMemberOffset(NEWTEXTMETRICW, tmHeight, tmFirstChar));
	CopyMemory(
		&NewTextMetricA.ntmTm.tmItalic,
		&lpntme->ntmTm.tmItalic,
		GetStructMemberOffset(NEWTEXTMETRICW, tmItalic, ntmAvgWidth) + sizeof(lpntme->ntmTm.ntmAvgWidth));

	WCharToMByte(&lpntme->ntmTm.tmFirstChar, 1, (LPSTR)&NewTextMetricA.ntmTm.tmFirstChar, 1);
	WCharToMByte(&lpntme->ntmTm.tmLastChar, 1, (LPSTR)&NewTextMetricA.ntmTm.tmLastChar, 1);
	WCharToMByte(&lpntme->ntmTm.tmDefaultChar, 1, (LPSTR)&NewTextMetricA.ntmTm.tmDefaultChar, 1);
	WCharToMByte(&lpntme->ntmTm.tmBreakChar, 1, (LPSTR)&NewTextMetricA.ntmTm.tmBreakChar, 1);
	if (pParam->bExVersion)
		NewTextMetricA.ntmFontSig = lpntme->ntmFontSig;

	return pParam->lpEnumFontFamProcA(
		(LPLOGFONTA)&EnumLogFontA,
		(LPTEXTMETRICA)&NewTextMetricA,
		FontType,
		pParam->lParam);
}

INT
WINAPI
MyEnumFontFamiliesA(
HDC           hDC,
LPCSTR        lpszFamily,
FONTENUMPROCA lpEnumFontFamProc,
LPARAM        lParam
)
{
	LPWSTR lpFamilyW;
	ENUM_FONT_FAMILIES_PARAM EnumParam;

	MByteToWCharStack(lpszFamily, -1, lpFamilyW, NULL);

	EnumParam.bExVersion = FALSE;
	EnumParam.lParam = lParam;
	EnumParam.lpEnumFontFamProcA = lpEnumFontFamProc;

	return EnumFontFamiliesW(hDC, lpFamilyW, (FONTENUMPROCW)EnumFontFamExProcW, (LPARAM)&EnumParam);
}

INT
WINAPI
MyEnumFontFamiliesExA(
HDC           hDC,
LPLOGFONTA    lpLogfont,
FONTENUMPROCA lpEnumFontFamExProc,
LPARAM        lParam,
DWORD         dwFlags
)
{
	LOGFONTW LogfontW;
	ENUM_FONT_FAMILIES_PARAM EnumParam;

	LogfontW.lfCharSet = GetEmuCharSet(lpLogfont->lfCharSet);
	LogfontW.lfPitchAndFamily = lpLogfont->lfPitchAndFamily;
	MByteToWChar(lpLogfont->lfFaceName, countof(lpLogfont->lfFaceName), LogfontW.lfFaceName, countof(LogfontW.lfFaceName));

	EnumParam.bExVersion = TRUE;
	EnumParam.lParam = lParam;
	EnumParam.lpEnumFontFamProcA = lpEnumFontFamExProc;

	return EnumFontFamiliesExW(hDC, &LogfontW, (FONTENUMPROCW)EnumFontFamExProcW, (LPARAM)&EnumParam, dwFlags);
}

HFONT WINAPI MyCreateFontIndirectA(LOGFONTA *lplf)
{
	LOGFONTW lf;
	CopyStruct(&lf, lplf, GetStructMemberOffset(LOGFONTA, lfHeight, lfFaceName));
	MByteToWChar(lplf->lfFaceName, -1, lf.lfFaceName, countof(lf.lfFaceName));
	lf.lfCharSet = GetEmuCharSet(lf.lfCharSet);
	return CreateFontIndirectW(&lf);
}

/************************************************************************/
/* user32.dll                                                           */
/************************************************************************/
typedef union
{
	CREATESTRUCTA    CreateStructA;
	MDICREATESTRUCTA MdiCreateStructA;
	TCITEMA          TcItemA;
} MESSAGE_PARAMETERA;

typedef union
{
	CREATESTRUCTW    CreateStructW;
	MDICREATESTRUCTW MdiCreateStructW;
	TCITEMW          TcItemW;
} MESSAGE_PARAMETERW;

BOOL WINAPI MySetWindowTextA(HWND hWnd, LPCSTR lpString)
{
	LPWSTR lpText;
	MByteToWCharStack(lpString, -1, lpText, NULL);
	return SetWindowTextW(hWnd, lpText);
}

INT WINAPI MyGetWindowTextA(HWND hWnd, LPSTR lpString, INT nMaxCount)
{
	DWORD  Length;
	LPWSTR lpText;

	if (lpString == NULL || nMaxCount == 0)
		return 0;

	lpString[0] = 0;
	Length = GetWindowTextLengthW(hWnd);
	if (Length == 0)
		return 0;

	++Length;
	lpText = (LPWSTR)AllocStack(Length * sizeof(WCHAR));
	Length = GetWindowTextW(hWnd, lpText, Length);
	if (Length == 0)
		return 0;

	lpText[Length] = 0;

	return WCharToMByte(lpText, -1, lpString, nMaxCount);
}

ASM BOOL WINAPI OldIsWindowUnicode(HWND hWnd)
{
	UNREFERENCED_PARAMETER(hWnd);
	ASM_DUMMY_AUTO();
}

BOOL WINAPI MyIsWindowUnicode(HWND hWnd)
{
	return GetPropInfo(hWnd) == NULL ? OldIsWindowUnicode(hWnd) : FALSE;
}

ASM LONG_PTR WINAPI OldGetWindowLongW(HWND hWnd, INT Index)
{
	UNREFERENCED_PARAMETER(hWnd);
	UNREFERENCED_PARAMETER(Index);
	ASM_DUMMY_AUTO();
}

LONG_PTR WINAPI MyGetWindowLongW(HWND hWnd, INT Index)
{
	WINDOW_PROP_INFO *pInfo;

	switch (Index)
	{
	case GWL_WNDPROC:
		pInfo = GetPropInfo(hWnd);
		if (pInfo == NULL)
			break;
		return (LONG_PTR)pInfo->OldWndProcA;
	}

	return OldGetWindowLongW(hWnd, Index);
}

ASM LONG_PTR WINAPI OldSetWindowLongW(HWND hWnd, INT Index, LONG_PTR NewLong)
{
	UNREFERENCED_PARAMETER(hWnd);
	UNREFERENCED_PARAMETER(Index);
	UNREFERENCED_PARAMETER(NewLong);
	ASM_DUMMY_AUTO();
}

LONG_PTR WINAPI MySetWindowLongW(HWND hWnd, INT Index, LONG_PTR NewLong)
{
	WINDOW_PROP_INFO *pInfo;

	switch (Index)
	{
	case GWL_WNDPROC:
		pInfo = GetPropInfo(hWnd);
		if (pInfo == NULL)
			break;

		return (LONG_PTR)InterlockedExchange((PLONG)&pInfo->OldWndProcA, NewLong);
	}

	return OldSetWindowLongW(hWnd, Index, NewLong);
}

ASM LONG_PTR WINAPI OldGetWindowLongA(HWND hWnd, INT Index)
{
	UNREFERENCED_PARAMETER(hWnd);
	UNREFERENCED_PARAMETER(Index);
	ASM_DUMMY_AUTO();
}

LONG_PTR WINAPI MyGetWindowLongA(HWND hWnd, INT Index)
{
	switch (Index)
	{
	case DWL_DLGPROC:
	case GWL_WNDPROC:
	case GWL_STYLE:
	case GWL_EXSTYLE:
	case GWL_HINSTANCE:
	case GWL_ID:
	case GWL_USERDATA:
	case DWL_MSGRESULT:
	case DWL_USER:
		break;

	default:
		return OldGetWindowLongA(hWnd, Index);
	}

	return GetWindowLongW(hWnd, Index);
}

ASM LONG_PTR WINAPI OldSetWindowLongA(HWND hWnd, INT Index, LONG_PTR NewLong)
{
	UNREFERENCED_PARAMETER(hWnd);
	UNREFERENCED_PARAMETER(Index);
	UNREFERENCED_PARAMETER(NewLong);
	ASM_DUMMY_AUTO();
}

LONG_PTR WINAPI MySetWindowLongA(HWND hWnd, INT Index, LONG_PTR NewLong)
{
	switch (Index)
	{
	case DWL_DLGPROC:
	case GWL_WNDPROC:
	case GWL_STYLE:
	case GWL_EXSTYLE:
	case GWL_HINSTANCE:
	case GWL_ID:
	case GWL_USERDATA:
	case DWL_MSGRESULT:
	case DWL_USER:
		break;

	default:
		return OldSetWindowLongA(hWnd, Index, NewLong);
	}

	return SetWindowLongW(hWnd, Index, NewLong);
}

typedef LRESULT(WINAPI *FRMPROC)(HWND hWnd, HWND hWndMDIClient, UINT uMsg, WPARAM wParam, LPARAM lParam);
typedef LRESULT(WINAPI *FCALLWNDPROC)(WNDPROC lpPrevWndFunc, HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
typedef LRESULT(WINAPI *FSNDMSG)(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
typedef BOOL(WINAPI *FSNDMSGCALLBACK)(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam, SENDASYNCPROC lpCallBack, ULONG_PTR dwData);
typedef LRESULT(WINAPI *FSNDMSGTIMEOUT)(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam, UINT fuFlags, UINT uTimeout, PDWORD_PTR lpdwResult);

enum
{
	WNDPROC_TYPE_FRMPROC,
	WNDPROC_TYPE_CALLWNDPROC,
	WNDPROC_TYPE_SNDMSGCALLBACK,
	WNDPROC_TYPE_SNDMSGTIMEOUT,
	WNDPROC_TYPE_WNDPROC,
	WNDPROC_TYPE_DLGPROC = WNDPROC_TYPE_WNDPROC,
	WNDPROC_TYPE_MDIPROC = WNDPROC_TYPE_WNDPROC,
	WNDPROC_TYPE_SNDMSG = WNDPROC_TYPE_WNDPROC,
};

typedef struct
{
	DWORD ProcType;
	union
	{
		FRMPROC         pfDefFrameProc;
		WNDPROC         pfDefWindowProc;
		FCALLWNDPROC    pfCallWindowProc;
		DLGPROC         pfDefDlgProc;
		WNDPROC         pfDefMDIChildProc;
		FSNDMSG         pfSendMessage;
		FSNDMSGCALLBACK pfSendMessageCallback;
		FSNDMSGTIMEOUT  pfSendMessageTimeOut;
	};
} WINDOW_MESSAGE_PROC;

/*ForceInline*/
LRESULT
VariableArgsWindowProc(
WINDOW_MESSAGE_PROC WindowMessageProc,
HWND                hWnd,
UINT                Message,
WPARAM              wParam,
LPARAM              lParam,
LPARAM              ExtraParam1,
LPARAM              ExtraParam2,
LPARAM              ExtraParam3
)
{
	switch (WindowMessageProc.ProcType)
	{
	default:
		return WindowMessageProc.pfDefWindowProc(hWnd, Message, wParam, lParam);

	case WNDPROC_TYPE_CALLWNDPROC:
		return WindowMessageProc.pfCallWindowProc((WNDPROC)ExtraParam1, hWnd, Message, wParam, lParam);

	case WNDPROC_TYPE_FRMPROC:
		return WindowMessageProc.pfDefFrameProc(hWnd, (HWND)ExtraParam1, Message, wParam, lParam);

	case WNDPROC_TYPE_SNDMSGCALLBACK:
		return WindowMessageProc.pfSendMessageCallback(hWnd, Message, wParam, lParam, (SENDASYNCPROC)ExtraParam1, (ULONG_PTR)ExtraParam2);

	case WNDPROC_TYPE_SNDMSGTIMEOUT:
		return WindowMessageProc.pfSendMessageTimeOut(hWnd, Message, wParam, lParam, (UINT)ExtraParam1, (UINT)ExtraParam2, (PDWORD_PTR)ExtraParam3);
	}
}

#define CALL_VA_WNDPROC(Message, wParam, lParam) \
        VariableArgsWindowProc(WindowMessageProc, (hWnd), (Message), (WPARAM)(wParam), (LPARAM)(lParam), ExtraParam1, ExtraParam2, ExtraParam3)

LRESULT
AnsiProcToUnicodeProcWorker(
WINDOW_MESSAGE_PROC WindowMessageProc,
HWND                hWnd,
UINT                Message,
WPARAM              wParam,
LPARAM              lParam,
LPARAM              ExtraParam1 = 0,
LPARAM              ExtraParam2 = 0,
LPARAM              ExtraParam3 = 0
)
{
	LRESULT Result;
	LPWSTR  lpUnicodeString;
	MESSAGE_PARAMETERW MsgParamW;
	union
	{
		LPCREATESTRUCTA    lpCreateStructA;
		LPMDICREATESTRUCTA lpMdiCreateStructA;
		LPTCITEMA          lpTcItemA;
	};

	WriteLog();
	switch (Message)
	{
	case LB_ADDSTRING:
	case LB_INSERTSTRING:
	case LB_SELECTSTRING:
	case LB_DIR:
	case LB_FINDSTRING:
	case LB_ADDFILE:
	case LB_FINDSTRINGEXACT:

	case CB_ADDSTRING:
	case CB_INSERTSTRING:
	case CB_DIR:
	case CB_FINDSTRING:
	case CB_SELECTSTRING:
	case CB_FINDSTRINGEXACT:

	case WM_SETTEXT:
	case WM_WININICHANGE:
	case WM_DEVMODECHANGE:
	case EM_REPLACESEL:
		MByteToWCharStack(lParam, -1, lParam, NULL);
		break;

	case TCM_SETITEMA:
	case TCM_INSERTITEMA:
		lpTcItemA = (LPTCITEMA)lParam;
		MsgParamW.TcItemW = *(LPTCITEMW)lpTcItemA;
		lParam = (LPARAM)&MsgParamW.TcItemW;
		if (!TEST_BITS(MsgParamW.TcItemW.mask, TCIF_TEXT))
		{
			MsgParamW.TcItemW.pszText = NULL;
			MsgParamW.TcItemW.cchTextMax = 0;
			break;
		}

		MByteToWCharStack(lpTcItemA->pszText, lpTcItemA->cchTextMax, MsgParamW.TcItemW.pszText, &MsgParamW.TcItemW.cchTextMax);
		break;

	case TCM_GETITEMA:
		lpTcItemA = (LPTCITEMA)lParam;
		MsgParamW.TcItemW = *(LPTCITEMW)lpTcItemA;
		if (TEST_BITS(MsgParamW.TcItemW.mask, TCIF_TEXT))
		{
			MsgParamW.TcItemW.pszText = (LPWSTR)AllocStack(MsgParamW.TcItemW.cchTextMax * sizeof(WCHAR));
		}
		else
		{
			MsgParamW.TcItemW.cchTextMax = 0;
			MsgParamW.TcItemW.pszText = NULL;
		}

		Result = CALL_VA_WNDPROC(TCM_GETITEMW, wParam, &MsgParamW.TcItemW);
		if (Result == FALSE || !TEST_BITS(MsgParamW.TcItemW.mask, TCIF_TEXT))
			return Result;

		WCharToMByte(MsgParamW.TcItemW.pszText, MsgParamW.TcItemW.cchTextMax, lpTcItemA->pszText, lpTcItemA->cchTextMax);
		return Result;

	case LB_GETTEXTLEN:
		lParam = 0;
	case LB_GETTEXT:
		break;

	case WM_GETTEXTLENGTH:
		lParam = 0;
		wParam = 0;
	case WM_GETTEXT:
		Result = CALL_VA_WNDPROC(WM_GETTEXTLENGTH, 0, 0);
		if (Result == 0)
			return 0;

		++Result;
		Result *= sizeof(WCHAR);
		lpUnicodeString = (LPWSTR)AllocStack(Result);
		Result = CALL_VA_WNDPROC(WM_GETTEXT, Result, lpUnicodeString);
		return WCharToMByte(lpUnicodeString, Result, (LPSTR)lParam, wParam);

	case WM_CREATE:
	case WM_NCCREATE:
		lpCreateStructA = (LPCREATESTRUCTA)lParam;
		MsgParamW.CreateStructW = *(LPCREATESTRUCTW)lpCreateStructA;
		lParam = (LPARAM)&MsgParamW.CreateStructW;
		MByteToWCharStack(lpCreateStructA->lpszClass, -1, MsgParamW.CreateStructW.lpszClass, NULL);
		MByteToWCharStack(lpCreateStructA->lpszName, -1, MsgParamW.CreateStructW.lpszName, NULL);
		break;

	case WM_MDICREATE:
		lpMdiCreateStructA = (LPMDICREATESTRUCTA)lParam;
		MsgParamW.MdiCreateStructW = *(LPMDICREATESTRUCTW)lpMdiCreateStructA;
		lParam = (LPARAM)&MsgParamW.MdiCreateStructW;
		MByteToWCharStack(lpMdiCreateStructA->szClass, -1, MsgParamW.MdiCreateStructW.szClass, NULL);
		MByteToWCharStack(lpMdiCreateStructA->szTitle, -1, MsgParamW.MdiCreateStructW.szTitle, NULL);
		break;
	}

	return CALL_VA_WNDPROC(Message, wParam, lParam);
}

LRESULT WINAPI WindowProcW(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	LRESULT Result;
	HANDLE  hHeap;
	LPSTR   lpAnsiString;
	WNDPROC OldWndProcA;
	WINDOW_PROP_INFO *pInfo;
	MESSAGE_PARAMETERA MsgParam;
	union
	{
		LPCREATESTRUCTW    lpCreateStructW;
		LPMDICREATESTRUCTW lpMdiCreateStructW;
		LPTCITEMW          lpTcItemW;
	};

	WriteLog();

	pInfo = GetPropInfo(hWnd);
	if (pInfo == NULL)
		return 0;

	OldWndProcA = pInfo->OldWndProcA;
	switch (Message)
	{
	case LB_ADDSTRING:
	case LB_INSERTSTRING:
	case LB_SELECTSTRING:
	case LB_DIR:
	case LB_FINDSTRING:
	case LB_ADDFILE:
	case LB_FINDSTRINGEXACT:

	case CB_ADDSTRING:
	case CB_INSERTSTRING:
	case CB_DIR:
	case CB_FINDSTRING:
	case CB_SELECTSTRING:
	case CB_FINDSTRINGEXACT:

	case WM_SETTEXT:
	case WM_WININICHANGE:
	case WM_DEVMODECHANGE:
	case EM_REPLACESEL:
		WCharToMByteStack(lParam, -1, lParam, NULL);
		break;

	case TCM_SETITEMA:
	case TCM_INSERTITEMA:
		lpTcItemW = (LPTCITEMW)lParam;
		MsgParam.TcItemA = *(LPTCITEMA)lpTcItemW;
		lParam = (LPARAM)&MsgParam.TcItemA;
		if (!TEST_BITS(MsgParam.TcItemA.mask, TCIF_TEXT))
		{
			MsgParam.TcItemA.pszText = NULL;
			MsgParam.TcItemA.cchTextMax = 0;
			break;
		}

		WCharToMByteStack(MsgParam.TcItemA.pszText, -1, MsgParam.TcItemA.pszText, &MsgParam.TcItemA.cchTextMax);
		break;

	case TCM_GETITEMA:
		MsgParam.TcItemA = *(LPTCITEMA)lParam;
		if (TEST_BITS(MsgParam.TcItemA.mask, TCIF_TEXT))
			MsgParam.TcItemA.pszText = (LPSTR)AllocStack(MsgParam.TcItemA.cchTextMax);

		Result = CallWindowProcA(OldWndProcA, hWnd, TCM_GETITEMA, wParam, (LPARAM)&MsgParam.TcItemA);
		if (!(BOOL)Result || !TEST_BITS(MsgParam.TcItemA.mask, TCIF_TEXT))
			return Result;

		((LPTCITEMW)lParam)->dwState = MsgParam.TcItemA.dwState;
		((LPTCITEMW)lParam)->dwStateMask = MsgParam.TcItemA.dwStateMask;
		((LPTCITEMW)lParam)->iImage = MsgParam.TcItemA.iImage;
		((LPTCITEMW)lParam)->lParam = MsgParam.TcItemA.lParam;
		MByteToWChar(MsgParam.TcItemA.pszText, MsgParam.TcItemA.cchTextMax, ((LPTCITEMW)lParam)->pszText, ((LPTCITEMW)lParam)->cchTextMax);
		return Result;

	case LB_GETTEXTLEN:
		lParam = 0;
	case LB_GETTEXT:
		break;

	case WM_GETTEXTLENGTH:
		lParam = 0;
		wParam = 0;
	case WM_GETTEXT:
		Result = CallWindowProcA(OldWndProcA, hWnd, WM_GETTEXTLENGTH, 0, 0);
		if (Result == 0)
			return 0;

		++Result;
		lpAnsiString = (LPSTR)AllocStack(Result);
		Result = CallWindowProcA(OldWndProcA, hWnd, WM_GETTEXT, Result, (LPARAM)lpAnsiString);
		return MByteToWChar(lpAnsiString, Result, (LPWSTR)lParam, wParam);

	case WM_CREATE:
	case WM_NCCREATE:
		lpCreateStructW = (LPCREATESTRUCTW)lParam;
		MsgParam.CreateStructA = *(LPCREATESTRUCTA)lpCreateStructW;
		lParam = (LPARAM)&MsgParam.CreateStructA;
		WCharToMByteStack(lpCreateStructW->lpszClass, -1, MsgParam.CreateStructA.lpszClass, NULL);
		WCharToMByteStack(lpCreateStructW->lpszName, -1, MsgParam.CreateStructA.lpszName, NULL);
		break;

	case WM_MDICREATE:
		lpMdiCreateStructW = (LPMDICREATESTRUCTW)lParam;
		MsgParam.MdiCreateStructA = *(LPMDICREATESTRUCTA)lpMdiCreateStructW;
		lParam = (LPARAM)&MsgParam.MdiCreateStructA;
		WCharToMByteStack(lpMdiCreateStructW->szClass, -1, MsgParam.MdiCreateStructA.szClass, NULL);
		WCharToMByteStack(lpMdiCreateStructW->szTitle, -1, MsgParam.MdiCreateStructA.szTitle, NULL);
		break;

	case WM_NCDESTROY:
		hHeap = g_hHeap;
		RemovePropW(hWnd, (LPWSTR)g_PropAtom);
		//            if (pInfo->hFont != NULL) DeleteObject(pInfo->hFont);
		RtlFreeHeap(hHeap, 0, pInfo);
		break;
	}

	return CallWindowProcA(OldWndProcA, hWnd, Message, wParam, lParam);
}

#if 0
INT_PTR WINAPI PreDialogProcW(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	DLGPROC DefProc;
	WINDOW_PROP_INFO *pPropInfo;
	LOCALE_EMULATOR_THREAD_INFO *pInfo;

	pInfo = GetTlsData();
	pPropInfo = GetPropInfo(hWnd, TRUE);
	if (pPropInfo != NULL)
	{
		//        OldSetWindowLongW(hWnd, DWLP_DLGPROC, (LONG_PTR)DialogProcW);
	}

	DefProc = pInfo->OldDlgProcA;
	pInfo->OldDlgProcA = NULL;

	return DefProc(hWnd, Message, wParam, lParam);
}
#endif

#define ANSI_TO_UNICODE_PROC(NewEntry, _ProcType, ...) \
        { \
            WINDOW_MESSAGE_PROC Proc; \
            Proc.ProcType = (_ProcType); \
            Proc.pfSendMessage = (FSNDMSG)(NewEntry); \
            return AnsiProcToUnicodeProcWorker(Proc, hWnd, Message, wParam, lParam, __VA_ARGS__); \
        }

LRESULT WINAPI MyDefWindowProcA(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	ANSI_TO_UNICODE_PROC(DefWindowProcW, WNDPROC_TYPE_WNDPROC);
}

LRESULT WINAPI MyDefDlgProcA(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	ANSI_TO_UNICODE_PROC(DefDlgProcW, WNDPROC_TYPE_DLGPROC);
}

LRESULT WINAPI MyDefMDIChildProcA(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	ANSI_TO_UNICODE_PROC(DefMDIChildProcW, WNDPROC_TYPE_MDIPROC);
}

LRESULT WINAPI MySendMessageA(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	ANSI_TO_UNICODE_PROC(SendMessageW, WNDPROC_TYPE_SNDMSG);
}

BOOL WINAPI MySendNotifyMessageA(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	ANSI_TO_UNICODE_PROC(SendNotifyMessageW, WNDPROC_TYPE_SNDMSG);
}

BOOL WINAPI MyPostMessageA(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	ANSI_TO_UNICODE_PROC(PostMessageW, WNDPROC_TYPE_SNDMSG);
}

LRESULT WINAPI MyDefFrameProcA(HWND hWnd, HWND hWndMDIClient, UINT Message, WPARAM wParam, LPARAM lParam)
{
	ANSI_TO_UNICODE_PROC(DefFrameProcW, WNDPROC_TYPE_FRMPROC, (LPARAM)hWndMDIClient);
}

BOOL WINAPI MySendMessageCallbackA(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam, SENDASYNCPROC lpResultCallBack, ULONG_PTR dwData)
{
	ANSI_TO_UNICODE_PROC(SendMessageCallbackW, WNDPROC_TYPE_SNDMSGCALLBACK, (LPARAM)lpResultCallBack, (LPARAM)dwData);
}

LRESULT WINAPI MySendMessageTimeoutA(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam, UINT fuFlags, UINT uTimeout, PDWORD_PTR lpdwResult)
{
	ANSI_TO_UNICODE_PROC(SendMessageTimeoutW, WNDPROC_TYPE_SNDMSGTIMEOUT, (LPARAM)fuFlags, (LPARAM)uTimeout, (LPARAM)lpdwResult);
}

ASM LRESULT WINAPI OldCallWindowProcA(WNDPROC lpPrevWndFunc, HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lpPrevWndFunc);
	UNREFERENCED_PARAMETER(hWnd);
	UNREFERENCED_PARAMETER(Message);
	UNREFERENCED_PARAMETER(wParam);
	UNREFERENCED_PARAMETER(lParam);
	ASM_DUMMY_AUTO();
}

LRESULT WINAPI MyCallWindowProcA(WNDPROC lpPrevWndFunc, HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	if (((ULONG_PTR)lpPrevWndFunc & 0xFFFF0000) == 0xFFFF0000)
	{
		ANSI_TO_UNICODE_PROC(CallWindowProcW, WNDPROC_TYPE_CALLWNDPROC, (LPARAM)lpPrevWndFunc);
	}

	return OldCallWindowProcA(lpPrevWndFunc, hWnd, Message, wParam, lParam);
}

LRESULT CALLBACK CBTProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	WINDOW_PROP_INFO *pPropInfo;
	LOCALE_EMULATOR_THREAD_INFO *pInfo;

	if (nCode == HCBT_CREATEWND && (pInfo = GetTlsData()) != NULL)
	{
		HWND hWnd = (HWND)wParam;

		pPropInfo = GetPropInfo(hWnd, TRUE);
		if (pPropInfo != NULL)
		{
			pPropInfo->OldWndProcA = (WNDPROC)OldGetWindowLongA(hWnd, GWLP_WNDPROC);
			if (pPropInfo->OldWndProcA != NULL)
				(WNDPROC)OldSetWindowLongW(hWnd, GWLP_WNDPROC, (LONG_PTR)WindowProcW);
		}

		if (UnhookWindowsHookEx(pInfo->hHook))
			pInfo->hHook = NULL;

		return 0;
	}

	return CallNextHookEx(NULL, nCode, wParam, lParam);
}

HHOOK BeginWindowsHook(LOCALE_EMULATOR_THREAD_INFO *pInfo)
{
	return pInfo == NULL ? NULL : (pInfo->hHook = SetWindowsHookExA(WH_CBT, CBTProc, NULL, GetCurrentThreadId()));
}

BOOL EndWindowsHook(LOCALE_EMULATOR_THREAD_INFO *pInfo)
{
	if (pInfo == NULL)
		return FALSE;

	if (pInfo->hHook != NULL)
	{
		UnhookWindowsHookEx(pInfo->hHook);
		pInfo->hHook = NULL;
	}

	return TRUE;
}

HWND
WINAPI
MyCreateWindowExA(
DWORD       dwExStyle,
LPCSTR      lpClassName,
LPCSTR      lpWindowName,
DWORD       dwStyle,
int         x,
int         y,
int         nWidth,
int         nHeight,
HWND        hWndParent,
HMENU       hMenu,
HINSTANCE   hInstance,
LPVOID      lpParam
)
{
	HWND   hWnd;
	LPWSTR pszClassName, pszWindowName;
	LOCALE_EMULATOR_THREAD_INFO *pInfo;

	MByteToWCharStack(lpClassName, -1, pszClassName, NULL);
	MByteToWCharStack(lpWindowName, -1, pszWindowName, NULL);

	pInfo = GetTlsData();
	BeginWindowsHook(pInfo);
	hWnd = CreateWindowExW(
		dwExStyle,
		pszClassName,
		pszWindowName,
		dwStyle,
		x,
		y,
		nWidth,
		nHeight,
		hWndParent,
		hMenu,
		hInstance,
		lpParam);
	EndWindowsHook(pInfo);

	return hWnd;
}

typedef INT_PTR(WINAPI *FCREATEDLG)(HINSTANCE hInstance, LPCDLGTEMPLATE hDialogTemplate, HWND hWndParent, DLGPROC lpDialogFunc, LPARAM dwInitParam);

INT_PTR
CreateDialogWorker(
FCREATEDLG pfCreateFunc,
BOOL       bIsTemplateName,
HINSTANCE  hInstance,
LPCVOID    pvTemplateOrName,
HWND       hWndParent,
DLGPROC    lpDialogFunc,
LPARAM     dwInitParam
)
{
	INT_PTR Result;
	LOCALE_EMULATOR_THREAD_INFO *pInfo;

	pInfo = GetTlsData();

	if (bIsTemplateName)
		MByteToWCharStack(pvTemplateOrName, -1, pvTemplateOrName, NULL);

	pInfo->OldDlgProcA = lpDialogFunc;
	BeginWindowsHook(pInfo);
	Result = pfCreateFunc(
		hInstance,
		(LPCDLGTEMPLATEW)pvTemplateOrName,
		hWndParent,
		lpDialogFunc, // PreDialogProcW,
		dwInitParam);
	EndWindowsHook(pInfo);

	pInfo->OldDlgProcA = NULL;

	return Result;
}

INT_PTR
WINAPI
MyDialogBoxIndirectParamA(
HINSTANCE       hInstance,
LPCDLGTEMPLATE  hDialogTemplate,
HWND            hWndParent,
DLGPROC         lpDialogFunc,
LPARAM          dwInitParam
)
{
	return (INT_PTR)CreateDialogWorker((FCREATEDLG)DialogBoxIndirectParamW, FALSE, hInstance, hDialogTemplate, hWndParent, lpDialogFunc, dwInitParam);
}

HWND
WINAPI
MyCreateDialogIndirectParamA(
HINSTANCE       hInstance,
LPCDLGTEMPLATE  lpTemplate,
HWND            hWndParent,
DLGPROC         lpDialogFunc,
LPARAM          lParamInit
)
{
	return (HWND)CreateDialogWorker((FCREATEDLG)CreateDialogIndirectParamW, FALSE, hInstance, lpTemplate, hWndParent, lpDialogFunc, lParamInit);
}

INT_PTR
WINAPI
MyDialogBoxParamA(
HINSTANCE hInstance,
LPCTSTR   lpTemplateName,
HWND      hWndParent,
DLGPROC   lpDialogFunc,
LPARAM    dwInitParam
)
{
	return (INT_PTR)CreateDialogWorker((FCREATEDLG)DialogBoxParamW, TRUE, hInstance, lpTemplateName, hWndParent, lpDialogFunc, dwInitParam);
}

HWND
WINAPI
MyCreateDialogParamA(
HINSTANCE       hInstance,
LPCDLGTEMPLATE  lpTemplate,
HWND            hWndParent,
DLGPROC         lpDialogFunc,
LPARAM          lParamInit
)
{
	return (HWND)CreateDialogWorker((FCREATEDLG)CreateDialogParamW, TRUE, hInstance, lpTemplate, hWndParent, lpDialogFunc, lParamInit);
}

/************************************************************************/
/* End                                                                  */
/************************************************************************/

DWORD GetNlsSectionName(PWCHAR pszSectionName, ULONG CodePage)
{
	PWCHAR pszCodePage;
	DWORD  Length;
	static WCHAR s_szNlsSectionPrefix[] = L"\\NLS\\NlsSectionCP";

	CopyStruct(pszSectionName, s_szNlsSectionPrefix, sizeof(s_szNlsSectionPrefix) - sizeof(WCHAR));
	pszCodePage = pszSectionName + countof(s_szNlsSectionPrefix) - 1;
	for (UINT cp = CodePage; cp != 0; cp /= 10)
		++pszCodePage;

	*pszCodePage = 0;
	Length = pszCodePage - pszSectionName;
	for (UINT cp = CodePage; cp != 0; cp /= 10)
		*--pszCodePage = cp % 10 + '0';

	return Length;
}

NTSTATUS CreateNlsSection(ULONG CodePage, PHANDLE pSectionHandle)
{
	NTSTATUS Status;
	//    CSR_API_MESSAGE msg;
	RTL_OSVERSIONINFOW Version;

	Status = RtlGetVersion(&Version);
	if (!NT_SUCCESS(Status))
		return Status;

	if (Version.dwMajorVersion == 5)
	{
		CHAR MBytes;
		WCHAR WideChar;

		MBytes = ' ';
		MultiByteToWideChar(CodePage, 0, &MBytes, 1, &WideChar, 1);

		Status = 5;
	}
	else if (Version.dwMajorVersion == 6)
	{
		static CHAR pszApiName[] = "NtGetNlsSectionPtr";
		ANSI_STRING ApiName;

		ApiName.Length = sizeof(pszApiName) - 1;
		ApiName.MaximumLength = sizeof(pszApiName);
		ApiName.Buffer = pszApiName;
		NTSTATUS(NTAPI *pfNtGetNlsSectionPtr)(ULONG, ULONG, PVOID, PVOID, PSIZE_T);
		Status = LdrGetProcedureAddress(GetNtdllHandle(), &ApiName, 0, (PVOID *)&pfNtGetNlsSectionPtr);
		if (NT_SUCCESS(Status))
		{
			SIZE_T SectionSize;
			Status = pfNtGetNlsSectionPtr(0xB, CodePage, NULL, pSectionHandle, &SectionSize);
			if (NT_SUCCESS(Status))
				Status = 6;
		}
	}
	else
	{
		Status = STATUS_UNKNOWN_REVISION;
	}

	return Status;
}

BOOL InitCodePageTable(ULONG CodePage)
{
	PUSHORT           pDefaultCodePageTableBase;
	WCHAR             szSectionName[50];
	HANDLE            hSection;
	PUSHORT           pCodePageBase;
	SIZE_T            ViewSize;
	NTSTATUS          NtStatus;
	UNICODE_STRING    NlsSection;
	OBJECT_ATTRIBUTES ObjectAttributes;

	if (CodePage == CP_ACP || CodePage == CP_OEMCP)
		return FALSE;

	NlsSection.Buffer = szSectionName;
	NlsSection.Length = GetNlsSectionName(szSectionName, CodePage) * sizeof(*szSectionName);
	NlsSection.MaximumLength = sizeof(szSectionName);
	InitializeObjectAttributes(&ObjectAttributes, &NlsSection, OBJ_CASE_INSENSITIVE, NULL, NULL);
	NtStatus = NtOpenSection(&hSection, SECTION_MAP_READ, &ObjectAttributes);
	if (!NT_SUCCESS(NtStatus))
	{
		NtStatus = CreateNlsSection(CodePage, &hSection);
		if (NtStatus == 5)
		{
			NtStatus = NtOpenSection(&hSection, SECTION_MAP_READ, &ObjectAttributes);
			if (NT_SUCCESS(NtStatus))
				NtStatus = 5;
		}

		if (!NT_SUCCESS(NtStatus))
		{
			return FALSE;
		}
	}
	else
	{
		NtStatus = 5;
	}

	switch (NtStatus)
	{
	case 5:
		ViewSize = 0;
		pCodePageBase = NULL;
		NtStatus = NtMapViewOfSection(
			hSection,
			NtCurrentProcess(),
			(PVOID *)&pCodePageBase,
			0,
			0,
			NULL,
			&ViewSize,
			ViewUnmap,
			0,
			PAGE_READONLY);

		NtClose(hSection);

		if (!NT_SUCCESS(NtStatus))
			return FALSE;

		break;

	case 6:
		pCodePageBase = (PUSHORT)hSection;
		break;

	default:
		return FALSE;
	}

	g_pCodePageBase = pCodePageBase;
	RtlInitCodePageTable(pCodePageBase, &g_CodePageTable);

	pDefaultCodePageTableBase = Nt_GetDefaultCodePageBase();
	RtlInitCodePageTable(pDefaultCodePageTableBase, &g_DefaultCodePageTable);

	return TRUE;
}


BOOL InitLocaleInfoByCodePage(LOCALE_INFO *pInfo, ULONG CodePage)
{
	FillMemory(pInfo, sizeof(*pInfo), -1);

	pInfo->CodePage = CodePage;

	switch (CodePage)
	{
	case CP_SHIFTJIS:
		pInfo->DefaultSystemLangID = 0x411;
		pInfo->FontCharSet = SHIFTJIS_CHARSET;
		break;

	case CP_GB2312:
		pInfo->DefaultSystemLangID = 0x804;
		pInfo->FontCharSet = GB2312_CHARSET;
		break;

	case CP_BIG5:
		pInfo->DefaultSystemLangID = 0xC04;
		pInfo->FontCharSet = CHINESEBIG5_CHARSET;
		break;

	default:
		pInfo->FontCharSet = DEFAULT_CHARSET;
		break;
	}

	return TRUE;
}

BOOL BeginLocalEmulator(ULONG CodePage)
{
	//    DWORD   TlsIndex;
	HMODULE hModule;
	NTSTATUS Status;
	INITCOMMONCONTROLSEX icex;

#if 0
	if (!InitCodePageTable(CodePage))
	{
		MessageBoxW(NULL, L"Failed to Initialize CodePage Table", LECommonErrorHeader, MB_OK | MB_ICONERROR);
		return FALSE;
	}
#endif

	if (g_hHeap == NULL)
	{
		g_hHeap = RtlCreateHeap(0, NULL, 0, 0, NULL, NULL);
		if (g_hHeap == NULL)
		{
			MessageBoxW(NULL, L"Failed to Create The Private Heap", LECommonErrorHeader, MB_OK | MB_ICONERROR);
			return FALSE;
		}
	}

	//    TlsIndex = g_TlsIndex;
	//    if (TlsIndex == TLS_OUT_OF_INDEXES)
	{
		UNICODE_STRING AtomString = RTL_CONSTANT_STRING(LOCALE_EMULATOR_PROP_NAME);
		if (g_PropAtom == 0)
		{
			Status = NtAddAtom(AtomString.Buffer, AtomString.Length, &g_PropAtom);
			if (!NT_SUCCESS(Status))
			{
				MessageBoxW(NULL, L"Failed to Add Atom", 
					LECommonErrorHeader, MB_OK | MB_ICONERROR);
				return FALSE;
			}
		}

		//        TlsIndex = TlsAlloc();
		//        if (TlsIndex == TLS_OUT_OF_INDEXES)
		//            return FALSE;

		//        g_TlsIndex = TlsIndex;
	}

	InitLocaleInfoByCodePage(&g_LocaleInfo, CodePage);

	icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icex.dwICC = 0xFFFF;
	InitCommonControlsEx(&icex);

	DWORD OldProtect;
	//VirtualProtect(&NlsAnsiCodePage, 4, PAGE_EXECUTE_READWRITE, &OldProtect);
	//NlsAnsiCodePage = CodePage;

	//VirtualProtect(&NlsMbCodePageTag, 1, PAGE_EXECUTE_READWRITE, &OldProtect);
	//NlsMbCodePageTag = TRUE;

	PVOID pMByteToWChar, pWCharToMByte;
	UNICODE_STRING DllName;
	RTL_OSVERSIONINFOW Version;

	Status = RtlGetVersion(&Version);
	if (!NT_SUCCESS(Status))
	{
		MessageBoxW(NULL, L"Failed to Get Version by NTDLL", LECommonErrorHeader, MB_OK | MB_ICONERROR);
		return FALSE;
	}

	switch (Version.dwMajorVersion)
	{
	case 5:
		pMByteToWChar = MultiByteToWideChar;
		pWCharToMByte = WideCharToMultiByte;
		break;

		//NT 10 Refers to Windows 10?
	case 6: case 10:
		RTL_CONST_STRING(DllName, L"KERNELBASE.dll");
		Status = LdrGetDllHandle((PWORD)1, 0, &DllName, (PVOID *)&hModule);
		if (!NT_SUCCESS(Status))
		{
			MessageBoxW(NULL, L"Failed to Get Handle for \"KernelBase.dll\"", 
				LECommonErrorHeader, MB_OK | MB_ICONERROR);
			return FALSE;
		}

		pMByteToWChar = GetProcAddress(hModule, "MultiByteToWideChar");
		pWCharToMByte = GetProcAddress(hModule, "WideCharToMultiByte");
		break;

	default:
		MessageBoxW(NULL, L"Unknown NT Version", 
			LECommonErrorHeader, MB_OK | MB_ICONERROR);
		return FALSE;
	}

#if 0
	RTL_CONST_STRING(DllName, L"GDI32.dll");
	Status = LdrLoadDll(NULL, 0, &DllName, (PVOID *)&hModule);
	if (!NT_SUCCESS(Status))
	{
		MessageBoxW(NULL, L"Failed to Load \"GDI32.dll\"", LECommonErrorHeader, MB_OK | MB_ICONERROR);
		return FALSE;
	}
#endif

	MIN_MEMORY_FUNCTION_PATCH f[] =
	{
#if 0
		{ GetProcAddress(hModule, "GdiGetCodePage"), MyGdiGetCodePage, NULL },
		{ EnumFontFamiliesA, MyEnumFontFamiliesA, NULL },
		{ EnumFontFamiliesExA, MyEnumFontFamiliesExA, NULL },
		{ CreateFontIndirectA, MyCreateFontIndirectA, NULL },
#endif 

		// user32
		{ SetWindowTextA, MySetWindowTextA, NULL },
		{ GetWindowTextA, MyGetWindowTextA, NULL },

		//{ CreateWindowExA, MyCreateWindowExA, NULL },

#if 0
		{ DialogBoxParamA, MyDialogBoxParamA, NULL },
		{ CreateDialogParamA, MyCreateDialogParamA, NULL },
		{ DialogBoxIndirectParamA, MyDialogBoxIndirectParamA, NULL },

		{ CreateDialogIndirectParamA, MyCreateDialogIndirectParamA, NULL },

		{ CallWindowProcA, MyCallWindowProcA, (PVOID*)&OldCallWindowProcA },
		{ DefWindowProcA, MyDefWindowProcA, NULL },

		{ DefMDIChildProcA, MyDefMDIChildProcA, NULL },
		{ DefDlgProcA, MyDefDlgProcA, NULL },
		{ DefFrameProcA, MyDefFrameProcA, NULL },
#endif


#if 0
		{ PostMessageA, MyPostMessageA, NULL },
		{ SendMessageA, MySendMessageA, NULL },
		{ SendNotifyMessageA, MySendNotifyMessageA, NULL },
		{ SendMessageCallbackA, MySendMessageCallbackA, NULL },
		{ SendMessageTimeoutA, MySendMessageTimeoutA, NULL },
#endif

#if 0

		{ SetWindowLongA, MySetWindowLongA, (PVOID*)&OldSetWindowLongA },
		{ GetWindowLongA, MyGetWindowLongA, (PVOID*)&OldGetWindowLongA },
		{ SetWindowLongW, MySetWindowLongW, (PVOID*)&OldSetWindowLongW },
		{ GetWindowLongW, MyGetWindowLongW, (PVOID*)&OldGetWindowLongW },
		//{ IsWindowUnicode, MyIsWindowUnicode, (PVOID*)&OldIsWindowUnicode },
#endif

		// kernel32
#if PATCH_KERNEL32
		{ GetACP, MyGetACP, NULL },
		{ GetSystemDefaultLangID, MyGetSystemDefaultLangID, (PVOID*)&OldGetSystemDefaultLangID },
		{ pWCharToMByte, MyWideCharToMultiByte, (PVOID*)&OldWideCharToMultiByte },
		{ pMByteToWChar, MyMultiByteToWideChar, (PVOID*)&OldMultiByteToWideChar },
		{ IsDBCSLeadByte, MyIsDBCSLeadByte },
		{ CreateProcessInternalA, MyCreateProcessInternalA, NULL },
		{ CreateProcessInternalW, MyCreateProcessInternalW, (PVOID*)&OldCreateProcessInternalW },
#endif

#if 0
		// ntdll
		{ RtlMultiByteToUnicodeN, MyRtlMultiByteToUnicodeN, NULL },
		{ RtlUnicodeToMultiByteN, MyRtlUnicodeToMultiByteN, NULL },
		{ RtlMultiByteToUnicodeSize, MyRtlMultiByteToUnicodeSize, NULL },
		{ RtlUnicodeToMultiByteSize, MyRtlUnicodeToMultiByteSize, NULL },
#endif
	};

	Nt_PatchMemory(f, countof(f));

	return TRUE;
}

BOOL ReleaseThreadLocalStorage()
{
	//    if (g_TlsIndex == TLS_OUT_OF_INDEXES)
	//        return FALSE;

	LOCALE_EMULATOR_THREAD_INFO *pInfo;

	pInfo = GetTlsData(FALSE);
	if (pInfo == NULL)
		return FALSE;

	RtlPopFrame(pInfo);

	return RtlFreeHeap(g_hHeap, 0, pInfo);
}
