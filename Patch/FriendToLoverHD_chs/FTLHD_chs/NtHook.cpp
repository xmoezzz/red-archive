#include "NtHook.h"
#include <Windows.h>
#include <crtdefs.h>
#include <assert.h>
#include <Shlwapi.h>
#include "NtDefine.h"
#include "Hook.h"
#include "LocaleEmulatorUser.h"

static LPCSTR szNtleaWndAscData = "NtleaWndAscData"; // CreateGlobalAtom -> ebx
static LPCSTR szNtleaDlgAscData = "NtleaDlgAscData";



CHAR const* SystemClassNameA[] =
{
	"BUTTON",
	"COMBOBOX",
	"ComboLBox",
	"EDIT",
	"LISTBOX",
	"MDICLIENT",
	"RichEdit",
	"RICHEDIT_CLASS",
	"SCROLLBAR",
	"STATIC",
	"SysTreeView32",
	"SysListView32",
	"SysAnimate32",
	"SysHeader32",
	"tooltips_class32"
};

WCHAR const* SystemClassNameW[] =
{
	L"BUTTON",
	L"COMBOBOX",
	L"ComboLBox",
	L"EDIT",
	L"LISTBOX",
	L"MDICLIENT",
	L"RichEdit",
	L"RICHEDIT_CLASS",
	L"SCROLLBAR",
	L"STATIC",
	L"SysTreeView32",
	L"SysListView32",
	L"SysAnimate32",
	L"SysHeader32",
	L"tooltips_class32"
};


static struct
{
	API_POINTER(GetTimeZoneInformation) lpGetTimeZoneInformation;
	API_POINTER(CallWindowProcA)        lpCallWindowProcAddress;
	API_POINTER(SetWindowLongPtrW)      lpSetWindowLongAddress;
	//check bug?
	API_POINTER(GetWindowLongPtrA)      lpGetWindowLongPtrAddress;
	API_POINTER(VerQueryValueA)         lpVerQueryValueAddress;
} Addresses = { 0 };


Settings settings = { 0 };

FORCEINLINE LPVOID AllocateZeroedMemory(SIZE_T size/*eax*/)
{
	if (!size) size = 10;
	return HeapAlloc(settings.hHeap, HEAP_ZERO_MEMORY, size);
}


FORCEINLINE LPVOID AllocateHeapInternal(SIZE_T size)
{
	assert(size);
	return HeapAlloc(settings.hHeap, 0, size);
}

FORCEINLINE NTLEA_TLS_DATA* GetTlsValueInternal() 
{
	DWORD n = GetLastError();
	NTLEA_TLS_DATA* p = (NTLEA_TLS_DATA*)TlsGetValue(settings.nTlsIndex);
	SetLastError(n);
	if (!p)
	{
		p = (NTLEA_TLS_DATA*)AllocateZeroedMemory(sizeof(NTLEA_TLS_DATA));
		TlsSetValue(settings.nTlsIndex, p);
		for (int i = 0; i < 15; ++i) {
			WNDCLASSA wndclassa;
			if (GetClassInfoA(NULL, SystemClassNameA[i], &wndclassa)) {
				p->SystemClassDesc[i].AnsiSystemClassProc = wndclassa.lpfnWndProc;
			}
			WNDCLASSW wndclassw;
			if (GetClassInfoW(NULL, SystemClassNameW[i], &wndclassw)) {
				p->SystemClassDesc[i].UnicodeSystemClassProc = wndclassw.lpfnWndProc;
			}
		}
		SetLastError(0);
	}
	return p;
}


FORCEINLINE LPCWSTR MultiByteToWideCharInternal(LPCSTR lpString)
{
	INT size = lstrlenA(lpString), n = 0;
	LPWSTR wstr = (LPWSTR)AllocateHeapInternal((size + 1) << 1);
	if (wstr) {
		n = MultiByteToWideChar(CP_ACP, 0, lpString, size, wstr, size);
		wstr[n] = NULL;
	}
	return wstr;
}

FORCEINLINE LPCSTR WideCharToMultiByteInternal(LPCWSTR lpString)
{
	INT size = lstrlenW(lpString), n = 0;
	LPSTR str = (LPSTR)AllocateHeapInternal((size + 1) << 1);
	if (str) {
		n = WideCharToMultiByte(CP_ACP, 0, lpString, size, str, size << 1, NULL, NULL);
		str[n] = NULL;
	}
	return str;
}


VOID FreeStringInternal(LPVOID pBuffer)
{
	HeapFree(settings.hHeap, 0, pBuffer);
}

static VOID ShowUnhandledExceptionMessage(const WCHAR* Msg)
{
	SetUnhandledExceptionFilter(NULL);
	MessageBoxW(GetForegroundWindow(), Msg, L"Nt Layer", MB_ICONHAND);
	ExitProcess(0);
}

static void CreateGlobalAtom()
{
	while (!InterlockedCompareExchange(&settings.bInternalLockCreateAtom, 1, 0))
	{
		Sleep(0);
	}

	if (!settings.bNtleaAtomInvoked)
	{
		if (!GlobalFindAtomA(szNtleaWndAscData))
		{
			GlobalAddAtomA(szNtleaWndAscData);
		}
		++settings.bNtleaAtomInvoked;
		if (!GlobalFindAtomA(szNtleaDlgAscData))
		{
			GlobalAddAtomA(szNtleaDlgAscData);
		}
	}

	InterlockedDecrement(&settings.bInternalLockCreateAtom);
}

static int CheckWindowStyle(HWND hWnd, DWORD type)
{
	LONG_PTR n = GetWindowLongPtrW(hWnd, GWL_STYLE);

	if (n == 0)
	{
		return (0);
	}
	else if (n == (WS_POPUP | WS_CLIPSIBLINGS | WS_BORDER | WS_DLGFRAME | WS_SYSMENU |
		WS_EX_RTLREADING | WS_EX_TOOLWINDOW | WS_EX_MDICHILD | WS_EX_TRANSPARENT | WS_EX_NOPARENTNOTIFY)) {
		return (0);
	}
	else if (!(n & (WS_EX_ACCEPTFILES | WS_EX_TRANSPARENT)))
	{
		return (0);
	}
	else if (!type && (n & WS_EX_CLIENTEDGE))
	{
		return (0);
	}
	else if (n & WS_EX_MDICHILD)
	{
		return (0);
	}
	return (-1);
}

FORCEINLINE NTLEA_WND_ASC_DATA* CheckProp(HWND hWnd)
{
	NTLEA_WND_ASC_DATA* p = (NTLEA_WND_ASC_DATA*)GetPropA(hWnd, szNtleaWndAscData);
	if (!p)
		ShowUnhandledExceptionMessage(L"Empty Window Prop");
	return p;
}

