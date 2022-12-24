#include "LocaleEmulator.h"


/************************************************************************/
/* kernel32.dll                                                         */
/************************************************************************/

UINT WINAPI MyGetACP()
{
	return 936;
}

UINT WINAPI MyGetOEMCP()
{
	return 936;
}


ULONG GetEmuCodePage(ULONG CodePage)
{
	switch (CodePage)
	{
	case CP_ACP:
	case CP_OEMCP:
	case CP_THREAD_ACP:
		return 932;

	default:
		return CodePage;
	}
}

API_POINTER(WideCharToMultiByte) OldWideCharToMultiByte = NULL;

int
WINAPI
MyWideCharToMultiByte(
UINT    CodePage,
DWORD   dwFlags,
LPCWSTR lpWideCharStr,
int     cchWideChar,
LPSTR   lpMultiByteStr,
int     cbMultiByte,
LPCSTR  lpDefaultChar,
LPBOOL  lpUsedDefaultChar
)
{
	return OldWideCharToMultiByte(
		932,
		dwFlags,
		lpWideCharStr,
		cchWideChar,
		lpMultiByteStr,
		cbMultiByte,
		lpDefaultChar,
		lpUsedDefaultChar);
}



API_POINTER(MultiByteToWideChar) OldMultiByteToWideChar = NULL;

int
WINAPI
MyMultiByteToWideChar(
UINT    CodePage,
DWORD   dwFlags,
LPCSTR  lpMultiByteStr,
int     cbMultiByte,
LPWSTR  lpWideCharStr,
int     cchWideChar
)
{
	return OldMultiByteToWideChar(
		932,
		dwFlags,
		lpMultiByteStr,
		cbMultiByte,
		lpWideCharStr,
		cchWideChar);
}

API_POINTER(IsDBCSLeadByteEx) OldIsDBCSLeadByteEx = NULL;

BOOL WINAPI MyIsDBCSLeadByteEx(UINT CodePage, BYTE TestChar)
{
	return OldIsDBCSLeadByteEx(936, TestChar);
}

BOOL WINAPI MyIsDBCSLeadByte(BYTE TestChar)
{
	return MyIsDBCSLeadByteEx(CP_ACP, TestChar);
}


