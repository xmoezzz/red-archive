#include "my.h"
#include <string>
#include <set>

#pragma comment(lib, "Version.lib")

ForceInline std::wstring FASTCALL ReplaceFileNameExtension(std::wstring& Path, PCWSTR NewExtensionName)
{
	ULONG_PTR Ptr;

	Ptr = Path.find_last_of(L".");
	if (Ptr == std::wstring::npos)
		return Path + NewExtensionName;

	return Path.substr(0, Ptr) + NewExtensionName;
}


ForceInline std::wstring FASTCALL GetFileName(std::wstring& Path)
{
	ULONG_PTR Ptr;

	Ptr = Path.find_last_of(L"\\");
	if (Ptr == std::wstring::npos)
		return Path;

	return Path.substr(Ptr + 1, std::wstring::npos);
}


ForceInline std::wstring FASTCALL GetFileNameExtension(std::wstring& Path)
{
	ULONG_PTR Ptr;

	Ptr = Path.find_last_of(L".");
	if (Ptr == std::wstring::npos)
		return NULL;

	return Path.substr(Ptr + 1, std::wstring::npos);
}


ForceInline std::wstring FASTCALL GetFileNamePrefix(std::wstring& Path)
{
	ULONG_PTR Ptr;

	Ptr = Path.find_last_of(L".");
	if (Ptr == std::wstring::npos)
		return Path;

	return Path.substr(0, Ptr);
}

/////////////////////////////////////////////////


NAKED VOID SarCheckFake()
{
	INLINE_ASM
	{
		mov esp, ebp;
		mov eax, 1;
		ret;
	}
}

#define MAX_SECTION_COUNT 64

inline PDWORD FASTCALL GetOffset(PBYTE ModuleBase, DWORD v)
{
	ULONG_PTR            Offset;
	IMAGE_SECTION_HEADER SectionTable[MAX_SECTION_COUNT];
	PIMAGE_DOS_HEADER    pDosHeader;
	PIMAGE_NT_HEADERS32  pNtHeader;

	pDosHeader = (PIMAGE_DOS_HEADER)ModuleBase;
	pNtHeader = (PIMAGE_NT_HEADERS32)(ModuleBase + pDosHeader->e_lfanew);
	RtlZeroMemory(SectionTable, sizeof(SectionTable));
	RtlCopyMemory(SectionTable, ModuleBase + sizeof(IMAGE_NT_HEADERS32) + pDosHeader->e_lfanew,
		pNtHeader->FileHeader.NumberOfSections * sizeof(IMAGE_SECTION_HEADER));

	for (ULONG_PTR i = 0; i < pNtHeader->FileHeader.NumberOfSections; i++)
	{
		if (SectionTable[i].VirtualAddress <= v && v <= SectionTable[i].VirtualAddress + SectionTable[i].SizeOfRawData)
		{
			ULONG_PTR Delta = v - SectionTable[i].VirtualAddress;
			v = SectionTable[i].PointerToRawData + Delta;
			break;
		}
	}
	v += (ULONG_PTR)ModuleBase;
	return (PDWORD)v;
}

typedef struct XBundler
{
	PCHAR pDllName;
	PBYTE pBuffer;
	DWORD dwSize;
}XBundler, *PXBundler;

PXBundler pSarcheck = NULL;

API_POINTER(ZwAllocateVirtualMemory) StubZwAllocateVirtualMemory = NULL;