typedef LRESULT(CALLBACK *CALLPROC)(WNDPROC, HWND, UINT, WPARAM, LPARAM);
static LRESULT CALLBACK TopLevelWindowProcEx(CALLPROC DefaultCallWindowProc, HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LPCSTR lpAnsiWindowName = NULL, lpAnsiClassName = NULL;
	CHAR CharBuffer[2]; // for unicode conversion -> local 
	CHAR ClassNameBuffer[MAX_PATH];
	int type = 0;

	NTLEA_WND_ASC_DATA* wndasc = CheckProp(hWnd);
	WNDPROC PrevWndProc = wndasc->PrevAnsiWindowProc;
	++GetTlsValueInternal()->InternalCall; // <---


	switch (uMsg)
	{
	case WM_CREATE:
	case WM_NCCREATE:
	{
		if (lParam)
		{
			CREATEWNDEX * p = (CREATEWNDEX *)AllocateHeapInternal(sizeof(CREATEWNDEX));
			memcpy(p, (LPVOID)lParam, sizeof(CREATEWNDEX));
			if (p->lpWindowName)
			{
				p->lpWindowName = (LPVOID)(lpAnsiWindowName = WideCharToMultiByteInternal((LPCWSTR)p->lpWindowName));
			}
			if ((DWORD_PTR)p->lpClassName & WM_CLASSMASK)
			{
				p->lpClassName = (LPVOID)(lpAnsiClassName = WideCharToMultiByteInternal((LPCWSTR)p->lpClassName));
			}
			// LN999
			LRESULT hr = CallWindowProcA(PrevWndProc, hWnd, uMsg, wParam, (LPARAM)p);
			// LN995
			if (lpAnsiWindowName) FreeStringInternal((LPVOID)lpAnsiWindowName);
			if (lpAnsiClassName) FreeStringInternal((LPVOID)lpAnsiClassName);
			if (p/*ebx*/) FreeStringInternal(p);
			return (hr);
		}
	}	break;
	case WM_MDICREATE: // LN106
	{
		if (lParam) {
			CREATEMDIWND* p = (CREATEMDIWND*)AllocateHeapInternal(sizeof(CREATEMDIWND));
			memcpy(p, (LPVOID)lParam, sizeof(CREATEMDIWND));
			if (p->szTitle) {
				p->szTitle = lpAnsiWindowName = WideCharToMultiByteInternal((LPCWSTR)p->szTitle);
			}
			if ((DWORD_PTR)p->szClass & WM_CLASSMASK) {
				p->szClass = lpAnsiClassName = WideCharToMultiByteInternal((LPCWSTR)p->szClass);
			}
			// LN999
			LRESULT hr = CallWindowProcA(PrevWndProc, hWnd, uMsg, wParam, (LPARAM)p);
			// LN995
			if (lpAnsiWindowName) FreeStringInternal((LPVOID)lpAnsiWindowName);
			if (lpAnsiClassName) FreeStringInternal((LPVOID)lpAnsiClassName);
			if (p/*ebx*/) FreeStringInternal(p);
			return (hr);
		}
	}	break;
	case EM_GETLINE: // LN124
	{
		int siz = *(short*)(DWORD_PTR)lParam + 1;
		LPSTR lParamA = (LPSTR)AllocateZeroedMemory(siz * sizeof(wchar_t));
		*(short*)(DWORD_PTR)lParam = (short)(siz - 1);
		int len = (int)CallWindowProcA(PrevWndProc, hWnd, uMsg, wParam, (LPARAM)lParamA);
		if (len) // if success : 
			len = MultiByteToWideChar(settings.dwCodePage, 0, lParamA, len + 1, (LPWSTR)lParam, siz); // --- bugfixed
		if (!len && lParam) *(LPWSTR)lParam = L'\0'; // handle failed case 
		else if (len) --len; // report not-including null-terminate string !
		if (lParamA) FreeStringInternal((LPVOID)lParamA);
		return len;
	}//	break;
	case WM_GETFONT:
	{
		LRESULT hr = CallWindowProcA(PrevWndProc, hWnd, uMsg, wParam, lParam);
		return (hr) ? (hr) : (LRESULT)GetStockObject(SYSTEM_FONT);
	}//	break;
	case EM_REPLACESEL: // LN113
	case WM_SETTEXT: // LN113
	case WM_SETTINGCHANGE: // LN113
	case WM_DEVMODECHANGE: // LN113
	{
		LPCSTR lParamA = lParam ? WideCharToMultiByteInternal((LPCWSTR)lParam) : NULL;
		//	ntprintfA(256, 1, "1. W(%S) -> A(%s)", (LPCWSTR)lParam, lParamA);
		LRESULT hr = CallWindowProcA(PrevWndProc, hWnd, uMsg, wParam, (LPARAM)lParamA); // lParamA null also requied ??? 
		// LN999 LN995
		if (lParamA) FreeStringInternal((LPVOID)lParamA);
		return hr;
	}//	break;
	case WM_GETTEXTLENGTH: // LN127
	{
		LRESULT len = CallWindowProcA(PrevWndProc, hWnd, WM_GETTEXTLENGTH, 0, 0);
		if (len > 0) {
			GetTlsValueInternal()->InternalCall++;
			LPSTR lParamA = (LPSTR)AllocateZeroedMemory((len + 1) * sizeof(wchar_t));
			CallWindowProcA(PrevWndProc, hWnd, WM_GETTEXT, (len + 1) * sizeof(wchar_t), (LPARAM)lParamA);
			len = MultiByteToWideChar(settings.dwCodePage, 0, lParamA, -1, NULL, 0) - 1;
			// LN995
			if (lParamA/*ebx*/) FreeStringInternal((LPVOID)lParamA);
		}
		return len;
	}//	break;
	case WM_GETTEXT: // LN114
	case WM_UNKNOWN: // LN114
	{
		if (IsBadWritePtr((LPVOID)lParam, 1))
		{
			--GetTlsValueInternal()->InternalCall;
			return (0);
		}
		else {
			LRESULT len = CallWindowProcA(PrevWndProc, hWnd, WM_GETTEXTLENGTH, 0, 0);
			if (len == 0)
			{
				*((LPWSTR)lParam) = NULL;
			}
			else
			{ // L116
				GetTlsValueInternal()->InternalCall++;
				LPSTR lParamA = (LPSTR)AllocateZeroedMemory((len + 1) * sizeof(WCHAR));
				CallWindowProcA(PrevWndProc, hWnd, uMsg, (len + 1) * sizeof(WCHAR), (LPARAM)lParamA);
				if (uMsg == WM_UNKNOWN) wParam = len + 1;
				len = MultiByteToWideChar(settings.dwCodePage, 0, lParamA, -1, (LPWSTR)lParam, (int)wParam) - 1;
				if (len > 0) {
					// L997
					if (lParamA) FreeStringInternal((LPVOID)lParamA);
				}
				else {
					// L119
					if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
						*((LPWSTR)lParam + wParam - 1) = L'\0';
					}
					else {
						// L120
						*((LPWSTR)lParam) = L'\0';
					}
					// LN994
					FreeStringInternal((LPVOID)lParamA);
				}
			}
			return len;
		}
	}//	break;
	case WM_IME_CHAR: // LN109
	case WM_CHAR: // LN109
	{
		if ((wchar_t)wParam > 0x7F) {
			WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)&wParam, 1, CharBuffer, 2, NULL, NULL);
			// here we exchange the order right ?
			*((char*)&wParam + 0) = CharBuffer[1];
			*((char*)&wParam + 1) = CharBuffer[0];
			//	wParam = (CharBuffer[1] << 8) | (CharBuffer[0] << 0);
		}
	}	break;
	case WM_NOTIFYFORMAT: // LN121
	{
		--GetTlsValueInternal()->InternalCall; // fix 
		GetClassNameA(hWnd, ClassNameBuffer, MAX_PATH);
		if (lstrcmpiA(ClassNameBuffer, "SysTreeView32") == 0) {
			DWORD_PTR n = GetWindowLongPtrW(hWnd, DWLP_MSGRESULT/*0*/);
			if (n && *(LPBYTE)(n + 0x10) == 1) { // what ??? 
				*(LPBYTE)(n + 0x10) &= -2; // 
			}
		}
		// L122
		return (lParam == NF_QUERY) ? NFR_ANSI : (0);
	}//	break;
	case WM_NCDESTROY: // check steps move out !!
	{
		LRESULT hr = CallWindowProcA(PrevWndProc, hWnd, uMsg, wParam, lParam);
		FreeStringInternal((LPVOID)GetPropA(hWnd, szNtleaWndAscData));
		RemovePropA(hWnd, szNtleaWndAscData);
		return hr;
	}//	break;
	// -------------------------------- 
	case LB_GETTEXTLEN: // LN131
	{
		int ret = CheckWindowStyle(hWnd, 1); // 1, inc ebx 
		if (ret != -1) {
			LRESULT len = CallWindowProcA(PrevWndProc, hWnd, LB_GETTEXTLEN, wParam, 0);
			if (len > 0) {
				GetTlsValueInternal()->InternalCall++;
				LPSTR lParamA = (LPSTR)AllocateZeroedMemory((len + 1) * sizeof(WCHAR));
				CallWindowProcA(PrevWndProc, hWnd, LB_GETTEXT, wParam, (LPARAM)lParamA);
				len = MultiByteToWideChar(settings.dwCodePage, 0, lParamA, -1, NULL, 0) - 1;
				// LN995
				if (lParamA/*ebx*/) FreeStringInternal((LPVOID)lParamA);
			}
			return len;
		}
	}	break;
	case LB_GETTEXT: // LN110
	{
		type = 1;
	}//	break;
	case CB_GETLBTEXT: // LN129
	{
		int ret = CheckWindowStyle(hWnd, type); // 0
		if (ret != -1) {
			LPSTR lParamA = (LPSTR)AllocateZeroedMemory(MAX_PATH * sizeof(WCHAR));
			int len = (int)CallWindowProcA(PrevWndProc, hWnd, uMsg, wParam, (LPARAM)lParamA);
			if (len) // if success : 
				len = MultiByteToWideChar(settings.dwCodePage, 0, lParamA, len + 1, (LPWSTR)lParam, MAX_PATH); // --- bugfixed
			if (!len && lParam) *(LPWSTR)lParam = L'\0'; // handle failed case !
			else if (len) --len; // report not-including null-terminate string !
			if (lParamA/*ebx*/) FreeStringInternal((LPVOID)lParamA);
			return len;
		}
	}	break;
	case LB_FINDSTRINGEXACT: // LN104
	case LB_ADDSTRING: // LN104
	case LB_INSERTSTRING: // L104
	case LB_SELECTSTRING: // L104
	case LB_DIR: // L104
	case LB_FINDSTRING: // L104
	case LB_ADDFILE: // L104
		type = 1; // ebx = 1
		//	break;
	case CB_FINDSTRINGEXACT: // LN105
	case CB_ADDSTRING: // LN105
	case CB_INSERTSTRING: // LN105
	case CB_SELECTSTRING: // LN105
	case CB_DIR: // LN105
	case CB_FINDSTRING: // LN105
	{
		int ret = CheckWindowStyle(hWnd, type); // ebx = 0 / 1
		if (ret != -1) { // send op
			// LN113
			LPCSTR lParamA = lParam ? WideCharToMultiByteInternal((LPCWSTR)lParam) : NULL;
			// LN999
			LRESULT hr = CallWindowProcA(PrevWndProc, hWnd, uMsg, wParam, (LPARAM)lParamA);
			// LN995
			if (lParamA/*ebx*/) FreeStringInternal((LPVOID)lParamA);
			return hr;
		}
		else { // recv op 
			// LN110
			ret = CheckWindowStyle(hWnd, type + 1); // ebx = 1 / 2
			if (ret != -1) {
				// <-- ebx 
				LPSTR lParamA = (LPSTR)AllocateZeroedMemory(MAX_PATH * sizeof(WCHAR));
				// LN999
				int len = (int)CallWindowProcA(PrevWndProc, hWnd, uMsg, wParam, (LPARAM)lParamA);
				if (len) // if success : 
					len = MultiByteToWideChar(settings.dwCodePage, 0, lParamA, len + 1, (LPWSTR)lParam, MAX_PATH); // --- bugfixed
				if (!len && lParam) *(LPWSTR)lParam = L'\0'; // L111
				else if (len) --len; // report not-including null-terminate string !
				// LN995
				if (lParamA) FreeStringInternal((LPVOID)lParamA);
				return len;
			}
		}
	}	break;
	default: // LN100	
		break;
	}
	// --------- 
	return DefaultCallWindowProc(PrevWndProc, hWnd, uMsg, wParam, lParam);
}


