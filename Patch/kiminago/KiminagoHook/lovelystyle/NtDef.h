#ifndef _NtDef_
#define _NtDef_

#include <Windows.h>

#ifndef INLINE
#define INLINE __inline
#endif//INLINE


#define MAXSYSCLASSDESC 15

typedef struct
{ // same as CREATESTRUCT
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

typedef struct
{ // same as MDICREATESTRUCT
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

typedef struct 
{
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

typedef struct
{
	UINT   mask;
	int	   fmt;
	int    cx;
	LPVOID pszText;		// mask including 'TCIF_TEXT'
	int    cchTextMax;	// 
	int    iSubItem;
} LVCOLUMN, *LPLVCOLUMN;

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

typedef struct 
{
	HANDLE  hParent;
	HANDLE  hInsertAfter;
	TVITEM	item;		// TVITEM or TVITEMEX
} TVINSERTSTRUCT, *LPTVINSERTSTRUCT;

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

typedef struct
{
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

typedef struct 
{
	WNDPROC		AnsiSystemClassProc;
	WNDPROC		UnicodeSystemClassProc;
} SYSTEM_CLASS_WNDPROC;

typedef struct
{
	DWORD		InternalCall;
	DWORD		DBCSLeadByte;
	DWORD		IsFontAvailable;
	DWORD		CurrentCallType;
	DLGPROC		DialogProc;
	DWORD		IsCreateFileCall;
	LONG		hWindowHooking; // counter for avoiding setup/uninst hook-recursive
	HHOOK		hWindowCbtHookAnsi;
	HHOOK		hWindowCbtHookUnicode;
	SYSTEM_CLASS_WNDPROC SystemClassDesc[MAXSYSCLASSDESC];
	//	WNDPROC		DynamicClassProc[MAXSYSCLASSDESC]; // runtime cache !?
} NTLEA_TLS_DATA;

typedef struct 
{
	WNDPROC		PrevAnsiWindowProc;
	WNDPROC		PrevUnicodeWindowProc;
} NTLEA_WND_ASC_DATA;

typedef struct 
{
	WORD		wLanguage;
	WORD		wCodePage;
} LANGANDCODEPAGE;

typedef struct 
{
	LPVOID		fontenumproc;
	LPARAM		fontenumpara;
	HDC			fontenumhdc;
	BYTE		fontenumface[LF_FACESIZE * 2];
	BYTE		fontenumname[LF_FACESIZE * 2];
} FONTENUMPROCPARAM;

#endif
