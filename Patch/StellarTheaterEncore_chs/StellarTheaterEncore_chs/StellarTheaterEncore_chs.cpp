#include "my.h"
#include "LocaleEmulator.h"
#include "cxdec.h"
#include <hash_map>
#include <string>
#include <set>

//#pragma comment(linker, "/ENTRY:DllMainEntry")
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


class tTJSCriticalSection
{
	CRITICAL_SECTION CS;
public:
	tTJSCriticalSection()  { InitializeCriticalSection(&CS); }
	~tTJSCriticalSection() { DeleteCriticalSection(&CS); }

	void Enter() { EnterCriticalSection(&CS); }
	void Leave() { LeaveCriticalSection(&CS); }
};

class tTJSCriticalSectionHolder
{
	tTJSCriticalSection *Section;
public:
	tTJSCriticalSectionHolder(tTJSCriticalSection &cs)
	{
		Section = &cs;
		Section->Enter();
	}

	~tTJSCriticalSectionHolder()
	{
		Section->Leave();
	}

};

std::set<HANDLE> HandleTable;

tTJSCriticalSection HandleSection;

Void FASTCALL AddHandle(HANDLE Handle)
{
	tTJSCriticalSectionHolder Holder(HandleSection);

	HandleTable.insert(Handle);
}


Void FASTCALL RemoveHandle(HANDLE Handle)
{
	tTJSCriticalSectionHolder Holder(HandleSection);

	auto Index = HandleTable.find(Handle);
	if (Index != HandleTable.end())
		HandleTable.erase(Index);
}