static LRESULT CALLBACK TopLevelWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// for normal custom window case : other messages also needs handled by named-controls (children) 
	return TopLevelWindowProcEx(CallWindowProcA, hWnd, uMsg, wParam, lParam);
}


static LRESULT CALLBACK TopLevelSimpleProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return TopLevelWindowProcEx(Addresses.lpCallWindowProcAddress, hWnd, uMsg, wParam, lParam);
}

static INT_PTR CALLBACK TopLevelDialogProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	NTLEA_TLS_DATA* p = GetTlsValueInternal();
	if (p->DialogProc)
	{
		CreateGlobalAtom();
		if (!GetPropA(hWnd, szNtleaWndAscData))
		{
			NTLEA_WND_ASC_DATA* wndasc = (NTLEA_WND_ASC_DATA*)AllocateZeroedMemory(sizeof(NTLEA_WND_ASC_DATA));
			wndasc->PrevAnsiWindowProc = (WNDPROC)p->DialogProc;
			SetPropA(hWnd, szNtleaWndAscData, (HANDLE)wndasc);
			p->DialogProc = NULL;
		}
	}
	return TopLevelWindowProc(hWnd, uMsg, wParam, lParam);
}

FORCEINLINE LRESULT CallWindowSendMessage(
	LPVOID lpProcAddress,
	HWND hWnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam,
	DWORD_PTR Param1,
	DWORD_PTR Param2,
	DWORD_PTR Param3,
	int FunctionType)
{
	switch (FunctionType) {
	case 0:
	default:
		return ((LRESULT(WINAPI*)(HWND, UINT, WPARAM, LPARAM))(DWORD_PTR)lpProcAddress)
			(hWnd, uMsg, wParam, lParam);
	case 1:
		return ((LRESULT(WINAPI*)(HWND, UINT, WPARAM, LPARAM, DWORD_PTR, DWORD_PTR))(DWORD_PTR)lpProcAddress)
			(hWnd, uMsg, wParam, lParam, Param1, Param2);
	case 2:
		return ((LRESULT(WINAPI*)(HWND, UINT, WPARAM, LPARAM, DWORD_PTR, DWORD_PTR, DWORD_PTR))(DWORD_PTR)lpProcAddress)
			(hWnd, uMsg, wParam, lParam, Param1, Param2, Param3);
	case 3:
		return ((LRESULT(WINAPI*)(DWORD_PTR, HWND, UINT, WPARAM, LPARAM))(DWORD_PTR)lpProcAddress)
			(Param1, hWnd, uMsg, wParam, lParam);
	}
}


