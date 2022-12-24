#pragma once

//#include "constants.h"
#include <Windows.h>


#define RegIP						Eip
#define IMAGE_FILE_MACHINE_VALID	IMAGE_FILE_MACHINE_I386
#define IMAGE_FILE_MACHINE_INVALID	IMAGE_FILE_MACHINE_AMD64
#define IMAGE_FILE_MACHINE_ESTRING	L"x64"
#define WM_CLASSMASK				0xFFFF0000
#define szTranslation				"\\VarFileInfo\\Translation"

typedef struct { // same as CREATESTRUCT
	LPVOID    	lpParam;
	HINSTANCE	hInstance;
	HMENU		hMenu;
	HWND		hWndParent;
	INT			nHeight;
	INT			nWidth;
	INT			y;
	INT			x;
	DWORD		dwStyle;
	LPVOID		lpWindowName;	// convert !!
	LPVOID		lpClassName;	// convert !!
	DWORD		dwExStyle;
} CREATEWNDEX;

typedef struct { // same as MDICREATESTRUCT
	LPCSTR		szClass;
	LPCSTR		szTitle;
	HWND		hOwner;
	INT			x;
	INT			y;
	INT			cx;
	INT			cy;
	DWORD		style;
	LPARAM		lParam;
} CREATEMDIWND;

typedef struct {
	LONG		lfHeight;
	LONG		lfWidth;
	LONG		lfEscapement;
	LONG		lfOrientation;
	LONG		lfWeight;
	BYTE		lfItalic;
	BYTE		lfUnderline;
	BYTE		lfStrikeOut;
	BYTE		lfCharSet;
	BYTE		lfOutPrecision;
	BYTE		lfClipPrecision;
	BYTE		lfQuality;
	BYTE		lfPitchAndFamily;
} LOGFONTSIMILAR;

// --------------------------- 
// high level control message 
// ---------------------------

// see : http://chokuto.ifdef.jp/urawaza/message/

#ifndef TCITEM
typedef struct 
{
	UINT   mask;
	DWORD  dwState;
	DWORD  dwStateMask;
	LPVOID pszText;		// mask including 'TCIF_TEXT'
	int    cchTextMax;  // 
	int    iImage;
	LPARAM lParam;
} TCITEM, *LPTCITEM;
#endif


#ifndef LVCOLUMN
typedef struct
{
	UINT   mask;
	int	   fmt;
	int    cx;
	LPVOID pszText;		// mask including 'TCIF_TEXT'
	int    cchTextMax;	// 
	int    iSubItem;
} LVCOLUMN, *LPLVCOLUMN;
#endif


#ifndef LVITEM
typedef struct 
{
	UINT   mask;
	int    iItem;
	int    iSubItem;
	UINT   state;
	UINT   stateMask;
	LPVOID pszText;		// mask including 'LVIF_TEXT'
	int    cchTextMax;	// 
	int    iImage;
	LPARAM lParam;
} LVITEM, *LPLVITEM;
#endif


#ifndef TVITEM
typedef struct 
{
	UINT    mask;
	HANDLE	hItem;
	UINT    state;
	UINT    stateMask;
	LPVOID  pszText;	// mask including 'LVIF_TEXT'
	int     cchTextMax;	// 
	int     iImage;
	int     iSelectedImage;
	int     cChildren;
	LPARAM  lParam;
	//	int     iIntegral;	// if ex 
} TVITEM, *LPTVITEM;
#endif

#ifndef TVINSERTSTRUCT
typedef struct
{
	HANDLE  hParent;
	HANDLE  hInsertAfter;
	TVITEM	item;		// TVITEM or TVITEMEX
} TVINSERTSTRUCT, *LPTVINSERTSTRUCT;
#endif

#if defined(_M_IX86)
#define __unaligned
#endif//__unaligned

typedef struct _ITEMIDLIST			__unaligned	*LPITEMIDLIST;
typedef struct _ITEMIDLIST const	__unaligned *LPCITEMIDLIST;

#if defined(_M_IX86)
#undef  __unaligned
#endif//__unaligned

