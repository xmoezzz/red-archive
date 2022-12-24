#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "undoc_k32.lib")

#include "LocaleEmulator.h"
#include "MyHook.h"


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
		return 936;

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
                GetEmuCodePage(CodePage),
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
                GetEmuCodePage(CodePage),
                dwFlags,
                lpMultiByteStr,
                cbMultiByte,
                lpWideCharStr,
                cchWideChar);
}

API_POINTER(IsDBCSLeadByteEx) OldIsDBCSLeadByteEx = NULL;

BOOL WINAPI MyIsDBCSLeadByteEx(UINT CodePage, BYTE TestChar)
{
	return OldIsDBCSLeadByteEx(GetEmuCodePage(CodePage), TestChar);
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
	StrCopyW(lf.lfFaceName, L"ºÚÌå");

    return CreateFontIndirectW(&lf);
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

	if (OldIsDBCSLeadByteEx(936, uChar >> 8))
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
	OldMultiByteToWideChar(936, 0, mbchs, len, (LPWSTR)&uChar, 1);

	return GetGlyphOutlineW(hdc, uChar, uFormat,
			lpgm, cbBuffer, lpvBuffer, lpmat2);
}



NTSTATUS BeginLocalEmulator(ULONG CodePage)
{
    PVoid   hModule;

	Nt_LoadLibrary(L"KERNEL32.DLL");
	hModule = (HMODULE)Nt_LoadLibrary(L"GDI32.dll");

     INLINE_PATCH_DATA f[] =
    {
        // gdi32
		{ Nt_GetProcAddress(hModule, "GdiGetCodePage"), MyGdiGetCodePage,      NULL },
		{ CreateFontIndirectA,                          MyCreateFontIndirectA, NULL },
		{ GetGlyphOutlineA,                             MyGetGlyphOutlineA,    NULL },

        // kernel32
		{ GetACP,              MyGetACP,              NULL },
		{ GetOEMCP,            MyGetOEMCP,            NULL },
		{ WideCharToMultiByte, MyWideCharToMultiByte, (PVOID*)&OldWideCharToMultiByte },
		{ MultiByteToWideChar, MyMultiByteToWideChar, (PVOID*)&OldMultiByteToWideChar },
		{ IsDBCSLeadByte,      MyIsDBCSLeadByte,      NULL },
		{ IsDBCSLeadByteEx,    MyIsDBCSLeadByteEx,    (PVOID*)&OldIsDBCSLeadByteEx    }
    };

	return NT_SUCCESS(InlinePatchMemory(f, countof(f)));
}