static LRESULT SendUnicodeMessage(LPVOID lpProcAddress, HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
	DWORD_PTR Param1/*ecx*/, DWORD_PTR Param2/*ecx*/, DWORD_PTR Param3/*ecx*/, int FunctionType)
{
	LPCWSTR lpUnicodeWindowName = NULL, lpUnicodeClassName = NULL;
	WCHAR CharBuffer[2];
	int type = 0;


	switch (uMsg) {
	case WM_CREATE: // L304
	case WM_NCCREATE:
	{
		if (lParam) {
			CREATEWNDEX * p = (CREATEWNDEX *)AllocateHeapInternal(sizeof(CREATEWNDEX));
			memcpy(p, (LPVOID)lParam, sizeof(CREATEWNDEX));
			if (p->lpWindowName) {
				p->lpWindowName = (LPVOID)(lpUnicodeWindowName = MultiByteToWideCharInternal((LPCSTR)p->lpWindowName));
			}
			if ((DWORD_PTR)p->lpClassName & WM_CLASSMASK) {
				p->lpClassName = (LPVOID)(lpUnicodeClassName = MultiByteToWideCharInternal((LPCSTR)p->lpClassName));
			}
			// LN310
			LRESULT hr = CallWindowSendMessage(lpProcAddress, hWnd, uMsg, wParam, (LPARAM)p, Param1, Param2, Param3, FunctionType);
			// LN793
			if (lpUnicodeWindowName) FreeStringInternal((LPVOID)lpUnicodeWindowName);
			if (lpUnicodeClassName) FreeStringInternal((LPVOID)lpUnicodeClassName);
			if (p/*ebx*/) FreeStringInternal(p);
			return hr;
		}
	}	break;
	case WM_MDICREATE: // LN307
	{
		if (lParam) {
			CREATEMDIWND* p = (CREATEMDIWND*)AllocateHeapInternal(sizeof(CREATEMDIWND));
			memcpy(p, (LPVOID)lParam, sizeof(CREATEMDIWND));
			if (p->szTitle) p->szTitle = (LPCSTR)(lpUnicodeClassName = MultiByteToWideCharInternal(p->szTitle));
			if ((DWORD_PTR)p->szClass & WM_CLASSMASK) {
				p->szClass = (LPCSTR)(lpUnicodeClassName = MultiByteToWideCharInternal(p->szClass));
			}
			// LN310
			LRESULT hr = CallWindowSendMessage(lpProcAddress, hWnd, uMsg, wParam, (LPARAM)p, Param1, Param2, Param3, FunctionType);
			// LN793
			if (lpUnicodeWindowName) FreeStringInternal((LPVOID)lpUnicodeWindowName);
			if (lpUnicodeClassName) FreeStringInternal((LPVOID)lpUnicodeClassName);
			if (p/*ebx*/) FreeStringInternal(p);
			return (hr);
		}
	}
	break;
	case WM_IME_CHAR: // LN309
	case WM_CHAR: // LN309
	{
		if ((WCHAR)wParam > 0x7F)
		{
			wParam = ((wParam & 0xFF) << 8) | ((wParam & 0xFF00) >> 8);
			MultiByteToWideChar(CP_ACP, 0, (LPCSTR)&wParam, -1, CharBuffer, 2);
			wParam = CharBuffer[0];
		}
	}
	break;

	case EM_GETLINE: // LN321
	{
		if (!IsBadWritePtr((LPVOID)lParam, 1))
		{
			int siz = *(short*)(DWORD_PTR)lParam + 1;
			LPWSTR lParamW = (LPWSTR)AllocateZeroedMemory(siz * sizeof(WCHAR));
			*(SHORT*)(DWORD_PTR)lParam = (short)(siz - 1);

			int len = (int)CallWindowSendMessage(lpProcAddress, hWnd, uMsg, wParam, (LPARAM)lParamW, Param1, Param2, Param3, FunctionType);

			if (len)
				len = WideCharToMultiByte(CP_ACP, 0, lParamW, len + 1, (LPSTR)lParam, siz, NULL, NULL); // --- bugfixed

			if (!len && lParam)
				*(LPSTR)lParam = NULL;
			else if
				(len) --len;

			if (lParamW)
				FreeStringInternal(lParamW);

			return len;
		}
	}	break;
	case EM_REPLACESEL: // LN320
	case WM_SETTEXT: // LN320
	case WM_SETTINGCHANGE: // LN320
	case WM_DEVMODECHANGE: // LN320
	{
		LPCWSTR lParamW = lParam ? MultiByteToWideCharInternal((LPCSTR)lParam) : NULL;

		LRESULT hr = CallWindowSendMessage(lpProcAddress, hWnd, uMsg, wParam, (LPARAM)lParamW, Param1, Param2, Param3, FunctionType);
		// LN301
		if (lParamW) FreeStringInternal((LPVOID)lParamW);
		return hr;
	}
	break;

	case WM_GETTEXTLENGTH: // LN327
	{
		LRESULT len = CallWindowSendMessage(lpProcAddress, hWnd, WM_GETTEXTLENGTH, 0, 0, Param1, Param2, Param3, FunctionType);
		if (len > 0) {
			LPWSTR lParamW = (LPWSTR)AllocateZeroedMemory((len + 1) * sizeof(WCHAR));
			CallWindowSendMessage(lpProcAddress, hWnd, WM_GETTEXT, (len + 1) * sizeof(wchar_t), (LPARAM)lParamW,
				Param1, Param2, Param3, FunctionType);
			len = WideCharToMultiByte(CP_ACP, 0, lParamW, -1, NULL, 0, NULL, NULL) - 1; // required
			// LN793
			if (lParamW) FreeStringInternal(lParamW);
		}
		return len;
	}
	break;

	case WM_GETTEXT: // LN310
	case WM_UNKNOWN: // LN310
	{
		if (IsBadWritePtr((LPVOID)lParam, 1))
		{
			return (0);
		}
		else
		{
			int len = (int)CallWindowSendMessage(lpProcAddress, hWnd, WM_GETTEXTLENGTH, 0, 0, Param1, Param2, Param3, FunctionType);
			// no needs check len == 0 ?? 
			LPWSTR lParamW = (LPWSTR)AllocateZeroedMemory((len + 1) * sizeof(wchar_t));
			len = (int)CallWindowSendMessage(lpProcAddress, hWnd, uMsg, wParam, (LPARAM)lParamW, Param1, Param2, Param3, FunctionType);
			if (uMsg != WM_UNKNOWN) len = (int)wParam; // wParam should be len + 1 !! 
			len = WideCharToMultiByte(CP_ACP, 0, lParamW, -1, (LPSTR)lParam, len, NULL, NULL) - 1;
			if (len > 0)
			{
				if (lParamW)
					FreeStringInternal(lParamW);
			}
			else
			{
				if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
				{
					*((LPSTR)lParam + wParam - 1) = NULL;
				}
				else
				{
					*((LPSTR)lParam) = NULL;
				}
			}
			return len;
		}
	}	break;
	// ------------------------------------------- 
	case LB_GETTEXTLEN: // LN329
	{
		int ret = CheckWindowStyle(hWnd, 1);
		if (ret != -1)
		{
			LRESULT len = CallWindowSendMessage(lpProcAddress, hWnd, LB_GETTEXTLEN, wParam, 0, Param1, Param2, Param3, FunctionType);
			if (len > 0)
			{
				LPWSTR lParamW = (LPWSTR)AllocateZeroedMemory((len + 1) * sizeof(wchar_t));
				CallWindowSendMessage(lpProcAddress, hWnd, LB_GETTEXT, wParam, (LPARAM)lParamW, Param1, Param2, Param3, FunctionType);
				len = WideCharToMultiByte(CP_ACP, 0, lParamW, -1, NULL, 0, NULL, NULL) - 1;
				// LN793
				if (lParamW/*ebx*/) FreeStringInternal(lParamW);
			}
			return len;
		}
	}	break;
	case LB_GETTEXT: // LN322
		type = 1;

	case CB_GETLBTEXT: // LN323
	{
		int ret = CheckWindowStyle(hWnd, type); // 0
		if (ret != -1) {
			LPWSTR lParamW = (LPWSTR)AllocateZeroedMemory(MAX_PATH * sizeof(wchar_t));
			int len = (int)CallWindowSendMessage(lpProcAddress, hWnd, uMsg, wParam, (LPARAM)lParamW, Param1, Param2, Param3, FunctionType);
			if (len) // if success, do conversion : 
				len = WideCharToMultiByte(CP_ACP, 0, lParamW, len + 1, (LPSTR)lParam, MAX_PATH, NULL, NULL); // --- bugfixed
			if (!len && lParam) *(LPSTR)lParam = '\0'; // handle failed case !
			else if (len) --len; // report not-including null-terminate string !
			if (lParamW/*ebx*/) FreeStringInternal(lParamW);
			return len;
		}
	}
	break;

	case LB_FINDSTRINGEXACT: // LN305
	case LB_ADDSTRING: // LN305
	case LB_INSERTSTRING: // LN305
	case LB_FINDSTRING: // LN305
	case LB_ADDFILE: // LN305
	case LB_SELECTSTRING: // LN305
	case LB_DIR: // LN305
		type = 1;

	case CB_FINDSTRINGEXACT: // LN306
	case CB_ADDSTRING: // LN306
	case CB_INSERTSTRING: // LN306
	case CB_SELECTSTRING: // LN306
	case CB_DIR: // LN306
	case CB_FINDSTRING: // LN306
	{
		int ret = CheckWindowStyle(hWnd, type); // ebx = 0 / 1
		if (ret != -1) {
			LPCWSTR lParamW = lParam ? MultiByteToWideCharInternal((LPCSTR)lParam) : NULL;
			// LN899
			LRESULT hr = CallWindowSendMessage(lpProcAddress, hWnd, uMsg, wParam, (LPARAM)lParamW, Param1, Param2, Param3, FunctionType);
			if (lParamW/*ebx*/) FreeStringInternal((LPVOID)lParamW);
			return hr;
		}
	}	break;
	// ----------- common controls ---------------
	case TCM_GETITEMA:
	{
		if (!settings.bNoFilterCommCtrl && lParam) {
			LPTCITEM pitem = (LPTCITEM)lParam;
			if (pitem->mask & TCIF_TEXT) {
				LRESULT hr = CallWindowSendMessage(lpProcAddress, hWnd, uMsg + TCM_ADD_UNICODE, wParam, lParam,
					Param1, Param2, Param3, FunctionType);

				LPCSTR pnewText = WideCharToMultiByteInternal((LPCWSTR)pitem->pszText);
				lstrcpyA((LPSTR)pitem->pszText, pnewText); FreeStringInternal((LPVOID)pnewText);
				return hr;
			}
		}
	}	break;
	case TCM_SETITEMA:
	case TCM_INSERTITEMA:
	{
		if (!settings.bNoFilterCommCtrl && lParam) {
			LPTCITEM pitem = (LPTCITEM)lParam;
			if (pitem->mask & TCIF_TEXT) {
				LPVOID poldText = pitem->pszText;
				pitem->pszText = (LPVOID)MultiByteToWideCharInternal((LPCSTR)poldText);
				LRESULT hr = CallWindowSendMessage(lpProcAddress, hWnd, uMsg + TCM_ADD_UNICODE, wParam, lParam,
					Param1, Param2, Param3, FunctionType);
				FreeStringInternal(pitem->pszText); pitem->pszText = poldText;
				return hr;
			}
		}
	}	break;
	// ----------- common controls end ---------------
	default: // LN301
		break;
	}
	// --------- 
	return CallWindowSendMessage(lpProcAddress, hWnd, uMsg, wParam, lParam, Param1, Param2, Param3, FunctionType);
}