#ifndef PIDLIST_ABSOLUTE         
#define PIDLIST_ABSOLUTE        LPITEMIDLIST
#endif//PIDLIST_ABSOLUTE         
#ifndef PCIDLIST_ABSOLUTE
#define PCIDLIST_ABSOLUTE       LPCITEMIDLIST
#endif//PCIDLIST_ABSOLUTE
#ifndef PCUIDLIST_ABSOLUTE
#define PCUIDLIST_ABSOLUTE      LPCITEMIDLIST
#endif//PCUIDLIST_ABSOLUTE

typedef struct {
	HWND				hwndOwner;
	PCIDLIST_ABSOLUTE	pidlRoot;
	LPCVOID				pszDisplayName;			// Return display name of item selected.
	LPCVOID				lpszTitle;				// text to go in the banner over the tree.
	UINT				ulFlags;				// Flags that control the return stuff
	LPVOID				lpfn;
	LPARAM				lParam;					// extra info that's passed back in callbacks
	int					iImage;					// output var: where to return the Image index.
} BROWSEINFO, *LPBROWSEINFO;

// --------------------- 

typedef struct {
	WNDPROC		AnsiSystemClassProc;
	WNDPROC		UnicodeSystemClassProc;
} SYSTEM_CLASS_WNDPROC;

typedef struct {
	DWORD		InternalCall;
	DWORD		DBCSLeadByte;
	DWORD		IsFontAvailable;
	DWORD		CurrentCallType;
	DLGPROC		DialogProc;
	DWORD		IsCreateFileCall;
	LONG		hWindowHooking; // counter for avoiding setup/uninst hook-recursive
	HHOOK		hWindowCbtHookAnsi;
	HHOOK		hWindowCbtHookUnicode;
	SYSTEM_CLASS_WNDPROC SystemClassDesc[15];
} NTLEA_TLS_DATA;

typedef struct {
	WNDPROC		PrevAnsiWindowProc;
	WNDPROC		PrevUnicodeWindowProc;
} NTLEA_WND_ASC_DATA;

typedef struct {
	WORD		wLanguage;
	WORD		wCodePage;
} LANGANDCODEPAGE;

typedef struct {
	LPVOID		fontenumproc;
	LPARAM		fontenumpara;
	HDC			fontenumhdc;
	BYTE		fontenumface[LF_FACESIZE * 2];
	BYTE		fontenumname[LF_FACESIZE * 2];
} FONTENUMPROCPARAM;

// ------------------- 

typedef struct {
	// -------------- inputparam 
	DWORD dwCompOption;
	DWORD dwCodePage;
	DWORD dwLCID;
	DWORD dwTimeZone;
	DWORD dwFontSizePercent;
	BYTE lpFontFaceName[LF_FACESIZE];
	// -------------- internal 
	HANDLE RcpEvent, RcpFileMap;
	LPVOID FileMappingAddress, ImageBase, EntryPoint;
	HMODULE hInstance;

	HANDLE hHeap;
	LANGANDCODEPAGE langcodepage;
	int   lfcharset, lfcharold;

	DWORD dwThreadIdMBtoWC;
	DWORD dwThreadIdWCtoMB;
	///	DWORD MbCodePageTag;
	///	DWORD nOSVer;
	DWORD nTlsIndex;
	// --- 
	LONG  bInternalLockMBtoWC;
	LONG  bInternalLockWCtoMB;
	LONG  bInternalLockCreateProc;
	LONG  bInternalLockVQV;
	//	LONG  bInternalLockRegClass;
	LONG  bInternalLockCreateAtom;
	// --- 
	int	  bNtleaAtomInvoked;
	int	  bInternalCallMBtoWC;
	int	  bInternalCallWCtoMB;
	int	  bErrorFlag;
	int	  bCreateProcUniFunc;
	int	  bSuspendThreadFlag;
	// bits : 
	int	  bForceSpecifyFont;	//  0
	int	  bCreateProcNative;	//  1
	int	  bChinesePath;			//  2
	int	  bNoFilterCommCtrl;	//  3
	int	  bNoAutoMatchLocal;	//  4
	int	  bMinidumpHelpFile;	// 15
	// ------------------------------- 
	LOGFONTSIMILAR logfontw;
} Settings;

#define CT_CREATE_NULL			0	// used for checking system create without known yet!
#define CT_CREATE_WINDOW        1	// used by any window
#define CT_CREATE_DIALOG        2	// used by dialog & dialogindirect series
#define CT_CREATE_PRESET		3	// used by SHBrowseForFolderA

