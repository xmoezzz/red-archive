#include "FriendToLoverHook.h"
#include "ImageEx.h"
#include "Compressor.h"
#include "MemoryLoader.h"
#include "NtDefine.h"
#include "MinHook.h"
#include <ntstatus.h>

//Allocator
//004AB518

PVOID CDECL HostAlloc(LONG Size)
{
	return HeapAlloc(GetProcessHeap(), 0, Size);
}

VOID CDECL HostFree(PVOID lpMem)
{
	if (!lpMem)
		return;

	__try
	{
		HeapFree(GetProcessHeap(), 0, lpMem);
	}
	__except (EXCEPTION_CONTINUE_EXECUTION)
	{
	}
}

//==========================
//sub_46F870
BOOL CDECL CheckOSDefaultLangID(ULONG PrimaryLangID)
{
	UNREFERENCED_PARAMETER(PrimaryLangID);
	return TRUE;
}

BOOL WINAPI HookTextOutA(HDC hDC, int X, int Y, LPCSTR lpString, int cbCount)
{
	BOOL    Result;
	WCHAR   c, ch[0x400];
	ULONG   CodePage;
	HFONT   hFont, hFontOld;

	hFont = NULL;
	hFontOld = NULL;

	CodePage = CP_GB2312;
	if (cbCount > 2)
	{
		//if (IsShiftJISString((PCHAR)lpString, cbCount))
		//	CodePage = CP_SHIFTJIS;

		goto MBYTE_TO_WCHAR;
	}
	else if (cbCount == 2)
	{
		c = *(PWCHAR)lpString;
		switch ((ULONG)c)
		{
		case SWAPCHAR(L'⑨'):
			ch[0] = 0x266A;
			break;

		default:
			//                if (IsShiftJISString((LPSTR)&c, 2))
			//                    CodePage = CP_SHIFTJIS;

			goto MBYTE_TO_WCHAR;
		}

		hFont = FriendToLoverHook::GetGlobalData()->DuplicateFontW(hDC, SHIFTJIS_CHARSET);
		if (hFont != NULL)
		{
			hFontOld = (HFONT)SelectObject(hDC, hFont);
			if (hFontOld == NULL)
			{
				DeleteObject(hFont);
				hFont = NULL;
			}
		}
		cbCount = 1;
		goto SKIP_CONV;
	}

MBYTE_TO_WCHAR:
	cbCount = MultiByteToWideChar(CodePage, 0, lpString, cbCount, ch, _countof(ch));

SKIP_CONV:

	FriendToLoverHook::GetGlobalData()->WriteInfo(ch, 1);
	Result = TextOutW(hDC, X, Y, ch, cbCount);

	if (hFont != NULL)
	{
		SelectObject(hDC, hFontOld);
		DeleteObject(hFont);
	}

	return Result;
}


BOOL WINAPI HookTextOutW(
	_In_ HDC     hdc,
	_In_ int     nXStart,
	_In_ int     nYStart,
	_In_ LPWSTR  lpString,
	_In_ int     cchString
	)
{
	//FriendToLoverHook::GetGlobalData()->WriteInfo(lpString, 1);
	BOOL Result;

	if (cchString == 1)
	{
		if (lpString[0] == (WORD)0x2468)
		{
			lpString[0] = (WORD)0x266A;
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
			Result = TextOutW(hdc, nXStart, nYStart, lpString, cchString);

			SelectObject(hdc, hOldFont);
			DeleteObject(hFont);
			HeapFree(GetProcessHeap(), 0, lplf);
			return Result;
		}
	}
	return TextOutW(hdc, nXStart, nYStart, lpString, cchString);
}


ATOM WINAPI HookRegisterClassExA(WNDCLASSEXA *lpwcx)
{
	WNDCLASSEXW   Info;
	LPWSTR        lpwszMenuName, lpwszClassName;

	if (lstrcmpA(lpwcx->lpszClassName, "BGI - Main window"))
		return RegisterClassExA(lpwcx);

	Info.cbSize = lpwcx->cbSize;
	Info.style = lpwcx->style;
	Info.lpfnWndProc = lpwcx->lpfnWndProc;
	Info.cbClsExtra = lpwcx->cbClsExtra;
	Info.cbWndExtra = lpwcx->cbWndExtra;
	Info.hInstance = lpwcx->hInstance;
	Info.hIcon = lpwcx->hIcon;
	Info.hCursor = lpwcx->hCursor;
	Info.hbrBackground = lpwcx->hbrBackground;
	Info.hIconSm = lpwcx->hIconSm;

	lpwszClassName = (LPWSTR)_alloca(MAX_PATH * 2);
	lpwszMenuName = (LPWSTR)_alloca(MAX_PATH * 2);

	RtlZeroMemory(lpwszClassName, MAX_PATH * 2);
	RtlZeroMemory(lpwszMenuName, MAX_PATH * 2);

	MultiByteToWideChar(932, 0, lpwcx->lpszClassName, lstrlenA(lpwcx->lpszClassName),
		lpwszClassName, MAX_PATH);
	MultiByteToWideChar(932, 0, lpwcx->lpszMenuName, lstrlenA(lpwcx->lpszMenuName),
		lpwszMenuName, MAX_PATH);

	return RegisterClassExW(&Info);
}