FORCEINLINE LRESULT CallProcAddress(LPVOID lpProcAddress, HWND hWnd, HWND hMDIClient,
	BOOL bMDIClientEnabled, INT uMsg, WPARAM wParam, LPARAM lParam)
{
	typedef LRESULT(WINAPI*fnWNDProcAddress)(HWND, int, WPARAM, LPARAM);
	typedef LRESULT(WINAPI*fnMDIProcAddress)(HWND, HWND, int, WPARAM, LPARAM);

	return (bMDIClientEnabled) ? ((fnMDIProcAddress)(DWORD_PTR)lpProcAddress)(hWnd, hMDIClient, uMsg, wParam, lParam)
		: ((fnWNDProcAddress)(DWORD_PTR)lpProcAddress)(hWnd, uMsg, wParam, lParam);
}
static LRESULT CALLBACK DefConversionProc(LPVOID lpProcAddress, HWND hWnd, HWND hMDIClient,
	BOOL bMDIClientEnabled, INT uMsg, WPARAM wParam, LPARAM lParam)
{
	LPCWSTR lpUnicodeWindowName = NULL, lpUnicodeClassName = NULL;
	WCHAR CharBuffer[2];
	int type = 0;

	if (hWnd && !IsWindowUnicode(hWnd))
		ShowUnhandledExceptionMessage(L"Failed to convert window");

	switch (uMsg) {
	case WM_CREATE: // L204
	case WM_NCCREATE: // 
	{
		if (lParam && GetTlsValueInternal()->CurrentCallType != CT_CREATE_PRESET)
		{
			CREATEWNDEX * p = (CREATEWNDEX *)AllocateHeapInternal(sizeof(CREATEWNDEX));
			memcpy(p, (LPVOID)lParam, sizeof(CREATEWNDEX));
			if (p->lpWindowName) {
				p->lpWindowName = (LPVOID)(lpUnicodeWindowName = MultiByteToWideCharInternal((LPCSTR)p->lpWindowName));
			}
			if ((DWORD_PTR)p->lpClassName & WM_CLASSMASK)
			{
				p->lpClassName = (LPVOID)(lpUnicodeClassName = MultiByteToWideCharInternal((LPCSTR)p->lpClassName));
			}
			// LN899
			LRESULT hr = CallProcAddress(lpProcAddress, hWnd, hMDIClient, bMDIClientEnabled, uMsg, wParam, (LPARAM)p);
			// LN893 
			if (lpUnicodeWindowName) FreeStringInternal((LPVOID)lpUnicodeWindowName);
			if (lpUnicodeClassName) FreeStringInternal((LPVOID)lpUnicodeClassName);
			if (p/*ebx*/) FreeStringInternal((LPVOID)p);
			return (hr);
		}
	}
	break;

	case WM_MDICREATE:
	{
		if (lParam) {
			CREATEMDIWND* p = (CREATEMDIWND*)AllocateHeapInternal(sizeof(CREATEMDIWND));
			memcpy(p, (LPVOID)lParam, sizeof(CREATEMDIWND));
			if (p->szTitle) p->szTitle = (LPCSTR)(lpUnicodeWindowName = MultiByteToWideCharInternal(p->szTitle));
			if ((DWORD_PTR)p->szClass & WM_CLASSMASK)
			{
				p->szClass = (LPCSTR)(lpUnicodeClassName = MultiByteToWideCharInternal(p->szClass));
			}
			// LN899
			LRESULT hr = CallProcAddress(lpProcAddress, hWnd, hMDIClient, bMDIClientEnabled, uMsg, wParam, (LPARAM)p);
			// LN893 
			if (lpUnicodeWindowName) FreeStringInternal((LPVOID)lpUnicodeWindowName);
			if (lpUnicodeClassName) FreeStringInternal((LPVOID)lpUnicodeClassName);
			if (p/*ebx*/) FreeStringInternal((LPVOID)p);
			return (hr);
		}
	}
	break;

	case WM_IME_CHAR: // LN209
	case WM_CHAR: // LN209
	{
		if ((WCHAR)wParam > 0x7F)
		{
			MultiByteToWideChar(settings.dwCodePage, 0, (LPCSTR)&wParam, -1, CharBuffer, 2);
			*((PCHAR)&wParam + 0) = (CHAR)CharBuffer[1];
			*((PCHAR)&wParam + 1) = (CHAR)CharBuffer[0];
		}
	}
	break;

	case EM_REPLACESEL: // LN220
	case WM_SETTEXT: // LN220
	case WM_SETTINGCHANGE: // LN220
	case WM_DEVMODECHANGE: // LN220
	{
		LPCWSTR lParamW = lParam ? MultiByteToWideCharInternal((LPCSTR)lParam) : NULL;
		LRESULT hr = CallProcAddress(lpProcAddress, hWnd, hMDIClient, bMDIClientEnabled, uMsg, wParam, (LPARAM)lParamW);
		// LN893
		if (lParamW) FreeStringInternal((LPVOID)lParamW);
		return hr;
	}// break;
	case WM_GETTEXTLENGTH: // LN223
	{
		LRESULT len = CallProcAddress(lpProcAddress, hWnd, hMDIClient, bMDIClientEnabled, WM_GETTEXTLENGTH, 0, 0);
		if (len > 0)
		{
			LPWSTR lParamW = (LPWSTR)AllocateZeroedMemory((len + 1) * sizeof(WCHAR));
			CallProcAddress(lpProcAddress, hWnd, hMDIClient, bMDIClientEnabled, WM_GETTEXT, (len + 1) * sizeof(wchar_t), (LPARAM)lParamW);
			len = WideCharToMultiByte(CP_ACP, 0, lParamW, -1, NULL, 0, NULL, NULL) - 1; // required

			if (lParamW)
				FreeStringInternal((LPVOID)lParamW);
		}
		return len;
	}

	case WM_GETTEXT: // LN210
	case WM_UNKNOWN: // LN210
	{
		if (IsBadWritePtr((LPVOID)lParam, 1)) {
			return (0);
		}
		else
		{
			int len = (int)CallProcAddress(lpProcAddress, hWnd, hMDIClient, bMDIClientEnabled, WM_GETTEXTLENGTH, 0, 0);
			// no needs check len == 0 ?? 
			LPSTR lParamW = (LPSTR)AllocateZeroedMemory((len + 1) * sizeof(wchar_t));
			CallProcAddress(lpProcAddress, hWnd, hMDIClient, bMDIClientEnabled, uMsg, wParam, (LPARAM)lParamW);
			if (uMsg != WM_UNKNOWN) len = (int)wParam; // wParam should be len + 1 !! 
			len = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)lParamW, -1, (LPSTR)lParam, len, NULL, NULL) - 1; // required
			if (len > 0)
			{
				if (lParamW) FreeStringInternal((LPVOID)lParamW);
			}
			else
			{
				if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
				{
					*((LPSTR)lParam + wParam - 1) = '\0';
				}
				else
				{
					*((LPSTR)lParam) = NULL;
				}
			}
			return len;
		}
	}//	break;

	case LB_GETTEXTLEN: // LN226
	{
		int ret = CheckWindowStyle(hWnd, 1);
		if (ret != -1) {
			int len = (int)CallProcAddress(lpProcAddress, hWnd, hMDIClient, bMDIClientEnabled, LB_GETTEXTLEN, wParam, 0);
			if (len > 0) {
				LPWSTR lParamW = (LPWSTR)AllocateZeroedMemory((len + 1) * sizeof(wchar_t));
				CallProcAddress(lpProcAddress, hWnd, hMDIClient, bMDIClientEnabled, LB_GETTEXT, wParam, (LPARAM)lParamW);
				len = WideCharToMultiByte(CP_ACP, 0, lParamW, -1, NULL, 0, NULL, NULL) - 1;
				// LN893
				if (lParamW/*ebx*/) FreeStringInternal((LPVOID)lParamW);
			}
			return len;
		}
	}	break;
	case LB_GETTEXT: // LN221
		type = 1;
		//	break;
	case CB_GETLBTEXT: // LN222
	{
		int ret = CheckWindowStyle(hWnd, type); // 0
		if (ret != -1) {
			LPWSTR lParamW = (LPWSTR)AllocateZeroedMemory(MAX_PATH * sizeof(WCHAR));
			int len = (int)CallProcAddress(lpProcAddress, hWnd, hMDIClient, bMDIClientEnabled, uMsg, wParam, (LPARAM)lParamW);
			if (!len || !WideCharToMultiByte(CP_ACP, 0, lParamW, len + 1, (LPSTR)lParam, MAX_PATH * sizeof(wchar_t), NULL, NULL)) {
				if (lParam) *(LPSTR)lParam = NULL;
			}
			// LN893
			if (lParamW/*ebx*/) FreeStringInternal((LPVOID)lParamW);
			return len;
		}
	}	break;
	case LB_FINDSTRINGEXACT: // LN205
	case LB_ADDSTRING: // LN205
	case LB_INSERTSTRING: // L205
	case LB_FINDSTRING: // L205
	case LB_ADDFILE: // L205
	case LB_SELECTSTRING: // L205
	case LB_DIR: // L205
		type = 1; // ebx = 1
		//	break;
	case CB_FINDSTRINGEXACT: // LN206
	case CB_ADDSTRING: // LN206
	case CB_INSERTSTRING: // LN206
	case CB_SELECTSTRING: // LN206
	case CB_DIR: // LN206
	case CB_FINDSTRING: // LN206
	{
		int ret = CheckWindowStyle(hWnd, type); // ebx = 0 / 1
		if (ret != -1)
		{
			LPCWSTR lParamW = lParam ? MultiByteToWideCharInternal((LPCSTR)lParam) : NULL;
			// LN899
			LRESULT hr = CallProcAddress(lpProcAddress, hWnd, hMDIClient, bMDIClientEnabled, uMsg, wParam, (LPARAM)lParamW);
			if (lParamW/*ebx*/) FreeStringInternal((LPVOID)lParamW);
			return hr;
		}
	}	break;
	default: // LN201
		break;
	}
	// --------- 
	return CallProcAddress(lpProcAddress, hWnd, hMDIClient, bMDIClientEnabled, uMsg, wParam, lParam);
}

static BOOL CheckDynamicWndProc(int i, WNDPROC PrevWindowProc, HWND hWnd) {
	UNREFERENCED_PARAMETER(PrevWindowProc);
	wchar_t classname[32]; // try recognize it!
	return GetClassNameW(hWnd, classname, ARRAYSIZE(classname)) &&
		(lstrcmpiW(classname, SystemClassNameW[i]) == 0);
}

void SetTopWindowProc(NTLEA_TLS_DATA* p, HWND hWnd, LONG_PTR TopLevelWndProc)
{
	NTLEA_WND_ASC_DATA* wndasc = (NTLEA_WND_ASC_DATA*)AllocateZeroedMemory(sizeof(NTLEA_WND_ASC_DATA));
	++p->InternalCall;
	WNDPROC wndproca = (WNDPROC)GetWindowLongPtrA(hWnd, GWLP_WNDPROC);
	if (wndproca)
	{
		wndasc->PrevAnsiWindowProc = wndproca;
		Addresses.lpSetWindowLongAddress(hWnd, GWLP_WNDPROC, TopLevelWndProc);
		SetPropA(hWnd, szNtleaWndAscData, (HANDLE)wndasc); // save previous wndproc 
	}
	else
	{
		FreeStringInternal(wndasc);
	}
}