UINT WINAPI MyGdiGetCodePage(HDC hDC)
{
	UNREFERENCED_PARAMETER(hDC);
	return 936;
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


WCHAR FontNameLocal[MAX_PATH];


HFONT WINAPI MyCreateFontIndirectA(LOGFONTA *lplf)
{
	LOGFONTW lf;
	RtlZeroMemory(&lf, sizeof(LOGFONTW));

	lf.lfHeight = lplf->lfHeight;
	lf.lfWidth = lplf->lfWidth;
	lf.lfEscapement = lplf->lfEscapement;
	lf.lfOrientation = lplf->lfOrientation;
	lf.lfWeight = lplf->lfWeight;
	lf.lfItalic = lplf->lfItalic;
	lf.lfUnderline = lplf->lfUnderline;
	lf.lfStrikeOut = lplf->lfStrikeOut;
	lf.lfCharSet = GB2312_CHARSET;
	lf.lfOutPrecision = lplf->lfOutPrecision;
	lf.lfClipPrecision = lplf->lfClipPrecision;
	lf.lfQuality = lplf->lfQuality;
	lf.lfPitchAndFamily = lplf->lfPitchAndFamily;

	if (StrLengthW(FontNameLocal))
		StrCopyW(lf.lfFaceName, FontNameLocal);
	else
		StrCopyW(lf.lfFaceName, L"黑体");

	return CreateFontIndirectW(&lf);
}


HWND
WINAPI
MyCreateWindowExA(
_In_ DWORD dwExStyle,
_In_opt_ LPCSTR lpClassName,
_In_opt_ LPCSTR lpWindowName,
_In_ DWORD dwStyle,
_In_ int X,
_In_ int Y,
_In_ int nWidth,
_In_ int nHeight,
_In_opt_ HWND hWndParent,
_In_opt_ HMENU hMenu,
_In_opt_ HINSTANCE hInstance,
_In_opt_ LPVOID lpParam)
{
	LPWSTR ClassName, WindowName;
	ULONG_PTR Length;

	Length = StrLengthA(lpClassName);
	ClassName = (PWSTR)AllocStack((Length + 1) * 2);
	RtlZeroMemory(ClassName, (Length + 1) * 2);
	MultiByteToWideChar(932, 0, lpClassName, Length, ClassName, (Length + 1) * 2);

	Length = StrLengthA(lpWindowName);
	WindowName = (PWSTR)AllocStack((Length + 1) * 2);
	RtlZeroMemory(WindowName, (Length + 1) * 2);
	MultiByteToWideChar(932, 0, lpWindowName, Length, WindowName, (Length + 1) * 2);

	if (StrCompareA(lpClassName, "GameWindowI") == 0)
		WindowName = L"[X'moe汉化组]Stellar☆Theater encore";

	return CreateWindowExW(dwExStyle,
		ClassName,
		WindowName,
		dwStyle,
		X,
		Y,
		nWidth,
		nHeight,
		hWndParent,
		hMenu,
		hInstance,
		lpParam);
}

LRESULT
WINAPI
MyDefWindowProcA(
_In_ HWND hWnd,
_In_ UINT Msg,
_In_ WPARAM wParam,
_In_ LPARAM lParam)
{
	return DefWindowProcW(hWnd, Msg, wParam, lParam);
}


ATOM
WINAPI
MyRegisterClassA(
_In_ CONST WNDCLASSA *lpWndClass)
{
	WNDCLASSW  ClassInfo;
	LPWSTR       MenuName, ClassName;


	MenuName = (LPWSTR)AllocStack(MAX_PATH * 2);
	ClassName = (LPWSTR)AllocStack(MAX_PATH * 2);

	RtlZeroMemory(MenuName, MAX_PATH * 2);
	RtlZeroMemory(ClassName, MAX_PATH * 2);

	MultiByteToWideChar(932, 0, lpWndClass->lpszMenuName, StrLengthA(lpWndClass->lpszMenuName), MenuName, MAX_PATH);
	MultiByteToWideChar(932, 0, lpWndClass->lpszClassName, StrLengthA(lpWndClass->lpszClassName), ClassName, MAX_PATH);

	ClassInfo.style = lpWndClass->style;
	ClassInfo.lpfnWndProc = lpWndClass->lpfnWndProc;
	ClassInfo.cbClsExtra = lpWndClass->cbClsExtra;
	ClassInfo.cbWndExtra = lpWndClass->cbWndExtra;
	ClassInfo.hInstance = lpWndClass->hInstance;
	ClassInfo.hIcon = lpWndClass->hIcon;
	ClassInfo.hCursor = lpWndClass->hCursor;
	ClassInfo.hbrBackground = lpWndClass->hbrBackground;
	ClassInfo.lpszMenuName = MenuName;
	ClassInfo.lpszClassName = ClassName;

	return RegisterClassW(&ClassInfo);
}


LRESULT WINAPI MyDispatchMessageA(MSG *lpMsg)
{
	return DispatchMessageW(lpMsg);
}


BOOL WINAPI MyPeekMessageA(
	_Out_ LPMSG lpMsg,
	_In_opt_ HWND hWnd,
	_In_ UINT wMsgFilterMin,
	_In_ UINT wMsgFilterMax,
	_In_ UINT wRemoveMsg)
{
	return PeekMessageW(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg);
}




BOOL WINAPI MyPostMessageA(
	_In_opt_ HWND hWnd,
	_In_ UINT Msg,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam)
{
	return PostMessageW(hWnd, Msg, wParam, lParam);
}


DWORD WINAPI MyGetGlyphOutlineA(
	_In_        HDC            hdc,
	_In_        UINT           uChar,
	_In_        UINT           uFormat,
	_Out_       LPGLYPHMETRICS lpgm,
	_In_        DWORD          cbBuffer,
	_Out_       LPVOID         lpvBuffer,
	_In_  const MAT2           *lpmat2
	)
{
	ULONG      len;
	CHAR       mbchs[2];
	HFONT      hFont, hOldFont;
	LOGFONTW*  lplf;
	WCHAR      OutChar;
	UINT       cp = 936;


	if (OldIsDBCSLeadByteEx(cp, uChar >> 8))
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
	OldMultiByteToWideChar(cp, 0, mbchs, len, (LPWSTR)&uChar, 1);

	DWORD Result = 0;

	//A2 E1 
	if (LOWORD(uChar) == (WORD)0x2468)
	{
		lplf = (LOGFONTW*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(LOGFONTW));

		hOldFont = (HFONT)GetCurrentObject(hdc, OBJ_FONT);
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

		hFont = CreateFontIndirectW(&Info);

		hOldFont = (HFONT)SelectObject(hdc, hFont);
		//6A 26 
		Result = GetGlyphOutlineW(hdc, (UINT)0x266A, uFormat,
			lpgm, cbBuffer, lpvBuffer, lpmat2);

		SelectObject(hdc, hOldFont);
		DeleteObject(hFont);
		HeapFree(GetProcessHeap(), 0, lplf);
	}
	else if (LOWORD(uChar) == (WORD)0x2467)
	{
		lplf = (LOGFONTW*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(LOGFONTW));

		hOldFont = (HFONT)GetCurrentObject(hdc, OBJ_FONT);
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
		Result = GetGlyphOutlineW(hdc, (UINT)0x2200, uFormat,
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

	return Result;
}



NTSTATUS BeginLocalEmulator(ULONG CodePage)
{
	PVoid      hModule;
	NtFileDisk File;
	BYTE       StackMemory[0x100];
	NTSTATUS   Status;
	DWORD      Crc32;

	Nt_LoadLibrary(L"USER32.DLL");
	hModule = Nt_LoadLibrary(L"GDI32.dll");

	LOOP_ONCE
	{
		RtlZeroMemory(StackMemory, sizeof(StackMemory));
		RtlZeroMemory(FontNameLocal, sizeof(FontNameLocal));

		StrCopyW(FontNameLocal, L"黑体");

		Status = File.Open(L"Anz.ini");
		if (NT_FAILED(Status))
			break;

		File.Read(StackMemory, File.GetSize32());
		Crc32 = GetCRC(StackMemory + 4, File.GetSize32() - 4);
		if (Crc32 == *(PDWORD)StackMemory)
		{
			RtlZeroMemory(FontNameLocal, sizeof(FontNameLocal));
			MultiByteToWideChar(CP_UTF8, 0, (PCSTR)StackMemory + 4, File.GetSize32() - 4, FontNameLocal, countof(FontNameLocal));
		}
		else
		{
			RtlZeroMemory(FontNameLocal, sizeof(FontNameLocal));
			StrCopyW(FontNameLocal, L"黑体");
		}
		File.Close();
	}


	Mp::PATCH_MEMORY_DATA f[] =
	{
		// gdi32
		Mp::FunctionJumpVa(Nt_GetProcAddress(hModule, "GdiGetCodePage"), MyGdiGetCodePage, NULL),
		Mp::FunctionJumpVa(CreateFontIndirectA, MyCreateFontIndirectA, NULL),
		Mp::FunctionJumpVa(GetGlyphOutlineA, MyGetGlyphOutlineA, NULL),

		// kernel32
		Mp::FunctionJumpVa(GetACP, MyGetACP, NULL),
		Mp::FunctionJumpVa(GetOEMCP, MyGetOEMCP, NULL),
		Mp::FunctionJumpVa(WideCharToMultiByte, MyWideCharToMultiByte, &OldWideCharToMultiByte),
		Mp::FunctionJumpVa(MultiByteToWideChar, MyMultiByteToWideChar, &OldMultiByteToWideChar),
		Mp::FunctionJumpVa(IsDBCSLeadByte, MyIsDBCSLeadByte, NULL),
		Mp::FunctionJumpVa(IsDBCSLeadByteEx, MyIsDBCSLeadByteEx, &OldIsDBCSLeadByteEx),

		//user32
		//Mp::FunctionJumpVa(DefWindowProcA, MyDefWindowProcA, NULL),
		Mp::FunctionJumpVa(DispatchMessageA, MyDispatchMessageA, NULL),
		Mp::FunctionJumpVa(PeekMessageA, MyPeekMessageA, NULL),
		Mp::FunctionJumpVa(PostMessageA, MyPostMessageA, NULL),
		Mp::FunctionJumpVa(CreateWindowExA, MyCreateWindowExA, NULL),
		Mp::FunctionJumpVa(RegisterClassA, MyRegisterClassA, NULL)

	};

	return NT_SUCCESS(Mp::PatchMemory(f, countof(f)));
}