NTSTATUS NTAPI HookZwAllocateVirtualMemory(
	IN HANDLE ProcessHandle,
	IN OUT PVOID *BaseAddress,
	IN ULONG ZeroBits,
	IN OUT PULONG RegionSize,
	IN ULONG AllocationType,
	IN ULONG Protect
	)
{

	NTSTATUS          Status;
	PIMAGE_DOS_HEADER DosHeader;
	PIMAGE_NT_HEADERS NtHeader;
	DWORD             OldProtect;

	if (pSarcheck &&
		!IsBadReadPtr(pSarcheck->pDllName, MAX_PATH) &&
		!IsBadReadPtr(pSarcheck->pBuffer, pSarcheck->dwSize) &&
		pSarcheck->pDllName + StrLengthA(pSarcheck->pDllName) + 5 == (PCHAR)pSarcheck->pBuffer&&
		*(PWORD)pSarcheck->pBuffer == 'ZM')
	{
		Status = StubZwAllocateVirtualMemory(ProcessHandle, BaseAddress, ZeroBits, RegionSize, AllocationType, Protect);

		Mp::PATCH_MEMORY_DATA p[] =
		{
			Mp::FunctionJumpVa(ZwAllocateVirtualMemory, HookZwAllocateVirtualMemory, (PVOID*)&StubZwAllocateVirtualMemory),
		};

		Mp::RestoreMemory(p, countof(p));
		Nt_ProtectMemory(NtCurrentProcess(), pSarcheck->pBuffer, pSarcheck->dwSize, PAGE_EXECUTE_READWRITE, &OldProtect);

		DosHeader = (PIMAGE_DOS_HEADER)pSarcheck->pBuffer;
		NtHeader = (PIMAGE_NT_HEADERS32)((PBYTE)DosHeader + DosHeader->e_lfanew);
		PDWORD pEntryPoint = GetOffset(pSarcheck->pBuffer, NtHeader->OptionalHeader.AddressOfEntryPoint);
		PIMAGE_EXPORT_DIRECTORY pIET = (PIMAGE_EXPORT_DIRECTORY)GetOffset(pSarcheck->pBuffer, NtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);
		*GetOffset(pSarcheck->pBuffer, pIET->AddressOfFunctions) = NtHeader->OptionalHeader.AddressOfEntryPoint + 3;



		//ret 0xc
		//ret 
		*pEntryPoint = 0xC3000CC2;
		//VirtualProtect(pEntryPoint, 4, OldProtect, &OldProtect);

		Nt_ProtectMemory(NtCurrentProcess(), pSarcheck->pBuffer, pSarcheck->dwSize, OldProtect, &OldProtect);

		return Status;
	}
	return StubZwAllocateVirtualMemory(ProcessHandle, BaseAddress, ZeroBits, RegionSize, AllocationType, Protect);
}

API_POINTER(VirtualAlloc) StubVirtualAlloc = NULL;

PVOID WINAPI HookVirtualAlloc(
	IN LPVOID lpAddress,
	IN SIZE_T dwSize,
	IN DWORD flAllocationType,
	IN DWORD flProtect
	)
{
	PWORD pByte = (PWORD)((PBYTE)_ReturnAddress() - 6);
	if (*pByte == 0x95FF)//call dword ptr[ebp+]
	{
		pSarcheck = (PXBundler)StubVirtualAlloc(lpAddress, dwSize, flAllocationType, flProtect);

		Mp::PATCH_MEMORY_DATA r[] =
		{
			Mp::FunctionJumpVa(VirtualAlloc, HookVirtualAlloc, &StubVirtualAlloc),
		};

		Mp::RestoreMemory(r, countof(r));

		Mp::PATCH_MEMORY_DATA p[] =
		{
			Mp::FunctionJumpVa(ZwAllocateVirtualMemory, HookZwAllocateVirtualMemory, (PVOID*)&StubZwAllocateVirtualMemory),
		};


		Mp::PatchMemory(p, countof(p));
		return pSarcheck;
	}
	return StubVirtualAlloc(lpAddress, dwSize, flAllocationType, flProtect);
}


API_POINTER(GetProcAddress) StubGetProcAddress = NULL;

