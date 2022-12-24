#include "BinaryCode.h"
#include "ScriptInfo.h"
#include "zlib128\zlib-1.2.8\zlib.h"

#pragma comment(lib, "zlib.lib")

/*
CPU Disasm
Address   Hex dump          Command                                                        Comments
00438F49  |.  52            push edx        lpFileName                                               ; /Arg1 => offset LOCAL.72
00438F4A  |.  8BCE          mov ecx,esi                                                    ; |
00438F4C  |.  E8 6FB5FFFF   call 004344C0    ReadFile                                              ; \Shinku.004344C0
*/


byte* Buffer = nullptr;
void *End_ReadHcb = (void*)0x00438F51;

void __stdcall Decompress();

__declspec(naked) void MyHeapAlloc()
{
	__asm
	{
		push esi
			push edi
			mov  edi, dword ptr ss : [esp + 0xC]
			mov  esi, ecx
			mov  eax, dword ptr ds : [esi]
			test edi, edi
			jg HandleOk

		HandleOk :
		push edi
			test eax, eax
			jnz  bFlagReAlloc

			push eax
			call dword ptr ds : [GetProcessHeap]
			push eax
			call dword ptr ds : [HeapAlloc]
			jmp  bFlagAllocDone

		bFlagReAlloc :
		push eax
			push 0
			call dword ptr ds : [GetProcessHeap]

			push eax
			call dword ptr ds : [HeapReAlloc]

		bFlagAllocDone :
					   mov dword ptr ds : [esi], eax
					   jmp Final

				   Final :
		mov dword ptr ds : [esi + 4], edi
			pop edi
			pop esi
			retn 4

	}
}


__declspec(naked) void MyReadhcbFile()//filename
{
	__asm
	{
		push ecx
			push ebx
			mov  ebx, dword ptr ss : [esp + 0xC]

			;; Debug-- No Error
			;; pushad
			;; push edx
			;; call MsgShow
			;; popad

			push esi
			push edi
			push 0
			push 0x8000000
			push 3
			push 0
			push 1
			push 0x80000000
			push ebx
			mov esi, ecx
			call dword ptr ds : [CreateFileA]
			mov edi, eax
			cmp edi, -1
			jnz ReadNoError
			;; --

		ReadNoError :
		mov eax, dword ptr ds : [esi]
			;; mov dword ptr[esi], 0
			push 0
			push edi
			mov  dword ptr ds : [esi + 4], 0
			call dword ptr ds : [GetFileSize]
			push eax
			mov  ecx, esi
			call MyHeapAlloc

			mov edx, dword ptr ds : [esi + 4]
			mov eax, dword ptr ds : [esi]

			push 0
			lea ecx, dword ptr ss : [esp + 0x10]
			push ecx
			push edx

			mov Buffer, eax
			push eax
			push edi
			call dword ptr ds : [ReadFile]

			push edi
			call dword ptr ds : [CloseHandle]

			pushad
			call Decompress
			popad
			mov eax, Buffer
			mov dword ptr ds : [esi], eax

			pop edi
			pop esi
			pop ebx
			pop ecx;;
		retn 4
	}
}


__declspec(naked) void HookReadHcbFile()
{
	__asm
	{
		call MyReadhcbFile
			jmp End_ReadHcb
	}
}

void WINAPI Decompress()
{
	DWORD OriLen = *(DWORD*)Buffer;
	DWORD CompressedLen = *(DWORD*)(Buffer + 4);
	byte *pBuffer = (byte*)HeapAlloc(GetProcessHeap(), 0, *(DWORD*)Buffer);
	uncompress((PBYTE)pBuffer, &OriLen, (PBYTE)(Buffer + 8), CompressedLen);

	Buffer = pBuffer;
}

void *Proc_ReadHcb = (void*)0x00438F4C;


/**************************************/
int result = 0;

int WINAPI RedirectSprintfA(char* buffer, const char* fmt, const char* addr, int no)
{
	__asm pushad
	if (!strcmp(fmt, "%s/save/s%03d.bin"))
	{
		char Info[MAX_PATH] = { 0 };
		GetCurrentDirectoryA(MAX_PATH, Info);
		result = sprintf(buffer, "/%s/save/s%03d.bin", Info, no);
	}
	else
	{
		result = sprintf(buffer, fmt, addr, no);
	}
	__asm popad
	return result;
}

DWORD InstallAddr[] = 
{
	0x0043833D,
	0x00438503,
	0x00438897,
	0x0043D478,
	0x0043D49F,
	0x0043EEC8,
	0x0043F812,
	0
};


VOID WINAPI InstallSaveData()
{
	DWORD iPos = 0;
	while (InstallAddr[iPos] != 0)
	{
		void* Addr = (void*)InstallAddr[iPos];
		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach((void**)&Addr, RedirectSprintfA);
		DetourTransactionCommit();
		iPos++;
	}
}


char* PathRedirect = nullptr;



VOID WINAPI DataFirst(const char* str)
{
	string a(str);
	string b = a.substr(a.find_last_of('/') + 1, a.length());
	memset(PathRedirect, 0, MAX_PATH);
	static const char* Prefix = "save_chs/";
	strcpy(PathRedirect, Prefix);
	strcat(PathRedirect, b.c_str());
}