HFONT WINAPI HookCreateFontA(
	int     cHeight,
	int     cWidth,
	int     cEscapement,
	int     cOrientation,
	int     cWeight,
	DWORD   bItalic,
	DWORD   bUnderline,
	DWORD   bStrikeOut,
	DWORD   iCharSet,
	DWORD   iOutPrecision,
	DWORD   iClipPrecision,
	DWORD   iQuality,
	DWORD   iPitchAndFamily,
	LPCSTR  pszFaceName
	)
{
	LOGFONTW lf = { 0 };

	UNREFERENCED_PARAMETER(iCharSet);
	UNREFERENCED_PARAMETER(iQuality);

	if (FriendToLoverHook::GetGlobalData()->GetDefFont())
	{
		lf.lfHeight = cHeight;
		lf.lfWidth = cWidth;
		lf.lfEscapement = cEscapement;
		lf.lfOrientation = cOrientation;
		lf.lfWeight = cWeight;
		lf.lfItalic = bItalic;
		lf.lfUnderline = bUnderline;
		lf.lfStrikeOut = bStrikeOut;
		lf.lfCharSet = GB2312_CHARSET;
		lf.lfOutPrecision = 0;
		lf.lfClipPrecision = iClipPrecision;
		lf.lfQuality = iQuality;
		lf.lfPitchAndFamily = iPitchAndFamily;

		//DFPYuanW5-GB
		lstrcpyW(lf.lfFaceName, L"华康圆体W5(P)");

		return CreateFontIndirectW(&lf);
	}
	else
	{
		lf.lfHeight = cHeight;
		lf.lfWidth = cWidth;
		lf.lfEscapement = cEscapement;
		lf.lfOrientation = cOrientation;
		lf.lfWeight = cWeight;
		lf.lfItalic = bItalic;
		lf.lfUnderline = bUnderline;
		lf.lfStrikeOut = bStrikeOut;
		lf.lfCharSet = GB2312_CHARSET;
		lf.lfOutPrecision = iOutPrecision;
		lf.lfClipPrecision = iClipPrecision;
		lf.lfQuality = CLEARTYPE_QUALITY;
		lf.lfPitchAndFamily = iPitchAndFamily;

#if 0
		FriendToLoverHook::GetGlobalData()->AnsiToUnicode(
			pszFaceName,
			-1,
			lf.lfFaceName,
			_countof(lf.lfFaceName),
			IsShiftJISString(pszFaceName, -1) ? CP_SHIFTJIS : CP_GB2312);
#else
		lstrcpyW(lf.lfFaceName, L"黑体");
#endif
		return CreateFontIndirectW(&lf);
	}
}

/*
List of windows, item 4
Handle = 0032080E
Text = 壴怓僿僾僞僌儔儉
Parent = Topmost
WinProc =
ID/menu =
Type = ASCII
Style = 94CA0000 WS_POPUP|WS_MINIMIZEBOX|WS_CAPTION|WS_SYSMENU|WS_VISIBLE|WS_CLIPSIBLINGS
ExtStyle = 00040100 WS_EX_WINDOWEDGE|WS_EX_APPWINDOW
Thread = Main
ClsProc = 00478330
ClsName = BGI - Main window
*/

