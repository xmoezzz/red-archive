#include <Windows.h>
#include "Common.h"
#include "detours.h"
#include <windows.h>
#include "WinFile.h"
#include <string>
#include <vector>
#include "MiniNtdll.h"
#include "FileManager.h"

using std::string;
using std::vector;

#pragma comment(lib, "detours.lib")

char* mbsPathPointer = nullptr;
ULONG dwMajorVersion = 6;

VOID WINAPI OutputStringA(const CHAR* lpString)
{
	HANDLE hOutputHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	DWORD nRet = 0;
	WriteConsoleA(hOutputHandle, lpString, lstrlenA(lpString), &nRet, NULL);
}

VOID WINAPI OutputStringW(const WCHAR* lpString)
{
	HANDLE hOutputHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	DWORD nRet = 0;
	WriteConsoleW(hOutputHandle, lpString, lstrlenW(lpString), &nRet, NULL);
}

/************************************/

string GetExtension(const char* lpString)
{
	string tmp(lpString);
	return tmp.substr(tmp.find_last_of(".") + 1, string::npos);
}

string GetFileName(const char* lpString)
{
	string tmp(lpString);
	return tmp.substr(tmp.find_last_of("\\") + 1, string::npos);
}


PVOID pfOldCreateFontIndirectA = NULL;
typedef HFONT(WINAPI *PfuncCreateFontIndirectA)(LOGFONTA *lplf);
HFONT WINAPI NewCreateFontIndirectA(LOGFONTA *lplf)
{
	LOGFONTW Info = { 0 };

	Info.lfHeight = lplf->lfHeight;
	Info.lfWidth = lplf->lfWidth;
	Info.lfEscapement = lplf->lfEscapement;
	Info.lfOrientation = lplf->lfOrientation;
	Info.lfWeight = lplf->lfWeight;
	Info.lfItalic = lplf->lfItalic;
	Info.lfUnderline = lplf->lfUnderline;
	Info.lfStrikeOut = lplf->lfStrikeOut;
	Info.lfCharSet = GB2312_CHARSET;
	Info.lfOutPrecision = lplf->lfOutPrecision;
	Info.lfClipPrecision = lplf->lfClipPrecision;
	Info.lfQuality = lplf->lfQuality;
	Info.lfPitchAndFamily = lplf->lfPitchAndFamily;
	lstrcpyW(Info.lfFaceName, L"黑体");
	lplf->lfCharSet = GB2312_CHARSET;

	return CreateFontIndirectW(&Info);
	//return ((PfuncCreateFontIndirectA)pfOldCreateFontIndirectA)(lplf);
}

PVOID pfMessageBoxA = NULL;
typedef int (WINAPI *PfunHookMessageBox)(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType);
int WINAPI HookMessageBox(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType)
{
	int result = 0;
	WCHAR* WInfo = (WCHAR*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, 1024);
	WCHAR* WTitle = (WCHAR*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, 1024);
	if (WInfo == NULL || WTitle == NULL)
	{
		result = ((PfunHookMessageBox)pfMessageBoxA)(hWnd, lpText, lpCaption, uType);
		if (WInfo)
		{
			HeapFree(GetProcessHeap(), 0, WInfo);
		}
		if (WTitle)
		{
			HeapFree(GetProcessHeap(), 0, WTitle);
		}
	}
	else
	{
		MultiByteToWideChar(932, 0, lpText, lstrlenA(lpText), WInfo, 1024);
		MultiByteToWideChar(CP_ACP, 0, lpCaption, lstrlenA(lpCaption), WTitle, 1024);

		result = MessageBoxW(hWnd, WInfo, WTitle, uType);

		HeapFree(GetProcessHeap(), 0, WInfo);
		HeapFree(GetProcessHeap(), 0, WTitle);
	}
	return result;
}

PVOID pfGetStringTypeA = NULL;
typedef BOOL(WINAPI *PfunGetStringTypeA)(LCID Locale, DWORD dwInfoType, LPCSTR lpSrcStr,
	int cchSrc, LPWORD lpCharType);

BOOL WINAPI HookGetStringTypeA(
	_In_  LCID   Locale,
	_In_  DWORD  dwInfoType,
	_In_  LPCSTR lpSrcStr,
	_In_  int    cchSrc,
	_Out_ LPWORD lpCharType
	)
{
	return (PfunGetStringTypeA(pfGetStringTypeA))(0x0804, dwInfoType, lpSrcStr, cchSrc, lpCharType);
}


/******************************************/
//PAL.dll

PVOID pfCreateFontA = NULL;
typedef HFONT(WINAPI *PfunHookCreateFont)(int  nHeight, int nWidth, int nEscapement,
	int nOrientation, int fnWeight, DWORD fdwItalic, DWORD fdwUnderline, DWORD fdwStrikeOut,
	DWORD fdwCharSet, DWORD fdwOutputPrecision, DWORD fdwClipPrecision, DWORD fdwQuality,
	DWORD fdwPitchAndFamily, LPSTR lpszFace);

HFONT WINAPI HookCreateFont(
	_In_ int     nHeight,
	_In_ int     nWidth,
	_In_ int     nEscapement,
	_In_ int     nOrientation,
	_In_ int     fnWeight,
	_In_ DWORD   fdwItalic,
	_In_ DWORD   fdwUnderline,
	_In_ DWORD   fdwStrikeOut,
	_In_ DWORD   fdwCharSet,
	_In_ DWORD   fdwOutputPrecision,
	_In_ DWORD   fdwClipPrecision,
	_In_ DWORD   fdwQuality,
	_In_ DWORD   fdwPitchAndFamily,
	_In_ LPSTR lpszFace
	)
{
	return CreateFontW(nHeight, nWidth, nEscapement, nOrientation,
		fnWeight, fdwItalic, fdwUnderline, fdwStrikeOut, GB2312_CHARSET,
		fdwOutputPrecision, fdwClipPrecision, fdwQuality, fdwPitchAndFamily,
		L"黑体");
}


PVOID pfGetGlyphOutlineA = NULL;
typedef DWORD(WINAPI *PfunGetGlyphOutline)(HDC hdc, UINT uChar, UINT uFormat,
	LPGLYPHMETRICS lpgm, DWORD cbBuffer, LPVOID lpvBuffer, const MAT2 *lpmat2);

