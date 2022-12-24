// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "stdafx.h"
#include "detours.h"
#include <windows.h>
#include "WinFile.h"
#include <string>
#include <vector>
#include "MiniNtdll.h"
#include "FileManager.h"

using std::string;
using std::vector;

//#define DebugOut
//#define TestVersion

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


PVOID pfOldCreateFontIndirectA=NULL;
typedef HFONT (WINAPI *PfuncCreateFontIndirectA)(LOGFONTA *lplf);
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
	lstrcpyW(Info.lfFaceName,L"黑体");
	lplf->lfCharSet = GB2312_CHARSET;

	return CreateFontIndirectW(&Info);
	//return ((PfuncCreateFontIndirectA)pfOldCreateFontIndirectA)(lplf);
}


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

#ifndef DebugOut
	string Ext = GetExtension(lpFileName);
	if (stricmp(Ext.c_str(), "pac") && dwDesiredAccess == GENERIC_READ)
	{
		string FileName = GetFileName(lpFileName);
		string FullFileName("ProjectDir\\");
		FullFileName += FileName;

		OutputStringA("Redirect : ");
		OutputStringA(FullFileName.c_str());
		OutputStringA("\n");

		return (PfunHookCreateFile(pfOldCreateFileA))(FullFileName.c_str(), dwDesiredAccess, dwShareMode,
			lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes,
			hTemplateFile);
	}
	else
	{
		return (PfunHookCreateFile(pfOldCreateFileA))(lpFileName, dwDesiredAccess, dwShareMode,
			lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes,
			hTemplateFile);
	}
#else
	OutputStringA(lpFileName);
	OutputStringA("\n");


	return (PfunHookCreateFile(pfOldCreateFileA))(lpFileName, dwDesiredAccess, dwShareMode,
		lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes,
		hTemplateFile);
#endif
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
typedef HFONT(WINAPI *PfunHookCreateFont)( int  nHeight, int nWidth, int nEscapement,
	int nOrientation, int fnWeight,  DWORD fdwItalic, DWORD fdwUnderline, DWORD fdwStrikeOut,
	DWORD fdwCharSet, DWORD fdwOutputPrecision, DWORD fdwClipPrecision, DWORD fdwQuality,
	DWORD fdwPitchAndFamily,LPSTR lpszFace);

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


