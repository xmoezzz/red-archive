// xmoe_haruno_chs.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include <map>
#include "detours.h"
#include "xmoe_haruno_chs.h"
#include "Objidl.h"
#include <Psapi.h>
#include "DummyAPI.h"
#include "resource.h"
#include "DataMap.h"

#pragma comment(lib, "detours.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "Psapi.lib ")

using std::map;


static DummyMemory* MemoryCxt = NULL;

//Threadid hFile
static map<DWORD, HANDLE> ThreadCtx;


typedef HANDLE(WINAPI *OldCreateFile)(LPCWSTR lpFileName,
	DWORD dwDesiredAccess,
	DWORD dwShareMode,
	LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	DWORD dwCreationDisposition,
	DWORD dwFlagsAndAttributes,
	HANDLE hTemplateFile
	);

typedef BOOL(WINAPI *OldReadFile)(
	HANDLE hFile,
	LPVOID lpBuffer,
	DWORD nNumberOfBytesToRead,
	LPDWORD lpNumberOfBytesRead,
	LPOVERLAPPED lpOverlapped
	);

typedef BOOL(WINAPI* OldCloseHandle)(
	HANDLE hObject
	);

PVOID pCreateFile = NULL;
PVOID pReadFile = NULL;
PVOID pCloseHandle = NULL;

void SetNopCode(BYTE* pnop, size_t size)
{
	DWORD oldProtect;
	VirtualProtect((PVOID)pnop, size, PAGE_EXECUTE_READWRITE, &oldProtect);
	for (size_t i = 0; i < size; i++)
	{
		pnop[i] = 0x90;
	}
}

/*
	CPU Disasm
	Address   Hex dump          Command                                       Comments
	0062BAA8  /$  E8 D3F2FFFF   call 0062AD80                                 ; haruno.TVPGetFunctionExporter(guessed void)
	0062BAAD  |.  B8 FCDE7100   mov eax,offset 0071DEFC                       ; ASCII "8òq"
	0062BAB2  \.  C3            retn

*/


void NopExporter()
{
	PBYTE start = (PBYTE)0x0062BAA8;

	SetNopCode(start, 0x0062BAB2 - 0x0062BAA8);
}

void memcopy(void* dest, void*src, size_t size)
{
	DWORD oldProtect;
	VirtualProtect(dest, size, PAGE_EXECUTE_READWRITE, &oldProtect);
	memcpy(dest, src, size);
}


BYTE ShellCode_fuckXP3[] = {0xE9, 0xA3, 0x03, 0x00, 0x00, 0x90};
BYTE ShellCode2_fuckXP3[] = {0xE9, 0xFA, 0x00, 0x00, 0x00, 0x90};

void* StartXP3 = (void*)0x00624307;
void* Start2XP3 = (void*)0x00624201;
void FuckXP3()
{
	memcopy(StartXP3, ShellCode_fuckXP3, 6);
	memcopy(Start2XP3, ShellCode2_fuckXP3, 6);
}

BYTE NewPack[] = {'x', 'm', '3'};

void* XP3Create = (void*)0x006153F4;

/*
CPU Disasm
Address   Hex dump          Command                                                 Comments
00623DD3  |.  68 F5B17100   push offset 0071B1F5                                    ; /Arg2 = ASCII "data.xp3"
*/
void* HookXP3 = (void*)0x0071B1FA;


ULONG  GetFileLen(LPVOID pBaseaddr, LPVOID pReadBuf)
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


BOOL StartHook(LPCSTR szDllName, PROC pfnOrg, PROC pfnNew)
{
	HMODULE hmod;
	LPCSTR szLibName;
	PIMAGE_IMPORT_DESCRIPTOR pImportDesc;
	PIMAGE_THUNK_DATA pThunk;
	DWORD dwOldProtect, dwRVA;
	PBYTE pAddr;

	hmod = GetModuleHandle(NULL);
	pAddr = (PBYTE)hmod;
	pAddr += *((DWORD*)&pAddr[0x3C]);
	dwRVA = *((DWORD*)&pAddr[0x80]);

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

					return TRUE;
				}

			}
		}
	}
	return FALSE;
}


static HANDLE MyhFile = INVALID_HANDLE_VALUE;

