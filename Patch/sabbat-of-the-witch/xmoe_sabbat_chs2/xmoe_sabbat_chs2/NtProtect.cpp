#include "NtProtect.h"
#include "WinFile.h"


#pragma pack(1)
typedef struct _UNICODE_STRING
{// UNICODE_STRING structure  
	USHORT Length;
	USHORT MaximumLength;
	PWSTR  Buffer;
} UNICODE_STRING;
typedef UNICODE_STRING *PUNICODE_STRING;
#pragma pack()


typedef struct
{
	HMODULE hExeModule;
	HMODULE hDllModule;
}NtInfo, *pNtInfo;


typedef NTSTATUS(WINAPI *pfLdrLoadDll) //LdrLoadDll function prototype  
(
IN PWCHAR PathToFile OPTIONAL,
IN ULONG Flags OPTIONAL,
IN PUNICODE_STRING ModuleFileName,
OUT PHANDLE ModuleHandle
);

LPVOID StubLdrLoadDll = nullptr;

WCHAR* KrkrInfo1 = L"KrkrExtract";
WCHAR* KrkrInfo2 = L"XP3Viewer";


HRESULT WINAPI FakeIATSection(PBYTE Buffer)
{
	HMODULE hModule = GetModuleHandleW(NULL);
	IMAGE_DOS_HEADER* pDosHeader = (IMAGE_DOS_HEADER*)hModule;

	PIMAGE_NT_HEADERS pPeHeaders = NULL;
	pPeHeaders = (PIMAGE_NT_HEADERS)((LONG)hModule + pDosHeader->e_lfanew);
	IMAGE_SECTION_HEADER* section = IMAGE_FIRST_SECTION(pPeHeaders);
	PBYTE pStart = (PBYTE)IMAGE_FIRST_SECTION(pPeHeaders);
	ULONG dwNumberOfSection = pPeHeaders->FileHeader.NumberOfSections;
	IMAGE_SECTION_HEADER* pCodeSection = IMAGE_FIRST_SECTION(pPeHeaders);
	PBYTE pCodeSectBuffer = (PBYTE)pCodeSection;
	for (int j = 0; j < dwNumberOfSection; j++)
	{
		if (RtlCompareMemory(pCodeSectBuffer, ".xmoe", 5)== 5|| 
			RtlCompareMemory(pCodeSectBuffer, ".Xmoe", 5)== 5||
			RtlCompareMemory(pCodeSectBuffer, ".Amano", lstrlenA(".Amano")) == lstrlenA(".Amano") ||
			RtlCompareMemory(pCodeSectBuffer, ".Amano2", lstrlenA(".Amano2")) == lstrlenA(".Amano2"))
		{
			//NtPrintf("%s\n", pCodeSectBuffer);
			return S_FALSE;
		}
		pCodeSectBuffer += IMAGE_SIZEOF_SECTION_HEADER;
	}
	return S_OK;
}

ULONG  GetFileLen2(LPVOID pBaseaddr, LPVOID pReadBuf)
{
	LPBYTE pBase = (LPBYTE)pBaseaddr;
	PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)pReadBuf;
	ULONG uSize = PIMAGE_OPTIONAL_HEADER((pBase + pDosHeader->e_lfanew + 4 + 20))->SizeOfHeaders;
	PIMAGE_SECTION_HEADER    pSec = (PIMAGE_SECTION_HEADER)(pBase + pDosHeader->e_lfanew + sizeof(IMAGE_NT_HEADERS));
	for (int i = 0; i<PIMAGE_FILE_HEADER(pBase + pDosHeader->e_lfanew + 4)->NumberOfSections; ++i)
	{
		uSize += pSec[i].SizeOfRawData;
	}
	return uSize;
}