FARPROC WINAPI HookGetProcAddress(
	IN HMODULE hModule,
	IN LPCSTR lpProcName
	)
{
	if (!IsBadReadPtr(lpProcName, 9) &&
		!StrNICompareA(lpProcName, "Sarcheck", 9, StrCmp_ToLower))
	{
		Mp::PATCH_MEMORY_DATA p[] =
		{
			Mp::FunctionJumpVa(GetProcAddress, HookGetProcAddress, &StubGetProcAddress),
		};

		Mp::RestoreMemory(p, countof(p));

		return (FARPROC)SarCheckFake;
	}
	return StubGetProcAddress(hModule, lpProcName);
}


API_POINTER(GetTimeZoneInformation) StubGetTimeZoneInformation = NULL;

DWORD
WINAPI
HookGetTimeZoneInformation(
_Out_ LPTIME_ZONE_INFORMATION lpTimeZoneInformation
)
{
	static WCHAR StdName[] = L"TOKYO Standard Time";
	static WCHAR DayName[] = L"TOKYO Daylight Time";

	StubGetTimeZoneInformation(lpTimeZoneInformation);

	lpTimeZoneInformation->Bias = -540;
	lpTimeZoneInformation->StandardBias = 0;

	RtlCopyMemory(lpTimeZoneInformation->StandardName, StdName, countof(StdName) * 2);
	RtlCopyMemory(lpTimeZoneInformation->DaylightName, DayName, countof(DayName) * 2);
	return 0;
}


API_POINTER(GetLocaleInfoW) StubGetLocaleInfoW = NULL;

int
WINAPI
HookGetLocaleInfoW(
LCID     Locale,
LCTYPE   LCType,
LPWSTR lpLCData,
int      cchData)
{
	if (Locale == 0x800u && LCType == 1)
	{
		RtlCopyMemory(lpLCData, L"0411", 10);
		return 5;
	}

	return StubGetLocaleInfoW(Locale, LCType, lpLCData, cchData);
}


API_POINTER(GetFileVersionInfoSizeW) StubGetFileVersionInfoSizeW = NULL;

DWORD
APIENTRY
HookGetFileVersionInfoSizeW(
_In_        LPCWSTR lptstrFilename, /* Filename of version stamped file */
_Out_opt_ LPDWORD lpdwHandle       /* Information for use by GetFileVersionInfo */
)
{
	auto IsKernel32 = [](LPCWSTR lpFileName)->BOOL
	{
		/*
		ULONG Length = StrLengthW(lpFileName);
		if (Length < 12)
		return FALSE;

		//sarcheck.dll
		return CHAR_UPPER4W(*(PULONG64)&lpFileName[Length - 0xC]) == TAG4W('KERN') &&
		CHAR_UPPER4W(*(PULONG64)&lpFileName[Length - 0x8]) == TAG4W('AL32') &&
		CHAR_UPPER4W(*(PULONG64)&lpFileName[Length - 0x4]) == TAG4W('.DLL');
		*/

		return wcsstr(lpFileName, L"kernel32.dll") != NULL;
	};

	if (IsKernel32(lptstrFilename))
		return StubGetFileVersionInfoSizeW(L"rewrite_plus_jp.dll", lpdwHandle);

	return StubGetFileVersionInfoSizeW(lptstrFilename, lpdwHandle);
}


API_POINTER(GetFileVersionInfoW) StubGetFileVersionInfoW = NULL;

BOOL
APIENTRY
HookGetFileVersionInfoW(
_In_                LPCWSTR lptstrFilename, /* Filename of version stamped file */
_Reserved_          DWORD dwHandle,          /* Information from GetFileVersionSize */
_In_                DWORD dwLen,             /* Length of buffer for info */
LPVOID lpData            /* Buffer to place the data structure */
)
{
	auto IsKernel32 = [](LPCWSTR lpFileName)->BOOL
	{
		return wcsstr(lpFileName, L"kernel32.dll") != NULL;
	};

	if (IsKernel32(lptstrFilename))
		return StubGetFileVersionInfoW(L"rewrite_plus_jp.dll", dwHandle, dwLen, lpData);

	return StubGetFileVersionInfoW(lptstrFilename, dwHandle, dwLen, lpData);
}