Bool FASTCALL FindHandle(HANDLE Handle)
{
	tTJSCriticalSectionHolder Holder(HandleSection);

	auto Index = HandleTable.find(Handle);
	return Index != HandleTable.end() ? TRUE : FALSE;
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

	if (!StrICompareW(PureFileName, L"system.dat", StrCmp_ToLower))
	{
		HANDLE Handle = StubCreateFileW(L"system_chs.dat", dwDesiredAccess, dwShareMode, lpSecurityAttributes,
			dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);

		if (Handle != INVALID_HANDLE_VALUE)
			AddHandle(Handle);

		return Handle;
	}

	return StubCreateFileW(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes,
		dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
}


API_POINTER(CloseHandle) StubCloseHandle = NULL;

BOOL
WINAPI
HookCloseHandle(
_In_ HANDLE hObject
)
{
	if (hObject != INVALID_HANDLE_VALUE && FindHandle(hObject))
		RemoveHandle(hObject);

	return StubCloseHandle(hObject);
}

API_POINTER(ReadFile) StubReadFile = NULL;

BOOL
WINAPI
HookReadFile(
_In_ HANDLE hFile,
LPVOID lpBuffer,
_In_ DWORD nNumberOfBytesToRead,
_Out_opt_ LPDWORD lpNumberOfBytesRead,
_Inout_opt_ LPOVERLAPPED lpOverlapped
)
{
	BOOL                     Result;
	DWORD                    Offset;
	IO_STATUS_BLOCK          IoStatus;
	PFILE_NAME_INFORMATION   FileInfo;
	ULONG                    AllocSize;


	auto IsSceneChsPack = [](LPCWSTR FileName)->BOOL
	{
		ULONG_PTR iPos = 0;

		for (ULONG i = 0; i < StrLengthW(FileName); i++)
		{
			if (FileName[i] == L'\\' || FileName[i] == L'/')
				iPos = i;
		}

		if (iPos != 0)
			iPos++;

		return StrCompareW(FileName + iPos, L"system_chs.dat") == 0;
	};

	Offset = SetFilePointer(hFile, 0, 0, FILE_CURRENT);
	AllocSize = sizeof(OBJECT_NAME_INFORMATION) + MAX_PATH * sizeof(WCHAR);
	FileInfo = (PFILE_NAME_INFORMATION)AllocateMemoryP(AllocSize, HEAP_ZERO_MEMORY);
	NtQueryInformationFile(hFile, &IoStatus, FileInfo, AllocSize, FileNameInformation);
	Result = StubReadFile(hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, lpOverlapped);
	if (hFile != INVALID_HANDLE_VALUE && FindHandle(hFile))
	{
		if (IsSceneChsPack(FileInfo->FileName))
		{
			CXDEC_INDO Info;
			Info.Buffer = (PBYTE)lpBuffer;
			Info.BufferSize = nNumberOfBytesToRead;
			Info.FileHash = 114514;
			Info.Offset = Offset;
			ArchiveExtractionFilter(&Info);
		}
	}
	FreeMemoryP(FileInfo);
	return Result;
}

API_POINTER(ReadFileEx) StubReadFileEx = NULL;


BOOL
WINAPI
HookReadFileEx(
_In_ HANDLE hFile,
LPVOID lpBuffer,
_In_ DWORD nNumberOfBytesToRead,
_Inout_ LPOVERLAPPED lpOverlapped,
_In_ LPOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
)
{
	BOOL                     Result;
	DWORD                    Offset;
	IO_STATUS_BLOCK          IoStatus;
	PFILE_NAME_INFORMATION   FileInfo;
	ULONG                    AllocSize;


	auto IsSceneChsPack = [](LPCWSTR FileName)->BOOL
	{
		ULONG_PTR iPos = 0;

		for (ULONG i = 0; i < StrLengthW(FileName); i++)
		{
			if (FileName[i] == L'\\' || FileName[i] == L'/')
				iPos = i;
		}

		if (iPos != 0)
			iPos++;

		return StrCompareW(FileName + iPos, L"system_chs.dat") == 0;
	};

	Offset = SetFilePointer(hFile, 0, 0, FILE_CURRENT);
	AllocSize = sizeof(OBJECT_NAME_INFORMATION) + MAX_PATH * sizeof(WCHAR);
	FileInfo = (PFILE_NAME_INFORMATION)AllocateMemoryP(AllocSize, HEAP_ZERO_MEMORY);
	NtQueryInformationFile(hFile, &IoStatus, FileInfo, AllocSize, FileNameInformation);
	Result = StubReadFileEx(hFile, lpBuffer, nNumberOfBytesToRead, lpOverlapped, lpCompletionRoutine);
	if (hFile != INVALID_HANDLE_VALUE && FindHandle(hFile))
	{
		if (IsSceneChsPack(FileInfo->FileName))
		{
			CXDEC_INDO Info;
			Info.Buffer = (PBYTE)lpBuffer;
			Info.BufferSize = nNumberOfBytesToRead;
			Info.FileHash = 114514;
			Info.Offset = Offset;
			ArchiveExtractionFilter(&Info);
		}
	}
	FreeMemoryP(FileInfo);
	return Result;
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


PCSTR NTAPI QueryText(PCSTR lpText)
{
	PrintConsoleA("%s\n", lpText);
	return NULL;
}


DWORD HookRenderTextEnd = 0x62EC85;

ASM VOID HookRenderText()
{
	INLINE_ASM
	{
		pushad;
		pushfd;
		
		mov ebx, dword ptr[ebp + 0xC];
		push ebx;
		call QueryText;
		test eax, eax;
		jz   NO_MODIFY;
		mov dword ptr[ebp + 0xC], eax;

	NO_MODIFY:
		popfd;
		popad;
		jmp HookRenderTextEnd;
	}
}


std::hash_map<std::string, std::string> ReplaceMap;

INT CDECL HookRenderText2(DWORD a1, char *a2);

API_POINTER(HookRenderText2) StubRenderText2 = NULL;
INT CDECL HookRenderText2(DWORD a1, char *a2)
{
	//PrintConsoleA("%s\n", a2);
	return StubRenderText2(a1, a2);
}


#define MAKE_HASH_PAIR(a, b) ReplaceMap.insert(std::make_pair(a, b))

ForceInline Void LoadHashTable()
{
	MAKE_HASH_PAIR("奍嵗偺惎嵗暈", "巨蟹座的星座服");
	MAKE_HASH_PAIR("恀敀", "真白");
	MAKE_HASH_PAIR("崟斅偺暥帤", "黑板上的文字");
	MAKE_HASH_PAIR("嶩寧偺儁儞", "皋月的笔");
	MAKE_HASH_PAIR("嶩寧偺庤巻", "皋月的信");
	MAKE_HASH_PAIR("渁俀", "彗２");
	MAKE_HASH_PAIR("嶩寧俀", "皋月２");
	MAKE_HASH_PAIR("僜儔", "ＳＯＲＡ");
	MAKE_HASH_PAIR("偨偄從偒", "鲷鱼烧");
	MAKE_HASH_PAIR("嶩寧偺鋾暱", "皋月的草莓内裤");
	MAKE_HASH_PAIR("惎垷", "星亚");
	MAKE_HASH_PAIR("婸栭", "辉夜");
	MAKE_HASH_PAIR("擫", "猫");
	MAKE_HASH_PAIR("揤壒", "天音");
	MAKE_HASH_PAIR("枺棳", "魅流");
	MAKE_HASH_PAIR("渁", "彗");
	MAKE_HASH_PAIR("嬧堦榊", "银一郎");
	MAKE_HASH_PAIR("梲壞", "阳夏");
	MAKE_HASH_PAIR("幍惎", "七星");
	MAKE_HASH_PAIR("儊僈僱", "眼镜");
	MAKE_HASH_PAIR("憢", "窗户");
	MAKE_HASH_PAIR("惎垷偺僞僆儖", "星亚的毛巾");
	MAKE_HASH_PAIR("惎垷偺悈拝", "星亚的泳装");
	MAKE_HASH_PAIR("旜偺敀偄崟擫", "尾巴是白色的黑猫");
	MAKE_HASH_PAIR("壻巕偺戃", "点心包装袋");
	MAKE_HASH_PAIR("偆偝偓偺偸偄偖傞傒", "兔子布偶");
	MAKE_HASH_PAIR("堷偭憕偒彎", "挠伤");
	MAKE_HASH_PAIR("抋惗擔", "生日");
	MAKE_HASH_PAIR("憮", "苍");
	MAKE_HASH_PAIR("崟嵠", "黑哉");
	MAKE_HASH_PAIR("柧梩", "明叶");
	MAKE_HASH_PAIR("嬻偺僨乕僩暈", "空的约会服");
	MAKE_HASH_PAIR("僑僗儘儕暈", "哥特萝莉服");
	MAKE_HASH_PAIR("昦堾", "医院");
	MAKE_HASH_PAIR("幍梉", "七夕");
	MAKE_HASH_PAIR("嬻", "空");
	MAKE_HASH_PAIR("嶩寧", "皋月");
	MAKE_HASH_PAIR("戝抧", "大地");
	MAKE_HASH_PAIR("崟擫", "黑猫");
	MAKE_HASH_PAIR("娤梩怉暔", "观叶植物");
	MAKE_HASH_PAIR("惎垷偺僞僆儖", "星亚的毛巾");
	MAKE_HASH_PAIR("濄", "昴");
}

BOOL NTAPI Initialization(HMODULE hModule)
{
	NTSTATUS Status;

	Status = ml::MlInitialize();
	if (NT_FAILED(Status))
		return FALSE;

	Mp::PATCH_MEMORY_DATA p[] =
	{
		Mp::FunctionJumpVa( CreateFileA, HookCreateFileA, NULL ),
		Mp::FunctionJumpVa( CreateFileW, HookCreateFileW, &StubCreateFileW ),
		Mp::FunctionJumpVa(ReadFile,     HookReadFile,    &StubReadFile),
		Mp::FunctionJumpVa(ReadFileEx,   HookReadFileEx,  &StubReadFileEx),
		Mp::FunctionJumpVa(CloseHandle,  HookCloseHandle, &StubCloseHandle),
		//Mp::FunctionJumpVa( 0x62EC7F,    HookRenderText,  NULL)
		Mp::FunctionJumpVa(0x62EC7C, HookRenderText2, &StubRenderText2)
	};

	LOOP_ONCE
	{
		Status = Mp::PatchMemory(p, countof(p));
		if (NT_FAILED(Status))
			break;

		Status = BeginLocalEmulator(936);
		if (NT_FAILED(Status))
			break;
	}

	LoadHashTable();

	return NT_SUCCESS(Status);
}


BOOL NTAPI UnInitialization(HMODULE hModule)
{
	UNREFERENCED_PARAMETER(hModule);
	return TRUE;
}


//BOOL NTAPI DllMainEntry(HMODULE hModule, DWORD Reason, LPVOID lpReserved)
BOOL NTAPI DllMain(HMODULE hModule, DWORD Reason, LPVOID lpReserved)
{
	switch (Reason)
	{
	case DLL_PROCESS_ATTACH:
		//AllocConsole();
		return Initialization(hModule);
	case DLL_PROCESS_DETACH:
		return UnInitialization(hModule);
	}
	return TRUE;
}
