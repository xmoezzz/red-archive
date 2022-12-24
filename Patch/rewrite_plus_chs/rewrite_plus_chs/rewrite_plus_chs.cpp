#include "my.h"
#include "cxdec.h"
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
		return StubGetFileVersionInfoSizeW(L"rewrite_plus_chs.dll", lpdwHandle);

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
		return StubGetFileVersionInfoW(L"rewrite_plus_chs.dll", dwHandle, dwLen, lpData);

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

//0xAF7F0C
HDC GetDrawDC()
{
	HDC ReturnDC;

	INLINE_ASM
	{
		mov eax, 0x6F7F0C;
		add eax, ThisModule;
		mov eax, [eax]
			mov eax, [eax]
			mov ReturnDC, eax
	}
	return ReturnDC;
}

DWORD CDECL GetTextWidth(LONG c)
{
	WCHAR Chars[4] = { 0 };
	SIZE  s;

	memcpy(Chars, &c, sizeof(LONG));
	MAT2 mat2 = { { 0, 1 }, { 0, 0 }, { 0, 0 }, { 0, 1 } };
	POINT buffer[1024];
	GLYPHMETRICS gm;

	GetGlyphOutlineW(GetDrawDC(), Chars[0], GGO_BEZIER, &gm, sizeof(buffer), buffer, &mat2);
	GetTextExtentPoint32W(GetDrawDC(), Chars, StrLengthW(Chars), &s);

	TTPOLYGONHEADER* pTTPH = (TTPOLYGONHEADER*)buffer;

	PrintConsoleW(L"[%s] (%d, %d) %d %d %d %d\n", Chars, gm.gmptGlyphOrigin.x, gm.gmptGlyphOrigin.y,
		gm.gmBlackBoxX, gm.gmBlackBoxY, gm.gmCellIncX, gm.gmCellIncY);

	PrintConsole(L"%d, %d\n", s.cx, s.cy);
	return min(min(gm.gmCellIncX - gm.gmBlackBoxX, gm.gmCellIncY - gm.gmBlackBoxY), 60); //gm.gmCellIncX - gm.gmBlackBoxX;
}

ULONG Width1 = 0;
ASM Void FixWidth1()
{
	INLINE_ASM
	{
		pushad;
		push esi;
		call GetTextWidth;
		add  esp, 4;
		mov  Width1, eax;
		popad;
		mov  eax, ThisModule;
		add  eax, 0x1B7541;
		push eax;
		mov  eax, Width1;
		add  eax, [ebp - 0x4BC];
		retn;
	}
}


ULONG Width2 = 0;
ASM Void FixWidth2()
{
	INLINE_ASM
	{
		pushad;
		push esi;
		call GetTextWidth;
		add  esp, 4;
		mov  Width2, eax;
		popad;
		mov  eax, ThisModule;
		add  eax, 0x1B754F;
		push eax;
		mov  eax, Width2;
		add  eax, [ebp - 0x4BC];
		retn;
	}
}


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

		//fuck table generation code
		PatchCodeNope((PBYTE)(ThisModule + 0x280C1C), 6);

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

	if (IsGameExe(lpFileName))
	{
		CurFileName = ReplaceFileNameExtension(std::wstring(lpFileName), L".chs");
		Attribute = Nt_GetFileAttributes(CurFileName.c_str());

		if ((Attribute == 0xffffffff) || (Attribute & FILE_ATTRIBUTE_DIRECTORY))
			CurFileName = lpFileName;
	}
	else if (IsScenePack(lpFileName))
	{
		CurFileName = ReplaceFileNameExtension(std::wstring(lpFileName), L".chs");
		Attribute = Nt_GetFileAttributes(CurFileName.c_str());

		IsScenePck = TRUE;

		if ((Attribute == 0xffffffff) || (Attribute & FILE_ATTRIBUTE_DIRECTORY))
			CurFileName = lpFileName;
	}
	else if (IsG00Image(lpFileName))
	{
		CurFileName = ReplaceFileNameExtension(std::wstring(lpFileName), L".g01");
		Attribute = Nt_GetFileAttributes(CurFileName.c_str());

		if ((Attribute == 0xffffffff) || (Attribute & FILE_ATTRIBUTE_DIRECTORY))
			CurFileName = lpFileName;
	}
	else
	{
		CurFileName = lpFileName;
	}

	Handle = StubCreateFileW(
		CurFileName.c_str(),
		dwDesiredAccess,
		dwShareMode,
		lpSecurityAttributes,
		dwCreationDisposition,
		dwFlagsAndAttributes,
		hTemplateFile
		);

	return Handle;
}


