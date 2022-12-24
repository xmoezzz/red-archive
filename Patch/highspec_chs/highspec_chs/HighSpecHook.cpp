#include "HighSpecHook.h"
#include "MinHook.h"
#include "tp_stub.h"
#include "IStreamAdapter.h"
#include "NtDefine.h"
#include <new>
#include <string>
#include <WinFile.h>

static std::wstring PathPrefix(L"ProjectDir\\");

HighSpecHook* HighSpecHook::m_Handle = nullptr;


HighSpecHook::HighSpecHook() :
	m_HookInited(FALSE),
	OldCreateIStream(nullptr),
	OldV2Link(nullptr),
	XmoeTVPFunctionExporter(nullptr),
	OldMultiByteToWideChar(nullptr)
{
}


DWORD NTAPI OutputString(LPCWSTR Info)
{
	DWORD  nRet;
	HANDLE ConsoleHandle;

	ConsoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	WriteConsoleW(ConsoleHandle, Info, lstrlenW(Info), &nRet, 0);
	WriteConsoleW(ConsoleHandle, L"\n", 1, &nRet, 0);

	return nRet;
}

HighSpecHook* HighSpecHook::GetData()
{
	LOOP_ONCE
	{
		if (m_Handle || MH_Initialize() != MH_OK)
			break;
		
		m_Handle = new HighSpecHook;
	}
	return m_Handle;
}


HRESULT WINAPI HookV2Link(iTVPFunctionExporter *exporter)
{
	TVPInitImportStub(exporter);
	HighSpecHook::GetData()->XmoeTVPFunctionExporter = exporter;
	HighSpecHook::GetData()->HookCreateStream();

	return HighSpecHook::GetData()->OldV2Link(exporter);
}


NTSTATUS HighSpecHook::InitHook(LPCSTR ModuleName, HMODULE ImageBase)
{
	ULONG_PTR   ExtensionName;
	PVOID       TargetV2Link;

	if (m_HookInited == TRUE || ImageBase == NULL)
		return STATUS_ALREADY_REGISTERED;
	
	ExtensionName =  *(PULONG_PTR)&ModuleName[lstrlenA(ModuleName) - 4];

	if (ExtensionName != TAG4('.dll') && ExtensionName != TAG4('.tpm'))
		return STATUS_UNSUCCESSFUL;

	OldV2Link    = (FuncV2Link)GetProcAddress(ImageBase, "V2Link");
	TargetV2Link = OldV2Link;

	if (!OldV2Link)
		return STATUS_UNSUCCESSFUL;

	MH_CreateHook(TargetV2Link, HookV2Link, (PVOID*)&OldV2Link);
	MH_EnableHook(TargetV2Link);
	m_HookInited = TRUE;

	return STATUS_SUCCESS;
}

IStream* NTAPI HookTVPCreateStream(const ttstr & _name, tjs_uint32 flags)
{
	static WCHAR*     Prefix = L"archive://";
	IStream*          Stream;

	Stream = NULL;
	if ((!wcsncmp(_name.c_str(), Prefix, lstrlenW(Prefix))))
	{
		OutputString(_name.c_str());
		Stream = HighSpecHook::GetData()->LocalCreateStream(_name, flags);
	}

	if (!Stream)
		Stream = HighSpecHook::GetData()->OldCreateIStream(_name, flags);

	return Stream;
}

NTSTATUS HighSpecHook::HookCreateStream()
{
	LPVOID Target;

	if (XmoeTVPFunctionExporter == nullptr)
	{
		MessageBoxW(NULL, L"Bad Refer", L"X'moe CoreLib", MB_OK);
		ExitProcess(-1);
	}

	static char szCreateIStream[] = "IStream * ::TVPCreateIStream(const ttstr &,tjs_uint32)";
	OldCreateIStream = (FuncCreateIStream)TVPGetImportFuncPtr(szCreateIStream);
	Target = OldCreateIStream;

	if (OldCreateIStream == nullptr)
	{
		MessageBoxW(NULL, L"Bad Refer", L"X'moe CoreLib", MB_OK);
		ExitProcess(-1);
	}

	MH_CreateHook(Target, HookTVPCreateStream, (PVOID*)&OldCreateIStream);
	MH_EnableHook(Target);

	return STATUS_SUCCESS;
}


std::wstring GetFileName(const WCHAR* Name)
{
	std::wstring Info(Name);
	std::wstring Result;

	if (Info.find_last_of(L">") != std::wstring::npos)
		Result = Info.substr(Info.find_last_of(L">") + 1, std::wstring::npos);
	else
		Result = Info.substr(Info.find_last_of(L"/") + 1, std::wstring::npos);
	
	return Result;
}