DWORD WINAPI HookGetGlyphOutline(
	_In_        HDC            hdc,
	_In_        UINT           uChar,
	_In_        UINT           uFormat,
	_Out_       LPGLYPHMETRICS lpgm,
	_In_        DWORD          cbBuffer,
	_Out_       LPVOID         lpvBuffer,
	_In_  const MAT2           *lpmat2
	)
{
	UINT OldChar = uChar;
	int len;
	char mbchs[2];
	UINT cp = 936;
	if (IsDBCSLeadByteEx(cp, uChar >> 8))
	{
		len = 2;
		mbchs[0] = (uChar & 0xff00) >> 8;
		mbchs[1] = (uChar & 0xff);
	}
	else
	{
		len = 1;
		mbchs[0] = (uChar & 0xff);
	}
	uChar = 0;
	MultiByteToWideChar(cp, 0, mbchs, len, (LPWSTR)&uChar, 1);

	DWORD Result = 0;

#if 1
	if (LOWORD(uChar) == (WORD)0x2467)
	{
		LOGFONTW* lplf = (LOGFONTW*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(LOGFONTW));

		HFONT hOldFont = (HFONT)GetCurrentObject(hdc, OBJ_FONT);
		GetObject(hOldFont, sizeof(LOGFONTW), lplf);
		LOGFONTW Info = { 0 };

		Info.lfHeight = lplf->lfHeight;
		Info.lfWidth = lplf->lfWidth;
		Info.lfEscapement = lplf->lfEscapement;
		Info.lfOrientation = lplf->lfOrientation;
		Info.lfWeight = lplf->lfWeight;
		Info.lfItalic = lplf->lfItalic;
		Info.lfUnderline = lplf->lfUnderline;
		Info.lfStrikeOut = lplf->lfStrikeOut;
		Info.lfOutPrecision = lplf->lfOutPrecision;
		Info.lfClipPrecision = lplf->lfClipPrecision;
		Info.lfQuality = lplf->lfQuality;
		Info.lfPitchAndFamily = lplf->lfPitchAndFamily;
		lstrcpyW(Info.lfFaceName, L"MS Gothic");
		lplf->lfCharSet = SHIFTJIS_CHARSET;

		HFONT hFont = CreateFontIndirectW(&Info);

		hOldFont = (HFONT)SelectObject(hdc, hFont);
		//6A 26 
		Result = GetGlyphOutlineW(hdc, (UINT)0x266A, uFormat,
			lpgm, cbBuffer, lpvBuffer, lpmat2);

		SelectObject(hdc, hOldFont);
		DeleteObject(hFont);
		HeapFree(GetProcessHeap(), 0, lplf);
	}
	else
	{
		Result = GetGlyphOutlineW(hdc, uChar, uFormat,
			lpgm, cbBuffer, lpvBuffer, lpmat2);
	}
#else
	Result = GetGlyphOutlineW(hdc, uChar, uFormat,
		lpgm, cbBuffer, lpvBuffer, lpmat2);
#endif
	return Result;
}

PVOID pfGetTextMetricsA = NULL;
typedef BOOL(WINAPI* PfunGetTextMetrics)(HDC hdc, LPTEXTMETRICA lptm);

BOOL WINAPI HookGetTextMetrics(
	_In_  HDC          hdc,
	_Out_ LPTEXTMETRICA lptm
	)
{
	lptm->tmCharSet = GB2312_CHARSET;

	return (PfunGetTextMetrics(pfGetTextMetricsA))(hdc, lptm);
}

PVOID pfGetOEMCP = NULL;
UINT WINAPI HookGetOEMCP()
{
	return (UINT)936;
}

PVOID pfGetACP = NULL;
typedef UINT(WINAPI* StubGetACP)();
UINT WINAPI HookGetACP()
{
	return (UINT)936;
}

UINT WINAPI GetACPXmoe()
{
	return ((StubGetACP)pfGetACP)();
}

void WINAPI VirtualMemoryCopy(void* dest, void*src, size_t size)
{
	DWORD oldProtect;
	VirtualProtect(dest, size, PAGE_EXECUTE_READWRITE, &oldProtect);
	memcpy(dest, src, size);
}

void WINAPI SetNopCode(BYTE* pnop, size_t size)
{
	DWORD oldProtect;
	VirtualProtect((PVOID)pnop, size, PAGE_EXECUTE_READWRITE, &oldProtect);
	for (size_t i = 0; i<size; i++)
	{
		pnop[i] = 0x90;
	}
}

/*******************************************************/
//Render
DWORD pfPalLoadSprite = NULL;
typedef DWORD(*PfunPalLoadSprite) (DWORD, CHAR* FileName, PBYTE DataBuffers, DWORD DataSize);
VOID GetBMPData(PBYTE BMPImage, LONG* pdwWidth, LONG* pdwHeight, int* pBitCount, BYTE** OutputBits, DWORD* BitsSize)
{
	BITMAPFILEHEADER* Header;
	BITMAPINFOHEADER* HeaderInfo;
	BYTE* SourceBits;
	BYTE* TargetBits;
	int TargetAlignWidth;
	int SourceAlignWidth;
	int BitCount;
	ULONG TargetBitsLength;
	BYTE* SourceLines;
	BYTE* TargetLines;
	LONG ProcessHeight;
	LONG ProcessWidth;


	Header = (BITMAPFILEHEADER*)BMPImage;
	HeaderInfo = (BITMAPINFOHEADER*)(BMPImage + sizeof(BITMAPFILEHEADER));

	SourceBits = &BMPImage[Header->bfOffBits];

	BitCount = HeaderInfo->biBitCount;

	TargetAlignWidth = (HeaderInfo->biWidth * 32 + 31) / 32;
	SourceAlignWidth = (HeaderInfo->biWidth*BitCount + 31) / 32;

#if 1
	TargetBitsLength = 4 * TargetAlignWidth * HeaderInfo->biHeight;

	TargetBits = (BYTE*)GlobalAlloc(0, TargetBitsLength);
	if (BitCount == 32)
	{
		memcpy(TargetBits, SourceBits, TargetBitsLength);
	}
	else
	{
		for (ProcessHeight = 0; ProcessHeight < HeaderInfo->biHeight; ProcessHeight++)
		{
			SourceLines = &SourceBits[SourceAlignWidth * ProcessHeight * 4];
			TargetLines = &TargetBits[TargetAlignWidth * ProcessHeight * 4];

			for (ProcessWidth = 0; ProcessWidth < HeaderInfo->biWidth; ProcessWidth++)
			{
				TargetLines[0] = SourceLines[0];
				TargetLines[1] = SourceLines[1];
				TargetLines[2] = SourceLines[2];
				if (BitCount == 32)
				{
					TargetLines[3] = SourceLines[3];
				}
				else
				{
					TargetLines[3] = 0xFF;
				}

				SourceLines += (BitCount / 8);
				TargetLines += 4;
			}
		}
	}
#else
	TargetBitsLength = (BitCount / 8) * TargetAlignWidth * HeaderInfo->biHeight;

	TargetBits = (BYTE*)GlobalAlloc(0, TargetBitsLength);
	memcpy(TargetBits, SourceBits, TargetBitsLength);
#endif

	*pdwWidth = HeaderInfo->biWidth;
	*pdwHeight = HeaderInfo->biHeight;
	*pBitCount = BitCount;
	*OutputBits = TargetBits;
	*BitsSize = TargetBitsLength;
}