HWND WINAPI HookCreateWindowExA(
	_In_     DWORD     dwExStyle,
	_In_opt_ LPCSTR   lpClassName,
	_In_opt_ LPCSTR   lpWindowName,
	_In_     DWORD     dwStyle,
	_In_     int       x,
	_In_     int       y,
	_In_     int       nWidth,
	_In_     int       nHeight,
	_In_opt_ HWND      hWndParent,
	_In_opt_ HMENU     hMenu,
	_In_opt_ HINSTANCE hInstance,
	_In_opt_ LPVOID    lpParam
	)
{
	LPWSTR  lpClassNameW  = (LPWSTR)_alloca(MAX_PATH * 2);
	LPWSTR  lpWindowNameW = (LPWSTR)_alloca(MAX_PATH * 2);


	HWND Result = nullptr;

	RtlZeroMemory(lpClassNameW,  MAX_PATH * 2);
	RtlZeroMemory(lpWindowNameW, MAX_PATH * 2);

	if (!lstrcmpA(lpClassName, "BGI - Main window"))
	{
		MultiByteToWideChar(932, 0, lpClassName, lstrlenA(lpClassName), lpClassNameW, MAX_PATH);
		MultiByteToWideChar(932, 0, lpWindowName, lstrlenA(lpWindowName), lpWindowNameW, MAX_PATH);
		
		Result = CreateWindowExW(dwExStyle, lpClassNameW, lpWindowNameW, dwStyle, x, y,
			nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
		FriendToLoverHook::GetGlobalData()->MainWin = Result;
	}
	else
	{
		Result = CreateWindowExA(dwExStyle, lpClassName, lpWindowName, dwStyle, x, y,
			nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
	}
	return Result;
}

BOOL WINAPI HookSetWindowTextA(
	_In_     HWND    hWnd,
	_In_opt_ LPCSTR lpString
	)
{
	if (FriendToLoverHook::GetGlobalData()->MainWin == hWnd)
	{
		wstring QueryInfo;
		FriendToLoverHook::GetGlobalData()->GetAppName(QueryInfo);
		return SetWindowTextW(hWnd, QueryInfo.c_str());
	}
	return SetWindowTextA(hWnd, lpString);
}



wstring WINAPI FriendToLoverHook::GetPackageName(wstring& fileName)
{
	wstring temp(fileName);
	wstring::size_type pos = temp.find_last_of(L"\\");

	if (pos != wstring::npos)
	{
		temp = temp.substr(pos + 1, temp.length());
	}

	wstring temp2(temp);
	wstring::size_type pos2 = temp2.find_last_of(L"/");
	if (pos2 != wstring::npos)
	{
		temp2 = temp2.substr(pos + 1, temp2.length());
	}
	return temp2;
}


int WINAPI HookMessageBoxA(
	_In_opt_ HWND    hWnd,
	_In_opt_ LPCSTR lpText,
	_In_opt_ LPCSTR lpCaption,
	_In_     UINT    uType
	)
{
	PWCHAR WideText = nullptr;
	PWCHAR WideCaption = nullptr;
	int    Result = 0;
	ULONG TextAllocSize = 0, CaptionAllocSize = 0;

	TextAllocSize = (lstrlenA(lpText) + 1) * 2;
	CaptionAllocSize = (lstrlenA(lpCaption) + 1) * 2;

	WideText = (PWCHAR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, TextAllocSize);
	WideCaption = (PWCHAR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, CaptionAllocSize);

	if (WideText && WideCaption)
	{
		FriendToLoverHook::GetGlobalData()->AnsiToUnicode(lpText, lstrlenA(lpText),
			WideText, TextAllocSize, IsShiftJISString(lpText, lstrlenA(lpText)) ? 932 : 936);
		FriendToLoverHook::GetGlobalData()->AnsiToUnicode(lpCaption, lstrlenA(lpCaption),
			WideCaption, CaptionAllocSize, IsShiftJISString(lpCaption, lstrlenA(lpCaption)) ? 932 : 936);

		Result = MessageBoxW(hWnd, WideText, WideCaption, uType);
	}
	else
	{
		TextAllocSize = MAX_PATH * 2;
		CaptionAllocSize = MAX_PATH * 2;

		WCHAR StackText[MAX_PATH * 2] = { 0 };
		WCHAR StackCaption[MAX_PATH * 2] = { 0 };

		FriendToLoverHook::GetGlobalData()->AnsiToUnicode(lpText, lstrlenA(lpText),
			StackText, TextAllocSize, IsShiftJISString(lpText, lstrlenA(lpText)) ? 932 : 936);
		FriendToLoverHook::GetGlobalData()->AnsiToUnicode(lpCaption, lstrlenA(lpCaption),
			StackCaption, CaptionAllocSize, IsShiftJISString(lpCaption, lstrlenA(lpCaption)) ? 932 : 936);

		Result = MessageBoxW(hWnd, StackText, StackCaption, uType);
	}

	if (WideText)
		HeapFree(GetProcessHeap(), 0, WideText);
	if (WideCaption)
		HeapFree(GetProcessHeap(), 0, WideCaption);

	return Result;
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
	if (CodePage == 932)
	{
		CodePage = 936;
	}

	/*
	if (cbMultiByte != -1)
	{
	if (IsShiftJISString(lpMultiByteStr, cbMultiByte))
	CodePage = 932;
	}
	*/
	return MultiByteToWideChar(CodePage, dwFlags, lpMultiByteStr, cbMultiByte, lpWideCharStr, cchWideChar);
}


//==DecompressInfo
//sub_4654A0
LONG CDECL HookDecompressFile(
	PVOID  pvDecompressed, //64M 预先分配的
	PULONG pOutSize,
	PVOID  pvCompressed,
	ULONG  InSize,
	ULONG  SkipBytes,
	ULONG  OutBytes
	)
{
	ULONG Result, Magic;

	Result = 5;
	LOOP_ONCE
	{
		Magic = *(PULONG)pvCompressed;
		if ((Magic & 0x00FFFFFF) == TAG3('UCI'))
		{
			UCI_INFO uci;
			UCI_META_INFO *pMeta;
			IMG_BITMAP_HEADER *pHeader;

			if (UCIDecodeEx(pvCompressed, InSize, &uci, TRUE) < 0)
				break;

			BGIBitmap bmp = { 0 };
			bmp.Width = pMeta->Width;
			bmp.Height = pMeta->Height;

			pHeader = (IMG_BITMAP_HEADER *)pvDecompressed;
			pMeta = (UCI_META_INFO *)uci.ExtraInfo;
			if (pMeta != NULL && pMeta->Magic == UCI_META_INFO_MAGIC)
				ConvertRawToBitMap(&uci, pMeta->Width, pMeta->Height, pMeta->BitsPerPixel, pvDecompressed, -1);
			else
				ConvertRawToBitMap(&uci, uci.Width, uci.Height, uci.BitsPerPixel, pvDecompressed, -1);

			UCIFreeEx(&uci);

			if (pOutSize != NULL)
				*pOutSize = pHeader->dwFileSize;
		}
		else if (Magic == TAG4('XMOE'))
		{
			XmoeImage* Header = (XmoeImage*)pvCompressed;
			PBYTE Data = (PBYTE)pvCompressed + sizeof(XmoeImage);

			LZ4_uncompress((char*)Data, (char*)pvDecompressed, Header->RawSize);
			if (pOutSize != NULL)
				*pOutSize = Header->RawSize;
		}
		else if (InSize > sizeof(MY_BURIKO_SCRIPT_MAGIC) &&
			sizeof(MY_BURIKO_SCRIPT_MAGIC) ==
			RtlCompareMemory(pvCompressed, MY_BURIKO_SCRIPT_MAGIC, sizeof(MY_BURIKO_SCRIPT_MAGIC)))
		{
			UINT OutSize;

			OutSize = UCL_NRV2E_DecompressASMFast32((PBYTE)pvCompressed + sizeof(MY_BURIKO_SCRIPT_MAGIC), pvDecompressed);
			if (pOutSize != NULL)
				*pOutSize = OutSize;
		}
		else
		{
			break;
		}

		Result = 0;
	}

		//尝试原始解析
		return Result == 0 ? Result :
		((StubDecompressFile)FriendToLoverHook::OldDecompressFile)(pvDecompressed, pOutSize, pvCompressed, InSize, SkipBytes, OutBytes);
}



LONG CDECL HookDecompressFile_FASTCALL(
	PVOID  pvCompressed,
	PULONG pOutSize,
	ULONG  InSize,
	ULONG  SkipBytes,
	ULONG  OutBytes
	)
{
	ULONG Result, Magic;
	PVOID  pvDecompressed;

	__asm mov pvDecompressed, ecx;

	Result = 5;
	LOOP_ONCE
	{
		Magic = *(PULONG)pvCompressed;
		if ((Magic & 0x00FFFFFF) == TAG3('UCI'))
		{
			UCI_INFO uci;
			UCI_META_INFO *pMeta;
			IMG_BITMAP_HEADER *pHeader;

			if (UCIDecodeEx(pvCompressed, InSize, &uci, TRUE) < 0)
				break;

			BGIBitmap bmp = { 0 };
			bmp.Width = pMeta->Width;
			bmp.Height = pMeta->Height;

			pHeader = (IMG_BITMAP_HEADER *)pvDecompressed;
			pMeta = (UCI_META_INFO *)uci.ExtraInfo;
			if (pMeta != NULL && pMeta->Magic == UCI_META_INFO_MAGIC)
				ConvertRawToBitMap(&uci, pMeta->Width, pMeta->Height, pMeta->BitsPerPixel, pvDecompressed, -1);
			else
				ConvertRawToBitMap(&uci, uci.Width, uci.Height, uci.BitsPerPixel, pvDecompressed, -1);

			UCIFreeEx(&uci);

			if (pOutSize != NULL)
				*pOutSize = pHeader->dwFileSize;
		}
		else if (Magic == TAG4('XMOE'))
		{
			XmoeImage* Header = (XmoeImage*)pvCompressed;
			PBYTE Data = (PBYTE)pvCompressed + sizeof(XmoeImage);

			LZ4_uncompress((char*)Data, (char*)pvDecompressed, Header->RawSize);
			if (pOutSize != NULL)
				*pOutSize = Header->RawSize;
		}
		else if (InSize > sizeof(MY_BURIKO_SCRIPT_MAGIC) &&
			sizeof(MY_BURIKO_SCRIPT_MAGIC) ==
			RtlCompareMemory(pvCompressed, MY_BURIKO_SCRIPT_MAGIC, sizeof(MY_BURIKO_SCRIPT_MAGIC)))
		{
			UINT OutSize;

			OutSize = UCL_NRV2E_DecompressASMFast32((PBYTE)pvCompressed + sizeof(MY_BURIKO_SCRIPT_MAGIC), pvDecompressed);
			if (pOutSize != NULL)
				*pOutSize = OutSize;
		}
		else
		{
			break;
		}

		Result = 0;
	}

	if (Result != 0)
	{
		__asm
		{
			push 0
			push 0
			push InSize
			push pOutSize
			push pvCompressed
			mov ecx, pvDecompressed
			call FriendToLoverHook::OldDecompressFile
			add esp, 14h
			mov Result, eax
		}
	}
	return Result;
}

ULONG UnicodeWin32FindDataToAnsi(LPWIN32_FIND_DATAW pwfdW, LPWIN32_FIND_DATAA pwfdA)
{
	pwfdA->dwFileAttributes = pwfdW->dwFileAttributes;
	pwfdA->ftCreationTime = pwfdW->ftCreationTime;
	pwfdA->ftLastAccessTime = pwfdW->ftLastAccessTime;
	pwfdA->ftLastWriteTime = pwfdW->ftLastWriteTime;
	pwfdA->nFileSizeHigh = pwfdW->nFileSizeHigh;
	pwfdA->nFileSizeLow = pwfdW->nFileSizeLow;
	pwfdA->dwReserved0 = pwfdW->dwReserved0;
	pwfdA->dwReserved1 = pwfdW->dwReserved1;

	WideCharToMultiByte(CP_ACP, 0, pwfdW->cFileName, -1, pwfdA->cFileName, sizeof(pwfdA->cFileName), 0, 0);
	return WideCharToMultiByte(CP_ACP, 0, pwfdW->cAlternateFileName, -1, pwfdA->cAlternateFileName, sizeof(pwfdA->cAlternateFileName), 0, 0);
}



LONG CDECL HookDecompressFile_USERDATA(
	PVOID  pvDecompressed,
	PVOID  pvCompressed,
	PULONG pOutSize,
	ULONG  InSize,
	ULONG  SkipBytes,
	ULONG  OutBytes
	)
{
	ULONG Result, Magic;

	Result = 5;
	LOOP_ONCE
	{
		Magic = *(PULONG)pvCompressed;
		if ((Magic & 0x00FFFFFF) == TAG3('UCI'))
		{
			UCI_INFO uci;
			UCI_META_INFO *pMeta;
			IMG_BITMAP_HEADER *pHeader;

			if (UCIDecodeEx(pvCompressed, InSize, &uci, TRUE) < 0)
				break;

			BGIBitmap bmp = { 0 };
			bmp.Width = pMeta->Width;
			bmp.Height = pMeta->Height;

			pHeader = (IMG_BITMAP_HEADER *)pvDecompressed;
			pMeta = (UCI_META_INFO *)uci.ExtraInfo;
			if (pMeta != NULL && pMeta->Magic == UCI_META_INFO_MAGIC)
				ConvertRawToBitMap(&uci, pMeta->Width, pMeta->Height, pMeta->BitsPerPixel, pvDecompressed, -1);
			else
				ConvertRawToBitMap(&uci, uci.Width, uci.Height, uci.BitsPerPixel, pvDecompressed, -1);

			UCIFreeEx(&uci);

			if (pOutSize != NULL)
				*pOutSize = pHeader->dwFileSize;
		}
		else if (Magic == TAG4('XMOE'))
		{
			XmoeImage* Header = (XmoeImage*)pvCompressed;
			PBYTE Data = (PBYTE)pvCompressed + sizeof(XmoeImage);

			LZ4_uncompress((char*)Data, (char*)pvDecompressed, Header->RawSize);
			if (pOutSize != NULL)
				*pOutSize = Header->RawSize;
		}
		else if (InSize > sizeof(MY_BURIKO_SCRIPT_MAGIC) &&
			sizeof(MY_BURIKO_SCRIPT_MAGIC) ==
			RtlCompareMemory(pvCompressed, MY_BURIKO_SCRIPT_MAGIC, sizeof(MY_BURIKO_SCRIPT_MAGIC)))
		{
			UINT OutSize;

			OutSize = UCL_NRV2E_DecompressASMFast32((PBYTE)pvCompressed + sizeof(MY_BURIKO_SCRIPT_MAGIC), pvDecompressed);
			if (pOutSize != NULL)
				*pOutSize = OutSize;
		}
		else
		{
#if 1
			CopyMemory(pvDecompressed, pvCompressed, InSize);
			if (pOutSize != NULL)
				*pOutSize = InSize;
#else
			//Save data should be decoded?
			break;
#endif
		}
		Result = 0;
	}

		//in friend to lover HD renew version,
		//The call of DecompressFile should be a fastcall proc.
		if (Result != 0)
		{
			__asm
			{
				push 0
					push 0
					push InSize
					push pOutSize
					push pvCompressed
					mov ecx, pvDecompressed
					call FriendToLoverHook::OldDecompressFile
					add esp, 14h
					mov Result, eax
			}
		}

	return Result;
}


//
//sub_465580
//Checked
//ReadFile Outer Firstly
LONG
CDECL
LoadFileBuffer(
PVOID  pvDecompressed, //64M 预先分配的
PULONG pOutSize,
ULONG  SkipBytes,
ULONG  OutBytes
)
{
	LPCSTR lpFileName;
	__asm mov lpFileName, ecx;

	LONG   RetValue = 5;
	PBYTE  InBuffer = nullptr;
	ULONG  InSize;

	if (FriendToLoverHook::GetGlobalData()->LoadFileBuffer(lpFileName, InBuffer, InSize) == S_OK)
	{
		__asm mov ecx, pvDecompressed;

#if 0
		RetValue = HookDecompressFile_FASTCALL(
			InBuffer,
			pOutSize,
			InSize,
			0,
			0);
#else
#if 0
		__asm
		{
			push 0
			push 0
			push InSize
			push pOutSize
			push InBuffer
			call HookDecompressFile_FASTCALL
			mov RetValue, eax
			add esp, 14h
		}
#else
		RetValue = HookDecompressFile_USERDATA(pvDecompressed, InBuffer, pOutSize, InSize, 0, 0);
#endif
#endif

		if (InBuffer)
			HostFree(InBuffer);

		return RetValue;
	}
	else
	{
		FriendToLoverHook::GetGlobalData()->WriteInfo(lpFileName, 1);
		//MessageBoxW(NULL, L"233", 0, 0);
		__asm
		{
			mov ecx, pvDecompressed;
			;; push 0;
			;; push 0;
			;; push InSize;
			;; push pOutSize;
			;; push InBuffer;
			;; int 3;

			push OutBytes;
			push SkipBytes;
			push pOutSize;
			push pvDecompressed;
			
			call FriendToLoverHook::OldLoadFileImm;
			mov RetValue, eax;
			;add esp, 10h
			add esp, 14h
		}
		return RetValue;
	}
}

//==========================
//GBK Environment Support

/*

Stub 1
00F0FB60                                    /$  3C 80          cmp al,80                                       ; フレラバ.00F0FB60(guessed void)
00F0FB62                                    |.  73 03          jae short 00F0FB67
00F0FB64                                    |.  33C0           xor eax,eax
00F0FB66                                    |.  C3             retn
00F0FB67                                    |>  3C A0          cmp al,0A0
00F0FB69                                    |.  73 06          jae short 00F0FB71
00F0FB6B                                    |.  B8 01000000    mov eax,1
00F0FB70                                    |.  C3             retn
00F0FB71                                    |>  3C E0          cmp al,0E0
00F0FB73                                    |.  1BC0           sbb eax,eax
00F0FB75                                    |.  40             inc eax
00F0FB76                                    \.  C3             retn

Stub 2
00F75B00                                    /$  3C 80          cmp al,80                                       ; フレラバ.00F75B00(guessed void)
00F75B02                                    |.  72 04          jb short 00F75B08
00F75B04                                    |.  3C A0          cmp al,0A0
00F75B06                                    |.  72 07          jb short 00F75B0F
00F75B08                                    |>  3C E0          cmp al,0E0
00F75B0A                                    |.  73 03          jae short 00F75B0F
00F75B0C                                    |.  33C0           xor eax,eax
00F75B0E                                    |.  C3             retn
00F75B0F                                    |>  B8 01000000    mov eax,1
00F75B14                                    \.  C3             retn

Stub 3
00F85BB0                                    /$  55             push ebp                                        ; フレラバ.00F85BB0(guessed Arg1)
00F85BB1                                    |.  8BEC           mov ebp,esp
00F85BB3                                    |.  8A45 08        mov al,byte ptr [ebp+8]
00F85BB6                                    |.  3C 80          cmp al,80
00F85BB8                                    |.  73 06          jae short 00F85BC0
00F85BBA                                    |.  33C0           xor eax,eax
00F85BBC                                    |.  5D             pop ebp
00F85BBD                                    |.  C2 0400        retn 4
00F85BC0                                    |>  3C A0          cmp al,0A0
00F85BC2                                    |.  73 09          jae short 00F85BCD
00F85BC4                                    |.  B8 01000000    mov eax,1
00F85BC9                                    |.  5D             pop ebp
00F85BCA                                    |.  C2 0400        retn 4
00F85BCD                                    |>  3C E0          cmp al,0E0
00F85BCF                                    |.  1BC0           sbb eax,eax
00F85BD1                                    |.  40             inc eax
00F85BD2                                    |.  5D             pop ebp
00F85BD3                                    \.  C2 0400        retn 4
*/

//sub_42FB60
__declspec(naked) INT CheckFontCode1()
{
	__asm
	{
		cmp     al, 80h
		jnb     loc_42FB67
		xor     eax, eax
		retn

		loc_42FB67 :
		cmp     al, 0FEh
		jnb     loc_42FB71
		mov     eax, 1
		retn

		loc_42FB71 :
		cmp     al, 0E0h
		sbb     eax, eax
		inc     eax
		retn
	}
}

//ssub_495B00
__declspec(naked) BOOL CheckFontCode2()
{
	__asm
	{
		cmp     al, 80h
		jb      loc_495B08

		cmp     al, 0FEh
		jb      loc_495B0F

		loc_495B08 :
		cmp     al, 0E0h
		jnb     loc_495B0F
		xor     eax, eax
		retn

		loc_495B0F :
		mov     eax, 1
		retn
	}
}

//sub_4A5BB0
INT NTAPI CheckFontCode3(BYTE a1)
{
	INT result = 0;
	if (a1 >= 0x80u)
	{
		if (a1 <= 0xFEu)
			result = 1;
		else
			result = 0;
	}
	else
	{
		result = 0;
	}
	return result;
}


UINT WINAPI HookGetOEMCP(void)
{
	return 936;
}

UINT WINAPI HookGetACP(void)
{
	return 936;
}


ASM INT CheckFontAsSJIS()
{
	INLINE_ASM
	{
		cmp al, 80h
		jae  loc_jmp_01
		mov eax, 0
		jmp loc_jmp_02

	loc_jmp_01:
		mov eax, 1

	loc_jmp_02:
		retn	
	}
}

ASM VOID HookToLower()
{
	__asm
	{
		cmp     byte ptr[edx], 0
		jz      locret_42EB8C

		loc_42EB65 :
		mov     cl, [edx]
		mov     al, cl
		call    CheckFontAsSJIS
		test    eax, eax
		jz      loc_42EB77
		add     edx, 2
		jmp     loc_42EB87

		loc_42EB77 :
		cmp     cl, 41h
		jl      loc_42EB86
		cmp     cl, 5Ah
		jg      loc_42EB86
		add     cl, 20h
		mov[edx], cl

		loc_42EB86 :
		inc     edx

		loc_42EB87 :
		cmp     byte ptr[edx], 0
		jnz     loc_42EB65

		locret_42EB8C :
		retn
	}
}


HANDLE
WINAPI
HookCreateFileA(
__in     LPCSTR lpFileName,
__in     DWORD dwDesiredAccess,
__in     DWORD dwShareMode,
__in_opt LPSECURITY_ATTRIBUTES lpSecurityAttributes,
__in     DWORD dwCreationDisposition,
__in     DWORD dwFlagsAndAttributes,
__in_opt HANDLE hTemplateFile
)
{
	//FriendToLoverHook::WriteInfo(lpFileName, 1);
	return CreateFileA(lpFileName, dwDesiredAccess,
		dwShareMode, lpSecurityAttributes,
		dwCreationDisposition, dwFlagsAndAttributes,
		hTemplateFile);
}


//==========================
//C++

FriendToLoverHook* FriendToLoverHook::Handle = nullptr;

PVOID FriendToLoverHook::OldDecompressFile       = (PVOID)0x004654A0;
PVOID FriendToLoverHook::OldCheckOSDefaultLangID = (PVOID)0x0046F870;
PVOID FriendToLoverHook::OldLoadFileImm          = (PVOID)0x00465700;
PVOID FriendToLoverHook::OldCheckFont1           = (PVOID)0x0042FB60;
PVOID FriendToLoverHook::OldCheckFont2           = (PVOID)0x00495B00;
PVOID FriendToLoverHook::OldCheckFont3           = (PVOID)0x004A5BB0;

FriendToLoverHook::FriendToLoverHook() :
hSelfModule(nullptr),
IndexBuffer(nullptr),
InitFileSystem(FALSE),
MainWin(nullptr),
DefFont(nullptr),
pfTextOutA(nullptr),
pfTextOutW(nullptr),
pfCreateFontA(nullptr),
pfGetOEMCP(nullptr),
pfGetACP(nullptr),
pfCreateWindowExA(nullptr),
pfSetWindowTextA(nullptr),
pfCreateFileA(nullptr),
pfMessageBoxA(nullptr),
pfMultiByteToWideChar(nullptr)
{

}

FriendToLoverHook::~FriendToLoverHook()
{
	if (IndexBuffer)
	{
		HeapFree(GetProcessHeap(), 0, IndexBuffer);
	}
}

FriendToLoverHook* FriendToLoverHook::GetGlobalData()
{
	if (!Handle)
	{
		Handle = new FriendToLoverHook;
		if (!Handle)
		{
			MessageBoxW(NULL, L"初始化错误", BaseAppName, MB_OK);
			return nullptr;
		}
	}
	return Handle;
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


void WriteMemory(PBYTE Code, ULONG Addr, ULONG Len)
{
	DWORD OldCode = 0;
	VirtualProtect((LPVOID)Addr, Len, PAGE_EXECUTE_READWRITE, &OldCode);
	RtlCopyMemory((PVOID)Addr, Code, Len);
}


HRESULT WINAPI FriendToLoverHook::Init(HMODULE hSelf)
{
	if (!hSelfModule)
	{
		hSelfModule = hSelf;
	}

	LoadFileSystem();
	SetAppName();
	
	BOOL Result = True;
#define IF_FAILED(x, y) if(!x) goto y

	pfTextOutW        = GetProcAddress(GetModuleHandleW(L"Gdi32.dll"),    "TextOutW");
	pfTextOutA        = GetProcAddress(GetModuleHandleW(L"Gdi32.dll"),    "TextOutA");
	pfCreateFontA     = GetProcAddress(GetModuleHandleW(L"Gdi32.dll"),    "CreateFontA");
	pfGetOEMCP        = GetProcAddress(GetModuleHandleW(L"Kernel32.dll"), "GetOEMCP");
	pfGetACP          = GetProcAddress(GetModuleHandleW(L"Kernel32.dll"), "GetACP");
	pfCreateFileA     = GetProcAddress(GetModuleHandleW(L"Kernel32.dll"), "CreateFileA");
	pfCreateWindowExA = GetProcAddress(GetModuleHandleW(L"User32.dll"),   "CreateWindowExA");
	pfSetWindowTextA  = GetProcAddress(GetModuleHandleW(L"User32.dll"),   "SetWindowTextA");
	pfMessageBoxA     = GetProcAddress(GetModuleHandleW(L"User32.dll"),   "MessageBoxA");
	pfRegisterClassExA= GetProcAddress(GetModuleHandleW(L"User32.dll"),   "RegisterClassExA");
	pfMultiByteToWideChar = GetProcAddress(GetModuleHandleW(L"Kernel32.dll"), "MultiByteToWideChar");

	Result = IATPatch("Gdi32.dll",    pfTextOutW, (PROC)HookTextOutW);                IF_FAILED(Result, ErrorEnd);
	Result = IATPatch("Gdi32.dll",    pfTextOutA, (PROC)HookTextOutA);                IF_FAILED(Result, ErrorEnd);
	Result = IATPatch("Gdi32.dll",    pfCreateFontA, (PROC)HookCreateFontA);          IF_FAILED(Result, ErrorEnd);
	Result = IATPatch("User32.dll",   pfCreateWindowExA, (PROC)HookCreateWindowExA);  IF_FAILED(Result, ErrorEnd);
	Result = IATPatch("User32.dll",   pfSetWindowTextA, (PROC)HookSetWindowTextA);    IF_FAILED(Result, ErrorEnd);
	Result = IATPatch("User32.dll",   pfMessageBoxA, (PROC)HookMessageBoxA);          IF_FAILED(Result, ErrorEnd);
	//Result = IATPatch("User32.dll",   pfRegisterClassExA, (PROC)HookRegisterClassExA);IF_FAILED(Result, ErrorEnd);
	//Result = IATPatch("Kernel32.dll", pfCreateFileA, (PROC)HookCreateFileA);          IF_FAILED(Result, ErrorEnd);
	Result = IATPatch("Kernel32.dll", pfMultiByteToWideChar,(PROC)HookMultiByteToWideChar); IF_FAILED(Result, ErrorEnd);

#if 1
	Result = IATPatch("Kernel32.dll", pfGetOEMCP, (PROC)HookGetOEMCP);    IF_FAILED(Result, ErrorEnd);
	Result = IATPatch("Kernel32.dll", pfGetACP,   (PROC)HookGetACP);      IF_FAILED(Result, ErrorEnd);
#endif

	LONG RelocateValue = (SIZE_T)GetModuleHandleW(nullptr) - 0x00400000;

	PVOID DynamicOldCheckOSDefaultLangID = (PVOID)((SIZE_T)OldCheckOSDefaultLangID + RelocateValue);
	OldCheckOSDefaultLangID = DynamicOldCheckOSDefaultLangID;

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(&OldCheckOSDefaultLangID, CheckOSDefaultLangID);
	DetourTransactionCommit();


#if 0
	PVOID DynamicOldDecompressFile = (PVOID)((SIZE_T)OldDecompressFile + RelocateValue);
	OldDecompressFile = DynamicOldDecompressFile;

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(&OldDecompressFile, HookDecompressFile_FASTCALL);
	DetourTransactionCommit();
#endif

#if 1
	PVOID DynamicOldLoadFileImm = (PVOID)((SIZE_T)OldLoadFileImm + RelocateValue);
	OldLoadFileImm = DynamicOldLoadFileImm;

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(&OldLoadFileImm, ::LoadFileBuffer);
	DetourTransactionCommit();

#endif


	PVOID DynamicOldCheckFont1 = (PVOID)((SIZE_T)OldCheckFont1 + RelocateValue);
	OldCheckFont1 = DynamicOldCheckFont1;

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(&OldCheckFont1, CheckFontCode1);
	DetourTransactionCommit();


	//sub_42EB60

#if 1
	PVOID DynamicOldToLower = (PVOID)((SIZE_T)0x0042EB60 + RelocateValue);
	auto OldToLower = DynamicOldToLower;

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(&OldToLower, HookToLower);
	DetourTransactionCommit();
#endif 

#if 1
	PVOID DynamicOldCheckFont2 = (PVOID)((SIZE_T)OldCheckFont2 + RelocateValue);
	OldCheckFont2 = DynamicOldCheckFont2;

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(&OldCheckFont2, CheckFontCode2);
	DetourTransactionCommit();

	PVOID DynamicOldCheckFont3 = (PVOID)((SIZE_T)OldCheckFont3 + RelocateValue);
	OldCheckFont3 = DynamicOldCheckFont3;

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(&OldCheckFont3, CheckFontCode3);
	DetourTransactionCommit();

#endif

	/*
	.text:0042E498                 cmp     ebx, 0EF40h
	.text:0042E4E8                 cmp     ebx, 0EF40h
	.text:0042F896                 cmp     ebx, 0EF40h
	.text:0042F8ED                 cmp     ebx, 0EF40h

	Code:
	00F1F896                      |.  81FB 40EF0000  cmp ebx,0EF40
	*/

	PVOID DynamicHookFontBounary1 = (PVOID)((SIZE_T)0x0042E498 + RelocateValue);
	PVOID DynamicHookFontBounary2 = (PVOID)((SIZE_T)0x0042E4E8 + RelocateValue);
	PVOID DynamicHookFontBounary3 = (PVOID)((SIZE_T)0x0042F896 + RelocateValue);
	PVOID DynamicHookFontBounary4 = (PVOID)((SIZE_T)0x0042F8ED + RelocateValue);

	static BYTE HookCodeEBX[] = { 0x81, 0xFB, 0x40, 0xFE };

	WriteMemory(HookCodeEBX, (ULONG_PTR)DynamicHookFontBounary1, sizeof(HookCodeEBX));
	WriteMemory(HookCodeEBX, (ULONG_PTR)DynamicHookFontBounary2, sizeof(HookCodeEBX));
	WriteMemory(HookCodeEBX, (ULONG_PTR)DynamicHookFontBounary3, sizeof(HookCodeEBX));
	WriteMemory(HookCodeEBX, (ULONG_PTR)DynamicHookFontBounary4, sizeof(HookCodeEBX));


	/*
	.text:0042E542                 cmp     ebx, 8140h
	.text:0042E889                 cmp     eax, 8140h
	*/

	PVOID DynamicHookFontSpace1 = (PVOID)((SIZE_T)0x0042E542 + RelocateValue);
	PVOID DynamicHookFontSpace2 = (PVOID)((SIZE_T)0x0042E889 + RelocateValue);
	static BYTE HookSpaceEBX[] = { 0x81, 0xFB, 0xA1, 0xA1 };
	static BYTE HookSpaceEAX[] = { 0x3D, 0x40, 0xA1, 0xA1 };

	WriteMemory(HookSpaceEBX, (ULONG_PTR)DynamicHookFontSpace1, sizeof(HookSpaceEBX));
	WriteMemory(HookSpaceEAX, (ULONG_PTR)DynamicHookFontSpace2, sizeof(HookSpaceEAX));

#if 0

	ULONG          DllSize;
	PBYTE          DllBuffer = NULL;
	HMODULE        hLib;
	UNICODE_STRING ModuleFileName;
	WCHAR          CurPath[MAX_PATH];
	NTSTATUS       Status;
	
	GetCurrentDirectoryW(MAX_PATH, CurPath);
	lstrcatW(CurPath, L"\\NtLayer.dll");
	RtlInitUnicodeString(&ModuleFileName, CurPath);
	MH_Initialize();
	
	Status = STATUS_UNSUCCESSFUL;

	if (LoadFileBuffer("NtLayer.dll", DllBuffer, DllSize) == S_OK)
	{
			Status = LoadDllFromMemory(
			DllBuffer,
			DllSize,
			&ModuleFileName,
			(PVOID*)&hLib,
			0);

			HeapFree(GetProcessHeap(), 0, DllBuffer);
	}

	UINT RetCode = 0;

	if (!NT_SUCCESS(Status))
	{
		WinFile ErrorLogIn, ErrorLogOut;
		do
		{
			if (ErrorLogIn.Open(L"_.LastStatus", WinFile::FileRead) == S_OK)
			{
				ErrorLogIn.Release();
				break;
			}

			RetCode = MessageBoxW(NULL, L"部分模块加载失败，但是汉化补丁依然能运行\n"
				L"选择\"是\"->下次不再提醒\n"
				L"选择\"否\"->下次依然提醒", L"X'moe CoreLib", MB_OKCANCEL | MB_ICONERROR);

			if (RetCode == IDOK)
			{
				ErrorLogOut.Open(L"_.LastStatus", WinFile::FileWrite);
				ErrorLogOut.Release();
			}
		} while (0);
	}

#endif

	return S_OK;

ErrorEnd:
	ExitMessage(L"运行错误=。=");
	return S_FALSE;
}

HRESULT WINAPI FriendToLoverHook::UnInit(HMODULE hSelf)
{
	UNREFERENCED_PARAMETER(hSelf);

	if (DefFont)
		RemoveFontMemResourceEx(DefFont);

	SystemFile.Release();
	return S_OK;
}

HFONT WINAPI FriendToLoverHook::DuplicateFontW(HDC hdc, UINT LangId)
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
	lstrcpyW(Info.lfFaceName, LangId == CP_SHIFTJIS ? L"MS Gothic" : L"黑体");
	lplf->lfCharSet = LangId;

	HFONT hFont = CreateFontIndirectW(&Info);

	HeapFree(GetProcessHeap(), 0, lplf);
	return hFont;
}


ULONG WINAPI FriendToLoverHook::AnsiToUnicode(
	LPCSTR lpAnsi,
	ULONG  Length,
	LPWSTR lpUnicodeBuffer,
	ULONG  BufferCount,
	ULONG  CodePage)
{
	if (CodePage == 932)
		CodePage = 936;

	return MultiByteToWideChar(CodePage, 0, lpAnsi, Length, lpUnicodeBuffer, BufferCount);
}

UINT WINAPI FriendToLoverHook::ExitMessage(const WCHAR* Info, const WCHAR* Title)
{
	UINT Code = MessageBoxW(nullptr, Info, Title, MB_OK);
	::ExitProcess(-1);
	return Code;
}


BOOL WINAPI FriendToLoverHook::WriteInfo(const WCHAR* lpInfo, BOOL ForRelease)
{
	if (!ForRelease)
		return FALSE;

	DWORD ByteRead = 0;
	return WriteConsoleW(GetStdHandle(STD_OUTPUT_HANDLE), lpInfo, lstrlenW(lpInfo), &ByteRead, nullptr) &&
		WriteConsoleW(GetStdHandle(STD_OUTPUT_HANDLE), L"\n", 1, &ByteRead, nullptr);
}

BOOL WINAPI FriendToLoverHook::WriteInfo(const CHAR* lpInfo, BOOL ForRelease)
{
	if (!ForRelease)
		return FALSE;

	DWORD ByteRead = 0;
	return WriteConsoleA(GetStdHandle(STD_OUTPUT_HANDLE), lpInfo, lstrlenA(lpInfo), &ByteRead, nullptr) &&
		WriteConsoleW(GetStdHandle(STD_OUTPUT_HANDLE), L"\n", 1, &ByteRead, nullptr);
}


BOOL WINAPI FriendToLoverHook::WriteInfo(const ULONG  Info, BOOL ForRelease)
{
	if (!ForRelease)
		return FALSE;

	CHAR NumStr[30] = { 0 };
	wsprintfA(NumStr, "Reg : 0x%08x", Info);
	DWORD ByteRead = 0;
	return WriteConsoleA(GetStdHandle(STD_OUTPUT_HANDLE), NumStr, lstrlenA(NumStr), &ByteRead, nullptr) &&
		WriteConsoleW(GetStdHandle(STD_OUTPUT_HANDLE), L"\n", 1, &ByteRead, nullptr);
}

#define ProjectDir L"ProjectDir\\"


wstring ToLowerString(wstring& Name)
{
	wstring Result;
	for (auto it : Name)
	{
		if ((it >= L'A') && (it <= L'Z'))
		{
			Result += tolower(it);
		}
		else
		{
			Result += it;
		}
	}
	return Result;
}

HRESULT WINAPI FriendToLoverHook::LoadFileBuffer(LPCSTR lpFileName, PBYTE& Buffer, ULONG& OutSize)
{
	HRESULT Result = S_OK;
	WCHAR   szFileName[MAX_PATH] = { 0 };

	AnsiToUnicode(lpFileName, lstrlenA(lpFileName), szFileName, MAX_PATH);
	wstring FileName = GetPackageName(wstring(szFileName));

	if (!InitFileSystem)
	{
		wstring Path = ProjectDir;
		Path += FileName;
		WinFile File;

		if (File.Open(Path.c_str(), WinFile::FileRead) == S_FALSE)
		{
			WriteInfo(Path.c_str());
			return S_FALSE;
		}

		OutSize = File.GetSize32();
		Buffer = (PBYTE)HostAlloc(OutSize);
		if (Buffer == nullptr)
		{
			Result = S_FALSE;
			goto ProjectReadEnd;
		}
		File.Read(Buffer, OutSize);

	ProjectReadEnd:
		File.Release();
	}
	else
	{
		//Direct-loading may cause a bug that will crash this process
#if 0
		WinFile File;
		

		if (File.Open(szFileName, WinFile::FileRead) == S_OK)
		{
			Buffer = (PBYTE)HostAlloc(File.GetSize32());
			File.Read(Buffer, File.GetSize32());
			OutSize = File.GetSize32();
			WriteInfo(L"Ok:", 1); WriteInfo(szFileName, 1);
			return S_OK;
		}
		else
#endif
		{
			auto it = ChunkList.find(Hash64(ToLowerString(FileName).c_str(), FileName.length() * 2));
			if (it != ChunkList.end())
			{
				OutSize = it->second.Size;
				Buffer = (PBYTE)HostAlloc(OutSize);
				if (Buffer == nullptr)
				{
					return S_FALSE;
				}
				SystemFile.Seek(it->second.Offset, FILE_BEGIN);
				SystemFile.Read(Buffer, OutSize);

			}
			else
			{
				return S_FALSE;
			}
		}
	}
	return Result;
}