NTSTATUS WINAPI HookLdrLoadDll(IN PWCHAR PathToFile OPTIONAL,
	IN ULONG Flags OPTIONAL,
	IN PUNICODE_STRING ModuleFileName,
	OUT PHANDLE ModuleHandle)
{
	//WinFile DllFile;
	NTSTATUS NtResult =0;

	//WCHAR PathName[260] = { 0 };
	//RtlCopyMemory(PathName, PathToFile ? PathToFile : ModuleFileName->Buffer,
	//	PathToFile ? lstrlenW(PathToFile) : ModuleFileName->Length);

#if 0
	if (DllFile.Open(PathName, WinFile::FileRead) == S_OK)
	{
		PBYTE Image = (PBYTE)VirtualAlloc(NULL, DllFile.GetSize32(), MEM_COMMIT, PAGE_READWRITE);
		if (Image != nullptr)
		{
			DllFile.Read(Image, DllFile.GetSize32());
			if (FakeIATSection(Image) != S_OK)
			{
				NtResult = 0xFFFFFFFF;
				VirtualFree(Image, NULL, MEM_RELEASE);
			}
			else
			{
				ULONG KrkrLen1 = lstrlenW(KrkrInfo1) * 2;
				ULONG KrkrLen2 = lstrlenW(KrkrInfo2) * 2;
				for (ULONG iPos = 0; iPos < DllFile.GetSize32() - max(lstrlenW(KrkrInfo1) * 2, lstrlenW(KrkrInfo2) * 2); iPos++)
				{
					if (RtlCompareMemory(Image + iPos, (PBYTE)KrkrInfo1, KrkrLen1) == KrkrLen1)
					{
						NtResult = 0xFFFFFFFF;
						break;
					}
					else if (RtlCompareMemory(Image + iPos, (PBYTE)KrkrInfo2, KrkrLen2) == KrkrLen2)
					{
						NtResult = 0xFFFFFFFF;
						break;
					}
				}
				VirtualFree(Image, NULL, MEM_RELEASE);
			}
		}
		else
		{
			NtResult = 0XFFFFFFFF;
		}
		DllFile.Release();
	}
	else
	{
		NtResult = 0xFFFFFFFF;
	}
	if (NtResult == 0)
	{
		return(pfLdrLoadDll(StubLdrLoadDll))(PathToFile, Flags, ModuleFileName, ModuleHandle);
	}
	else
	{
		//MessageBoxW(NULL, L"错误", L"X'moe-CoreLib", MB_OK);
		*ModuleHandle = NULL;
		return 0xFFFFFFFF;
	}
#else
	NTSTATUS Result = (pfLdrLoadDll(StubLdrLoadDll))(PathToFile, Flags, ModuleFileName, ModuleHandle);
	return Result;

	/*
	HRESULT ResultCode = S_OK;
	if (Result >= 0)
	{
		ULONG Protect;
		VirtualProtect(*ModuleHandle, 0x1000, PAGE_EXECUTE_READWRITE, &Protect);
		if (IsBadReadPtr(*ModuleHandle, 0x1000))
		{
			goto ReturnImm;
		}
		if (FakeIATSection((PBYTE)*ModuleHandle) == S_OK)
		//if (true)
		{
			ULONG KrkrLen1 = lstrlenW(KrkrInfo1) * 2;
			ULONG KrkrLen2 = lstrlenW(KrkrInfo2) * 2;
			for (ULONG iPos = 0; iPos < GetFileLen2(*ModuleHandle, *ModuleHandle) - max(lstrlenW(KrkrInfo1) * 2, lstrlenW(KrkrInfo2) * 2); iPos++)
			{
				if (RtlCompareMemory((PBYTE)(*ModuleHandle) + iPos, (PBYTE)KrkrInfo1, KrkrLen1) == KrkrLen1)
				{
					ResultCode = S_FALSE;
					break;
				}
				else if (RtlCompareMemory((PBYTE)(*ModuleHandle) + iPos, (PBYTE)KrkrInfo2, KrkrLen2) == KrkrLen2)
				{
					ResultCode = S_FALSE;
					break;
				}
			}
		}
		else
		{
			ResultCode = S_FALSE;
		}
	}
	if (ResultCode != S_OK)
	{
		FreeLibrary((HMODULE)*ModuleHandle);
		*ModuleHandle = NULL;
		return 0xFFFFFFFF;
	}
	else
	{
		ReturnImm:
		return Result;
	}
	*/
#endif
}