typedef struct GDI_ENUM_FONT_PARAM
{
	LPARAM                  lParam;
	FONTENUMPROCW           Callback;
} GDI_ENUM_FONT_PARAM, *PGDI_ENUM_FONT_PARAM;

API_POINTER(EnumFontFamiliesExW) StubEnumFontFamiliesExW = NULL;


typedef struct
{
	BYTE  Flag1;
	BYTE  Flag2;
	BYTE  Padding[2];
	DWORD Data1;
	DWORD Data2;
}FONT_INFO;

int CALLBACK EnumFamCallBack(LOGFONT *lpelfe,
	const TEXTMETRIC *lpntme,
	DWORD      FontType,
	LPARAM     lParam)
{
	PGDI_ENUM_FONT_PARAM Param = (PGDI_ENUM_FONT_PARAM)lParam;

	//lpelfe->lfCharSet = GB2312_CHARSET;

	FONT_INFO* Info = (FONT_INFO*)Param->lParam;
	PrintConsole(L"%d\n", lpelfe->lfPitchAndFamily);
	//lpelfe->lfPitchAndFamily = DEFAULT_PITCH;//VARIABLE_PITCH
	PrintConsole(L"%d %d %d %d\n", Info->Flag1, Info->Flag2, Info->Data1, Info->Data2);

	return Param->Callback(lpelfe, lpntme, FontType, Param->lParam);
}


/*
___:00685608                 push    58h
___:0068560A                 mov     [ecx+14h], eax
___:0068560D                 lea     eax, [ebp+var_88]
___:00685613                 push    0
___:00685615                 push    eax
___:00685616                 mov     byte ptr [edi], 0
___:00685619                 mov     dword ptr [ecx+10h], 80h //从lfHeight开始，Logfont剩下的字节数
___:00685620                 mov     [ebp+var_8C], 0
___:0068562A                 call    sub_721260          //memset
___:0068562F                 add     esp, 0Ch            //memset的平衡
___:00685632                 mov     [ebp+var_75], 1     //Logfont.lfCharSet 设定为Default
___:00685636                 push    ebx                 //SiglusEngine主窗口HWND
___:00685637                 call    ds:dword_A0F3D0      //GetDC
___:0068563D                 push    0                    //EnumFontFamiliesExW.dwFlags
___:0068563F                 push    edi                  //EnumFontFamiliesExW.lParam 前12字节是普通参数，后面还有三个std::wstring
___:00685640                 mov     esi, eax
___:00685642                 lea     eax, [ebp+var_8C]
___:00685648                 push    offset sub_6857E0   //EnumFontFamiliesExW.Proc
___:0068564D                 push    eax                 //EnumFontFamiliesExW.lpLogfont
___:0068564E                 push    esi                 //EnumFontFamiliesExW.hDc
___:0068564F                 call    ds:off_A0F0AC      //call EnumFontFamiliesExW
*/

int NTAPI HookEnumFontFamiliesExW(
	_In_ HDC           hdc,
	_In_ LPLOGFONTW    lpLogfont,
	_In_ FONTENUMPROCW lpEnumFontFamExProc,
	_In_ LPARAM        lParam,
	DWORD              dwFlags
	)
{
	GDI_ENUM_FONT_PARAM Param;

	lpLogfont->lfCharSet = GB2312_CHARSET;
	//RtlZeroMemory(lpLogfont->lfFaceName, sizeof(lpLogfont->lfFaceName));

	tagLOGFONTW Logfont;
	Logfont.lfHeight = 0;
	memset(&Logfont.lfWidth, 0, 0x58u);
	Logfont.lfCharSet = GB2312_CHARSET;

	Param.Callback = lpEnumFontFamExProc;
	Param.lParam = lParam;
	return StubEnumFontFamiliesExW(hdc, &Logfont, (FONTENUMPROCW)EnumFamCallBack, (LPARAM)&Param, dwFlags);
}

