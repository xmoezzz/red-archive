#ifndef _ComStreamRefer_
#define _ComStreamRefer_

#include "tp_stub.h"
#include <string>
#include "ComStreamRefer.h"
#include "IStreamEx.h"
#include "WinFile.h"
#include "CMem.h"
#include "DebugTool.h"
#include "FileSystem.h"

using std::wstring;

IStream* WINAPI XmoeCreateStream(const ttstr & _name, tjs_uint32 flags);

static ULONG DllStart = 0;
static ULONG DllLength = 0;

VOID WINAPI InitPsbDllArea(ULONG S, ULONG L);

BOOL WINAPI IsVaildArea(ULONG Pos);

#endif
