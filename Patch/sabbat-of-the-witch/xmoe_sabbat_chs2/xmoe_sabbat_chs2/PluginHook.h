#ifndef _PluginHook_
#define _PluginHook_

#include <Windows.h>
#include "tp_stub.h"
#include "detours.h"
#include "ComStreamRefer.h"
#include "DebugTool.h"
#include <string>
#include "WinFile.h"

using std::string;

#pragma comment(lib, "detours.lib")

static BOOL PluginHookInited = FALSE;
static iTVPFunctionExporter* XmoeTVPFunctionExporter = nullptr;

typedef IStream * (__stdcall * FuncCreateIStream)(const ttstr &, tjs_uint32);
static FuncCreateIStream pfCreateIStream = nullptr;

class PluginHook
{
public:
	static HRESULT WINAPI InitHook(const char* ModuleName, HMODULE ImageBase);
	static HRESULT WINAPI HookCreateStream();
};

#endif
