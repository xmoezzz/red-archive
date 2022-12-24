#include "InitWindowUnicodeLayer.h"
#include "NtFunction.h"

char const* SystemClassNameA[] = 
{
	"BUTTON", "COMBOBOX", "ComboLBox", /*"#32770",*/ "EDIT", "LISTBOX", "MDICLIENT", "RichEdit", "RICHEDIT_CLASS",
	"SCROLLBAR", "STATIC", "SysTreeView32", "SysListView32", "SysAnimate32", "SysHeader32", "tooltips_class32",
	//	"SysTabControl32", "ToolbarWindow32", "ComboBoxEx32", "SysDateTimePick32", "SysMonthCal32", "ReBarWindow32", 
	//	"msctls_progress32", "msctls_trackbar32", "msctls_statusbar32", "msctls_updown32", "msctls_hotkey32",
	/*NULL, */
};
wchar_t const* SystemClassNameW[] = 
{
	L"BUTTON", L"COMBOBOX", L"ComboLBox", /*L"32770",*/ L"EDIT", L"LISTBOX", L"MDICLIENT", L"RichEdit", L"RICHEDIT_CLASS",
	L"SCROLLBAR", L"STATIC", L"SysTreeView32", L"SysListView32", L"SysAnimate32", L"SysHeader32", L"tooltips_class32",
	//	L"SysTabControl32", L"ToolbarWindow32", L"ComboBoxEx32", L"SysDateTimePick32", L"SysMonthCal32", L"ReBarWindow32",
	//	L"msctls_progress32", L"msctls_trackbar32", L"msctls_statusbar32", L"msctls_updown32", L"msctls_hotkey32",
	/*NULL, */
};

INLINE LPVOID AllocateZeroedMemory(SIZE_T size/*eax*/)
{
	return HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, size);
}

INLINE LPVOID AllocateHeapInternal(SIZE_T size/*ecx*/)
{
	return HeapAlloc(GetProcessHeap(), 0, size);
}

INLINE VOID FreeStringInternal(LPVOID pBuffer/*ecx*/)
{
	HeapFree(GetProcessHeap(), 0, pBuffer);
}

INLINE LPCWSTR MultiByteToWideCharInternal(LPCSTR lpString)
{
	int size = lstrlenA(lpString)/* size without '\0' */, n = 0;
	LPWSTR wstr = (LPWSTR)AllocateHeapInternal((size + 1) << 1);
	if (wstr) {
		n = MultiByteToWideChar(CP_ACP, 0, lpString, size, wstr, size);
		wstr[n] = L'\0'; // make tail ! 
	}
	return wstr;
}

INLINE LPCSTR WideCharToMultiByteInternal(LPCWSTR lpString)
{
	int size = lstrlenW(lpString)/* size without '\0' */, n = 0;
	LPSTR str = (LPSTR)AllocateHeapInternal((size + 1) << 1); // TODO: support UTF-8 3bytes ??? 
	if (str) {
		n = WideCharToMultiByte(CP_ACP, 0, lpString, size, str, size << 1, NULL, NULL);
		str[n] = '\0'; // make tail ! 
	}
	return str;
}

static VOID ShowUnhandledExceptionMessage(WCHAR const* errorstr)
{
	SetUnhandledExceptionFilter(NULL);
	int i = MessageBoxW(GetForegroundWindow(), errorstr, L"X'moe-Core Lib Exception", MB_ICONHAND);
	ExitProcess(i);
}