API_POINTER(CreateFontIndirectW) StubCreateFontIndirectW = NULL;


std::wstring GlobalFontName = L"新宋体";

HFONT WINAPI HookCreateFontIndirectW(LOGFONTW *lplf)
{
	LOGFONTW lf;
	memcpy(&lf, lplf, sizeof(LOGFONTW));
	lf.lfCharSet = DEFAULT_CHARSET;

	if (lplf->lfFaceName[0] == L'@')
		FormatStringW(lf.lfFaceName, L"@%s", GlobalFontName.c_str());
	else
		StrCopyW(lf.lfFaceName, GlobalFontName.c_str());

	return StubCreateFontIndirectW(&lf);
}

/*
esi寄存器保存的是当前的字符
将eax的值替换成当前的Width

注意：判断themida解码完成之后再patch
___:005B753B                 add     eax, [ebp+var_4BC]  //patch1(6个字节 直接写jmp)
___:005B7541                 add     [ebx+0E4h], eax
___:005B7547                 jmp     short loc_5B7555
___:005B7549 ; ---------------------------------------------------------------------------
___:005B7549
___:005B7549 loc_5B7549:                             ; CODE XREF: sub_5B70A0+499j
___:005B7549                 add     eax, [ebp+var_4BC]  //patch2(6个字节 直接写jmp)
___:005B754F                 add     [ebx+0E0h], eax
*/


/*
追踪绘图HDC：新版本的Siglus内部有些东西被优化成xmm指令了，草。
调用两次GetGlyphOutlineW的上一层就是绘图HDC，看能不能用异常处理，一次断点自动找出HDC。
___:006C83D2                 cmovz   eax, ecx
___:006C83D5                 mov     [ebp+var_14], 10000h
___:006C83DC                 lea     ecx, [ebp+var_54]
___:006C83DF                 mov     [ebp+var_28], 10000h
___:006C83E6                 push    ecx
___:006C83E7                 push    0
___:006C83E9                 push    0
___:006C83EB                 movdqu  [ebp+var_40], xmm0
___:006C83F0                 lea     ecx, [ebp+var_44]
___:006C83F3                 movdqu  xmm0, xmmword ptr [eax]
___:006C83F7                 mov     eax, [ebp+arg_0]
___:006C83FA                 push    ecx
___:006C83FB                 movzx   eax, ax
___:006C83FE                 push    6
___:006C8400                 push    eax
___:006C8401                 push    edi
___:006C8402                 movdqu  [ebp+var_54], xmm0
___:006C8407                 mov     [ebp+var_98], eax
___:006C840D                 call    ds:off_A0F110     //GetGlyphOutlineW
*/


ULONG_PTR ThisModule = 0;



Void PatchCodeViaJmp(PVOID src, PVOID dst)
{
	DWORD OldProtect;
	VirtualProtect(src, 10, PAGE_EXECUTE_READWRITE, &OldProtect);

	INLINE_ASM
	{
		mov eax, src;
		mov ecx, dst;
		sub ecx, eax;
		sub ecx, 5;
		mov byte ptr[eax], 0xE9;
		inc eax;
		mov dword ptr[eax], ecx;
	}
}

Void PatchCodeNope(PBYTE src, ULONG Size)
{
	DWORD OldProtect;
	VirtualProtect(src, Size, PAGE_EXECUTE_READWRITE, &OldProtect);

	for (ULONG i = 0; i < Size; i++)
		src[i] = 0x90;
}


Void PatchReplaceCode(PBYTE src, PBYTE code, ULONG Size)
{
	DWORD OldProtect;
	VirtualProtect(src, Size, PAGE_EXECUTE_READWRITE, &OldProtect);

	for (ULONG i = 0; i < Size; i++)
		src[i] = code[i];
}