void BMP_TO_DIB(PBYTE data, int width, int height, int BitCount)
{
	BYTE* TempBuffer;
	int i;
	int widthlen;

	int nAlignWidth = (width * 32 + 31) / 32;
	size_t BufferSize = 4 * nAlignWidth * height;
	TempBuffer = (BYTE*)GlobalAlloc(0, BufferSize);

	widthlen = width * (BitCount / 8);
	for (i = 0; i<height; i++)
	{
		memcpy(&TempBuffer[(((height - i) - 1)*widthlen)], &data[widthlen * i], widthlen);
	}
	memcpy(data, TempBuffer, BufferSize);

	GlobalFree(TempBuffer);
}

BYTE* g_ImageBits  = nullptr;
ULONG g_ImageBitsLength = 0;
LONG g_ImageWidth = 0;
LONG g_ImageHeight = 0;
int g_ImageBitCount = 32;

DWORD PalLoadSprEx(DWORD unk, CHAR* FileName, PBYTE DataBuffers, DWORD DataSize)
{
	DWORD result;

	char ConvertName[128] = { 0 };
	int ConvertNameLen;

	lstrcpyA(ConvertName, FileName);
	ConvertNameLen = lstrlenA(ConvertName);
	ConvertName[ConvertNameLen - 3] = 'b';
	ConvertName[ConvertNameLen - 2] = 'm';
	ConvertName[ConvertNameLen - 1] = 'p';

	ULONG FileSize = 0;
	PBYTE FileBuffer = 0;
	if (FileManager::GetFileManager()->QueryFile(ConvertName, FileBuffer, FileSize))
	{
		GetBMPData(FileBuffer, &g_ImageWidth, &g_ImageHeight, &g_ImageBitCount, &g_ImageBits, &g_ImageBitsLength);
		BMP_TO_DIB(g_ImageBits, g_ImageWidth, g_ImageHeight, 32);
	}

	result = (PfunPalLoadSprite(pfPalLoadSprite))(unk, FileName, DataBuffers, DataSize);

	if (FileBuffer)
	{
		GlobalFree(FileBuffer);
		FileBuffer = NULL;
	}

	if (g_ImageBits)
	{
		GlobalFree(g_ImageBits);
		g_ImageBits = NULL;
	}

	return result;
}

VOID Fix_Width(PBYTE SourceDIB, int Width, int Height, int BitCount, int NewWidth, BYTE** Output, DWORD* OutputLength)
{
	BYTE* TmpBuffer;
	int i, j;
	BYTE* SourceLines;
	BYTE* TargetLines;

	int nAlignWidth = (Width * 32 + 31) / 32;
	int nNewAlignWidth = (NewWidth * 32 + 31) / 32;
	*OutputLength = 4 * nNewAlignWidth * Height;
	*Output = (BYTE*)GlobalAlloc(0, *OutputLength);
	TmpBuffer = *Output;

	memset(TmpBuffer, 0, *OutputLength);

	for (i = 0; i<Height; i++)
	{
		SourceLines = &SourceDIB[nAlignWidth * i * 4];
		TargetLines = &TmpBuffer[nNewAlignWidth * i * 4];
		for (j = 0; j<Width; j++)
		{
			TargetLines[0] = SourceLines[0];
			TargetLines[1] = SourceLines[1];
			TargetLines[2] = SourceLines[2];
			TargetLines[3] = SourceLines[3];
			SourceLines += (BitCount / 8);
			TargetLines += 4;
		}
	}
}

static ULONG FixedList[] = { 1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096 };
#define FixedListLength 13

void PALCopyImage(PBYTE draw_buf, int new_width)
{
	BYTE* Output = NULL;
	ULONG OutputLength;

	ULONG FixedWidth = new_width;
	BOOL Found = FALSE;
	for (ULONG i = 1; i < FixedListLength; i++)
	{
		if (FixedList[i - 1] == FixedWidth || FixedList[i] == FixedWidth)
		{
			Found = TRUE;
			break;
		}
		else if (FixedList[i - 1] < FixedWidth && FixedList[i] > FixedWidth)
		{
			FixedWidth = FixedList[i];
			Found = TRUE;
			break;
		}
	}
	if (g_ImageBits && Found)
	{

		Fix_Width(g_ImageBits, g_ImageWidth, g_ImageHeight, 32, FixedWidth, &Output, &OutputLength);


#if 0
		if (g_ImageBitCount != 32)
		{
			//memcpy(draw_buf, Output, OutputLength);
			memcpy(draw_buf, g_ImageBits, OutputLength);
		}
		else
		{
			nNewAlignWidth = (new_width * 32 + 31) / 32;

			for (i = 0; i<g_ImageHeight; i++)
			{
				SourceLines = &Output[nNewAlignWidth * i * 4];
				TargetLines = &draw_buf[nNewAlignWidth * i * 4];
				for (j = 0; j<new_width; j++)
				{
					TargetLines[0] = SourceLines[0];
					TargetLines[1] = SourceLines[1];
					TargetLines[2] = SourceLines[2];
					TargetLines[3] = SourceLines[3];

					SourceLines += 4;
					TargetLines += 4;
				}
			}
		}
#else
		memcpy(draw_buf, Output, OutputLength);
#endif
	}
	if (Output)
	{
		GlobalFree(Output);
		Output = NULL;
	}
}

int screen_width;

PBYTE type3_draw_1_ret;
PBYTE type3_draw_1_call;

PBYTE type3_draw_2_ret;
PBYTE type3_draw_2_call;

PBYTE type3_draw_3_ret;
PBYTE type3_draw_3_call;

PBYTE type3_draw_4_ret;
PBYTE type3_draw_4_call;