#define WM_UNKNOWN				0x43E


#define TCIF_TEXT			0x0001
#define TCM_ADD_UNICODE		55
#define TCM_GETITEMA		0x1305
#define TCM_GETITEMW		0x133C
#define TCM_SETITEMA		0x1306
#define TCM_SETITEMW		0x133D
#define TCM_INSERTITEMA		0x1307
#define TCM_INSERTITEMW		0x133E

// ------ ListView ------
#define LVIF_TEXT			0x0001
#define LVCF_TEXT			0x0004
#define LVM_ADD_UNICODE		70
#define LVM_GETITEMA		0x1005
#define LVM_GETITEMW		0x104B
#define LVM_SETITEMA		0x1006
#define LVM_SETITEMW		0x104C
#define LVM_INSERTITEMA		0x1007
#define LVM_INSERTITEMW		0x104D
#define LVM_GETCOLUMNA		0x1019
#define LVM_GETCOLUMNW		0x105F
#define LVM_SETCOLUMNA		0x101A
#define LVM_SETCOLUMNW		0x1060
#define LVM_INSERTCOLUMNA	0x101B
#define LVM_INSERTCOLUMNW	0x1061

// ------ TreeView ------
#define TVIF_TEXT			0x0001
#define TVM_ADD_UNICODE		50
#define TVM_GETITEMA		0x110C
#define TVM_GETITEMW		0x113E
#define TVM_SETITEMA		0x110D
#define TVM_SETITEMW		0x113F
#define TVM_INSERTITEMA		0x1100
#define TVM_INSERTITEMW		0x1132

typedef LRESULT(CALLBACK *SUBCLASSPROC)(
	HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData
	);

BOOL WINAPI RunDllMain(HINSTANCE, DWORD, LPVOID);
VOID FreeStringInternal(LPVOID pBuffer);