API_POINTER(CreateFileW) StubCreateFileW = NULL;

BOOL PatchOnlyOnce = FALSE;

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
	ULONG        Attribute;
	HANDLE       Handle;
	BOOL         IsScenePck;
	std::wstring CurFileName;

	auto IsScenePack = [](LPCWSTR FileName)->BOOL
	{
		ULONG_PTR iPos = 0;

		for (ULONG i = 0; i < StrLengthW(FileName); i++)
		{
			if (FileName[i] == L'\\' || FileName[i] == L'/')
				iPos = i;
		}

		if (iPos != 0)
			iPos++;

		return StrCompareW(FileName + iPos, L"Scene.pck") == 0;
	};


	auto IsGameExe = [](LPCWSTR FileName)->BOOL
	{
		ULONG_PTR iPos = 0;

		for (ULONG i = 0; i < StrLengthW(FileName); i++)
		{
			if (FileName[i] == L'\\' || FileName[i] == L'/')
				iPos = i;
		}

		if (iPos != 0)
			iPos++;

		return StrCompareW(FileName + iPos, L"Gameexe.dat") == 0;
	};

	auto IsG00Image = [](LPCWSTR FileName)->BOOL
	{
		ULONG Length = StrLengthW(FileName);
		if (Length <= 4)
			return FALSE;

		if (*(PULONG64)&FileName[Length - 4] == TAG4W('.g00'))
			return TRUE;

		if (CHAR_UPPER4W(*(PULONG64)&FileName[Length - 4]) == TAG4W('.G00'))
			return TRUE;

		return FALSE;
	};


	IsScenePck = FALSE;

	if (IsGameExe(lpFileName) && (PatchOnlyOnce == FALSE))
	{
		/*
		___:005B58FF                 movzx   eax, si
		___:005B5902                 cmp     ds:byte_AC3CC0[eax], 1
		___:005B5909                 jnz     short loc_5B5918
		___:005B590B                 mov     eax, [ebx+0F0h]
		___:005B5911                 cdq
		___:005B5912                 sub     eax, edx
		___:005B5914                 sar     eax, 1
		___:005B5916                 jmp     short loc_5B591E
		*/
		//1B5914
		PatchCodeNope((PBYTE)(ThisModule + 0x1B5914), 2);
		//1B5902
		//直接干掉查表
		PatchCodeNope((PBYTE)(ThisModule + 0x1B5902), 9);
		//干掉Check disk

		/*
		sig:
		___:005F492F                 push    offset asc_A1F7E0 ; "\n"
		___:005F4934                 lea     ecx, [ebp-44h]
		___:005F4937                 mov     dword ptr [ebp-34h], 0
		___:005F493E                 mov     [ebp-44h], ax
		___:005F4942                 call    sub_50B080
		___:005F4947                 mov     byte ptr [ebp-4], 2
		___:005F494B                 lea     ecx, [ebp-5Ch]
		___:005F494E                 push    2
		*/

		//___:005F4900                 mov     bl, al
		//mov bl, 1
		static BYTE Opcode[] = { 0xb3, 0x01 };
		PatchReplaceCode((PBYTE)(ThisModule + 0x1F4900), Opcode, countof(Opcode));

		//mov eax, 1
		static BYTE Opcode2[] = { 0xb8, 0x01, 0x00, 0x00, 0x00 };
		PatchReplaceCode((PBYTE)(ThisModule + 0x1F48F2), Opcode2, countof(Opcode2));
		PatchOnlyOnce = TRUE;
	}

	Handle = StubCreateFileW(
		lpFileName,
		dwDesiredAccess,
		dwShareMode,
		lpSecurityAttributes,
		dwCreationDisposition,
		dwFlagsAndAttributes,
		hTemplateFile
		);

	return Handle;
}



