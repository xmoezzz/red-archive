#ifndef _LOCALEEMULATOR_H_
#define _LOCALEEMULATOR_H_

#pragma once
#include <Windows.h>
#include <commctrl.h>
#include "NtDefine.h"
#include <malloc.h>

#define AllocStack _alloca

#define LECommonErrorHeader L"NT Layer"

#if defined(MY_COMPILER_INTEL)
#pragma warning(disable:1899)
#else
#pragma warning(disable:627)
#endif

#pragma pack(1)

#define LOCALE_EMULATOR_THREAD_INFO_MAGIC   TAG4('LETM')
#define LOCALE_EMULATOR_BEGIN               -1
#define LOCALE_EMULATOR_END                 -2

typedef BOOL(STDCALL *BeginLocalEmulatorRoutine)(PVOID BaseAddress, ULONG Reason, ULONG CodePage);
typedef BOOL(STDCALL *EndLocalEmulatorRoutine)(PVOID BaseAddress, ULONG Reason, PVOID Context);

struct LOCALE_EMULATOR_THREAD_INFO : public TEB_ACTIVE_FRAME
{
	//    BOOL    bRecursion;
	HHOOK   hHook;
	DLGPROC OldDlgProcA;
};

typedef struct
{
	WNDPROC OldWndProcA;
	//    HFONT   hFont;
} WINDOW_PROP_INFO;

typedef struct
{
	ULONG   CodePage;
	ULONG   FontCharSet;
	LANGID  DefaultSystemLangID;
	LANGID  DefaultUserLangID;
	LCID    DefaultSystemLCID;
	LCID    DefaultUserLCID;
} LOCALE_INFO;

#pragma pack()

BOOL BeginLocalEmulator(ULONG CodePage);
BOOL EndLocalEmulator();
BOOL ReleaseThreadLocalStorage();

#define LOCALE_EMULATOR_PROP_NAME L"AMANO_LOCALE_EMULATOR_PROP_NAME"

FORCEINLINE LONG_PTR FASTCALL StrLengthA(CONST PCHAR pString)
{
	Long ch;
	Long_Ptr SizeOfUnit;
	PCChar pBuffer = pString;

	if (pString == NULL)
		return 0;

	SizeOfUnit = sizeof(Int32);
	while ((Long_Ptr)(pBuffer)& 3)
	{
		if (*pBuffer++ == 0)
		{
			--pBuffer;
			goto end_of_calc;
		}
	}

	while (1)
	{
		Long temp;
		ch = *(PLong)pBuffer;
		pBuffer += SizeOfUnit;

		temp = (0x7EFEFEFF + ch) ^ (ch ^ -1);
		if ((temp & 0x81010100) == 0)
			continue;

		if (LoByte(ch) == 0)
		{
			pBuffer -= SizeOfUnit;
			break;
		}

		if ((ch & 0xFF00) == 0)
		{
			pBuffer -= SizeOfUnit - 1;
			break;
		}

		if ((ch & 0x00FF0000) == 0)
		{
			pBuffer -= SizeOfUnit - 2;
			break;
		}

		if ((ch & 0xFF000000) == 0)
		{
			pBuffer -= SizeOfUnit - 3;
			break;
		}
	}

	end_of_calc:
	return pBuffer - pString;
}


FORCEINLINE LONG_PTR FASTCALL StrLengthW(PCWChar pString)
{
	Long ch;
	PCWChar pBuffer;

	if (pString == NULL)
		return 0;

	pBuffer = pString;
	while ((Int_Ptr)pBuffer & 3)
	{
		if (*pBuffer++ == 0)
		{
			--pBuffer;
			goto end_of_strlen;
		}
	}

	while (1)
	{
		ch = *(PInt)pBuffer;
		if ((ch & 0xFFFF) == 0)
		{
			break;
		}
		else if ((ch & 0xFFFF0000) == 0)
		{
			pBuffer = (PCWChar)((PByte)pBuffer + sizeof(*pBuffer));
			break;
		}

		pBuffer = (PCWChar)((PByte)pBuffer + sizeof(ch));
	}

	end_of_strlen:

	return pBuffer - pString;
}

#define WCharToMByteStack(lpUnicode, Length, lpAnsiBuffer, pLength) \
    if ((ULONG_PTR)(lpUnicode) & 0xFFFF0000) \
	    { \
        DWORD __Length; \
        LPWSTR _pUnicode = (LPWSTR)(lpUnicode); \
        __Length = StrLengthW(_pUnicode) + 1; \
        __Length *= sizeof(WCHAR); \
        *(LPSTR *)&(lpAnsiBuffer) = (LPSTR)AllocStack(__Length); \
        __Length = WCharToMByte(_pUnicode, __Length / sizeof(WCHAR) - 1, (LPSTR)(lpAnsiBuffer), __Length); \
        if (pLength) \
            *(PDWORD)pLength = __Length; \
	    } \
	    else \
    { \
        *(ULONG_PTR *)&(lpAnsiBuffer) = (ULONG_PTR)(lpUnicode); \
    }

#define MByteToWCharStack(lpAnsi, Length, lpUnicodeBuffer, pLength) \
    if ((ULONG_PTR)(lpAnsi) & 0xFFFF0000) \
	    { \
        DWORD __Length; \
        LPSTR _pAnsi = (LPSTR)(lpAnsi); \
        __Length = StrLengthA(_pAnsi) + 1; \
        *(LPWSTR *)&(lpUnicodeBuffer) = (LPWSTR)AllocStack(__Length * sizeof(WCHAR)); \
        MByteToWChar(_pAnsi, __Length - 1, (LPWSTR)(lpUnicodeBuffer), __Length); \
        if (pLength) \
            *(PDWORD)pLength = __Length; \
	    } \
	    else \
    { \
        *(ULONG_PTR *)&(lpUnicodeBuffer) = (ULONG_PTR)(lpAnsi); \
    }


#define WriteLog()
#ifndef WriteLog
#define WriteLog() \
        { \
        FILE *fp; \
        WCHAR path[MAX_PATH]; \
        GetExecuteDirectoryW(path, countof(path)); \
        lstrcatW(path, L"log.txt"); \
        fp = _wfopen(path, L"ab"); \
        if (fp) \
		            { \
            SYSTEMTIME st; \
            GetClassNameW(hWnd, path, countof(path)); \
            GetLocalTime(&st); \
            fprintf(fp, \
            "%02d:%02d:%02d %-31s: MSG=%04X, WPARAM=%08X, LPARAM=%08X, HWND=%08X, %S\r\n", \
            st.wHour, st.wMinute, st.wSecond, \
            __FUNCTION__, Message, wParam, lParam, hWnd, path); \
            fclose(fp); \
		            } \
        }

#endif

#endif // _LOCALEEMULATOR_H_