static int CheckWindowStyle(HWND hWnd, DWORD type/*ebx*/) 
{

	LONG_PTR n = GetWindowLongPtrW(hWnd, GWL_STYLE);
	// window no needs conversion ?? 
	if (n == 0) 
	{
		return (0);
	}
	else if (n == /*0x84C820E4*/(WS_POPUP | WS_CLIPSIBLINGS | WS_BORDER | WS_DLGFRAME | WS_SYSMENU |
		WS_EX_RTLREADING | WS_EX_TOOLWINDOW | WS_EX_MDICHILD | WS_EX_TRANSPARENT | WS_EX_NOPARENTNOTIFY)) 
	{
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
	// other case : 
	return (-1); // xor ebx, ebx !
}

#if 0

INLINE NTLEA_WND_ASC_DATA* CheckProp(HWND hWnd) 
{
	NTLEA_WND_ASC_DATA* p = GetPropA(hWnd, szNtleaWndAscData);
	if (!p)
		ShowUnhandledExceptionMessage("Unacceptable Empty Window Prop Detected, Force Exit.");
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

	//	char classname[256]; GetClassNameA(hWnd, classname, sizeof(classname));
	//	if (lstrcmpiA(classname, "TRadioButton") == 0) {
	//	ntprintfA(256, 1, "%s: proc-%p hwnd=%p, msg=%04x, wParam=%d, lParam=%d\n", __FUNCTION__, PrevWndProc, hWnd, uMsg, wParam, lParam);
	//	}

	switch (uMsg)
	{
	case WM_CREATE: // L103
	case WM_NCCREATE: // general case !! 
	{
		if (lParam)
		{
			CREATEWNDEX * p = (CREATEWNDEX *)AllocateHeapInternal(sizeof(CREATEWNDEX));
			memcpy(p, (LPVOID)lParam, sizeof(CREATEWNDEX));
			if (p->lpWindowName) { // L103
				p->lpWindowName = (LPVOID)(lpAnsiWindowName = WideCharToMultiByteInternal(p->lpWindowName));
			}
			if ((DWORD_PTR)p->lpClassName & WM_CLASSMASK) { // L101
				p->lpClassName = (LPVOID)(lpAnsiClassName = WideCharToMultiByteInternal(p->lpClassName));
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
			CREATEMDIWND* p = AllocateHeapInternal(sizeof(CREATEMDIWND));
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
		else
		{
			LRESULT len = CallWindowProcA(PrevWndProc, hWnd, WM_GETTEXTLENGTH, 0, 0);
			if (len == 0) {
				*((LPWSTR)lParam) = L'\0';
			}
			else 
			{ // L116
				GetTlsValueInternal()->InternalCall++;
				LPSTR lParamA = (LPSTR)AllocateZeroedMemory((len + 1) * sizeof(wchar_t));
				CallWindowProcA(PrevWndProc, hWnd, uMsg, (len + 1) * sizeof(wchar_t), (LPARAM)lParamA);
				if (uMsg == WM_UNKNOWN) wParam = len + 1;
				len = MultiByteToWideChar(settings.dwCodePage, 0, lParamA, -1, (LPWSTR)lParam, (int)wParam) - 1;
				if (len > 0)
				{
					// L997
					if (lParamA) FreeStringInternal((LPVOID)lParamA);
				}
				else 
				{
					// L119
					if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
					{
						*((LPWSTR)lParam + wParam - 1) = L'\0';
					}
					else 
					{
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
		if ((wchar_t)wParam > 0x7F) 
		{
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
		if (lstrcmpiA(ClassNameBuffer, "SysTreeView32") == 0)
		{
			DWORD_PTR n = GetWindowLongPtrW(hWnd, DWLP_MSGRESULT/*0*/);
			if (n && *(LPBYTE)(n + 0x10) == 1) 
			{ // what ??? 
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
			if (len > 0)
			{
				GetTlsValueInternal()->InternalCall++;
				LPSTR lParamA = (LPSTR)AllocateZeroedMemory((len + 1) * sizeof(wchar_t));
				CallWindowProcA(PrevWndProc, hWnd, LB_GETTEXT, wParam, (LPARAM)lParamA);
				len = MultiByteToWideChar(settings.dwCodePage, 0, lParamA, -1, NULL, 0) - 1;
				// LN995
				if (lParamA/*ebx*/) FreeStringInternal((LPVOID)lParamA);
			}
			return len;
		}
	}
	break;
	case LB_GETTEXT: // LN110
	{
		type = 1;
	}//	break;
	case CB_GETLBTEXT: // LN129
	{
		int ret = CheckWindowStyle(hWnd, type); // 0
		if (ret != -1) 
		{
			LPSTR lParamA = (LPSTR)AllocateZeroedMemory(MAX_PATH * sizeof(wchar_t));
			int len = (int)CallWindowProcA(PrevWndProc, hWnd, uMsg, wParam, (LPARAM)lParamA);
			if (len) // if success : 
			{
				len = MultiByteToWideChar(settings.dwCodePage, 0, lParamA, len + 1, (LPWSTR)lParam, MAX_PATH); // --- bugfixed
			}
			if (!len && lParam)
			{
				*(LPWSTR)lParam = L'\0'; // handle failed case !
			}
			else if (len)
			{
				--len; // report not-including null-terminate string !
			}

			if (lParamA/*ebx*/)
			{
				FreeStringInternal((LPVOID)lParamA);
			}
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
		else 
		{ // recv op 
			// LN110
			ret = CheckWindowStyle(hWnd, type + 1); // ebx = 1 / 2
			if (ret != -1) 
			{
				// <-- ebx 
				LPSTR lParamA = (LPSTR)AllocateZeroedMemory(MAX_PATH * sizeof(wchar_t));
				// LN999
				int len = (int)CallWindowProcA(PrevWndProc, hWnd, uMsg, wParam, (LPARAM)lParamA);
				if (len) // if success : 
				{
					len = MultiByteToWideChar(settings.dwCodePage, 0, lParamA, len + 1, (LPWSTR)lParam, MAX_PATH); // --- bugfixed
				}
				if (!len && lParam)
				{
					*(LPWSTR)lParam = L'\0'; // L111
				}
				else if (len)
				{
					--len; // report not-including null-terminate string !
				}
				// LN995
				if (lParamA)
				{
					FreeStringInternal((LPVOID)lParamA);
				}
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

static void CreateGlobalAtom(void)
{
	while (!InterlockedCompareExchange(&settings.bInternalLockCreateAtom, 1/*ecx*/, 0/*eax*/))
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

static void SetTopWindowProc(NTLEA_TLS_DATA* p, HWND hWnd, LONG_PTR TopLevelWndProc)
{
	NTLEA_WND_ASC_DATA* wndasc = (NTLEA_WND_ASC_DATA*)AllocateZeroedMemory(sizeof(NTLEA_WND_ASC_DATA));
	++p->InternalCall;
	WNDPROC wndproca = (WNDPROC)GetWindowLongPtrA(hWnd, GWLP_WNDPROC);
	if (wndproca)
	{
		wndasc->PrevAnsiWindowProc = wndproca;
		SetWindowLongPtrJ(hWnd, GWLP_WNDPROC, TopLevelWndProc);
		SetPropA(hWnd, szNtleaWndAscData, (HANDLE)wndasc); // save previous wndproc 
	}
	else 
	{ // no wndproc, no needs ??? 
		FreeStringInternal(wndasc);
	}
}

static void HookWindowProc(NTLEA_TLS_DATA* p, HWND hWnd)
{
	CreateGlobalAtom();
	// initialize custom window proc storage : 
	if (!GetPropA(hWnd, szNtleaWndAscData)) {
		SetTopWindowProc(p, hWnd, (LONG_PTR)TopLevelWindowProc);
	}
}

static LRESULT CbtHookProc(NTLEA_TLS_DATA* p, HHOOK hhook, int nCode, WPARAM wParam, LPARAM lParam) 
{
	// we filter msg we don't take care : 
	if (nCode != HCBT_CREATEWND || // or a unicode dialog won't cause hook !! 
		(p->CurrentCallType != CT_CREATE_WINDOW && IsWindowUnicode((HWND)wParam)))
	{
		return CallNextHookEx(hhook, nCode, wParam, lParam);
	}
	// for createwindow ansi window : 
	WCHAR ClassNameBuffer[MAX_PATH];
	HWND hwnd = (HWND)wParam;
	if (GetClassNameW(hwnd, ClassNameBuffer, sizeof(ClassNameBuffer)) > 0) 
	{
		// not a speicified dialog, we have to hook the whole custom window msg proc!! 
		if (lstrcmpiW(L"NewDlgClass", ClassNameBuffer) != 0) 
		{
			HookWindowProc(p, hwnd);
		}
		// maybe more ??? 
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

INLINE void InstallCbtHook(NTLEA_TLS_DATA * ptls) { // ebx 
	// most hook-based Locale Emulator used to hook : computer-based training (CBT) message  
	// for more info, see : http://msdn.microsoft.com/en-us/library/windows/desktop/ms644977(v=vs.85).aspx
	if (ptls->hWindowHooking == 0) { // use tls-reference on window hooking, only unblock the most outside !
		ptls->hWindowCbtHookAnsi = SetWindowsHookExA(WH_CBT, CbtHookProcA, NULL, GetCurrentThreadId());
		ptls->hWindowCbtHookUnicode = SetWindowsHookExW(WH_CBT, CbtHookProcW, NULL, GetCurrentThreadId());
	}
	ptls->hWindowHooking++;
}

INLINE void UninstallCbtHook(NTLEA_TLS_DATA * ptls) { // ebx 
	ptls->hWindowHooking--; // guard 
	if (ptls->hWindowHooking == 0) 
	{
		UnhookWindowsHookEx(ptls->hWindowCbtHookAnsi);
		UnhookWindowsHookEx(ptls->hWindowCbtHookUnicode);
	}
	else if (ptls->hWindowHooking < 0)
	{
		// reference error occured : 
		ShowUnhandledExceptionMessage(szAppCallRefUnmatch);
	}
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
	// 2. createwindow unicode, each window msghandler is unrelated : http://hi.baidu.com/tiancao222/item/d2f0dc370617dff3e6bb7a61
	HWND hwnd = CreateWindowExW(dwExStyle, (lpClassNameW ? lpClassNameW : (LPCWSTR)lpClassName), lpWindowNameW, dwStyle,
		x, y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
	//	ntprintfA(128, 1, "%p, %p - %p, %p\n", lpClassName, lpWindowName, lpClassNameW, lpWindowNameW);
	// 3. unprepare the thread hook
	UninstallCbtHook(p);
	if (lpWindowNameW) FreeStringInternal((LPVOID)lpWindowNameW);
	if (lpClassNameW) FreeStringInternal((LPVOID)lpClassNameW);
	// 4. cleanup resource 
	p->CurrentCallType = PrevCallType;
	return hwnd;
}

INLINE NTLEA_TLS_DATA* GetTlsValueInternal()
{
	DWORD n = GetLastError();
	NTLEA_TLS_DATA* p = (NTLEA_TLS_DATA*)TlsGetValue(settings.nTlsIndex);
	SetLastError(n); // thus the tlsgetvalue won't affect the env 
	if (!p) 
	{
		p = (NTLEA_TLS_DATA*)AllocateZeroedMemory(sizeof(NTLEA_TLS_DATA));
		TlsSetValue(settings.nTlsIndex, p);
		for (int i = 0; i < MAXSYSCLASSDESC; ++i) 
		{
			WNDCLASSA wndclassa;
			if (GetClassInfoA(NULL, SystemClassNameA[i], &wndclassa))
			{
				p->SystemClassDesc[i].AnsiSystemClassProc = wndclassa.lpfnWndProc;
			}
			WNDCLASSW wndclassw;
			if (GetClassInfoW(NULL, SystemClassNameW[i], &wndclassw)) 
			{
				p->SystemClassDesc[i].UnicodeSystemClassProc = wndclassw.lpfnWndProc;
			}
			//	ntprintfA(256, 1, "info: %s - %p %p\n", SystemClassNameA[i], wndclassa.lpfnWndProc, wndclassw.lpfnWndProc);
		}
		SetLastError(0); // also restore the errorstate !
	}
	return p;
}

/************************************************************/

HRESULT WINAPI InitWindowUnicodeLayer()
{

}


#endif
