#ifndef _InitWindowUnicodeLayer_
#define _InitWindowUnicodeLayer_

#include <Windows.h>
#include "NtDef.h"

#define CT_CREATE_NULL			0	// used for checking system create without known yet!
#define CT_CREATE_WINDOW        1	// used by any window
#define CT_CREATE_DIALOG        2	// used by dialog & dialogindirect series
#define CT_CREATE_PRESET		3	// used by SHBrowseForFolderA

#define WM_UNKNOWN				0x43E

#define WM_CLASSMASK				0xFFFF0000

#if 0
HRESULT WINAPI InitWindowUnicodeLayer();
#endif

#endif
