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
	MAKE_HASH_PAIR("�I���̐�����", "��з����������");
	MAKE_HASH_PAIR("�^��", "���");
	MAKE_HASH_PAIR("���̕���", "�ڰ��ϵ�����");
	MAKE_HASH_PAIR("�H���̃y��", "���µı�");
	MAKE_HASH_PAIR("�H���̎莆", "���µ���");
	MAKE_HASH_PAIR("�a�Q", "�磲");
	MAKE_HASH_PAIR("�H���Q", "���£�");
	MAKE_HASH_PAIR("�\��", "�ӣϣң�");
	MAKE_HASH_PAIR("�����Ă�", "������");
	MAKE_HASH_PAIR("�H����䕕�", "���µĲ�ݮ�ڿ�");
	MAKE_HASH_PAIR("����", "����");
	MAKE_HASH_PAIR("�P��", "��ҹ");
	MAKE_HASH_PAIR("�L", "è");
	MAKE_HASH_PAIR("�V��", "����");
	MAKE_HASH_PAIR("����", "����");
	MAKE_HASH_PAIR("�a", "��");
	MAKE_HASH_PAIR("���Y", "��һ��");
	MAKE_HASH_PAIR("�z��", "����");
	MAKE_HASH_PAIR("����", "����");
	MAKE_HASH_PAIR("���K�l", "�۾�");
	MAKE_HASH_PAIR("��", "����");
	MAKE_HASH_PAIR("�����̃^�I��", "���ǵ�ë��");
	MAKE_HASH_PAIR("�����̐���", "���ǵ�Ӿװ");
	MAKE_HASH_PAIR("���̔������L", "β���ǰ�ɫ�ĺ�è");
	MAKE_HASH_PAIR("�َq�̑�", "���İ�װ��");
	MAKE_HASH_PAIR("�������̂ʂ������", "���Ӳ�ż");
	MAKE_HASH_PAIR("�����~����", "����");
	MAKE_HASH_PAIR("�a����", "����");
	MAKE_HASH_PAIR("��", "��");
	MAKE_HASH_PAIR("����", "����");
	MAKE_HASH_PAIR("���t", "��Ҷ");
	MAKE_HASH_PAIR("��̃f�[�g��", "�յ�Լ���");
	MAKE_HASH_PAIR("�S�X������", "���������");
	MAKE_HASH_PAIR("�a�@", "ҽԺ");
	MAKE_HASH_PAIR("���[", "��Ϧ");
	MAKE_HASH_PAIR("��", "��");
	MAKE_HASH_PAIR("�H��", "����");
	MAKE_HASH_PAIR("��n", "���");
	MAKE_HASH_PAIR("���L", "��è");
	MAKE_HASH_PAIR("�ϗt�A��", "��Ҷֲ��");
	MAKE_HASH_PAIR("�����̃^�I��", "���ǵ�ë��");
	MAKE_HASH_PAIR("��", "��");
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