HANDLE WINAPI MyCreateFileW(
	LPCWSTR lpFileName,
	DWORD dwDesiredAccess,
	DWORD dwShareMode,
	LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	DWORD dwCreationDisposition,
	DWORD dwFlagsAndAttributes,
	HANDLE hTemplateFile
	)
{
	/*
	HANDLE hOutputHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	DWORD nRet = 0;
	TCHAR buf[100] = { 0 };//用来输出字符的缓冲区

	wsprintfW(buf, L"%s\n", lpFileName);
	WriteConsole(hOutputHandle, buf, lstrlen(buf), &nRet, NULL);
	*/
	if (wcsstr(lpFileName, L"data.xm3"))
	{
		//HANDLE MyhFile = INVALID_HANDLE_VALUE;
		//DWORD ThreadId = GetCurrentThreadId();
		//map<DWORD, HANDLE>::iterator it;
		/*
		MyhFile = CreateFile(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes,
			dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
		*/
		//TCHAR szPath[MAX_PATH];
		//GetModuleFileNameEx(GetCurrentProcess(), NULL, szPath, MAX_PATH);
		
		MyhFile = CreateFile(L"haruno.exe", dwDesiredAccess, dwShareMode, lpSecurityAttributes,
			dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
		if (MyhFile == INVALID_HANDLE_VALUE)
		{
			MessageBox(NULL, L"Loader Error", L"X'moe Core", MB_OK);
			ExitProcess(-1);
		}

		//DeleteFile(L"data.xm3");
		return MyhFile;
	}
	else
	{
		return CreateFileW(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes,
			dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
	}
}

DWORD WINAPI MySetFilePointer(
	_In_        HANDLE hFile,
	_In_        LONG   lDistanceToMove,
	_Inout_opt_ PLONG  lpDistanceToMoveHigh,
	_In_        DWORD  dwMoveMethod
	)
{
	if (hFile == MyhFile)
	{
		return MemoryCxt->SetFilePointer(hFile, lDistanceToMove, lpDistanceToMoveHigh, dwMoveMethod);
	}
	else
	{
		return SetFilePointer(hFile, lDistanceToMove, lpDistanceToMoveHigh, dwMoveMethod);
	}
}

BOOL WINAPI MyReadFile(
	    HANDLE hFile,
	    LPVOID lpBuffer,
	    DWORD nNumberOfBytesToRead,
	    LPDWORD lpNumberOfBytesRead,
	    LPOVERLAPPED lpOverlapped
	)
{
	BOOL ret;
	if (hFile == MyhFile)
	{
		//MessageBox(NULL, L"1", L"", MB_OK);
		/*
		ret = ReadFile(hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead,
			lpOverlapped);
		*/
		//__asm pushad
		ret = MemoryCxt->DummyReadFile(hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead,
			lpOverlapped);
		//__asm popad
		for (DWORD i = 0; i < nNumberOfBytesToRead; i++)
		{
			((BYTE*)lpBuffer)[i] ^= 0x7F;
			((BYTE*)lpBuffer)[i] = (~((BYTE*)lpBuffer)[i] & 0xFF);
		}
	}
	else
	{
		ret = ReadFile(hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead,
			lpOverlapped);
	}
	return ret;
}

BOOL WINAPI MyCloseHandle(
	HANDLE hObject
	)
{
	if (hObject == MyhFile)
	{
		MyhFile = INVALID_HANDLE_VALUE;
	}
	return CloseHandle(hObject);
}


/*
Hook WindowsExA
CPU Stack
Address   Value      Comments
0018E794  /00010000  ; |ExtStyle = WS_EX_CONTROLPARENT
0018E798  |0018E8A4  ; |ClassName = "TTVPWindowForm"
0018E79C  |00000000  ; |WindowName = NULL
*/


HWND WINAPI MyCreateWindowExA(
	DWORD dwExStyle,
	LPCSTR lpClassName,
	LPCSTR lpWindowName,
	DWORD dwStyle,
	int x,
	int y,
	int nWidth,
	int nHeight,
	HWND hWndParent,
	HMENU hMenu,
	HINSTANCE hInstance,
	LPVOID lpParam
	)
{
	if (strncmp(lpClassName, "TTVPWindowForm", strlen("TTVPWindowForm")) == 0)
	{
		HMENU    hMenu2;
		hMenu2 = CreateMenu();
		AppendMenuW(hMenu, MF_STRING, 0x2FF2, TEXT("X'moe汉化组(>_<)"));
		return CreateWindowExA(dwExStyle, lpClassName, lpWindowName,
			dwStyle, x, y, nWidth, nHeight, hWndParent, hMenu2, hInstance, lpParam);
	}
	else
	{
		return CreateWindowExA(dwExStyle, lpClassName, lpWindowName,
			dwStyle, x, y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
	}
}



/*
CPU Disasm
Address   Hex dump          Command                                                 Comments
006153FA  |.  8B4D FC       |mov ecx,dword ptr [ebp-4]
006153FD  |.  8941 04       |mov dword ptr [ecx+4],eax

00615400  |. /EB 74         |jmp short 00615476
*/


struct Cxt
{
	DWORD eax;
	DWORD ebx;
	DWORD ecx;
	DWORD edx;
	DWORD esi;
	DWORD edi;
	DWORD esp;
	DWORD ebp;
};

Cxt cxt;

void *XP3CreateEnd = (void*)0x00615400;
__declspec(naked) void HookXP3File()
{
	__asm
	{
		mov cxt.eax, eax
		mov cxt.ebx, ebx
		mov cxt.ecx, ecx
		mov cxt.edx, edx
		mov cxt.esi, esi
		mov cxt.edi, edi
		call MyCreateFileW

		mov ebx, cxt.ebx
		mov ecx, cxt.ecx
		mov edx, cxt.edx
		mov esi, cxt.esi
		mov edi, cxt.edi

		mov ecx, dword ptr[ebp - 4]
		mov dword ptr[ecx + 4], eax
		jmp XP3CreateEnd
	}
}


HFONT WINAPI MyCreateFontIndirectA(LOGFONTA *lplf)
{
	lplf->lfCharSet = GB2312_CHARSET;
	return CreateFontIndirectA(lplf);
}


BOOL Install(HMODULE hModule)
{
	//FuckXP3();
	//AllocConsole();



	MemoryCxt = new DummyMemory;
	BYTE* buff = XP3Data;
	DWORD len = DataSize;
	MemoryCxt->SetBuffer(buff, len);


	memcopy(HookXP3, NewPack, 3);
	//NopExporter();
	
	BOOL Ret = TRUE;
	BOOL ret;

	//void* XP3Create = (void*)0x006153F4;
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(&XP3Create, HookXP3File);
	DetourTransactionCommit();
	

	FARPROC gRead = GetProcAddress(GetModuleHandleA("KERNEL32.dll"), "ReadFile");
	ret = StartHook("KERNEL32.dll", gRead, (PROC)MyReadFile);
	if (!ret)
	{
		MessageBoxW(NULL, L"游戏初始化失败(<ゝω·)Kira☆~[Code : 2]", L"晴霁之后，定是菜花盛开的好天气", MB_OK);
	}
	Ret = ret && Ret;

	FARPROC gClose = GetProcAddress(GetModuleHandleA("KERNEL32.dll"), "CloseHandle");
	ret = StartHook("KERNEL32.dll", gClose, (PROC)MyCloseHandle);
	if (!ret)
	{
		MessageBoxW(NULL, L"游戏初始化失败(<ゝω·)Kira☆~[Code : 3]", L"晴霁之后，定是菜花盛开的好天气", MB_OK);
	}
	Ret = ret && Ret;

	FARPROC gSet = GetProcAddress(GetModuleHandleA("KERNEL32.dll"), "SetFilePointer");
	ret = StartHook("KERNEL32.dll", gSet, (PROC)MySetFilePointer);
	if (!ret)
	{
		MessageBoxW(NULL, L"游戏初始化失败(<ゝω·)Kira☆~[Code : 4]", L"晴霁之后，定是菜花盛开的好天气", MB_OK);
	}
	Ret = ret && Ret;


	FARPROC gFont = GetProcAddress(GetModuleHandleA("Gdi32.dll"), "CreateFontIndirectA");
	StartHook("Gdi32.dll", gFont, (PROC)MyCreateFontIndirectA);

	
	/*
	FARPROC gWin = GetProcAddress(GetModuleHandleA("user32.dll"), "CreateWindowExA");
	ret = StartHook("user32.dll", gWin, (PROC)MyCreateWindowExA);
	if (!ret)
	{
		MessageBoxW(NULL, L"游戏初始化失败(<ゝω·)Kira☆~[Code : 4]", L"晴霁之后，定是菜花盛开的好天气", MB_OK);
	}
	Ret = ret && Ret;
	*/
	/*
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	pCreateFile = DetourFindFunction("kernel32.dll", "CreateFileW");
	DetourAttach(&pCreateFile, MyCreateFileW);
	DetourTransactionCommit();

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	pReadFile = DetourFindFunction("kernel32.dll", "ReadFile");
	DetourAttach(&pReadFile, MyReadFile);
	DetourTransactionCommit();

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	pReadFile = DetourFindFunction("kernel32.dll", "CloseHandle");
	DetourAttach(&pCloseHandle, MyCloseHandle);
	DetourTransactionCommit();
	*/
	//MessageBox(NULL, L"Error", L"", MB_OK);
	return TRUE;
}

BOOL UnInstall()
{
	//ThreadCtx.clear();
	return TRUE;
}

VOID Uni()
{
	/*
	CONTEXT cxt;
	GetThreadContext(GetCurrentThread(), &cxt);
	MEMORY_BASIC_INFORMATION mem;
	VirtualQuery((void*)cxt.Eip, &mem, sizeof(mem));
	*/
	/*
	if(mem.BaseAddress != GetModuleHandle(NULL))
		ExitProcess(0);
	*/
	HANDLE hProcess = GetCurrentProcess();
	
	//NtQuerySystemInformation(SystemProcessInformation, )
}

static DWORD Keys[1024] = 
{
	0x26e45fe3, 0x151b2d93, 0x60767fb8, 0x32310feb, 0x221e085a, 0x70e64685, 0x416f24f3,
	0x326d0b81, 0x015d4337, 0x6a8031cf, 0x55940246, 0x1d542b38, 0x700e08d4, 0x79cc2fa3, 0x4ce40687,
	0x63ed6991, 0x306464a0, 0x03924beb, 0x4b6569de, 0x2b6074fd, 0x79d7559f, 0x2bfd42c2, 0x4f9e3649,
	0x677d7c62, 0x67a17ee8, 0x391d34f9, 0x291d2019, 0x05344b9a, 0x77402826, 0x05662c94, 0x7ef32e72,
	0x251b37f3, 0x16f70e18, 0x022f714a, 0x663333cf, 0x65ec5cb0, 0x444c0311, 0x4a352d09, 0x0877167c,
	0x3f032787, 0x3a8b5f78, 0x44166070, 0x5a5b691a, 0x6ae43521, 0x31424248, 0x54d62753, 0x61ff531e,
	0x61b37d9d, 0x2ac32191, 0x225c4d3a, 0x0d894952, 0x63bb510d, 0x52a62af5, 0x65f631e2, 0x19a11653,
	0x13e823f7, 0x6c44742d, 0x4ed17dbb, 0x07f33312, 0x224c2bd5, 0x513d7b82, 0x74822668, 0x25b16f53,
	0x069e4b95, 0x7ff37854, 0x59814342, 0x220f6a31, 0x4ab6501a, 0x3a0c7b96, 0x7da831d5, 0x34c55a97,
	0x65127cba, 0x32f46050, 0x00b80a62, 0x549027c8, 0x235567be, 0x4e590c1c, 0x54d4645c, 0x7db151d9,
	0x24c226e5, 0x6ead7fa9, 0x01046aeb, 0x086c2a32, 0x64c54be1, 0x53a7373b, 0x73b3416b, 0x0f8a5c10,
	0x556b30d9, 0x08c25b28, 0x653137ed, 0x66d84506, 0x09e514e5, 0x63bd405d, 0x4a324fb5, 0x21a61d77,
	0x71090895, 0x131938d7, 0x564e0fbb, 0x2948711e, 0x1fd12a6a, 0x3c9f342b, 0x0e7f292b, 0x63996786,
	0x2bda335b, 0x2bd72fff, 0x6ba66be5, 0x69735c92, 0x15e55351, 0x1092788e, 0x0d060afe, 0x4d3848f7,
	0x445a5dab, 0x4d613928, 0x7ac7313c, 0x714d7abc, 0x0dc045bb, 0x561b5caf, 0x7874659e, 0x6e991dc1,
	0x53466b48, 0x1e5b3e1b, 0x677e3fd2, 0x0b0a1435, 0x2b3e370a, 0x17ff28f7, 0x39b76cbc, 0x40101f1f,
	0x1b9c0731, 0x41aa29e2, 0x73d702f6, 0x512156d6, 0x647c6bde, 0x45432f0f, 0x3ffb274a, 0x72337388,
	0x5a9933a8, 0x267578c5, 0x1021013a, 0x7e4565b7, 0x51d64819, 0x012d59e0, 0x50ad7f79, 0x3dd63eb8,
	0x17b95a2e, 0x581f484e, 0x2ae76c70, 0x3d6c6932, 0x7deb5edb, 0x73403d93, 0x577a52b9, 0x340f31a5,
	0x74ba5b84, 0x4e4d6745, 0x00f731a7, 0x79cb1ee0, 0x35970285, 0x17432791, 0x365001bc, 0x0e321a89,
	0x7d981fab, 0x3ce565b4, 0x6d5d0932, 0x2ed7699a, 0x57f754b8, 0x0d3a2e83, 0x155b0073, 0x7dd474de,
	0x789125e3, 0x640c24e3, 0x396806a0, 0x38456178, 0x26685656, 0x696a4251, 0x3308660e, 0x7cca795c,
	0x362114af, 0x6027675c, 0x6ca328c4, 0x220963d3, 0x1487777f, 0x04583c26, 0x34047cfe, 0x1d2a2dfc,
	0x610649ce, 0x79da60e7, 0x1cdc69ae, 0x785a2346, 0x18302795, 0x4aca6e6a, 0x733d52f6, 0x594975f7,
	0x4e3c6a42, 0x760b468d, 0x3e1f4eaf, 0x17ab37a8, 0x79826538, 0x0dc334c5, 0x51df48e4, 0x63bb21c7,
	0x4d01724c, 0x65de5e98, 0x72bb7857, 0x7cb32e13, 0x22d94e49, 0x528a0422, 0x075722fc, 0x77557f23,
	0x76d1456c, 0x76b91091, 0x5b3b327a, 0x146518e0, 0x70d22fe9, 0x22a37aa8, 0x715218ad, 0x272c6906,
	0x7f6a3e65, 0x42417540, 0x666d0426, 0x0bf71fa7, 0x024a167a, 0x5bd36fc2, 0x63be64a8, 0x2e9657a8,
	0x04c83f36, 0x1e5a66ae, 0x215e3fad, 0x20de0f42, 0x085e5d9c, 0x001f0417, 0x78c754df, 0x41287083,
	0x5f294121, 0x6d2a1025, 0x075a12a0, 0x70cf69ca, 0x166b4030, 0x05cc3191, 0x60da5a83, 0x5ab6164e,
	0x710964a7, 0x6d167e2d, 0x51ef15d0, 0x49bf7698, 0x720e6858, 0x275f5b5a, 0x32a51a04, 0x0f557904,
	0x77260189, 0x08c22e8f, 0x48ea5d4f, 0x79e35244, 0x63247f74, 0x339e5dd9, 0x3b147b14, 0x5b5a25dd,
	0x587d36c8, 0x27142056, 0x1258086c, 0x1faf7ea8, 0x03ca3e26, 0x5d8c1eb8, 0x4d5438a3, 0x735b1752,
	0x764a7aa4, 0x7b3163c8, 0x028651ba, 0x79da72dc, 0x105d7c4d, 0x0c701ce0, 0x12d270e3, 0x142c451c,
	0x7c0a2a9f, 0x547d2a70, 0x6c001f08, 0x37572b3b, 0x377a410d, 0x2bce007b, 0x5b3c3545, 0x52e33435,
	0x2f7b1b7b, 0x6e9e5717, 0x6f941169, 0x475d395c, 0x69fe52c4, 0x7b6b2af0, 0x6c7d1a79, 0x6cd406d4,
	0x40992937, 0x41780dc5, 0x4c4f152d, 0x295f5418, 0x2b064715, 0x5f4c46e9, 0x52c54871, 0x17950c74,
	0x19a24714, 0x513143c3, 0x2f7e71e5, 0x3d13678a, 0x5fef12e0, 0x2fb6584f, 0x307e0a5d, 0x50fa51cd,
	0x2f130f95, 0x7e2e4f9b, 0x04ad5a62, 0x126e2509, 0x20561a46, 0x092f4c4b, 0x0e575eaf, 0x2f1830d9,
	0x4fa85479, 0x55147915, 0x45aa7cb5, 0x39a5132e, 0x061840a9, 0x1c7b0945, 0x2b3b0717, 0x304560cf,
	0x745e2ec3, 0x5ec7093b, 0x4a82122f, 0x132c1dd4, 0x7d5378a9, 0x7e9e7ee8, 0x4c5a1887, 0x0b15062a,
	0x10730eb2, 0x706c5a55, 0x19826f61, 0x1fba2612, 0x14625427, 0x78df361c, 0x0d1e0b30, 0x7e5d42a2,
	0x61644bc8, 0x7b6a67ed, 0x3736141c, 0x50431242, 0x4be31445, 0x58c2610a, 0x2f364a82, 0x21334530,
	0x3eed34c5, 0x5d645ecb, 0x766c3b72, 0x55fb5dfd, 0x66b33963, 0x400c6b1a, 0x6a8e452f, 0x32eb5a0d,
	0x6b0b1fac, 0x303f2cf8, 0x48316bb2, 0x72582a1b, 0x39ef1322, 0x74c308f7, 0x3d537d27, 0x6b1b7ab3,
	0x61fd79bc, 0x1a2111be, 0x0bd2066f, 0x470f4cb6, 0x7cf55064, 0x312a4889, 0x3bf3179c, 0x49975dda,
	0x2a3d5777, 0x1d6f2da6, 0x5edc5878, 0x26166127, 0x19600f49, 0x73c820f8, 0x61196cff, 0x6674077b,
	0x248a049e, 0x68ce1278, 0x6d1b29e0, 0x61a05806, 0x7b0e6d32, 0x4f6002af, 0x5db41900, 0x420858cf,
	0x5be11432, 0x2722533d, 0x409d4df7, 0x1c22072d, 0x601c16c1, 0x3af96755, 0x68f10a91, 0x14e7204f,
	0x557e7074, 0x4f91143f, 0x11af334d, 0x185339b5, 0x28e857d6, 0x61d761d5, 0x103b13e2, 0x1fe729b5,
	0x60df6ae5, 0x757f1b06, 0x16dd73b5, 0x09273ff7, 0x280d2b92, 0x737f2e56, 0x07417a65, 0x7c1c4df8,
	0x67c04c45, 0x18925e5c, 0x54f6643f, 0x61d27f8b, 0x72694c57, 0x73b74242, 0x77ef06ca, 0x6adb0353,
	0x3e1f6496, 0x74af1649, 0x6f05253c, 0x25cb034b, 0x2f1a43c5, 0x0a825c43, 0x52721503, 0x25ba6d3f,
	0x72381b19, 0x51fa4c17, 0x7658323e, 0x38c50b50, 0x677b7abd, 0x54271440, 0x1d382440, 0x2e8d6c73,
	0x1c887e4f, 0x54da6a4e, 0x3a7b7214, 0x2eb61cf2, 0x572a4961, 0x31296b63, 0x44ed66f4, 0x1f6a2eea,
	0x2fce53f9, 0x4df14cb7, 0x193d46d0, 0x1bd312cb, 0x3c050711, 0x164f5c15, 0x6c7e52cd, 0x7aa43fdb,
	0x49042917, 0x0a26505c, 0x4ea91dc3, 0x64912cb4, 0x26271a6e, 0x5c9d69ff, 0x3d1930bf, 0x7ad317c1,
	0x7f6a61eb, 0x229e6386, 0x450d7f7f, 0x0da51fc5, 0x47ee095a, 0x1158320a, 0x36292cf8, 0x62c92c54,
	0x347b49f7, 0x4cbe15bc, 0x64f51fd3, 0x0c042658, 0x45f808f5, 0x46067a5f, 0x7d5d66ec, 0x4d9e008e,
	0x63f423fa, 0x2a2a27ca, 0x65306dd2, 0x14e31006, 0x07200da0, 0x606a4267, 0x2ea1014a, 0x7ea434a6,
	0x73d339f5, 0x18c81bb7, 0x1ac923cc, 0x6db651a8, 0x04855afc, 0x6a8b52cb, 0x2c233203, 0x31721617,
	0x04546d2b, 0x02bd44cc, 0x490e5751, 0x3c341556, 0x198213eb, 0x62ac4d74, 0x6e4f524a, 0x69dc2f99,
	0x3ff5461c, 0x2e6d5793, 0x718c0934, 0x56504a6a, 0x53b64a8e, 0x0b543d8b, 0x53d26e8d, 0x43f85925,
	0x2b720488, 0x0e7d79d4, 0x240f3585, 0x1240357d, 0x42fc1044, 0x3b472779, 0x71995680, 0x441a47f5,
	0x75c92f71, 0x11d45298, 0x4ea66395, 0x16790067, 0x497305b0, 0x2d8a18e6, 0x62d12d12, 0x26d71e80,
	0x48362518, 0x73951a29, 0x0d9c35f5, 0x29b04a42, 0x6b766ab2, 0x516338bd, 0x18e87874, 0x31057c81,
	0x16372afe, 0x0b262a10, 0x7b7f7a76, 0x02da3767, 0x1fa42e6c, 0x1a555726, 0x2b8a3218, 0x7fb80ef0,
	0x6d897de3, 0x1c2b0d15, 0x011b3a2a, 0x192c016e, 0x1ed87f3e, 0x50267d89, 0x28a556af, 0x58462006,
	0x462861ca, 0x268a0f41, 0x257f4960, 0x741b0732, 0x34315ac9, 0x5edc7e91, 0x64647629, 0x7843273d,
	0x525131f2, 0x36684dde, 0x5df657aa, 0x7b5b5cc9, 0x0d0a1def, 0x26bb0625, 0x493643b8, 0x6584594c,
	0x4e8270dd, 0x34294774, 0x5e0e7fda, 0x46e35b8f, 0x090214d0, 0x4c482970, 0x27c725cc, 0x3e1f382e,
	0x5178584c, 0x34736bcd, 0x679357ff, 0x6ee6321b, 0x09f50acd, 0x084876d9, 0x07054617, 0x0868231b,
	0x1c2f6940, 0x482a2bf2, 0x1a94016c, 0x5bda7447, 0x43ff5a87, 0x77c0060a, 0x741b218a, 0x02f5668d,
	0x69e47bf9, 0x4c750a2a, 0x455c38b0, 0x16742b2c, 0x0d7f7de0, 0x6bf607ed, 0x52781855, 0x749a4c3b,
	0x40154ffa, 0x3ab62a00, 0x3479659e, 0x17a96522, 0x2f111df8, 0x3a6d56a9, 0x2bc97de9, 0x7c6c2b20,
	0x3e7f1c02, 0x7895603d, 0x02b72b46, 0x18ae45c4, 0x33912331, 0x0cec05a8, 0x7ff928f9, 0x61c17773,
	0x6f1e1e13, 0x27f542e9, 0x692577f9, 0x62f815e8, 0x381e452b, 0x31777193, 0x15370373, 0x642d52b0,
	0x162f2b6e, 0x76fc394d, 0x0f0e1548, 0x203c53aa, 0x3c141ac7, 0x6a535053, 0x47ef1a8b, 0x0b851b8d,
	0x02314094, 0x700e0bf3, 0x5a003804, 0x2a6e4261, 0x710f2a26, 0x3e054111, 0x5ace2eb0, 0x77df7e04,
	0x5bde1145, 0x49d174a4, 0x3dc8103e, 0x5bc57aa7, 0x0aee78aa, 0x47525c36, 0x46c24393, 0x3190034f,
	0x76351884, 0x372a2e68, 0x0c735947, 0x5eb47a55, 0x0fcd1af3, 0x053f436b, 0x0af63027, 0x792b21e6,
	0x1e732890, 0x373d0588, 0x464d69b1, 0x7df13483, 0x280944e3, 0x2b123199, 0x7cd92e9a, 0x17874d82,
	0x6c147aeb, 0x6570678e, 0x69e4434b, 0x7472218b, 0x6e3f599a, 0x704e0ae9, 0x18186c60, 0x2db9071c,
	0x10d64056, 0x49687342, 0x44052328, 0x3d6a4f06, 0x3f4b7b79, 0x60ba6cc4, 0x4e9e1a27, 0x05156cee,
	0x28b530d1, 0x270908ae, 0x3fbc1197, 0x644f6fcc, 0x0a4c1c21, 0x2c5a3dd3, 0x589a7be3, 0x5f304a6f,
	0x09ef1b9f, 0x4e79591a, 0x3658722b, 0x54d56bf7, 0x209e0c74, 0x77733dfe, 0x047878c3, 0x45e0285a,
	0x1500773f, 0x6c1c770f, 0x3f6413b4, 0x2af370e0, 0x05de0c92, 0x2a8a1670, 0x06e62b39, 0x5b395ca7,
	0x04a67173, 0x58986657, 0x00ae4042, 0x02dd0120, 0x3fe95bdd, 0x42656991, 0x4acf70f5, 0x29901a8f,
	0x3ddd7f3c, 0x68d22bfb, 0x7e424d28, 0x4907048f, 0x26dc48f4, 0x20076309, 0x41627ae9, 0x737b028c,
	0x1fe36cdb, 0x3def5e43, 0x6a6f2af6, 0x0a285847, 0x351441bb, 0x58b747c3, 0x320c5d45, 0x03ce3256,
	0x54346dd1, 0x155334b8, 0x75c0757d, 0x43335ea1, 0x572d6350, 0x05f905e6, 0x0a781f7b, 0x7d9f54e5,
	0x1e8e2cde, 0x18a31824, 0x1f0303ce, 0x315e0f35, 0x3c060a16, 0x159244dd, 0x2e954c3b, 0x2c423275,
	0x2ced5c05, 0x2dc6328f, 0x0345783a, 0x221e06dd, 0x24bb61ad, 0x1987754f, 0x488f0177, 0x534c407c,
	0x678f4485, 0x46df7f43, 0x2dd25053, 0x432817b2, 0x34a974f6, 0x181d6126, 0x18d30060, 0x7e9331b4,
	0x40f056e0, 0x32535ac8, 0x683874e8, 0x7271590c, 0x416d3e13, 0x5bd93b8c, 0x460e3d66, 0x522b0617,
	0x05ce3ad7, 0x6ac912e8, 0x0a454a0b, 0x0e2e3786, 0x22e43664, 0x438130e8, 0x2d2e703b, 0x5a691add,
	0x2d255f6a, 0x672476ab, 0x4a033f0e, 0x44d304f6, 0x032b668b, 0x121876e4, 0x315f23d0, 0x5be33a7f,
	0x28330adc, 0x6a89665b, 0x0bc25e80, 0x65170878, 0x2e9f7667, 0x3ee55c69, 0x0c0f4655, 0x236e2cb6,
	0x32756aac, 0x545e6380, 0x320d5e34, 0x2ded0e63, 0x63dd3d1b, 0x456c59a1, 0x1ce9393b, 0x561d467c,
	0x21a7239c, 0x704820e3, 0x6db22f3a, 0x1e8b7851, 0x23c25108, 0x75721ff3, 0x39dc6134, 0x41487a09,
	0x35c661ad, 0x462c128e, 0x0dbe0de3, 0x46674d1b, 0x016b17ce, 0x42fc2a09, 0x7f143631, 0x2a8166d6,
	0x69106820, 0x6a2e7dc9, 0x4f7e11c0, 0x153448d9, 0x7236564e, 0x164f4bcc, 0x1eff5362, 0x1f9f699c,
	0x40022175, 0x4cb4091e, 0x2e7e3da1, 0x2ae86ce5, 0x1dbe40a9, 0x1bf14265, 0x32480739, 0x46b62c54,
	0x19582f6f, 0x0a624c56, 0x348c0f99, 0x27b70fd8, 0x2de10a41, 0x14a6443c, 0x07de6367, 0x2e1c3637,
	0x7e0f7b0d, 0x3c1e6078, 0x49b510f9, 0x7c186d8b, 0x1ebc75b6, 0x257210fc, 0x74ed4cdc, 0x1c657bbf,
	0x71654491, 0x470d6fcf, 0x04466650, 0x38bf3716, 0x0eac64e9, 0x279c018d, 0x24e30bc9, 0x60666ea4,
	0x40d7337c, 0x2c9345e4, 0x78cb5f70, 0x5ea022d3, 0x0e4e68fc, 0x78a81818, 0x696b5ba0, 0x21350ddf,
	0x5421668f, 0x5a565f7f, 0x0a13076a, 0x2ef27c5a, 0x707f524f, 0x4a5b1005, 0x0a747b12, 0x2e2675aa,
	0x7d4003ca, 0x7a3b7aaa, 0x39283490, 0x7b283486, 0x1a5d4083, 0x72bb6dff, 0x162b3c10, 0x4ecf6f7c,
	0x48724870, 0x426626ad, 0x755a1871, 0x74f8716e, 0x5343327a, 0x3c0b0fee, 0x30fb13c9, 0x13040210,
	0x4c341900, 0x453d5411, 0x6c354fe0, 0x7e571e6c, 0x14cf1654, 0x34d23cfa, 0x65932ab1, 0x22da015e,
	0x7942113c, 0x416464a0, 0x598572ed, 0x797a7c18, 0x5ade5972, 0x7fd3358e, 0x74df6c77, 0x0ea71e9f,
	0x6a9a1425, 0x71c13b62, 0x575824e8, 0x18d6304c, 0x738e7876, 0x24154351, 0x260c180d, 0x1eff784c,
	0x35785bfb, 0x5d794ca1, 0x2dfb2464, 0x2f205621, 0x4f3a0f40, 0x5cdc492e, 0x16884fa3, 0x24b82a1f,
	0x39590a41, 0x27f12de6, 0x23fa5b31, 0x7f4c0def, 0x508068f1, 0x69ad534c, 0x09ff28ab, 0x48e65d10,
	0x6ffc37b6, 0x60cd25f8, 0x4e246e60, 0x0c910d51, 0x1c3e0feb, 0x5e4e2715, 0x3a5e3bd5, 0x5cde5758,
	0x3d5b045d, 0x53f33ce2, 0x5f844e42, 0x6a622f1e, 0x698f5dce, 0x72c25333, 0x27d23513, 0x2a360c70,
	0x3fb62775, 0x59874bed, 0x79684668, 0x0c76036f, 0x51d10b7b, 0x53503f8d, 0x68c96396, 0x42c22d11,
	0x1f877f7f, 0x25ef0da1, 0x7b5c0da3, 0x16c05f9f, 0x20a24114, 0x707b3d4d, 0x79ef49ce, 0x50983735,
	0x5f8e223e, 0x19d02dc8, 0x532f5605, 0x2d766e45, 0x23dd25f9, 0x4f0a16dc, 0x0e312d6c, 0x660c0614,
	0x2cc56cb2, 0x120e596a, 0x4cec5cdd, 0x450d3f3a, 0x7ba070cb, 0x58011fe3, 0x5ebc2763, 0x4db36227,
	0x2e6c131b, 0x37cf4ed0, 0x62e17abd, 0x723a5799, 0x6a49776c, 0x28a5454b, 0x7afe33e2, 0x5a631127,
	0x55fd30fb, 0x50776d84, 0x0d9b3377, 0x39f241b9, 0x24733efb, 0x627c1d3d, 0x18a3425a, 0x3730660d,
	0x2f385913, 0x0dad464e, 0x13e6461b, 0x61691d34, 0x20fd0bdc, 0x7b497722, 0x6398457d, 0x376f5113,
	0x30172564, 0x5d542b38, 0x5ad03cf9, 0x3e162ee2, 0x690271ad, 0x0d136ba3, 0x4e0a433c, 0x26b66fb0,
	0x08da472e, 0x39913f8a, 0x35a67da4, 0x05ac70de, 0x67e06351, 0x261d6ca9, 0x606764c7, 0x18d91c9f,
	0x73fc16f4, 0x78ca07ce, 0x35f558ec, 0x1e22227f, 0x3b3542e9, 0x18ee555d, 0x095b068f, 0x39ee7fd9,
	0x063a2475, 0x1da479cc, 0x7b8a1ae2, 0x6dab585e, 0x02dc71d4, 0x4c4a7a28, 0x6dd44847, 0x1e491e95,
	0x7e9246b3
};


extern "C" _declspec (dllexport) int xmoeMakeSure(const DWORD* in)
{
	int ok = 1;
	for (unsigned int i = 0; i < 1024; i++)
	{
		__try
		{
			__try
			{
				__try
				{
					ok = in[i] == Keys[i];
					if (ok == 0)
					{
						//MessageBox(NULL, L"com", L"", MB_OK);
						return 0;
					}
				}
				__except (EXCEPTION_ARRAY_BOUNDS_EXCEEDED)
				{
					//MessageBox(NULL, L"1", L"", MB_OK);
					ExitProcess(0);
					return 0;
				}
			}
			__except (EXCEPTION_ACCESS_VIOLATION)
			{
				//MessageBox(NULL, L"2", L"", MB_OK);
				ExitProcess(0);
				return 0;
			}
		}
		__except (EXCEPTION_ACCESS_VIOLATION)
		{
			//MessageBox(NULL, L"3", L"", MB_OK);
			ExitProcess(0);
			return 0;
		}
	}
	return ok;
}