__declspec(naked)void update_bmp_draw_4()
{
	__asm
	{
		push[esp + 4]
			pop screen_width
			call type3_draw_4_call
			pushad
			push screen_width
			push dword ptr[ebp + 8]
			call PALCopyImage
			add esp, 0x8
			popad
			jmp type3_draw_4_ret
	}
}

__declspec(naked)void update_bmp_draw_3()
{
	__asm
	{
		push[esp + 0xC]
			pop screen_width
			call type3_draw_3_call
			pushad
			push screen_width
			push dword ptr[ebp + 8]
			call PALCopyImage
			add esp, 0x8
			popad
			jmp type3_draw_3_ret
	}
}

__declspec(naked)void update_bmp_draw_2()
{
	__asm
	{
		push[esp + 0xC]
			pop screen_width
			call type3_draw_2_call
			pushad
			push screen_width
			push dword ptr[ebp + 8]
			call PALCopyImage
			add esp, 0x8
			popad
			jmp type3_draw_2_ret
	}
}
__declspec(naked)void update_bmp_draw_1()
{
	__asm
	{
		push[esp + 4]
			pop screen_width
			call type3_draw_1_call
			pushad
			push screen_width
			push dword ptr[ebp + 8]
			call PALCopyImage
			add esp, 0x8
			popad
			jmp type3_draw_1_ret
	}
}

void ReplaceJump(PBYTE src, PBYTE dst)
{
	DWORD oldProtect;

	VirtualProtect((LPVOID)src, 10, PAGE_EXECUTE_READWRITE, &oldProtect);

	src[0] = 0xE9;
	*(DWORD*)&src[1] = (DWORD)(dst - src - 5);
}

PBYTE GetCALLTarget(PBYTE cal)
{
	return cal + *(DWORD*)&cal[1] + 5;
}

//ok
VOID WINAPI PatchRender(HMODULE hModule)
{

	PBYTE PalModule = (PBYTE)hModule;
	/*
	(int)&unk_1312B8F8,
	(unsigned int *)v10,
	*/
	PBYTE PatchAddr = &PalModule[0x1C601];
	type3_draw_1_ret = PatchAddr + 5;
	type3_draw_1_call = GetCALLTarget(PatchAddr);
	ReplaceJump(PatchAddr, (PBYTE)&update_bmp_draw_1);

	PatchAddr = PalModule + 0x21108;
	type3_draw_2_ret = PatchAddr + 5;
	type3_draw_2_call = GetCALLTarget(PatchAddr);
	ReplaceJump(PatchAddr, (PBYTE)&update_bmp_draw_2);

	//---------------------------------------------
	/*
	CPU Disasm
	Address               Hex dump                  Command                                  Comments
	100A439C              |.  E8 EFFEFFFF           call 100A4290
	*/

	/********************/
	//32Bit
	PatchAddr = PalModule + 0x2115C;
	type3_draw_3_ret = PatchAddr + 5;
	type3_draw_3_call = GetCALLTarget(PatchAddr);
	ReplaceJump(PatchAddr, (PBYTE)&update_bmp_draw_3);

	/*
	CPU Disasm
	Address               Hex dump                  Command                                  Comments
	100A43A6              |> \E8 75FCFFFF           call 100A4020
	*/

	PatchAddr = PalModule + 0x2117E;
	type3_draw_4_ret = PatchAddr + 5;
	type3_draw_4_call = GetCALLTarget(PatchAddr);
	ReplaceJump(PatchAddr, (PBYTE)&update_bmp_draw_4);

	//sub_10022DA0
	pfPalLoadSprite = (DWORD)&PalModule[0x22DA0];

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach((void**)&pfPalLoadSprite, PalLoadSprEx);
	DetourTransactionCommit();
}


//004011B6  |.  83FA 24       cmp edx,24
//004012A6  |.  83FA 24       cmp edx,24
//00401396  |.  83FA 24       cmp edx,24
//00401486  |.  83FA 24       cmp edx,24
//00403F22  |.  83FA 24       cmp edx,24

void WINAPI InstallAfterCreate()
{
#if 0
	static BYTE HandlePatch1[] = { 0x83, 0xFA, 0x23 };
	VirtualMemoryCopy((PVOID)0x004011B6, HandlePatch1, sizeof(HandlePatch1));
	VirtualMemoryCopy((PVOID)0x004012A6, HandlePatch1, sizeof(HandlePatch1));
	VirtualMemoryCopy((PVOID)0x00401396, HandlePatch1, sizeof(HandlePatch1));
	VirtualMemoryCopy((PVOID)0x00401486, HandlePatch1, sizeof(HandlePatch1));
	VirtualMemoryCopy((PVOID)0x00403F22, HandlePatch1, sizeof(HandlePatch1));
#endif
}

//10008A2A                81FF 40810000           cmp edi,8140
//100082ED                81FF 40810000           cmp edi,8140
//Checked
void WINAPI InstallSpacePatch(BYTE* Offset)
{
	PVOID pfSapceCheck1 = (PVOID)&Offset[0x8A2A];
	static BYTE SpacePatch1[] = { 0x81, 0xFF, 0xA1, 0xA1 };
	VirtualMemoryCopy(pfSapceCheck1, SpacePatch1, sizeof(SpacePatch1));

	PVOID pfSapceCheck2 = (PVOID)&Offset[0x82ED];
	static BYTE SpacePatch2[] = { 0x81, 0xFF, 0xA1, 0xA1 };
	VirtualMemoryCopy(pfSapceCheck2, SpacePatch2, sizeof(SpacePatch2));
}

//ok
void WINAPI HookPalFontSetType(HMODULE hModule)
{
	BYTE* pfPalFontSetType = (BYTE*)GetProcAddress((HMODULE)hModule, "PalFontSetType");
	SetNopCode((pfPalFontSetType + 0x1E), 7);
}