static void HookWindowProc(NTLEA_TLS_DATA* p, HWND hWnd)
{
	CreateGlobalAtom();
	if (!GetPropA(hWnd, szNtleaWndAscData))
	{
		SetTopWindowProc(p, hWnd, (LONG_PTR)TopLevelWindowProc);
	}
}

static LRESULT CbtHookProc(NTLEA_TLS_DATA* p, HHOOK hhook, int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode != HCBT_CREATEWND ||
		(p->CurrentCallType != CT_CREATE_WINDOW && IsWindowUnicode((HWND)wParam)))
	{
		return CallNextHookEx(hhook, nCode, wParam, lParam);
	}
	WCHAR ClassNameBuffer[MAX_PATH];
	HWND hwnd = (HWND)wParam;
	if (GetClassNameW(hwnd, ClassNameBuffer, sizeof(ClassNameBuffer)) > 0)
	{
		if (lstrcmpiW(L"NewDlgClass", ClassNameBuffer) != 0)
		{
			HookWindowProc(p, hwnd);
		}
	}
	return CallNextHookEx(hhook, nCode, wParam, lParam);
}
static LRESULT CALLBACK CbtHookProcA(int code, WPARAM wParam, LPARAM lParam)
{
	NTLEA_TLS_DATA* p = GetTlsValueInternal();
	return CbtHookProc(p, p->hWindowCbtHookAnsi, code, wParam, lParam);
}
static LRESULT CALLBACK CbtHookProcW(int code, WPARAM wParam, LPARAM lParam)
{
	NTLEA_TLS_DATA* p = GetTlsValueInternal();
	return CbtHookProc(p, p->hWindowCbtHookUnicode, code, wParam, lParam);
}

__inline void InstallCbtHook(NTLEA_TLS_DATA * ptls)
{
	if (ptls->hWindowHooking == 0)
	{
		ptls->hWindowCbtHookAnsi = SetWindowsHookExA(WH_CBT, CbtHookProcA, NULL, GetCurrentThreadId());
		ptls->hWindowCbtHookUnicode = SetWindowsHookExW(WH_CBT, CbtHookProcW, NULL, GetCurrentThreadId());
	}
	ptls->hWindowHooking++;
}

__inline void UninstallCbtHook(NTLEA_TLS_DATA * ptls) { // ebx 
	ptls->hWindowHooking--; // guard 
	if (ptls->hWindowHooking == 0) {
		UnhookWindowsHookEx(ptls->hWindowCbtHookAnsi);
		UnhookWindowsHookEx(ptls->hWindowCbtHookUnicode);
	}
	else if (ptls->hWindowHooking < 0) {
		// reference error occured : 
		ShowUnhandledExceptionMessage(L"Unmatched");
	}
}

LRESULT WINAPI HookCallWindowProc(WNDPROC PrevWindowProc, HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	// NOTE: Don't use stack variable for TLSValue!
	if (GetTlsValueInternal()->InternalCall)
		GetTlsValueInternal()->InternalCall--;
	for (int i = 0; i < 15; ++i)
	{
		if (GetTlsValueInternal()->SystemClassDesc[i].AnsiSystemClassProc == PrevWindowProc
			|| CheckDynamicWndProc(i, PrevWindowProc, hWnd))
		{
			return SendUnicodeMessage((LPVOID)(DWORD_PTR)CallWindowProcW, hWnd, uMsg, wParam, lParam,
				(DWORD_PTR)GetTlsValueInternal()->SystemClassDesc[i].UnicodeSystemClassProc, i, i, 3);
		}
	}
	return Addresses.lpCallWindowProcAddress(PrevWindowProc, hWnd, uMsg, wParam, lParam);
}


LRESULT WINAPI HookSendMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return SendUnicodeMessage((LPVOID)(DWORD_PTR)SendMessageW, hWnd, uMsg, wParam, lParam, 0, 0, 0, 0);
}


LRESULT WINAPI HookPostMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return SendUnicodeMessage((LPVOID)(DWORD_PTR)PostMessageW, hWnd, uMsg, wParam, lParam, 0, 0, 0, 0);
}

LRESULT WINAPI HookSendNotifyMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return SendUnicodeMessage((LPVOID)(DWORD_PTR)SendNotifyMessageW, hWnd, uMsg, wParam, lParam, 0, 0, 0, 0);
}

LRESULT WINAPI HookSendMessageCallback(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, SENDASYNCPROC lpCallBack, ULONG_PTR dwData)
{
	return SendUnicodeMessage((LPVOID)(DWORD_PTR)SendMessageCallbackW, hWnd, uMsg, wParam, lParam, (DWORD_PTR)lpCallBack, dwData, 0, 1);
}

LRESULT WINAPI HookSendMessageTimeout(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT fuFlags, UINT uTimeout, PDWORD_PTR lpdwResult)
{
	return SendUnicodeMessage((LPVOID)(DWORD_PTR)SendMessageTimeoutW, hWnd, uMsg, wParam, lParam, fuFlags, uTimeout, (DWORD_PTR)lpdwResult, 2);
}

typedef INT_PTR(CALLBACK *fnDialogBoxIndirectParamProc)(HINSTANCE hInstance, LPCDLGTEMPLATE hDialogTemplate,
	HWND hWndParent, DLGPROC lpDialogFunc, LPARAM dwInitParam);
typedef INT_PTR(CALLBACK *fnDialogBoxParamProc)(HINSTANCE hInstance, LPCWSTR lpTemplateName,
	HWND hWndParent, DLGPROC lpDialogFunc, LPARAM dwInitParam);

static INT_PTR DialogBoxIndirectParamProc(fnDialogBoxIndirectParamProc dialogboxindirectparamproc, HINSTANCE hInstance,
	LPCDLGTEMPLATE hDialogTemplate, HWND hWndParent, DLGPROC lpDialogFunc, LPARAM dwInitParam) {
	NTLEA_TLS_DATA * p = GetTlsValueInternal();
	InstallCbtHook(p);
	DLGPROC DialogProc = p->DialogProc; p->DialogProc = lpDialogFunc;
	DWORD CurrentCallType = p->CurrentCallType; p->CurrentCallType = CT_CREATE_DIALOG;
	// ---- 
	INT_PTR ret = dialogboxindirectparamproc(hInstance, hDialogTemplate, hWndParent, TopLevelDialogProc, dwInitParam);
	p->CurrentCallType = CurrentCallType;
	p->DialogProc = DialogProc;
	UninstallCbtHook(p);
	return ret;
}


static INT_PTR DialogBoxParamProc(fnDialogBoxParamProc dialogboxparamproc, HINSTANCE hInstance, LPCSTR lpTemplateName,
	HWND hWndParent, DLGPROC lpDialogFunc, LPARAM lParamInit)
{
	NTLEA_TLS_DATA * p = GetTlsValueInternal();
	InstallCbtHook(p);
	DLGPROC PrevDialogFunc = p->DialogProc; p->DialogProc = lpDialogFunc;
	DWORD PrevCallType = p->CurrentCallType; p->CurrentCallType = CT_CREATE_DIALOG;
	LPCWSTR lpTemplateW = ((DWORD_PTR)lpTemplateName & WM_CLASSMASK ? MultiByteToWideCharInternal(lpTemplateName) : NULL);
	// Y105
	INT_PTR ret = dialogboxparamproc(hInstance, (lpTemplateW ? lpTemplateW : (LPCWSTR)lpTemplateName),
		hWndParent, TopLevelDialogProc, lParamInit);
	p->CurrentCallType = PrevCallType;
	p->DialogProc = PrevDialogFunc;
	UninstallCbtHook(p); // order change OK ?? 
	if (lpTemplateW) FreeStringInternal((LPVOID)lpTemplateW);
	return ret;
}
INT_PTR WINAPI HookDialogBoxIndirectParam(HINSTANCE hInstance, LPCDLGTEMPLATE hDialogTemplate,
	HWND hWndParent, DLGPROC lpDialogFunc, LPARAM dwInitParam)
{
	return DialogBoxIndirectParamProc((fnDialogBoxIndirectParamProc)DialogBoxIndirectParamW,
		hInstance, hDialogTemplate, hWndParent, lpDialogFunc, dwInitParam);
}
HWND WINAPI HookCreateDialogIndirectParam(HINSTANCE hInstance, LPCDLGTEMPLATE lpTemplate,
	HWND hWndParent, DLGPROC lpDialogFunc, LPARAM lParamInit)
{
	return (HWND)DialogBoxIndirectParamProc((fnDialogBoxIndirectParamProc)CreateDialogIndirectParamW,
		hInstance, lpTemplate, hWndParent, lpDialogFunc, lParamInit);
}
INT_PTR WINAPI HookDialogBoxParam(HINSTANCE hInstance, LPCSTR lpTemplateName, HWND hWndParent,
	DLGPROC lpDialogFunc, LPARAM dwInitParam)
{
	return DialogBoxParamProc((fnDialogBoxParamProc)DialogBoxParamW,
		hInstance, lpTemplateName, hWndParent, lpDialogFunc, dwInitParam);
}
HWND WINAPI HookCreateDialogParam(HINSTANCE hInstance, LPCSTR lpTemplateName, HWND hWndParent,
	DLGPROC lpDialogFunc, LPARAM dwInitParam)
{
	return (HWND)DialogBoxParamProc((fnDialogBoxParamProc)CreateDialogParamW,
		hInstance, lpTemplateName, hWndParent, lpDialogFunc, dwInitParam);
}