ULONG WINAPI NtProcessThread(LPVOID LpParam)
{
	NtInfo Info = { 0 };
	RtlCopyMemory(&Info, LpParam, sizeof(NtInfo));
	ULONG StartAddr = 0x0001E030 + (ULONG)Info.hExeModule;
	BYTE ByteCode[16] = { 0 };
	ULONG OldProtect = 0;
	ULONG NewProtect = 0;
	VirtualProtect((PBYTE)StartAddr, 16, PAGE_EXECUTE_READWRITE, &OldProtect);
	RtlCopyMemory(ByteCode, (PBYTE)StartAddr, 16);
	VirtualProtect((PBYTE)StartAddr, 16, OldProtect, &NewProtect);
	while (1)
	{
		//Debugger Checker
		int result = 0;
		__asm
		{
			mov eax, fs:[0x30]; //PEB地址
			mov eax, [eax + 0x68];//NtGlobalFlag成员
			mov result, eax;
		}
		if (result == 0x70)
		{
			ExitProcess(-1);
		}
		UCHAR NewByteCode[16];
		VirtualProtect((PBYTE)StartAddr, 16, PAGE_EXECUTE_READWRITE, &OldProtect);
		RtlCopyMemory(NewByteCode, (PBYTE)StartAddr, 16);
		VirtualProtect((PBYTE)StartAddr, 16, OldProtect, &NewProtect);
		if (RtlCompareMemory(NewByteCode, ByteCode, 16) != 16)
		{
			PBYTE ImageStart = (PBYTE)GetModuleHandleW(NULL);
			VirtualProtect(ImageStart, 0x1000, PAGE_EXECUTE_READWRITE, &OldProtect);
			for (ULONG Index = 0; Index < 0x1000; Index++)
			{
				ImageStart[Index] = 0xcc;
			}
			ExitProcess(-233);
		}
		Sleep(500);
	}
	return 0;
}


HRESULT WINAPI ThreadWorker(HMODULE hModule)
{
	ULONG ThreadId = GetCurrentThreadId();
	return S_OK;
}

HRESULT WINAPI SetupNtProtect(HMODULE hModule)
{
	/*
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	StubLdrLoadDll = DetourFindFunction("ntdll.dll", "LdrLoadDll");
	DetourAttach(&StubLdrLoadDll, HookLdrLoadDll);
	DetourTransactionCommit();
	*/
	NtInfo Info;
	ULONG ThreadId = 0;
	HRESULT Result = S_OK;

	STARTUPINFOW si;
	GetStartupInfoW(&si);

	if ((si.dwX != 0) || (si.dwY != 0) || (si.dwXCountChars != 0) || (si.dwYCountChars != 0) ||
		(si.dwFillAttribute != 0) || (si.dwXSize != 0) || (si.dwYSize != 0) ||
		(si.dwFlags & STARTF_FORCEOFFFEEDBACK))
	{
		ULONG OldProtection = 0;
		PBYTE AddrStart = (PBYTE)GetModuleHandleW(NULL);
		VirtualProtect(AddrStart, 0x1000, PAGE_EXECUTE_READWRITE, &OldProtection);
		for (ULONG Index = 0; Index < 0x1000; Index++)
		{
			AddrStart[Index] = 0xcc;
		}
		ExitProcess(-233);
		return S_FALSE;
	}

	Info.hDllModule = hModule;
	Info.hExeModule = GetModuleHandleW(NULL);
	HANDLE hHandle = CreateThread(NULL, NULL, NtProcessThread, &Info, NULL, &ThreadId);
	if (hHandle == INVALID_HANDLE_VALUE)
	{
		MessageBoxW(NULL, L"内部错误", L"X'moe-CoreLib", MB_OK);
		ExitProcess(-(ULONG)'EOMX');
		Result = S_FALSE;
	}
	else
	{
		SetThreadPriority(hHandle, THREAD_PRIORITY_BELOW_NORMAL);
	}
	return Result;
}