BOOL IATPatch(LPCSTR szDllName, PROC pfnOrg, PROC pfnNew)
{
	HMODULE hmod;
	LPCSTR szLibName;
	PIMAGE_IMPORT_DESCRIPTOR pImportDesc;
	PIMAGE_THUNK_DATA pThunk;
	DWORD dwOldProtect, dwRVA;
	PBYTE pAddr;

	hmod = GetModuleHandleW(NULL);
	pAddr = (PBYTE)hmod;
	pAddr += *((DWORD*)&pAddr[0x3C]);
	dwRVA = *((DWORD*)&pAddr[0x80]);

	pImportDesc = (PIMAGE_IMPORT_DESCRIPTOR)((DWORD)hmod + dwRVA);

	for (; pImportDesc->Name; pImportDesc++)
	{
		szLibName = (LPCSTR)((DWORD)hmod + pImportDesc->Name);
		if (!stricmp(szLibName, szDllName))
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


BOOL IATPatchPAL(LPCSTR szDllName, PROC pfnOrg, PROC pfnNew)
{
	HMODULE hmod;
	LPCSTR szLibName;
	PIMAGE_IMPORT_DESCRIPTOR pImportDesc;
	PIMAGE_THUNK_DATA pThunk;
	DWORD dwOldProtect, dwRVA;
	PBYTE pAddr;

	hmod = GetModuleHandleW(L"PAL.dll");
	pAddr = (PBYTE)hmod;
	pAddr += *((DWORD*)&pAddr[0x3C]);
	dwRVA = *((DWORD*)&pAddr[0x80]);

	pImportDesc = (PIMAGE_IMPORT_DESCRIPTOR)((DWORD)hmod + dwRVA);

	for (; pImportDesc->Name; pImportDesc++)
	{
		szLibName = (LPCSTR)((DWORD)hmod + pImportDesc->Name);
		if (!stricmp(szLibName, szDllName))
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


HRESULT WINAPI InstallDllPatch(HMODULE hModule)
{
	InstallSpacePatch((PBYTE)hModule);

	return S_OK;
}

static HMODULE hPALModule = 0;

LPVOID pfLoadLibrary = NULL;
typedef HMODULE(WINAPI* PfunLoadLibrary)(LPCSTR lpFileName);

//Hook PAL.dll
HMODULE WINAPI HookLoadLibrary(LPCSTR lpFileName)
{
	if (strstr(lpFileName, "PAL.dll"))
	{
		if (!hPALModule)
		{
			hPALModule = ((PfunLoadLibrary)pfLoadLibrary)(lpFileName);
			InstallDllPatch(hPALModule);
			HookPalFontSetType(hPALModule);
			PatchRender(hPALModule);

		}
		return hPALModule;
	}
	else
	{
		return ((PfunLoadLibrary)pfLoadLibrary)(lpFileName);
	}
}

static vector<string> TextPool;
static ULONG TextCount = 0;
#define TextMagic "_TEXT_LIST__"

void WINAPI AttachTextBuffer(const PBYTE FileBuffer, const ULONG Length)
{
	if (RtlCompareMemory(FileBuffer, TextMagic, 0xC) != 0xC)
	{
		MessageBoxW(NULL, L"汉化文件被损毁", L"无法启动游戏", MB_OK);
		ExitProcess(-1);
	}
	ULONG Offset = 0xC;
	ULONG Count = *(ULONG*)(FileBuffer + Offset);
	TextCount = Count;
	Offset += 4;
	for (ULONG Index = 0; Index < Count; Index++)
	{
		Offset += 4;
		TextPool.push_back((char*)(FileBuffer + Offset));
		Offset += lstrlenA((char*)(FileBuffer + Offset)) + 1;
	}
}

//OK
char* (*pfLoadText)(int Args1, int Args2, int Args3, int Args4);

char* HookLoadText(int Args1, int Args2, int Args3, int Args4)
{
	char* result = nullptr;

	result = pfLoadText(Args1, Args2, Args3, Args4);

	if ((Args3 & 0x10000000) || Args4 == 0xFFFFFFF)
	{
		return result;
	}

	ULONG Index = *(ULONG*)result;
	if (Index < TextCount)
	{
		char* pStringStart = result + sizeof(ULONG);
		lstrcpyA(pStringStart, TextPool[Index].c_str());
	}
	return result;
}

///Text.Dat的处理
void WINAPI ReDirectReadFileData()
{
	//测试时候使用ProjectDir
	PBYTE FileBuffer = NULL;
	ULONG FileSize = 0;

	if (FileManager::Handle->QueryFile("TEXT_chs.DAT", FileBuffer, FileSize) == false)
	{
		MessageBoxW(NULL, L"缺少汉化文件", L"游戏启动失败", MB_OK);
		ExitProcess(-1);
	}
	AttachTextBuffer(FileBuffer, FileSize);
	GlobalFree(FileBuffer);
}


//00424B90
//OK
BOOL HookCheckFontProc(int a1)
{
	return ((signed int)*(BYTE *)a1 >= 0x81 && (signed int)*(BYTE *)a1 <= 0xFE)
		&& (((signed int)*(BYTE *)(a1 + 1) >= 0x40 && (signed int)*(BYTE *)(a1 + 1) <= 0x7E)
		|| ((signed int)*(BYTE *)(a1 + 1) >= 0x80 && (signed int)*(BYTE *)(a1 + 1) <= 0xFE));
}


__declspec(dllexport)WCHAR* WINAPI GetPatchVersion()
{
	return L"2015/12/06";
}

#include <Shlobj.h>
#include <Shlwapi.h>

#pragma comment(lib, "Shell32.lib")
#pragma comment(lib, "Shlwapi.lib")

//sub_431430
BOOL FakeDiskChecker(LPSTR String, int args)
{
	return TRUE;
}


//xp Save
//sub_431530
LPVOID pfFakeSaver = (LPVOID)0x00431530;
typedef int(*StubFakeSaver)(char*);

static  char mbsPath[MAX_PATH] = { 0 };
static WCHAR WidePath[MAX_PATH] = { 0 };

int FakeSaver(char* Path)
{
	/*
	OutputStringA(Path);
	OutputStringA("\n");
	RtlCopyMemory(Path, mbsPathPointer, MAX_PATH);

	OutputStringA(Path);
	OutputStringA("\n\n");
	*/

	if (dwMajorVersion < 6 || GetACPXmoe() != 936)
	{
		if (!IsBadReadPtr(Path, MAX_PATH) && !IsBadWritePtr(Path, MAX_PATH))
		{
			RtlCopyMemory(Path, mbsPath, MAX_PATH);
			MultiByteToWideChar(CP_ACP, 0, Path, lstrlenA(Path), WidePath, 260);
			SHCreateDirectory(nullptr, WidePath);
			return 1;
		}
		else
		{
			MultiByteToWideChar(CP_ACP, 0, mbsPath, lstrlenA(mbsPath), WidePath, 260);
			SHCreateDirectory(nullptr, WidePath);
			return 1;
		}
	}
	else
	{
		RtlCopyMemory(mbsPath, Path, MAX_PATH);
		return ((StubFakeSaver)pfFakeSaver)(Path);
	}
}

//HANDLE __cdecl sub_41B6C0(int a1, __int16 a2)
LPVOID pfCreateSave = (LPVOID)0x0041B6C0;
typedef HANDLE(__cdecl* StubCreateSave)(int a1, __int16 a2);

HANDLE __cdecl CreateSave(int a1, __int16 a2)
{
	if (dwMajorVersion < 6)
	{
		HANDLE result; // eax@5
		CHAR pszPath[260] = { 0 }; // [sp+0h] [bp-118h]@2
		DWORD dwDesiredAccess; // [sp+110h] [bp-8h]@6
		DWORD dwShareMode; // [sp+114h] [bp-4h]@6

		if (a1 == -1)
		{
			wsprintfA(pszPath, "%s\\%s", mbsPathPointer, "continue.dat");
		}
		else
		{
			wsprintfA(pszPath, "%s\\save%03d.dat", mbsPathPointer, a1);
		}
		if (PathFileExistsA(pszPath))
		{
			dwDesiredAccess = 0;
			dwShareMode = 0;
			if (a2 & 1)
			{
				dwDesiredAccess |= 0x80000000;
				dwShareMode |= 1u;
			}
			if (a2 & 2)
			{
				dwDesiredAccess |= 0x40000000u;
				dwShareMode |= 2u;
			}
			result = CreateFileA(pszPath, dwDesiredAccess, dwShareMode, 0, 3u, 0x80u, 0);
		}
		else
		{
			//PalDebugPrintf(10, 0, "No SaveData. Therefore disregards");
			result = (HANDLE)-1;
		}
		return result;
	}
	else
	{
		return ((StubCreateSave)pfCreateSave)(a1, a2);
	}
}

/*
CPU Disasm
Address               Hex dump                  Command                                  Comments
0041C87F              |.  68 00264500           push offset 00452600                     ; |<%s>
*/

PVOID SaveChange1Start = (PVOID)0x0041C87F;
PVOID SaveChange1End = (PVOID)0x0041C884;
ULONG EaxCh1 = 0;
__declspec(naked) void SaveChange1()
{
	__asm
	{
		mov EaxCh1, eax
			mov eax, mbsPathPointer
			push eax
			mov eax, EaxCh1
		jmp SaveChange1End
	}
}


/*
CPU Disasm
Address               Hex dump                  Command                                  Comments
0041C8A0              |.  68 00264500           push offset 00452600                     ; |<%s>
*/

PVOID SaveChange2Start = (PVOID)0x0041C8A0;
PVOID SaveChange2End = (PVOID)0x0041C8A5;
ULONG EaxCh2 = 0;
__declspec(naked) void SaveChange2()
{
	__asm
	{
		mov EaxCh2, eax
			mov eax, mbsPathPointer
			push eax
			mov eax, EaxCh2
		jmp SaveChange2End
	}
}

/*
CPU Disasm
Address               Hex dump                  Command                                  Comments
0041C9A9              |.  68 00264500           push offset 00452600                     ; |<%s>
*/

PVOID SaveExChange1Start = (PVOID)0x0041C9A9;
PVOID SaveExChange1End = (PVOID)0x0041C9AE;
ULONG EaxEx1 = 0;
__declspec(naked) void SaveExChange1()
{
	__asm
	{
		mov EaxEx1, eax
			mov eax, mbsPathPointer
			push eax
			mov eax, EaxEx1
		jmp SaveExChange1End
	}
}


/*
CPU Disasm
Address               Hex dump                  Command                                  Comments
0041C9CA              |.  68 00264500           push offset 00452600                     ; |<%s>
*/

PVOID SaveExChange2Start = (PVOID)0x0041C9CA;
PVOID SaveExChange2End = (PVOID)0x0041C9CF;
ULONG EaxEx2 = 0;
__declspec(naked) void SaveExChange2()
{
	__asm
	{
		mov EaxEx2, eax
			mov eax, mbsPathPointer
			push eax
			mov eax, EaxEx2
		jmp SaveExChange2End
	}
}

/*
CPU Disasm
Address               Hex dump                  Command                                  Comments
0041C9E4              |.  68 00264500           push offset 00452600                     ; /<%s>
*/
PVOID SaveExChange3Start = (PVOID)0x0041C9E4;
PVOID SaveExChange3End = (PVOID)0x0041C9E9;
ULONG EaxEx3 = 0;
__declspec(naked) void SaveExChange3()
{
	__asm
	{
		mov EaxEx3, eax
			mov eax, mbsPathPointer
			push eax
			mov eax, EaxEx3
		jmp SaveExChange3End
	}
}

/*
CPU Disasm
Address               Hex dump                  Command                                  Comments
0041CB54              |.  68 00264500           push offset 00452600                     ; |<%s>
*/

PVOID SaveCon1Start = (PVOID)0x0041CB54;
PVOID SaveCon1End = (PVOID)0x0041CB59;
ULONG EaxCon1 = 0;
__declspec(naked) void SaveCon1()
{
	__asm
	{
		mov EaxCon1, eax
			mov eax, mbsPathPointer
			push eax
			mov eax, EaxCon1
		jmp SaveCon1End
	}
}


/*
CPU Disasm
Address               Hex dump                  Command                                  Comments
0041CB77              |.  68 00264500           push offset 00452600                     ; |<%s>
*/

PVOID SaveCon2Start = (PVOID)0x0041CB77;
PVOID SaveCon2End = (PVOID)0x0041CB7C;
ULONG EaxCon2 = 0;
__declspec(naked) void SaveCon2()
{
	__asm
	{
		mov EaxCon2, eax
			mov eax, mbsPathPointer
			push eax
			mov eax, EaxCon2
		jmp SaveCon2End
	}
}

/*
CPU Disasm
Address               Hex dump                  Command                                  Comments
0041D9AB              |.  68 00264500           |push offset 00452600                    ; |<%s>
*/

PVOID HasSaveStart = (PVOID)0x0041D9AB;
PVOID HasSaveEnd = (PVOID)0x0041D9B0;
ULONG EaxHas = 0;
__declspec(naked) void HasSave()
{
	__asm
	{
		mov EaxHas, eax
			mov eax, mbsPathPointer
			push eax
			mov eax, EaxHas
		jmp HasSaveEnd
	}
}


/*
CPU Disasm
Address               Hex dump                  Command                                  Comments
0041E8B6              |.  68 00264500           push offset 00452600                     ; |<%s>
*/

PVOID SaveStub1Start = (PVOID)0x0041E8B6;
PVOID SaveStub1End = (PVOID)0x0041E8BB;
ULONG Eax1 = 0;
__declspec(naked) void SaveStub1()
{
	__asm
	{
		mov Eax1, eax
			mov eax, mbsPathPointer
			push eax
			mov eax, Eax1
		jmp SaveStub1End
	}
}

/*
CPU Disasm
Address               Hex dump                  Command                                  Comments
0041EA62              |.  68 00264500           push offset 00452600                     ; |<%s>
*/

PVOID SaveStub2Start = (PVOID)0x0041EA62;
PVOID SaveStub2End = (PVOID)0x0041EA67;
ULONG Eax2 = 0;
__declspec(naked) void SaveStub2()
{
	__asm
	{
		mov Eax2, eax
			mov eax, mbsPathPointer
			push eax
			mov eax, Eax2

		jmp SaveStub2End
	}
}


PVOID SaveStub3Start = (PVOID)0x0041EA62;
PVOID SaveStub3End = (PVOID)0x0041EA67;
ULONG Eax3 = 0;
__declspec(naked) void SaveStub3()
{
	__asm
	{
		mov Eax3, eax
			mov eax, mbsPathPointer
			push eax
			mov eax, Eax3
		jmp SaveStub3End
	}
}

/*
CPU Disasm
Address               Hex dump                  Command                                  Comments
00430F68              |.  68 00264500           push offset 00452600                     ; |<%s>
*/

PVOID SaveStub4Start = (PVOID)0x00430F68;
PVOID SaveStub4End = (PVOID)0x00430F6D;
ULONG Eax4 = 0;
__declspec(naked) void SaveStub4()
{
	__asm
	{
		mov Eax4, eax
			mov eax, mbsPathPointer
			push eax
			mov eax, Eax4
		jmp SaveStub4End
	}
}


/*
CPU Disasm
Address               Hex dump                  Command                                  Comments
004304A0              |.  68 00264500           push offset 00452600                     ; /<%s>
*/

PVOID SaveStub5Start = (PVOID)0x004304A0;
PVOID SaveStub5End = (PVOID)0x004304A5;
ULONG Eax5 = 0;
__declspec(naked) void SaveStub5()
{
	__asm
	{
		mov Eax5, eax
			mov eax, mbsPathPointer
			push eax
			mov eax, Eax5
		jmp SaveStub5End
	}
}

/*
CPU Disasm
Address               Hex dump                  Command                                  Comments
00430640              |.  68 00264500           push offset 00452600                     ; /<%s>
*/

PVOID SaveStub6Start = (PVOID)0x00430640;
PVOID SaveStub6End = (PVOID)0x00430645;
ULONG Eax6 = 0;
__declspec(naked) void SaveStub6()
{
	__asm
	{
		mov Eax6, eax
		mov eax, mbsPathPointer
		push eax
		mov eax, Eax6
		jmp SaveStub6End
	}
}



/*
CPU Disasm
Address               Hex dump                  Command                                  Comments
004307E3              |.  68 00264500           push offset 00452600                     ; /<%s>
*/

PVOID SaveStub7Start = (PVOID)0x004307E3;
PVOID SaveStub7End = (PVOID)0x004307E8;
ULONG Eax7 = 0;
__declspec(naked) void SaveStub7()
{
	__asm
	{
		mov Eax7, eax
			mov eax, mbsPathPointer
			push eax
			mov eax, Eax7
		jmp SaveStub7End
	}
}

/*
CPU Disasm
Address               Hex dump                  Command                                  Comments
00430894              |.  68 00264500           |push offset 00452600                    ; |<%s>
*/

PVOID SaveStub8Start = (PVOID)0x00430894;
PVOID SaveStub8End = (PVOID)0x00430899;
ULONG Eax8 = 0;
__declspec(naked) void SaveStub8()
{
	__asm
	{
		mov Eax8, eax
			mov eax, mbsPathPointer
			push eax
			mov eax, Eax8
		jmp SaveStub8End
	}
}

/*
CPU Disasm
Address               Hex dump                  Command                                  Comments
00430918              |.  68 00264500           push offset 00452600                     ; |<%s>
*/

PVOID SaveStub9Start = (PVOID)0x00430918;
PVOID SaveStub9End = (PVOID)0x0043091D;
ULONG Eax9 = 0;
__declspec(naked) void SaveStub9()
{
	__asm
	{
		mov Eax9, eax
			mov eax, mbsPathPointer
			push eax
			mov eax, Eax9
		jmp SaveStub9End
	}
}


/*
CPU Disasm
Address               Hex dump                  Command                                  Comments
00430CD8              |.  68 00264500           push offset 00452600                     ; |<%s>
*/

PVOID SaveStub10Start = (PVOID)0x00430CD8;
PVOID SaveStub10End = (PVOID)0x00430CDD;
ULONG Eax10 = 0;
__declspec(naked) void SaveStub10()
{
	__asm
	{
		mov Eax10, eax
			mov eax, mbsPathPointer
			push eax
			mov eax, Eax10
		jmp SaveStub10End
	}
}



/*
CPU Disasm
Address               Hex dump                  Command                                  Comments
00430F68              |.  68 00264500           push offset 00452600                     ; |<%s>
*/

PVOID SaveStub11Start = (PVOID)0x00430F68;
PVOID SaveStub11End = (PVOID)0x00430F6D;
ULONG Eax11 = 0;
__declspec(naked) void SaveStub11()
{
	__asm
	{
		mov Eax11, eax
			mov eax, mbsPathPointer
			push eax
			mov eax, Eax11
		jmp SaveStub11End
	}
}



void WINAPI InstallSavePatch()
{
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach((void**)&pfCreateSave, CreateSave);
	DetourTransactionCommit();

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach((void**)&HasSaveStart, HasSave);
	DetourTransactionCommit();

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach((void**)&SaveCon1Start, SaveCon1);
	DetourTransactionCommit();

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach((void**)&SaveCon2Start, SaveCon2);
	DetourTransactionCommit();

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach((void**)&SaveExChange1Start, SaveExChange1);
	DetourTransactionCommit();

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach((void**)&SaveExChange2Start, SaveExChange2);
	DetourTransactionCommit();

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach((void**)&SaveExChange3Start, SaveExChange3);
	DetourTransactionCommit();

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach((void**)&SaveChange1Start, SaveChange1);
	DetourTransactionCommit();

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach((void**)&SaveChange2Start, SaveChange2);
	DetourTransactionCommit();

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach((void**)&SaveStub1Start, SaveStub1);
	DetourTransactionCommit();

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach((void**)&SaveStub2Start, SaveStub2);
	DetourTransactionCommit();

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach((void**)&SaveStub3Start, SaveStub3);
	DetourTransactionCommit();

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach((void**)&SaveStub4Start, SaveStub4);
	DetourTransactionCommit();

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach((void**)&SaveStub5Start, SaveStub5);
	DetourTransactionCommit();

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach((void**)&SaveStub6Start, SaveStub6);
	DetourTransactionCommit();

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach((void**)&SaveStub7Start, SaveStub7);
	DetourTransactionCommit();

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach((void**)&SaveStub8Start, SaveStub8);
	DetourTransactionCommit();

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach((void**)&SaveStub9Start, SaveStub9);
	DetourTransactionCommit();

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach((void**)&SaveStub10Start, SaveStub10);
	DetourTransactionCommit();

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach((void**)&SaveStub11Start, SaveStub11);
	DetourTransactionCommit();
}


/***********************************/

PVOID pfOldCreateFileA = NULL;
typedef HANDLE(WINAPI *PfunHookCreateFile)(LPCSTR lpFileName, DWORD  dwDesiredAccess, DWORD dwShareMode,
	LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD  dwFlagsAndAttributes,
	HANDLE hTemplateFile);

HANDLE WINAPI HookCreateFile(
	_In_     LPCSTR               lpFileName,
	_In_     DWORD                 dwDesiredAccess,
	_In_     DWORD                 dwShareMode,
	_In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	_In_     DWORD                 dwCreationDisposition,
	_In_     DWORD                 dwFlagsAndAttributes,
	_In_opt_ HANDLE                hTemplateFile
	)
{
	OutputStringA(lpFileName);
	OutputStringA("\n");

	if (!strncmp(lpFileName, mbsPathPointer, lstrlenA(mbsPath)))
	{
		string Name = GetFileName(lpFileName);
		string FullPath(mbsPath);
		FullPath += "\\";
		FullPath += Name;

		OutputStringA(FullPath.c_str());
		OutputStringA("\n");

		return (PfunHookCreateFile(pfOldCreateFileA))(FullPath.c_str(), dwDesiredAccess, dwShareMode,
			lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes,
			hTemplateFile);

	}
	else
	{
		return (PfunHookCreateFile(pfOldCreateFileA))(lpFileName, dwDesiredAccess, dwShareMode,
			lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes,
			hTemplateFile);
	}
}



HRESULT WINAPI Init(HMODULE hDll)
{
	FileManager::Create();
	

	ULONG dwVersion = GetVersion();
	dwMajorVersion = (DWORD)(LOBYTE(LOWORD(dwVersion)));

	//自定义保存位置
	GetCurrentDirectoryA(MAX_PATH, mbsPath);
	lstrcatA(mbsPath, "\\save");
	mbsPathPointer = mbsPath;


	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	pfGetACP = DetourFindFunction("Kernel32.dll", "GetACP");
	DetourAttach(&pfGetACP, HookGetACP);
	DetourTransactionCommit();

	if (dwMajorVersion < 6 || GetACPXmoe() != 936)
	{
		InstallSavePatch();
	}

	//FileSystem
	ReDirectReadFileData();

	*(ULONG*)&pfLoadText = 0x0042FC40;

	///IA32 HookPort
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach((void**)&pfLoadText, HookLoadText);
	DetourTransactionCommit();


	LPVOID pfDiskChecker = (LPVOID)0x00431430;
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach((void**)&pfDiskChecker, FakeDiskChecker);
	DetourTransactionCommit();


	//FakeSaver
	//WinMain下的启动
	if (dwMajorVersion < 6 || GetACPXmoe() != 936)
	{
		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach((void**)&pfFakeSaver, FakeSaver);
		DetourTransactionCommit();
	}


	LPVOID pfCheckFont = (LPVOID)0x00424B90;
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach((void**)&pfCheckFont, HookCheckFontProc);
	DetourTransactionCommit();

	//HookCreateFile
	/*
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	pfOldCreateFileA = DetourFindFunction("Kernel32.dll", "CreateFileA");
	DetourAttach(&pfOldCreateFileA, HookCreateFile);
	DetourTransactionCommit();
	*/

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	pfOldCreateFontIndirectA = DetourFindFunction("GDI32.dll", "CreateFontIndirectA");
	DetourAttach(&pfOldCreateFontIndirectA, NewCreateFontIndirectA);
	DetourTransactionCommit();

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	pfMessageBoxA = DetourFindFunction("User32.dll", "MessageBoxA");
	DetourAttach(&pfMessageBoxA, HookMessageBox);
	DetourTransactionCommit();


	//PAL.dll
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	pfCreateFontA = DetourFindFunction("GDI32.dll", "CreateFontA");
	DetourAttach(&pfCreateFontA, HookCreateFont);
	DetourTransactionCommit();


	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	pfGetGlyphOutlineA = DetourFindFunction("GDI32.dll", "GetGlyphOutlineA");
	DetourAttach(&pfGetGlyphOutlineA, HookGetGlyphOutline);
	DetourTransactionCommit();

	/*
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	pfGetTextMetricsA = DetourFindFunction("GDI32.dll", "GetTextMetricsA");
	DetourAttach(&pfGetTextMetricsA, HookGetTextMetrics);
	DetourTransactionCommit();

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	pfGetStringTypeA = DetourFindFunction("Kernel32.dll", "GetStringTypeA");
	DetourAttach(&pfGetStringTypeA, HookGetStringTypeA);
	DetourTransactionCommit();
	*/

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	pfLoadLibrary = DetourFindFunction("Kernel32.dll", "LoadLibraryA");
	DetourAttach(&pfLoadLibrary, HookLoadLibrary);
	DetourTransactionCommit();

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	pfGetOEMCP = DetourFindFunction("Kernel32.dll", "GetOEMCP");
	DetourAttach(&pfGetOEMCP, HookGetOEMCP);
	DetourTransactionCommit();

	return S_OK;
}

HRESULT WINAPI UnInit(HMODULE hDll)
{
	UNREFERENCED_PARAMETER(hDll);
	return S_OK;
}


