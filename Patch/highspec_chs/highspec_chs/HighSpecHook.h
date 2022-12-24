#pragma once

#include <Windows.h>
#include <ntstatus.h>
#include "tp_stub.h"
#include "NtDefine.h"

typedef HRESULT   (WINAPI * FuncV2Link)       (iTVPFunctionExporter *);
typedef IStream * (WINAPI * FuncCreateIStream)(const ttstr &, tjs_uint32);


class HighSpecHook
{
	HighSpecHook();
	static HighSpecHook* m_Handle;

public:

	static HighSpecHook* GetData();

	NTSTATUS InitHook(LPCSTR ModuleName, HMODULE ImageBase);
	NTSTATUS HookCreateStream();
	NTSTATUS InitNativeHook();
	NTSTATUS StartHook(LPCSTR szDllName, PROC pfnOrg, PROC pfnNew);

	IStream* LocalCreateStream(const ttstr & _name, tjs_uint32 flags);

	BOOL                             m_HookInited;
	FuncCreateIStream                OldCreateIStream;
	FuncV2Link                       OldV2Link;
	iTVPFunctionExporter*            XmoeTVPFunctionExporter;
	API_POINTER(MultiByteToWideChar) OldMultiByteToWideChar;
};