__inline int WideCharToMultiBytePartial(UINT CodePage, DWORD dwFlags,
	LPCWSTR lpWideCharStr, int cchWideChar, LPSTR lpMultiByteStr, int cbMultiByte, LPCSTR lpDefaultChar, LPBOOL lpUsedDefaultChar)
{
	int len = WideCharToMultiByte(CodePage, dwFlags, lpWideCharStr, cchWideChar, NULL, 0, lpDefaultChar, lpUsedDefaultChar);
	assert(len >= cbMultiByte);
	LPSTR tempbuffer = (LPSTR)AllocateHeapInternal(len * sizeof(char));
	WideCharToMultiByte(CodePage, dwFlags, lpWideCharStr, cchWideChar, tempbuffer, len, lpDefaultChar, lpUsedDefaultChar);
	memcpy(lpMultiByteStr, tempbuffer, cbMultiByte * sizeof(char)); FreeStringInternal(tempbuffer);
	return cbMultiByte;
}

__inline int MultiByteToWideCharPartial(UINT CodePage, DWORD dwFlags,
	LPCSTR lpMultiByteStr, int cbMultiByte, LPWSTR lpWideCharStr, int cchWideChar)
{
	int len = MultiByteToWideChar(CodePage, dwFlags, lpMultiByteStr, cbMultiByte, NULL, 0);
	assert(len >= cchWideChar);
	LPWSTR tempbuffer = (LPWSTR)AllocateHeapInternal(len * sizeof(wchar_t));
	MultiByteToWideChar(CodePage, dwFlags, lpMultiByteStr, cbMultiByte, tempbuffer, len);
	memcpy(lpWideCharStr, tempbuffer, cchWideChar * sizeof(wchar_t)); FreeStringInternal(tempbuffer);
	return cchWideChar;
}

NTSTATUS WINAPI HookUnicodeToMultiByte(LPSTR AnsiBuffer, DWORD MultiByteLength,
	LPDWORD lpNumberOfBytesConverted, LPCWSTR UnicodeBuffer, DWORD WideCharLength/*bytes*/)
{
	while (!InterlockedCompareExchange(&settings.bInternalLockWCtoMB, 1, 0)
		&& GetCurrentThreadId() != settings.dwThreadIdWCtoMB) Sleep(0); // spin wait

	settings.dwThreadIdWCtoMB = GetCurrentThreadId();
	++settings.bInternalCallWCtoMB;
	int ret = WideCharToMultiByte(CP_ACP, 0, UnicodeBuffer, WideCharLength >> 1, AnsiBuffer, MultiByteLength, NULL, NULL);
	if (!ret) { // widechar to multibyte won't support partial convert, so ... 
		DWORD err = GetLastError(); SetLastError(0); // reset state !! 
		if (err == ERROR_INSUFFICIENT_BUFFER)
		{
			ret = WideCharToMultiBytePartial(CP_ACP, 0, UnicodeBuffer, WideCharLength >> 1, AnsiBuffer, MultiByteLength, NULL, NULL);
		}
	}
	--settings.bInternalCallWCtoMB;
	InterlockedDecrement(&settings.bInternalLockWCtoMB);
	if (lpNumberOfBytesConverted) *lpNumberOfBytesConverted = ret;
	return (0);
}
NTSTATUS WINAPI HookMultiByteToUnicode(
	LPWSTR UnicodeBuffer, DWORD WideCharLength, DWORD* lpNumberOfBytesConverted, LPCSTR AnsiBuffer, DWORD MultiByteLength)
{
	// [var] = 0
	// if (eax==[var]) { zf=1, [var]=ecx } else { zf=0 }
	while (!InterlockedCompareExchange(&settings.bInternalLockMBtoWC, 1/*ecx*/, 0/*eax*/)
		&& GetCurrentThreadId() != settings.dwThreadIdMBtoWC) Sleep(0); // spin wait

	settings.dwThreadIdMBtoWC = GetCurrentThreadId(); // 
	++settings.bInternalCallMBtoWC;
	int ret = MultiByteToWideChar(CP_ACP, 0, AnsiBuffer, MultiByteLength, UnicodeBuffer, WideCharLength >> 1);
	if (!ret)
	{
		DWORD err = GetLastError(); SetLastError(0); // reset state !! 
		if (err == ERROR_INSUFFICIENT_BUFFER) {
			ret = MultiByteToWideCharPartial(CP_ACP, 0, AnsiBuffer, MultiByteLength, UnicodeBuffer, WideCharLength >> 1);
		}
	}
	--settings.bInternalCallMBtoWC;

	InterlockedDecrement(&settings.bInternalLockMBtoWC);
	if (lpNumberOfBytesConverted) *lpNumberOfBytesConverted = ret << 1;
	return (0);
}

NTSTATUS WINAPI HookUnicodeToMultiByteSize(int* lpNumberOfBytesConverted, LPCWSTR UnicodeBuffer, int WideCharLength/*bytes*/) {
	int ret = WideCharToMultiByte(CP_ACP, 0, UnicodeBuffer, WideCharLength >> 1, NULL, 0, NULL, NULL);
	if (lpNumberOfBytesConverted) *lpNumberOfBytesConverted = ret;
	return (0);
}

NTSTATUS WINAPI HookMultiByteToUnicodeSize(int* lpNumberOfBytesConverted, LPCSTR AnsiBuffer, int MultiByteLength)
{
	int ret = MultiByteToWideChar(CP_ACP, 0, AnsiBuffer, MultiByteLength, NULL, 0) << 1;
	if (lpNumberOfBytesConverted) *lpNumberOfBytesConverted = ret;
	return (0);
}


BOOL WINAPI HookIsDBCSLeadByte(BYTE TestChar)
{
	return IsDBCSLeadByteEx(settings.dwCodePage, TestChar);
}

LPSTR WINAPI HookCharPrev(LPCSTR lpStart, LPCSTR lpCurrentChar)
{
	return CharPrevExA((WORD)settings.dwCodePage, lpStart, lpCurrentChar, 0);
}

LPSTR WINAPI HookCharNext(LPCSTR lpCurrentChar)
{
	return CharNextExA((WORD)settings.dwCodePage, lpCurrentChar, 0);
}


HWND WINAPI HookCreateWindowEx(
	DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle,
	int x, int y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam)
{
	NTLEA_TLS_DATA* p = GetTlsValueInternal();
	DWORD PrevCallType = p->CurrentCallType; p->CurrentCallType = CT_CREATE_WINDOW;
	// 1. prepare the thread hook for the next step windowproc hook, then createwindow and hook it!
	LPCWSTR lpWindowNameW = (lpWindowName) ? MultiByteToWideCharInternal(lpWindowName) : NULL;
	LPCWSTR lpClassNameW = ((DWORD_PTR)lpClassName & WM_CLASSMASK) ? MultiByteToWideCharInternal(lpClassName) : NULL;
	InstallCbtHook(p);

	HWND hwnd = CreateWindowExW(dwExStyle, (lpClassNameW ? lpClassNameW : (LPCWSTR)lpClassName), lpWindowNameW, dwStyle,
		x, y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);

	UninstallCbtHook(p);
	if (lpWindowNameW) FreeStringInternal((LPVOID)lpWindowNameW);
	if (lpClassNameW) FreeStringInternal((LPVOID)lpClassNameW);
	// 4. cleanup resource 
	p->CurrentCallType = PrevCallType;
	return hwnd;
}

int WINAPI HookGetWindowText(HWND hWindow, LPSTR lpString, int nMaxCount)
{
	int len = (int)SendMessageW(hWindow, WM_GETTEXTLENGTH, 0, 0) + 1;
	LPWSTR lpStringW = (LPWSTR)AllocateZeroedMemory(len * sizeof(wchar_t));

	int ret = GetWindowTextW(hWindow, lpStringW, len);
	if (ret > 0) {
		int size = WideCharToMultiByte(CP_ACP, 0, lpStringW, -1, lpString, nMaxCount, NULL, NULL);
		if (size > 0) ret = size - 1;
		else {
			if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
				lpString[nMaxCount - 1] = '\0'; ret = nMaxCount - 1;
			}
			else {
				lpString[0] = '\0'; ret = 0;
			}
		}
	}
	else {
		lpString[0] = '\0'; ret = 0;
	}
	FreeStringInternal(lpStringW);
	return ret;
}

BOOL WINAPI HookSetWindowText(HWND hWindow, LPCSTR lpstrText)
{
	LPCWSTR wstr = NULL;
	if (lpstrText)
	{
		wstr = MultiByteToWideCharInternal(lpstrText);
	}
	BOOL ret = SetWindowTextW(hWindow, wstr);
	if (wstr)
	{
		FreeStringInternal((LPVOID)wstr);
	}
	return ret;
}