IStream* HighSpecHook::LocalCreateStream(const ttstr & _name, tjs_uint32 flags)
{
	ULONG                 FileSize;
	PBYTE                 FileBuffer;
	WCHAR                 CurrentPath[MAX_PATH];
	std::wstring          FileName;
	std::wstring          FullFileName;
	WinFile               File;
	StreamHolder*         Holder;
	IStreamAdapter*       StreamAdapter;

	RtlZeroMemory(CurrentPath, _countof(CurrentPath) * sizeof(CurrentPath[0]));
	GetCurrentDirectoryW(MAX_PATH, CurrentPath);
	FileName = GetFileName(_name.c_str());
	FullFileName = CurrentPath;
	FullFileName += L"\\";
	FullFileName += PathPrefix;
	FullFileName += FileName;

	LOOP_ONCE
	{
		StreamAdapter = nullptr;

		if (File.Open(FullFileName.c_str(), WinFile::FileRead) != S_OK)
			break;

		FileSize = File.GetSize32();
		FileBuffer = (PBYTE)HeapAlloc(GetProcessHeap(), 0, FileSize);

		if (!FileBuffer)
			break;

		File.Read(FileBuffer, FileSize);

		Holder        = new StreamHolder(FileBuffer, FileSize);
		StreamAdapter = new IStreamAdapter(Holder);
	}
	File.Release();

	return StreamAdapter;
}


HMODULE WINAPI HookLoadLibraryW(LPCWSTR lpFileName)
{
	HMODULE       hModule;
	CHAR          AnsiName[MAX_PATH];

	RtlZeroMemory(AnsiName, MAX_PATH);
	hModule = LoadLibraryW(lpFileName);
	WideCharToMultiByte(CP_ACP, NULL, lpFileName, -1, AnsiName, MAX_PATH, NULL, NULL);
	HighSpecHook::GetData()->InitHook(AnsiName, hModule);

	return hModule;
}


int WINAPI HookMultiByteToWideCharInline(
	_In_      UINT   CodePage,
	_In_      DWORD  dwFlags,
	_In_      LPCSTR lpMultiByteStr,
	_In_      int    cbMultiByte,
	_Out_opt_ LPWSTR lpWideCharStr,
	_In_      int    cchWideChar
	)
{
	switch(CodePage)
	{
	case CP_ACP: 
	case CP_THREAD_ACP:
		return HighSpecHook::GetData()->OldMultiByteToWideChar(932, dwFlags, lpMultiByteStr,
			cbMultiByte, lpWideCharStr, cchWideChar);
	
	default:
		return HighSpecHook::GetData()->OldMultiByteToWideChar(CodePage, dwFlags, lpMultiByteStr,
			cbMultiByte, lpWideCharStr, cchWideChar);
	}
}


NTSTATUS HighSpecHook::StartHook(LPCSTR szDllName, PROC pfnOrg, PROC pfnNew)
{
	HMODULE                  hmod;
	LPCSTR                   szLibName;
	PIMAGE_IMPORT_DESCRIPTOR pImportDesc;
	PIMAGE_THUNK_DATA        pThunk;
	DWORD                    dwOldProtect, dwRVA;
	PBYTE                    pAddr;

	hmod   =  GetModuleHandleW(NULL);
	pAddr  =  (PBYTE)hmod;
	pAddr  += *((DWORD*)&pAddr[0x3C]);
	dwRVA  =  *((DWORD*)&pAddr[0x80]);

	pImportDesc = (PIMAGE_IMPORT_DESCRIPTOR)((DWORD)hmod + dwRVA);

	for (; pImportDesc->Name; pImportDesc++)
	{
		szLibName = (LPCSTR)((DWORD)hmod + pImportDesc->Name);
		if (!_stricmp(szLibName, szDllName))
		{
			pThunk = (PIMAGE_THUNK_DATA)((DWORD)hmod + pImportDesc->FirstThunk);
			for (; pThunk->u1.Function; pThunk++)
			{
				if (pThunk->u1.Function == (DWORD)pfnOrg)
				{
					VirtualProtect((LPVOID)&pThunk->u1.Function, 4, PAGE_EXECUTE_READWRITE, &dwOldProtect);
					pThunk->u1.Function = (DWORD)pfnNew;
					VirtualProtect((LPVOID)&pThunk->u1.Function, 4, dwOldProtect, &dwOldProtect);
					return STATUS_SUCCESS;
				}
			}
		}
	}
	return STATUS_UNSUCCESSFUL;
}

NTSTATUS HighSpecHook::InitNativeHook()
{
	PVOID          Target;

	AllocConsole();

	if (!NT_SUCCESS(StartHook("KERNEL32.dll", 
		GetProcAddress(GetModuleHandleW(L"KERNEL32.dll"), "LoadLibraryW"), 
		(PROC)HookLoadLibraryW)))
	{
		MessageBoxW(NULL, L"∆Ù∂Ø ß∞‹", L"…Ò‘¬Ω„Ω„∫√√»∞°", MB_OK);
		return STATUS_NOT_FOUND;
	}

	OldMultiByteToWideChar = (API_POINTER(MultiByteToWideChar))
		GetProcAddress(GetModuleHandleW(L"KERNEL32.dll"), "MultiByteToWideChar");
	Target = OldMultiByteToWideChar;

	if (!Target)
	{
		MessageBoxW(NULL, L"∆Ù∂Ø ß∞‹", L"…Ò‘¬Ω„Ω„∫√√»∞°", MB_OK);
		return STATUS_NOT_FOUND;
	}

	MH_CreateHook(Target, HookMultiByteToWideCharInline, (PVOID*)&OldMultiByteToWideChar);
	MH_EnableHook(Target);

	return STATUS_SUCCESS;
}