BOOL FASTCALL Initialize(HMODULE DllModule)
{
	PIMAGE_DOS_HEADER   DosHeader;
	PIMAGE_NT_HEADERS32 NtHeader;
	PIMAGE_SECTION_HEADER SectionHeader;
	ULONG_PTR FirstSection;
	ULONG_PTR FirstSize;
	ULONG     i;
	NTSTATUS   Status;

	ThisModule = (ULONG_PTR)Nt_GetExeModuleHandle();

	DosHeader = (PIMAGE_DOS_HEADER)Nt_GetExeModuleHandle();
	NtHeader = (PIMAGE_NT_HEADERS32)((ULONG_PTR)DosHeader + DosHeader->e_lfanew);
	SectionHeader = IMAGE_FIRST_SECTION(NtHeader);
	FirstSection = SectionHeader->VirtualAddress + (ULONG_PTR)DosHeader;
	FirstSize = SectionHeader->Misc.VirtualSize;
	i = 0;

	while (i++ < NtHeader->FileHeader.NumberOfSections)
	{
		if (*(PDWORD)&SectionHeader++->Name[1] == 'crsr')
		{
			while (*(PDWORD)&SectionHeader->Name[1] != 'tadi'&&
				*(PWORD)&SectionHeader->Name[5] != 'a'&&
				++i < NtHeader->FileHeader.NumberOfSections)//寻找.idata段
			{
				SectionHeader++;
			}

			if (*(PDWORD)&SectionHeader->Name[1] == 'ttes'&&
				*(PWORD)&SectionHeader->Name[5] == 'ce')//settec ---AlphaROM
			{
				//hGFNA = InstallHookStub(GetModuleFileNameA, My_GetModuleFileNameA);
			}

			if (i < NtHeader->FileHeader.NumberOfSections)
			{
				Mp::PATCH_MEMORY_DATA p[] =
				{
					Mp::FunctionJumpVa(VirtualAlloc, HookVirtualAlloc, &StubVirtualAlloc),
				};

				Mp::PatchMemory(p, countof(p));
			}
			else
			{
				DWORD dwOld;
				VirtualProtect((PVOID)FirstSection, FirstSize, PAGE_EXECUTE_READWRITE, &dwOld);

				Mp::PATCH_MEMORY_DATA p[] =
				{
					Mp::FunctionJumpVa(GetProcAddress, HookGetProcAddress, &StubGetProcAddress),
				};

				Mp::PatchMemory(p, countof(p));
			}
			break;
		}
	}

	Mp::PATCH_MEMORY_DATA p[] =
	{
		Mp::FunctionJumpVa(GetTimeZoneInformation, HookGetTimeZoneInformation, &StubGetTimeZoneInformation),
		Mp::FunctionJumpVa(GetLocaleInfoW, HookGetLocaleInfoW, &StubGetLocaleInfoW),
		Mp::FunctionJumpVa(GetFileVersionInfoSizeW, HookGetFileVersionInfoSizeW, &StubGetFileVersionInfoSizeW),
		Mp::FunctionJumpVa(GetFileVersionInfoW, HookGetFileVersionInfoW, &StubGetFileVersionInfoW),
		//Mp::FunctionJumpVa(EnumFontFamiliesExW,     HookEnumFontFamiliesExW,     &StubEnumFontFamiliesExW),
		Mp::FunctionJumpVa(CreateFontIndirectW, HookCreateFontIndirectW, &StubCreateFontIndirectW),
		Mp::FunctionJumpVa(CreateFileW, HookCreateFileW, &StubCreateFileW),
	};

	return NT_SUCCESS(Mp::PatchMemory(p, countof(p)));
}



BOOL APIENTRY DllMain(HMODULE hModule, DWORD Reason, LPVOID lpReserved)
{
	switch (Reason)
	{
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(hModule);
		ml::MlInitialize();
		return Initialize(hModule);

	case DLL_PROCESS_DETACH:
		ml::MlUnInitialize();
		break;
	}
	return TRUE;
}