LRESULT WINAPI
HookCallWindowProc(WNDPROC PrevWindowProc, HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT WINAPI
HookSendMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT WINAPI
HookPostMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT WINAPI
HookSendNotifyMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT WINAPI
HookSendMessageCallback(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, SENDASYNCPROC lpCallBack, ULONG_PTR dwData);
LRESULT WINAPI
HookSendMessageTimeout(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT fuFlags, UINT uTimeout, PDWORD_PTR lpdwResult);
INT_PTR WINAPI
HookDialogBoxIndirectParam(HINSTANCE hInstance, LPCDLGTEMPLATE hDialogTemplate, HWND hWndParent, DLGPROC lpDialogFunc, LPARAM dwInitParam);
HWND WINAPI
HookCreateDialogIndirectParam(HINSTANCE hInstance, LPCDLGTEMPLATE lpTemplate, HWND hWndParent, DLGPROC lpDialogFunc, LPARAM lParamInit);
INT_PTR WINAPI
HookDialogBoxParam(HINSTANCE hInstance, LPCSTR lpTemplateName, HWND hWndParent, DLGPROC lpDialogFunc, LPARAM dwInitParam);
HWND WINAPI
HookCreateDialogParam(HINSTANCE hInstance, LPCSTR lpTemplateName, HWND hWndParent, DLGPROC lpDialogFunc, LPARAM dwInitParam);
NTSTATUS WINAPI
HookUnicodeToMultiByte(LPSTR AnsiBuffer, DWORD MultiByteLength, LPDWORD lpNumberOfBytesConverted, LPCWSTR UnicodeBuffer, DWORD WideCharLength);
NTSTATUS WINAPI
HookMultiByteToUnicode(LPWSTR UnicodeBuffer, DWORD WideCharLength, DWORD* lpNumberOfBytesConverted, LPCSTR AnsiBuffer, DWORD MultiByteLength);
NTSTATUS WINAPI
HookUnicodeToMultiByteSize(int* lpNumberOfBytesConverted, LPCWSTR UnicodeBuffer, int WideCharLength);
NTSTATUS WINAPI
HookMultiByteToUnicodeSize(int* lpNumberOfBytesConverted, LPCSTR AnsiBuffer, int MultiByteLength);
BOOL WINAPI
HookIsDBCSLeadByte(BYTE TestChar);
LPSTR WINAPI
HookCharPrev(LPCSTR lpStart, LPCSTR lpCurrentChar);
LPSTR WINAPI
HookCharNext(LPCSTR lpCurrentChar);
HGDIOBJ WINAPI
HookGetStockObject(int fnObject);
HFONT WINAPI
HookCreateFontIndirect(const LOGFONT *lplf);
int WINAPI
HookGetMenuString(HMENU hMenu, UINT uIDItem, LPSTR lpString, int nMaxCount, UINT uFlag);
BOOL WINAPI
HookGetMenuItemInfo(HMENU hMenu, UINT uItem, BOOL fByPosition, LPMENUITEMINFOA lpmii);
BOOL WINAPI
HookSetMenuItemInfo(HMENU hMenu, UINT uItem, BOOL fByPosition, LPMENUITEMINFOA lpmii);
int WINAPI
HookEnumFontFamiliesExA(HDC hdc, LPLOGFONTA lpLogfont, FONTENUMPROCA lpEnumFontFamExProc, LPARAM lParam, DWORD dwFlags);
int WINAPI
HookEnumFontFamiliesExW(HDC hdc, LPLOGFONTW lpLogfont, FONTENUMPROCW lpEnumFontFamExProc, LPARAM lParam, DWORD dwFlags);
int WINAPI
HookEnumFontFamiliesA(HDC hdc, LPCSTR lpszFamily, FONTENUMPROCA lpEnumFontFamProc, LPARAM lParam);
int WINAPI
HookEnumFontFamiliesW(HDC hdc, LPCWSTR lpszFamily, FONTENUMPROCW lpEnumFontFamProc, LPARAM lParam);
int WINAPI
HookEnumFontsA(HDC hdc, LPCSTR lpFaceName, FONTENUMPROCA lpFontFunc, LPARAM lParam);
int WINAPI
HookEnumFontsW(HDC hdc, LPCWSTR lpFaceName, FONTENUMPROCW lpFontFunc, LPARAM lParam);
HWND WINAPI
HookCreateWindowEx(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, int x, int y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam);
int WINAPI
HookGetWindowText(HWND hWindow, LPSTR lpString, int nMaxCount);
BOOL WINAPI
HookSetWindowText(HWND hWindow, LPCSTR lpstrText);
LONG_PTR WINAPI
HookGetWindowLongPtr(HWND hWnd, int nIndex);
LONG_PTR WINAPI
HookSetWindowLongPtr(HWND hWnd, int nIndex, LONG_PTR dwNewLong);
LONG_PTR WINAPI
HookSetWindowLongPtrW(HWND hWnd, int nIndex, LONG_PTR dwNewLong);
LRESULT WINAPI
HookDefWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT WINAPI
HookDefMDIChildProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT WINAPI
HookDefDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT WINAPI
HookDefFrameProc(HWND hWnd, HWND hWndMDIClient, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL WINAPI
HookVerQueryValue(LPCVOID pBlock, LPCSTR lpSubBlock, LPVOID *lplpBuffer, PUINT puLen);
DWORD WINAPI
HookGetTimeZoneInformation(LPTIME_ZONE_INFORMATION lpTimeZoneInformation);
int WINAPI
HookCompareString(LCID Locale, DWORD dwCmpFlags, LPCSTR lpString1, int cchCount1, LPCSTR lpString2, int cchCount2);
HANDLE WINAPI
HookCreateFile(LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile);
int WINAPI
HookMBtoWC(UINT CodePage, DWORD dwFlags, LPCSTR lpMultiByteStr, int cbMultiByte, LPWSTR lpWideCharStr, int cchWideChar);
int WINAPI
HookWCtoMB(UINT CodePage, DWORD dwFlags, LPCWSTR lpWideCharStr, int cchWideChar, LPSTR lpMultiByteStr, int cbMultiByte, LPCSTR lpDefaultChar, LPBOOL lpUsedDefaultChar);
LCID WINAPI
HookGetLocaleID(void);
BOOL WINAPI
HookGetCPInfo(UINT CodePage, LPCPINFO lpCPInfo);
UINT WINAPI
HookGetACP(void);
UINT WINAPI
HookGdiGetCodePage(HDC hdc);