const UINT32 table[] = {
	0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f, 0xe963a535, 0x9e6495a3,
	0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988, 0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91,
	0x1db71064, 0x6ab020f2, 0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
	0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9, 0xfa0f3d63, 0x8d080df5,
	0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172, 0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,
	0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
	0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423, 0xcfba9599, 0xb8bda50f,
	0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924, 0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d,
	0x76dc4190, 0x01db7106, 0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
	0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d, 0x91646c97, 0xe6635c01,
	0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e, 0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457,
	0x65b0d9c6, 0x12b7e950, 0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
	0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb,
	0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0, 0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9,
	0x5005713c, 0x270241aa, 0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
	0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81, 0xb7bd5c3b, 0xc0ba6cad,
	0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a, 0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683,
	0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
	0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb, 0x196c3671, 0x6e6b06e7,
	0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc, 0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5,
	0xd6d6a3e8, 0xa1d1937e, 0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
	0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55, 0x316e8eef, 0x4669be79,
	0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236, 0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f,
	0xc5ba3bbe, 0xb2bd0b28, 0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
	0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f, 0x72076785, 0x05005713,
	0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38, 0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21,
	0x86d3d2d4, 0xf1d4e242, 0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
	0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69, 0x616bffd3, 0x166ccf45,
	0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2, 0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db,
	0xaed16a4a, 0xd9d65adc, 0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
	0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693, 0x54de5729, 0x23d967bf,
	0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94, 0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d,
};

UINT32 GetCRC(BYTE* buf, int nLength)
{
	if (nLength < 1)
		return 0xffffffff;

	UINT32 crc = 0;

	for (int i = 0; i != nLength; ++i)
	{
		crc = table[(crc ^ buf[i]) & 0xff] ^ (crc >> 8);
	}

	crc = crc ^ 0xffffffff;
	return crc;
}


BOOL CheckExe();

BOOL FASTCALL Initialize(HMODULE DllModule)
{
	PIMAGE_DOS_HEADER   DosHeader;
	PIMAGE_NT_HEADERS32 NtHeader;
	PIMAGE_SECTION_HEADER SectionHeader;
	ULONG_PTR FirstSection;
	ULONG_PTR FirstSize;
	ULONG     i;
	NtFileDisk File;
	NTSTATUS   Status;
	DWORD      Crc32;
	BYTE       StackMemory[0x100];
	WCHAR      FontNameLocal[MAX_PATH];

#if 0
	if (!CheckExe())
	{
		MessageBoxW(NULL, L"本补丁要求SiglusEngine.exe没经过任何方式的破解\n或者游戏版本不兼容", L"补丁错误", MB_OK | MB_ICONERROR);
		Ps::ExitProcess(114514);
	}
#endif

	LOOP_ONCE
	{
		RtlZeroMemory(StackMemory, sizeof(StackMemory));
		RtlZeroMemory(FontNameLocal, sizeof(FontNameLocal));

		Status = File.Open(L"Anz.ini");
		if (NT_FAILED(Status))
			break;

		File.Read(StackMemory, File.GetSize32());
		Crc32 = GetCRC(StackMemory + 4, File.GetSize32() - 4);
		if (Crc32 == *(PDWORD)StackMemory)
		{
			MultiByteToWideChar(CP_UTF8, 0, (PCSTR)StackMemory + 4, File.GetSize32() - 4, FontNameLocal, countof(FontNameLocal));
			GlobalFontName = FontNameLocal;
		}
		File.Close();
	}

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
		//AllocConsole();
		DisableThreadLibraryCalls(hModule);
		ml::MlInitialize();
		return Initialize(hModule);

	case DLL_PROCESS_DETACH:
		ml::MlUnInitialize();
		break;
	}
	return TRUE;
}
