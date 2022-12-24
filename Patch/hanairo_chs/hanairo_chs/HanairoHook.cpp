#include "HanairoHook.h"
#include "ImageEx.h"
#include "Compressor.h"
#include "TTFData.h"

//Allocator
//0x00488A7B

DWORD ImmAlloc = 0x0048A967;
DWORD ImmFree = 0x0048A6AA;

ASM PVOID CDECL HostAlloc(LONG Size)
{
	INLINE_ASM
	{
		push 1
		push dword ptr[esp + 8]
		call ImmAlloc
		pop ecx
		pop ecx
		retn
	}
}

//sub_488A70
ASM VOID CDECL HostFree(PVOID lpMem)
{
	INLINE_ASM
	{
		push dword ptr[esp + 4]
		call ImmFree
		pop ecx
		retn
	}
}

//==========================
//sub_455CE0
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
		if (IsShiftJISString((PCHAR)lpString, cbCount))
			CodePage = CP_SHIFTJIS;

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

		hFont = HanairoHook::GetGlobalData()->DuplicateFontW(hDC, SHIFTJIS_CHARSET);
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

	HanairoHook::GetGlobalData()->WriteInfo(ch);
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
	//HanairoHook::GetGlobalData()->WriteInfo(lpString);
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

	if (HanairoHook::GetGlobalData()->GetDefFont())
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
		lstrcpyW(lf.lfFaceName, L"华康少女文字W5(P)");

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
		HanairoHook::GetGlobalData()->AnsiToUnicode(
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
	HWND Result = nullptr;
	if (!lstrcmpA(lpClassName, "BGI - Main window"))
	{
		Result = CreateWindowExA(dwExStyle, lpClassName, lpWindowName, dwStyle, x, y,
			nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
		HanairoHook::GetGlobalData()->MainWin = Result;
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
	if (HanairoHook::GetGlobalData()->MainWin == hWnd)
	{
		wstring QueryInfo;
		HanairoHook::GetGlobalData()->GetAppName(QueryInfo);
		return SetWindowTextW(hWnd, QueryInfo.c_str());
	}
	return SetWindowTextA(hWnd, lpString);
}


HANDLE
WINAPI
HookCreateFileA(
LPCSTR                  lpFileName,
DWORD                   dwDesiredAccess,
DWORD                   dwShareMode,
LPSECURITY_ATTRIBUTES   lpSecurityAttributes,
DWORD                   dwCreationDisposition,
DWORD                   dwFlagsAndAttributes,
HANDLE                  hTemplateFile
)
{
	WCHAR   szFile[MAX_PATH] = {0};
	ULONG   Length;

	Length = HanairoHook::GetGlobalData()->AnsiToUnicode(lpFileName, lstrlenA(lpFileName), szFile, countof(szFile));
	//HanairoHook::WriteInfo(szFile);
	return CreateFileW(szFile, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
}

int WINAPI HookMessageBoxA(
	_In_opt_ HWND    hWnd,
	_In_opt_ LPCSTR lpText,
	_In_opt_ LPCSTR lpCaption,
	_In_     UINT    uType
	)
{
	PWCHAR WideText    = nullptr;
	PWCHAR WideCaption = nullptr;
	int    Result = 0;
	ULONG TextAllocSize = 0, CaptionAllocSize = 0;

	TextAllocSize = (lstrlenA(lpText) + 1) * 2;
	CaptionAllocSize = (lstrlenA(lpCaption) + 1) * 2;

	WideText    = (PWCHAR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, TextAllocSize);
	WideCaption = (PWCHAR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, CaptionAllocSize);

	if (WideText && WideCaption)
	{
		HanairoHook::GetGlobalData()->AnsiToUnicode(lpText,    lstrlenA(lpText),
			WideText, TextAllocSize,       IsShiftJISString(lpText,    lstrlenA(lpText))    ? 932 : 936);
		HanairoHook::GetGlobalData()->AnsiToUnicode(lpCaption, lstrlenA(lpCaption),
			WideCaption, CaptionAllocSize, IsShiftJISString(lpCaption, lstrlenA(lpCaption)) ? 932 : 936);

		Result = MessageBoxW(hWnd, WideText, WideCaption, uType);
	}
	else
	{
		TextAllocSize = MAX_PATH * 2;
		CaptionAllocSize = MAX_PATH * 2;
		
		WCHAR StackText[MAX_PATH * 2] = { 0 };
		WCHAR StackCaption[MAX_PATH * 2] = { 0 };

		HanairoHook::GetGlobalData()->AnsiToUnicode(lpText,    lstrlenA(lpText),
			StackText,    TextAllocSize,    IsShiftJISString(lpText,    lstrlenA(lpText))    ? 932 : 936);
		HanairoHook::GetGlobalData()->AnsiToUnicode(lpCaption, lstrlenA(lpCaption),
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
	return MultiByteToWideChar(CodePage, dwFlags, lpMultiByteStr, cbMultiByte, lpWideCharStr, cchWideChar);
}


//==DecompressInfo
//sub_44E380
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
	((StubDecompressFile)HanairoHook::OldDecompressFile)(pvDecompressed, pOutSize, pvCompressed, InSize, SkipBytes, OutBytes);
}


ULONG UnicodeWin32FindDataToAnsi(LPWIN32_FIND_DATAW pwfdW, LPWIN32_FIND_DATAA pwfdA)
{
	pwfdA->dwFileAttributes = pwfdW->dwFileAttributes;
	pwfdA->ftCreationTime   = pwfdW->ftCreationTime;
	pwfdA->ftLastAccessTime = pwfdW->ftLastAccessTime;
	pwfdA->ftLastWriteTime  = pwfdW->ftLastWriteTime;
	pwfdA->nFileSizeHigh    = pwfdW->nFileSizeHigh;
	pwfdA->nFileSizeLow     = pwfdW->nFileSizeLow;
	pwfdA->dwReserved0      = pwfdW->dwReserved0;
	pwfdA->dwReserved1      = pwfdW->dwReserved1;

	WideCharToMultiByte(CP_ACP, 0, pwfdW->cFileName, -1, pwfdA->cFileName, sizeof(pwfdA->cFileName), 0, 0);
	return WideCharToMultiByte(CP_ACP, 0, pwfdW->cAlternateFileName, -1, pwfdA->cAlternateFileName, sizeof(pwfdA->cAlternateFileName), 0, 0);
}


//sub_44E630
//ReadFile Outer Firstly
LONG 
CDECL 
LoadFileBuffer(
	PVOID  pvDecompressed, //64M 预先分配的
	PULONG pOutSize,
	LPCSTR lpFileName,
	ULONG  SkipBytes,
	ULONG  OutBytes
)
{
	LONG   RetValue = 5; 
	PBYTE  InBuffer = nullptr;
	ULONG  InSize;

	if (HanairoHook::GetGlobalData()->LoadFileBuffer(lpFileName, InBuffer, InSize) == S_OK)
	{
		RetValue = HookDecompressFile(
			pvDecompressed,
			pOutSize,
			InBuffer,
			InSize,
			0,
			0);

		if(InBuffer)
			HostFree(InBuffer);

		return RetValue;
	}
	else
	{
		HanairoHook::WriteInfo(L"Ori:");
		HanairoHook::WriteInfo(lpFileName);
		return (StubLoadFile(HanairoHook::OldLoadFileImm))(pvDecompressed, pOutSize, lpFileName, SkipBytes, OutBytes);
	}
}

//==========================
//GBK Environment Support

/*

Stub 1
00425640              /$  8A4424 04             mov al,byte ptr [esp+4]                      ; Heptagram.00425640(guessed Arg1)
00425644              |.  3C 80                 cmp al,80
00425646              |.  73 03                 jae short 0042564B
00425648              |.  33C0                  xor eax,eax
0042564A              |.  C3                    retn
0042564B              |>  3C A0                 cmp al,0A0
0042564D              |.  73 06                 jae short 00425655
0042564F              |.  B8 01000000           mov eax,1
00425654              |.  C3                    retn
00425655              |>  3C E0                 cmp al,0E0
00425657              |.  1BC0                  sbb eax,eax
00425659              |.  40                    inc eax
0042565A              \.  C3                    retn

Stub2
00476100              /$  8A4424 04             mov al,byte ptr [esp+4]                      ; Heptagram.00476100(guessed Arg1)
00476104              |.  3C 80                 cmp al,80
00476106              |.  72 04                 jb short 0047610C
00476108              |.  3C A0                 cmp al,0A0
0047610A              |.  72 07                 jb short 00476113
0047610C              |>  3C E0                 cmp al,0E0
0047610E              |.  73 03                 jae short 00476113
00476110              |.  33C0                  xor eax,eax
00476112              |.  C3                    retn
00476113              |>  B8 01000000           mov eax,1
00476118              \.  C3                    retn
*/

//sub_425640
int CDECL CheckFontCode1(BYTE a1)
{
	int result = 0; 
	if (a1 >= 0x81u)
	{
		result = 1;
	}
	else
	{
		result = 0;
	}
	return result;
}

//sub_476100
BOOL CDECL CheckFontCode2(BYTE a1)
{
	return a1 >= 0x81u && a1 < 0xFFu;
}


UINT WINAPI HookGetOEMCP(void)
{
	return 936;
}

UINT WINAPI HookGetACP(void)
{
	return 936;
}

//==========================
//C++

HanairoHook* HanairoHook::Handle = nullptr;

PVOID HanairoHook::OldDecompressFile       = (PVOID)0x0044E380;
PVOID HanairoHook::OldCheckOSDefaultLangID = (PVOID)0x00455CE0;
PVOID HanairoHook::OldLoadFileImm          = (PVOID)0x0044E630;
PVOID HanairoHook::OldCheckFont1           = (PVOID)0x00425640;
PVOID HanairoHook::OldCheckFont2           = (PVOID)0x00476100;

HanairoHook::HanairoHook() :
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

HanairoHook::~HanairoHook()
{
	if (IndexBuffer)
	{
		HeapFree(GetProcessHeap(), 0, IndexBuffer);
	}
}

HanairoHook* HanairoHook::GetGlobalData()
{
	if (!Handle)
	{
		Handle = new HanairoHook;
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


HRESULT WINAPI HanairoHook::Init(HMODULE hSelf)
{
	if (!hSelfModule)
	{
		hSelfModule = hSelf;
	}

	LoadFileSystem();
	SetAppName();

	DWORD nFont;
	DefFont = (HFONT)AddFontMemResourceEx(TTFDataStatic, TTFDataStaticSize, NULL, &nFont);
	//DefFont = (HFONT)AddFontResourceExW(L"ca.ttf", FR_PRIVATE, NULL);
	if (DefFont)
	{
		//
	}

	BOOL Result = True;
#define IF_FAILED(x, y) if(!x) goto y

	pfTextOutW            = GetProcAddress(GetModuleHandleW(L"Gdi32.dll"),    "TextOutW");
	pfTextOutA            = GetProcAddress(GetModuleHandleW(L"Gdi32.dll"),    "TextOutA");
	pfCreateFontA         = GetProcAddress(GetModuleHandleW(L"Gdi32.dll"),    "CreateFontA");
	pfGetOEMCP            = GetProcAddress(GetModuleHandleW(L"Kernel32.dll"), "GetOEMCP");
	pfGetACP              = GetProcAddress(GetModuleHandleW(L"Kernel32.dll"), "GetACP");
	pfCreateWindowExA     = GetProcAddress(GetModuleHandleW(L"User32.dll"),   "CreateWindowExA");
	pfSetWindowTextA      = GetProcAddress(GetModuleHandleW(L"User32.dll"),   "SetWindowTextA");
	pfCreateFileA         = GetProcAddress(GetModuleHandleW(L"Kernel32.dll"), "CreateFileA");
	pfMessageBoxA         = GetProcAddress(GetModuleHandleW(L"User32.dll"),   "MessageBoxA");
	pfMultiByteToWideChar = GetProcAddress(GetModuleHandleW(L"Kernel32.dll"), "MultiByteToWideChar");

	Result = IATPatch("Gdi32.dll",   pfTextOutW,           (PROC)HookTextOutW);           IF_FAILED(Result, ErrorEnd);
	Result = IATPatch("Gdi32.dll",   pfTextOutA,           (PROC)HookTextOutA);           IF_FAILED(Result, ErrorEnd);
	Result = IATPatch("Gdi32.dll",   pfCreateFontA,        (PROC)HookCreateFontA);        IF_FAILED(Result, ErrorEnd);
	Result = IATPatch("User32.dll",  pfCreateWindowExA,    (PROC)HookCreateWindowExA);    IF_FAILED(Result, ErrorEnd);
	Result = IATPatch("User32.dll",  pfSetWindowTextA,     (PROC)HookSetWindowTextA);     IF_FAILED(Result, ErrorEnd);
	Result = IATPatch("Kernel32.dll",pfCreateFileA,        (PROC)HookCreateFileA);        IF_FAILED(Result, ErrorEnd);
	Result = IATPatch("User32.dll",  pfMessageBoxA,        (PROC)HookMessageBoxA);        IF_FAILED(Result, ErrorEnd);
	Result = IATPatch("Kernel32.dll",pfMultiByteToWideChar,(PROC)HookMultiByteToWideChar);IF_FAILED(Result, ErrorEnd);

#if 1
	Result = IATPatch("Kernel32.dll", pfGetOEMCP,    (PROC)HookGetOEMCP);    IF_FAILED(Result, ErrorEnd);
	Result = IATPatch("Kernel32.dll", pfGetACP,      (PROC)HookGetACP);      IF_FAILED(Result, ErrorEnd);
#endif

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(&OldCheckOSDefaultLangID, CheckOSDefaultLangID);
	DetourTransactionCommit();

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(&OldDecompressFile, HookDecompressFile);
	DetourTransactionCommit();

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(&OldLoadFileImm, ::LoadFileBuffer);
	DetourTransactionCommit();

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(&OldCheckFont1, CheckFontCode1);
	DetourTransactionCommit();

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(&OldCheckFont2, CheckFontCode2);
	DetourTransactionCommit();

	/*
	00425124              |.  81FB 40EF0000         cmp ebx,0EF40
	00425194              |.  81FB 40EF0000         cmp ebx,0EF40
	00426425              |.  3D 40EF0000           cmp eax,0EF40
	00426469              |.  3D 40EF0000           cmp eax,0EF40
	*/

	static BYTE HookCodeEBX[] = { 0x81, 0xEF, 0x40, 0xFE };
	static BYTE HookCodeEAX[] = { 0x3D, 0x40, 0xFE };

	//WriteMemory(HookCodeEBX, 0x00425124, sizeof(HookCodeEBX));
	//WriteMemory(HookCodeEBX, 0x00425194, sizeof(HookCodeEBX));
	//WriteMemory(HookCodeEAX, 0x00426425, sizeof(HookCodeEAX));
	//WriteMemory(HookCodeEAX, 0x00426469, sizeof(HookCodeEAX));

	/*
	00425202              |.  81FB 40810000         cmp ebx,8140
	00425375              |.  81FB 40810000         cmp ebx,8140
	*/
	static BYTE HookSpace[] = { 0x81, 0xFB, 0xA1, 0xA1 };

	WriteMemory(HookSpace, 0x00425202, sizeof(HookSpace));
	WriteMemory(HookSpace, 0x00425375, sizeof(HookSpace));

	return S_OK;

ErrorEnd:
	ExitMessage(L"运行错误=。=");
	return S_FALSE;
}

HRESULT WINAPI HanairoHook::UnInit(HMODULE hSelf)
{
	UNREFERENCED_PARAMETER(hSelf);
	
	//MessageBox(0, L"UnLoad", 0, 0);

	if (DefFont)
		RemoveFontMemResourceEx(DefFont);
	
	SystemFile.Release();
	return S_OK;
}

HFONT WINAPI HanairoHook::DuplicateFontW(HDC hdc, UINT LangId)
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


ULONG WINAPI HanairoHook::AnsiToUnicode(
	LPCSTR lpAnsi,
	ULONG  Length,
	LPWSTR lpUnicodeBuffer,
	ULONG  BufferCount,
	ULONG  CodePage)
{
	return MultiByteToWideChar(CodePage, 0, lpAnsi, Length, lpUnicodeBuffer, BufferCount);
}

UINT WINAPI HanairoHook::ExitMessage(const WCHAR* Info, const WCHAR* Title)
{
	UINT Code = MessageBoxW(nullptr, Info, Title, MB_OK);
	::ExitProcess(-1);
	return Code;
}

wstring WINAPI HanairoHook::GetPackageName(wstring& fileName)
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


BOOL WINAPI HanairoHook::WriteInfo(const WCHAR* lpInfo, BOOL ForRelease)
{
	if (!ForRelease)
		return FALSE;

	DWORD ByteRead = 0;
	return WriteConsoleW(GetStdHandle(STD_OUTPUT_HANDLE), lpInfo, lstrlenW(lpInfo), &ByteRead, nullptr) &&
		WriteConsoleW(GetStdHandle(STD_OUTPUT_HANDLE), L"\n", 1, &ByteRead, nullptr);
}

BOOL WINAPI HanairoHook::WriteInfo(const CHAR* lpInfo, BOOL ForRelease)
{
	if (!ForRelease)
		return FALSE;

	DWORD ByteRead = 0;
	return WriteConsoleA(GetStdHandle(STD_OUTPUT_HANDLE), lpInfo, lstrlenA(lpInfo), &ByteRead, nullptr) &&
		WriteConsoleW(GetStdHandle(STD_OUTPUT_HANDLE), L"\n", 1, &ByteRead, nullptr);
}

#define ProjectDir L"ProjectDir\\"

HRESULT WINAPI HanairoHook::LoadFileBuffer(LPCSTR lpFileName, PBYTE& Buffer, ULONG& OutSize)
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
		auto it = ChunkList.find(Hash64(FileName.c_str(), FileName.length() * 2));
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

#if 0
			WCHAR lpInfo[100] = { 0 };
			wsprintfW(lpInfo, L"%08x", it->second.Offset);
			MessageBox(0, lpInfo, 0, 0);

			FILE* f = _wfopen(it->second.lpFileName, L"wb");
			fwrite(Buffer, 1, OutSize, f);
			fclose(f);
#endif
		}
		else
		{
			return S_FALSE;
		}
	}
	return Result;
}