void* SaveStart1 = (void*)0x00438342;
void* SaveEnd1 = (void*)0x00438347;
__declspec(naked) void Save1()
{
	__asm
	{
		lea eax, [esp + 0x24];
		pushad
		push eax
		call DataFirst
		popad
		mov eax, PathRedirect
		push eax
		jmp SaveEnd1
	}
}


void* SaveStart2 = (void*)0x00438511;
void* SaveEnd2 = (void*)0x00438516;
__declspec(naked) void Save2()
{
	__asm
	{
		lea eax, [esp + 4]
		pushad
		push eax
		call DataFirst
		popad
		mov eax, PathRedirect
		push eax
		jmp SaveEnd2
	}
}

void* SaveStart3 = (void*)0x004388A2;
void* SaveEnd3 = (void*)0x004388A7;

__declspec(naked) void Save3()
{
	__asm
	{
		lea eax, [esp + 0x24]
		pushad
		push eax
		call DataFirst
		popad
		mov eax, PathRedirect
		push eax
		jmp SaveEnd3
	}
}

void* SaveStart4 = (void*)0x0043D4A9;
void* SaveEnd4 = (void*)0x0043D4B6;

__declspec(naked) void Save4()
{
	__asm
	{
		lea ecx, [esp + 0x10]
		pushad
		push ecx
		call DataFirst
		popad
		mov ecx, PathRedirect
		push ecx
		lea edx, [esp + 0x118]
		pushad
		push edx
		call DataFirst
		popad
		mov edx, PathRedirect
		push edx
		jmp SaveEnd4
	}

}

void* SaveStart5 = (void*)0x0043EED0;
void* SaveEnd5 = (void*)0x0043EED5;
__declspec(naked) void Save5()
{
	__asm
	{
		lea ecx, [esp + 0x10]
		pushad
		push ecx
		call DataFirst
		popad
		mov ecx, PathRedirect
		push ecx
		jmp SaveEnd5
	}
}


void* SaveStart6 = (void*)0x0043F81A;
void* SaveEnd6 = (void*)0x0043F81E;
__declspec(naked) void Save6()
{
	__asm
	{
		lea edx, [esp + 0x14]
		pushad
		push edx
		call DataFirst
		popad
		mov edx, PathRedirect
		push edx
		jmp SaveEnd6
	}
}


VOID WINAPI InstallSaveCode()
{
	PathRedirect = (char*)HeapAlloc(GetProcessHeap(), 0, MAX_PATH);
		 
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach((void**)&SaveStart1, Save1);
	DetourTransactionCommit();

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach((void**)&SaveStart2, Save2);
	DetourTransactionCommit();

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach((void**)&SaveStart3, Save3);
	DetourTransactionCommit();

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach((void**)&SaveStart4, Save4);
	DetourTransactionCommit();

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach((void**)&SaveStart5, Save5);
	DetourTransactionCommit();

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach((void**)&SaveStart6, Save6);
	DetourTransactionCommit();
}


VOID WINAPI InstallBinaryCode()
{
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach((void**)&Proc_ReadHcb, HookReadHcbFile);
	DetourTransactionCommit();
}

static char* szSaveBin = "save_chs\\data.bin";
static char* szSaveBinBuffer = nullptr;

void* SaveBinStart1 = (void*)0x0043935D;
void* SaveBinEnd1 = (void*)0x00439362;
__declspec(naked) void SaveBin1()
{
	__asm
	{
		lea eax, [esp + 0x28]
		mov eax, szSaveBin

		push eax
		jmp SaveBinEnd1
		
	}
}

void* SaveBinStart2 = (void*)0x00439B9E;
void* SaveBinEnd2 = (void*)0x00439BA3;
__declspec(naked) void SaveBin2()
{
	__asm
	{
		mov edx, szSaveBin
		push edx
		lea ecx, [esp + 0x18]
		jmp SaveBinEnd2
	}
}


VOID WINAPI ZeroSaveBinBuffer()
{
	RtlZeroMemory(szSaveBinBuffer, MAX_PATH);
}

VOID WINAPI WriteSaveBinString(const char* str)
{
	ZeroSaveBinBuffer();
	if (strstr(str, "save/save.bin"))
	{
		strcpy(szSaveBinBuffer, szSaveBin);
	}
	else
	{
		strcpy(szSaveBinBuffer, str);
	}
	MessageBoxW(NULL, L"Debug", L"", MB_OK);
}

void* SaveBinStart3 = (void*)0x00439386;
void* SaveBinEnd3 = (void*)0x0043938B;
__declspec(naked) void SaveBin3()
{
	__asm
	{
		lea ecx, [esp + 0x28]
		pushad
		push ecx
		call WriteSaveBinString
		popad

		mov ecx, szSaveBinBuffer

		push ecx
		jmp SaveBinEnd3
	}
}


VOID WINAPI InstallSaveBin()
{
	szSaveBinBuffer = (char*)HeapAlloc(GetProcessHeap(), 0, MAX_PATH);
	RtlZeroMemory(szSaveBinBuffer, MAX_PATH);
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach((void**)&SaveBinStart1, SaveBin1);
	DetourTransactionCommit();

	
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach((void**)&SaveBinStart2, SaveBin2);
	DetourTransactionCommit();
	
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach((void**)&SaveBinStart3, SaveBin3);
	DetourTransactionCommit();
	
	
}
