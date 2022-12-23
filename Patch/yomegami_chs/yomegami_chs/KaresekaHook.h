#pragma once

#include "my.h"
#include "tp_stub.h"
#include "IStreamExXP3.h"
#include "StreamHolderXP3.h"

typedef HRESULT(NTAPI *FuncV2Link)(iTVPFunctionExporter *);
typedef tTJSBinaryStream* (FASTCALL * FuncCreateStream)(const ttstr &, tjs_uint32);
typedef PVOID(CDECL * FuncHostAlloc)(ULONG);

class KaresekaHook
{
public:
	KaresekaHook();
	static KaresekaHook* Handle;

	BOOL     Init(HMODULE hModule);
	BOOL     UnInit();
	IStream* CreateLocalStream(LPCWSTR lpFileName);
	NTSTATUS InitKrkrHook(LPCWSTR lpFileName, PVOID Module);
	NTSTATUS QueryFile(LPCWSTR QueryPathName, LPCWSTR FileName, PBYTE& FileBuffer, ULONG& FileSize, ULONG64& Hash);

	FuncCreateStream      StubTVPCreateStream;
	FuncHostAlloc         StubHostAlloc;
	FuncV2Link            StubV2Link;
	ULONG_PTR             IStreamAdapterVtable;
	iTVPFunctionExporter* TVPFunctionExporter;
	PVOID                 m_SelfModule;

private:
	BOOL   Inited;
	BOOL   FileSystemInited;
};

KaresekaHook* FASTCALL GetKareseka();