//处理非GBK集合而暂时代替的字体
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
	else if (LOWORD(OldChar) == (WORD)0x8140)
	{
		OutputStringW(L"JP Char\n");
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
		Result = GetGlyphOutlineW(hdc, (UINT)L'|', uFormat,
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


//#error "Set Unicode Filter for GetTextMetrics"
PVOID pfGetTextMetricsA = NULL;
typedef BOOL (WINAPI* PfunGetTextMetrics)( HDC hdc, LPTEXTMETRIC lptm);

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


HWND MainWnd = nullptr;
LPVOID pfCreateWindowExAInline = nullptr;
typedef HWND(WINAPI* PfunCreateWindowExA)(DWORD dwExStyle, 
	LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, int X, int Y, 
	int nWidth, int nHeight, HWND hWndParent, 
	HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam);
//IAT
HWND WINAPI HookCreateWindowExA(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam)
{
	RECT    rcWordArea;
	ULONG   Length;
	LPWSTR  ClassName, WindowName;


	Length = lstrlenA(lpClassName) + 1;
	ClassName = (LPWSTR)alloca(Length * sizeof(WCHAR));
	RtlZeroMemory(ClassName, Length * sizeof(WCHAR));
	MultiByteToWideChar(932, 0, lpClassName, Length, ClassName, Length * sizeof(WCHAR));

	Length = lstrlenA(lpWindowName) + 1;
	WindowName = (LPWSTR)alloca(Length * sizeof(WCHAR));
	RtlZeroMemory(WindowName, Length * sizeof(WCHAR));
	MultiByteToWideChar(932, 0, lpWindowName, Length, WindowName, Length * sizeof(WCHAR));

	if (dwStyle == 0x16CB0000)
	{
		return ((PfunCreateWindowExA)pfCreateWindowExAInline)(dwExStyle, lpClassName, lpWindowName, dwStyle, X, Y, nWidth,
			nHeight, hWndParent, hMenu, hInstance, lpParam);
		//L"君之余影静静地摇曳着"
	//return MainWnd;
	}
	else
	{
		return ((PfunCreateWindowExA)pfCreateWindowExAInline)(dwExStyle, lpClassName, lpWindowName, dwStyle, X, Y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
	}
}

/*
List of windows, item 5
Handle = 00490520
Text = 君の名残は静かに揺れて
Parent = Topmost
WinProc =
ID/menu =
Type = ASCII
Style = 16CB0000 WS_OVERLAPPED|WS_MINIMIZEBOX|WS_MAXIMIZEBOX|WS_CAPTION|WS_SYSMENU|WS_VISIBLE|WS_CLIPSIBLINGS|WS_CLIPCHILDREN
ExtStyle = 00000100 WS_EX_WINDOWEDGE
Thread = Main
ClsProc = 100DE010
ClsName = 君の名残は静かに揺れて
*/
PVOID pfSetWindowTextA = nullptr;
typedef BOOL(WINAPI* PfunSetWindowTextA)(HWND hWnd, LPCSTR lpString);
BOOL WINAPI HookSetWindowTextA(HWND hWnd, LPCSTR lpString)
{
	if (GetWindowLongW(hWnd, GWL_STYLE) == 0x16CB0000)
	{
		return SetWindowTextW(hWnd, L"君之余影静静地摇曳着");
	}
	else
	{
		return ((PfunSetWindowTextA)pfSetWindowTextA)(hWnd, lpString);
	}
}

/********************************/

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

BYTE* g_ImageBits;
ULONG g_ImageBitsLength;
LONG g_ImageWidth;
LONG g_ImageHeight;
int g_ImageBitCount;

DWORD PalLoadSprEx(DWORD unk, CHAR* FileName, PBYTE DataBuffers, DWORD DataSize)
{
	void* f_buf;
	unsigned long f_length;

	DWORD result;


	char ConvertName[128] = { 0 };
	int ConvertNameLen;

	lstrcpyA(ConvertName, FileName);
	ConvertNameLen = lstrlenA(ConvertName);
	ConvertName[ConvertNameLen - 3] = 'b';
	ConvertName[ConvertNameLen - 2] = 'm';
	ConvertName[ConvertNameLen - 1] = 'p';

	ULONG FileSize;
	PBYTE FileBuffer;
	if (FileManager::GetFileManager()->QueryFile(ConvertName, FileBuffer, FileSize))
	{
		GetBMPData(FileBuffer, &g_ImageWidth, &g_ImageHeight, &g_ImageBitCount, &g_ImageBits, &g_ImageBitsLength);
		BMP_TO_DIB(g_ImageBits, g_ImageWidth, g_ImageHeight, 32);
	}

	result = (PfunPalLoadSprite(pfPalLoadSprite))(unk, FileName, DataBuffers, DataSize);

	if (g_ImageBits)
	{
		GlobalFree(g_ImageBits);
		g_ImageBits = NULL;
	}
	if (FileBuffer)
	{
		GlobalFree(FileBuffer);
		FileBuffer = NULL;
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
	int nNewAlignWidth;
	BYTE* Output = NULL;
	ULONG OutputLength;
	BYTE* SourceLines;
	BYTE* TargetLines;
	int i, j;

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
		push[esp + 0x8]
			pop screen_width
			call type3_draw_4_call
			pushad
			push screen_width
			push dword ptr[ebp + 8]
			;; push dword ptr[esp + 4]
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
		push[esp + 0x8]
			pop screen_width
			call type3_draw_3_call
			pushad
			push screen_width
			push dword ptr[ebp + 8]
			;; push dword ptr[esp + 4]
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
		push[esp + 0x8]
			pop screen_width
			call type3_draw_2_call
			pushad
			push screen_width
			;; push dword ptr[ebp + 0x10]
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
		push[esp + 8]
			pop screen_width
			call type3_draw_1_call
			pushad
			push screen_width
			;; push dword ptr[ebp + 0x10]
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


VOID WINAPI PatchRender(HMODULE hModule)
{

	PBYTE PalModule = (PBYTE)hModule;
	/*
	(int)&unk_1312B8F8,
	(unsigned int *)v10,
	*/
	PBYTE PatchAddr = &PalModule[0xC2B76];
	type3_draw_1_ret = PatchAddr + 5;
	type3_draw_1_call = GetCALLTarget(PatchAddr);
	ReplaceJump(PatchAddr, (PBYTE)&update_bmp_draw_1);

	PatchAddr = PalModule + 0xC2B54;
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
	PatchAddr = PalModule + 0xC2B04;
	type3_draw_3_ret = PatchAddr + 5;
	type3_draw_3_call = GetCALLTarget(PatchAddr);
	ReplaceJump(PatchAddr, (PBYTE)&update_bmp_draw_3);

	/*
	CPU Disasm
	Address               Hex dump                  Command                                  Comments
	100A43A6              |> \E8 75FCFFFF           call 100A4020
	*/

	PatchAddr = PalModule + 0xC2B26;
	type3_draw_4_ret = PatchAddr + 5;
	type3_draw_4_call = GetCALLTarget(PatchAddr);
	ReplaceJump(PatchAddr, (PBYTE)&update_bmp_draw_4);
	
	//sub_100E0D60
	pfPalLoadSprite = (DWORD)&PalModule[0xE0D60];

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach((void**)&pfPalLoadSprite, PalLoadSprEx);
	DetourTransactionCommit();
}


///Install Font Checker
//一共3个
/*
CPU Disasm
Address   Hex dump          Command                                   Comments
00421809      81F9 80000000 cmp ecx,80
*/

/*
CPU Disasm
Address   Hex dump          Command                                   Comments
00421817      3D FE000000   cmp eax,0FE
*/

/*
CPU Disasm
Address   Hex dump          Command                                   Comments
0042184D      81FA FE000000 cmp edx,0FE

*/

//0x24改成0x23
//一共五处

//004011B6  |.  83FA 24       cmp edx,24
//004012A6  |.  83FA 24       cmp edx,24
//00401396  |.  83FA 24       cmp edx,24
//00401486  |.  83FA 24       cmp edx,24
//00403F22  |.  83FA 24       cmp edx,24

void WINAPI InstallAfterCreate()
{
	static BYTE HandlePatch1[] = { 0x83, 0xFA, 0x23 };
	VirtualMemoryCopy((PVOID)0x004011B6, HandlePatch1, sizeof(HandlePatch1));
	VirtualMemoryCopy((PVOID)0x004012A6, HandlePatch1, sizeof(HandlePatch1));
	VirtualMemoryCopy((PVOID)0x00401396, HandlePatch1, sizeof(HandlePatch1));
	VirtualMemoryCopy((PVOID)0x00401486, HandlePatch1, sizeof(HandlePatch1));
	VirtualMemoryCopy((PVOID)0x00403F22, HandlePatch1, sizeof(HandlePatch1));
}

//100D79D6    817D D4 4081000 cmp dword ptr [ebp-2C],8140
//100D7523    817D D0 4081000 cmp dword ptr [ebp-30],8140

void WINAPI InstallSpacePatch(BYTE* Offset)
{
	PVOID pfSapceCheck1 = (PVOID)&Offset[0xD79D6];
	static BYTE SpacePatch1[] = { 0x81, 0x7D, 0xD4, 0xA1, 0xA1};
	VirtualMemoryCopy(pfSapceCheck1, SpacePatch1, sizeof(SpacePatch1));

	PVOID pfSapceCheck2 = (PVOID)&Offset[0xD7523];
	static BYTE SpacePatch2[] = { 0x81, 0x7D, 0xD0, 0xA1, 0xA1 };
	VirtualMemoryCopy(pfSapceCheck2, SpacePatch2, sizeof(SpacePatch2));
}


void WINAPI HookPalFontSetType(HMODULE hModule)
{
	BYTE* pfPalFontSetType = (BYTE*)GetProcAddress((HMODULE)hModule, "PalFontSetType");
	SetNopCode((pfPalFontSetType + 0xC), 7);
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

int WINAPI HookMultiByteToWideChar(
	_In_      UINT   CodePage,
	_In_      DWORD  dwFlags,
	_In_      LPCSTR lpMultiByteStr,
	_In_      int    cbMultiByte,
	_Out_opt_ LPWSTR lpWideCharStr,
	_In_      int    cchWideChar
	)
{
	int Result = MultiByteToWideChar(CodePage, dwFlags, lpMultiByteStr,
		cbMultiByte, lpWideCharStr, cchWideChar
		);

	OutputStringW(lpWideCharStr);
	return Result;
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

			//PROC pfCreateWindowExA = GetProcAddress(GetModuleHandleW(L"User32.dll"), "CreateWindowExA");
			//IATPatchPAL("User32.dll", pfCreateWindowExA, (PROC)HookCreateWindowExA);

#if 0
			PROC pfSetWindowTextA = GetProcAddress(GetModuleHandleW(L"User32.dll"), "SetWindowTextA");
			Result = IATPatch("User32.dll", pfSetWindowTextA, (PROC)HookSetWindowTextA);
#else
			DetourTransactionBegin();
			DetourUpdateThread(GetCurrentThread());
			pfSetWindowTextA = DetourFindFunction("User32.dll", "SetWindowTextA");
			DetourAttach(&pfSetWindowTextA, HookSetWindowTextA);
			DetourTransactionCommit();
#endif
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
	for (INT32 Index = 0; Index < Count; Index++)
	{
		Offset += 4;
		TextPool.push_back((char*)(FileBuffer + Offset));
		Offset += lstrlenA((char*)(FileBuffer + Offset)) + 1;
	}
}


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


//00421800 
BOOL HookCheckFontProc(int a1)
{
	return ((signed int)*(BYTE *)a1 >= 0x81 && (signed int)*(BYTE *)a1 <= 0xFE)
		&& (((signed int)*(BYTE *)(a1 + 1) >= 0x40 && (signed int)*(BYTE *)(a1 + 1) <= 0x7E)
		|| ((signed int)*(BYTE *)(a1 + 1) >= 0x80 && (signed int)*(BYTE *)(a1 + 1) <= 0xFE));
}


BOOL APIENTRY SetHook()
{
#if defined(DebugOut) || defined(TestVersion)
	AllocConsole();
#endif

	//AllocConsole();

	FileManager::Create();

	//FileSystem
	ReDirectReadFileData();

	*(ULONG*)&pfLoadText = 0x004054D0;

	///IA32 HookPort
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach((void**)&pfLoadText, HookLoadText);
	DetourTransactionCommit();


	LPVOID pfCheckFont = (LPVOID)0x00421800;
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach((void**)&pfCheckFont, HookCheckFontProc);
	DetourTransactionCommit();


	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	pfOldCreateFontIndirectA=DetourFindFunction("GDI32.dll","CreateFontIndirectA");
	DetourAttach(&pfOldCreateFontIndirectA,NewCreateFontIndirectA);
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

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	pfGetACP = DetourFindFunction("Kernel32.dll", "GetACP");
	DetourAttach(&pfGetACP, HookGetACP);
	DetourTransactionCommit();

	/*
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	pfCreateWindowExAInline = DetourFindFunction("User32.dll", "CreateWindowExA");
	DetourAttach(&pfCreateWindowExAInline, HookSetWindowTextA);
	DetourTransactionCommit();
	*/
	return TRUE;
}

//卸载Hook
BOOL APIENTRY DropHook()
{
#if 0
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourDetach(&g_pOldCreateFontIndirectA, NewCreateFontIndirectA);
	LONG ret=DetourTransactionCommit();
	return ret==NO_ERROR;
#endif

	return TRUE;
}

static HMODULE s_hDll;
HMODULE WINAPI XmoeLink()
{
	return s_hDll;
}

__declspec(dllexport)void WINAPI Init()
{

}

BOOL APIENTRY DllMain( HMODULE hModule,DWORD  ul_reason_for_call,LPVOID lpReserved )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	{
		s_hDll = hModule;
		DisableThreadLibraryCalls(hModule);
		if (FileManager::Create() == nullptr)
		{
			MessageBoxW(GetForegroundWindow(), L"初始化失败-内部错误", L"X'moe-CoreLib", MB_OK);
			ExitProcess(-1);
		}
		FileManager::GetFileManager()->Init();
		SetHook();
	}
	break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		DropHook();
		break;
	}
	return TRUE;
}