LONG_PTR WINAPI HookGetWindowLongPtr(HWND hWnd, int nIndex)
{
	if (nIndex == GWLP_WNDPROC) {
		// G200: check internal call ? 
		if (GetTlsValueInternal()->InternalCall == 0) {
			// G202: retrieve the proc --> for subclass but why not took effect in some cases ??? 
			// TODO: confirm it with my patch.. 
			if (GetPropA(hWnd, szNtleaWndAscData)) {
				return (LONG_PTR)((NTLEA_WND_ASC_DATA*)GetPropA(hWnd, szNtleaWndAscData))->PrevAnsiWindowProc;
			}
		}
		else {
			--GetTlsValueInternal()->InternalCall;
		}
		// G201:
		return Addresses.lpGetWindowLongPtrAddress(hWnd, nIndex);
	}
	else {
		return GetWindowLongPtrW(hWnd, nIndex);
	}
}

__inline VOID ValidateDialogProc(HWND hWnd)
{
	NTLEA_TLS_DATA* p = GetTlsValueInternal();
	if (!GetPropA(hWnd, szNtleaWndAscData))
	{
		if (p->CurrentCallType == CT_CREATE_DIALOG && p->DialogProc == NULL)
		{
			SetTopWindowProc(p, hWnd, (LONG_PTR)TopLevelWindowProc);
		}
		else if (p->CurrentCallType == CT_CREATE_NULL) 
		{
			SetTopWindowProc(p, hWnd, (LONG_PTR)TopLevelSimpleProc);
		}
	}
}


__inline LONG_PTR SetWindowLongPtrProc(HWND hWnd, LONG_PTR dwNewLong)
{
	NTLEA_WND_ASC_DATA* p = (NTLEA_WND_ASC_DATA*)GetPropA(hWnd, szNtleaWndAscData);
	LONG_PTR PrevAnsiWindowProc = (LONG_PTR)p->PrevAnsiWindowProc; // in actual, new wndproc may be unicode ... 
	p->PrevAnsiWindowProc = (WNDPROC)dwNewLong;
	return PrevAnsiWindowProc;
}

LONG_PTR WINAPI HookSetWindowLongPtr(HWND hWnd, int nIndex, LONG_PTR dwNewLong)
{
	if (nIndex == GWLP_WNDPROC)
	{
		ValidateDialogProc(hWnd);
		if (GetPropA(hWnd, szNtleaWndAscData))
		{
			return SetWindowLongPtrProc(hWnd, dwNewLong);
		}
	}
	return Addresses.lpSetWindowLongAddress(hWnd, nIndex, dwNewLong);
}

LONG_PTR WINAPI HookSetWindowLongPtrW(HWND hWnd, int nIndex, LONG_PTR dwNewLong)
{
	if (nIndex == GWLP_WNDPROC)
	{
		if (!IsWindowUnicode(hWnd)) ValidateDialogProc(hWnd);
		if (GetPropA(hWnd, szNtleaWndAscData)) 
		{ // G210:
			return SetWindowLongPtrProc(hWnd, dwNewLong);
		}
	}
	return Addresses.lpSetWindowLongAddress(hWnd, nIndex, dwNewLong);
}

LRESULT WINAPI HookDefWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return DefConversionProc((LPVOID)(DWORD_PTR)DefWindowProcW, hWnd, NULL, FALSE, uMsg, wParam, lParam);
}
LRESULT WINAPI HookDefMDIChildProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return DefConversionProc((LPVOID)(DWORD_PTR)(DWORD_PTR)DefMDIChildProcW, hWnd, NULL, FALSE, uMsg, wParam, lParam);
}
LRESULT WINAPI HookDefDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return DefConversionProc((LPVOID)(DWORD_PTR)DefDlgProcW, hWnd, NULL, FALSE, uMsg, wParam, lParam);
}
LRESULT WINAPI HookDefFrameProc(HWND hWnd, HWND hWndMDIClient, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return DefConversionProc((LPVOID)(DWORD_PTR)DefFrameProcW, hWnd, hWndMDIClient, TRUE, uMsg, wParam, lParam);
}


BOOL WINAPI HookVerQueryValue(LPCVOID pBlock, LPCSTR lpSubBlock, LPVOID *lplpBuffer, PUINT puLen)
{
	while (!InterlockedCompareExchange(&settings.bInternalLockVQV, 1/*ecx*/, 0/*eax*/)) Sleep(0);

	if (lstrcmpA(lpSubBlock, szTranslation) == 0)
	{
		if (Addresses.lpVerQueryValueAddress(pBlock, lpSubBlock, lplpBuffer, puLen) && *puLen >= sizeof(DWORD)){
			LANGANDCODEPAGE *lpTranslate = (LANGANDCODEPAGE*)(*lplpBuffer);
			settings.langcodepage.wLanguage = lpTranslate->wLanguage;
			settings.langcodepage.wCodePage = lpTranslate->wCodePage;
			lpTranslate->wLanguage = (WORD)settings.dwLCID;
			lpTranslate->wCodePage = (WORD)settings.dwCodePage; // change the first default one !
			InterlockedDecrement(&settings.bInternalLockVQV);
			return TRUE;
		}
		else
		{
			InterlockedDecrement(&settings.bInternalLockVQV);
			return FALSE;
		}
	}
	else if (lstrlenA(lpSubBlock) > 2 && lpSubBlock[0] == '\\' && lpSubBlock[1] == 'S')
	{
		InterlockedDecrement(&settings.bInternalLockVQV);

		LPCSTR type = lpSubBlock + lstrlenA(lpSubBlock);
		while (type > lpSubBlock && '\\' != *type) --type;
		// build info : 
		char SubBlock[64]; wsprintfA(SubBlock, "\\StringFileInfo\\%04x%04x\\%s",
			settings.langcodepage.wLanguage, settings.langcodepage.wCodePage, type + 1);
		return Addresses.lpVerQueryValueAddress(pBlock, SubBlock, lplpBuffer, puLen);
	}
	else
	{
		InterlockedDecrement(&settings.bInternalLockVQV);
		return Addresses.lpVerQueryValueAddress(pBlock, lpSubBlock, lplpBuffer, puLen);
	}
}


FORCEINLINE VOID AcquireCreateProcLock()
{
	while (!InterlockedCompareExchange(&settings.bInternalLockCreateProc, 1/*ecx*/, 0/*eax*/)) Sleep(0);
}

FORCEINLINE VOID ReleaseCreateProcLock(void)
{
	InterlockedDecrement(&settings.bInternalLockCreateProc);
}

UINT WINAPI HookGdiGetCodePage(HDC hdc)
{
	UNREFERENCED_PARAMETER(hdc);
	return settings.dwCodePage;
}

DWORD CALLBACK InitUnicodeLayer(HMODULE hinstDll, ULONG CodePage, ULONG LCID)
{
	settings.hHeap = HeapCreate(0, 0, 0);
	settings.dwTimeZone = (DWORD)-480;
	settings.hInstance = hinstDll;

	settings.bChinesePath = 0;
	settings.bCreateProcNative = 0;
	settings.bForceSpecifyFont = 0;
	settings.bNoFilterCommCtrl = 0;

	settings.dwCodePage = 936;
	settings.dwLCID = 0x0411/*0x0804*/;
	settings.dwTimeZone = (DWORD)-540;


	MIN_MEMORY_FUNCTION_PATCH f[] =
	{
		{ DefWindowProcA, HookDefWindowProc, NULL },
		{ DefMDIChildProcA, HookDefMDIChildProc, NULL },
		{ DefDlgProcA, HookDefDlgProc, NULL },
		{ DefFrameProcA, HookDefFrameProc, NULL },
		{ CreateWindowExA, HookCreateWindowEx, NULL },
		{ CallWindowProcA, HookCallWindowProc, (PVOID*)&Addresses.lpCallWindowProcAddress },
		{ GetWindowTextA, HookGetWindowText, NULL },
		{ DialogBoxParamA, HookDialogBoxParam, NULL },
		{ DialogBoxIndirectParamA, HookDialogBoxIndirectParam, NULL },
		{ CreateDialogParamA, HookCreateDialogParam, NULL },
		{ CreateDialogIndirectParamA, HookCreateDialogIndirectParam, NULL },
		{ SendMessageA, HookSendMessage, NULL },
		{ SendMessageCallbackA, HookSendMessageCallback, NULL },
		{ SendMessageTimeoutA, HookSendMessageTimeout, NULL },
		{ SendNotifyMessageA, HookSendNotifyMessage, NULL },
		{ PostMessageA, HookPostMessage, NULL },
		{ SetWindowLongPtrA, HookSetWindowLongPtr, NULL },
		{ SetWindowLongPtrW, HookSetWindowLongPtrW, (PVOID*)&Addresses.lpSetWindowLongAddress },
		{ GetWindowLongPtrA, HookGetWindowLongPtr, (PVOID*)&Addresses.lpGetWindowLongPtrAddress },
		//{ GetMenuStringA, HookGetMenuString, NULL }
	};

	Nt_PatchMemory(f, _countof(f));
	return 0;
}
