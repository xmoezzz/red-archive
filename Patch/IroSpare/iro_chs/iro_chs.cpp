#include "my.h"

//ML_OVERLOAD_NEW

#include "ShinkuHook.h"
#include "tp_stub.h"
#include "KAGParser.h"

#pragma comment(linker, "/SECTION:.text,ERW /MERGE:.rdata=.text /MERGE:.data=.text")
#pragma comment(linker, "/SECTION:.Xmoe,ERW /MERGE:.text=.Xmoe")

static tjs_int GlobalRefCountAtInit = 0;

typedef Void(NTAPI* GetInfo)(PBYTE Info);

Void NTAPI InitDebugPort()
{
	ShinkuHook* Handle;
	ULONG       Attribute;
	PVOID       Module;
	GetInfo     StubGetInfo;
	BYTE        InfoArray[0x8];
	
	LOOP_ONCE
	{
		Handle    = ShinkuHook::GetHook();
		Attribute = Nt_GetFileAttributes(L"XmoeDebug.dll");
		if (Attribute == 0xFFFFFFFFUL)
			break;

		Module = Nt_LoadLibrary(L"XmoeDebug.dll");
		if (!Module)
			break;

		StubGetInfo = (GetInfo)Nt_GetProcAddress(Module, "GetInfo");
		if (!StubGetInfo)
			break;

		StubGetInfo(InfoArray);
		if (*(PDWORD)InfoArray == TAG4('xMoE') && *(PDWORD)(InfoArray + 4) == TAG4('lNik'))
			Handle->DebugPort = TRUE;

		LdrUnloadDll(Module);
	}
}

BOOL NTAPI DllMain(HMODULE hModule, DWORD Reason, LPVOID lpReserved)
{
	switch (Reason)
	{
	case DLL_PROCESS_ATTACH:
		InitDebugPort();
		ml::MlInitialize();
		if (ShinkuHook::GetHook()->DebugPort) 
			AllocConsole();
		if (NT_FAILED(ShinkuHook::GetHook()->Init(hModule)))
			Ps::ExitProcess(0x233);
		break;
	case DLL_THREAD_ATTACH:
		ShinkuHook::GetHook()->NotifyThreadAttach(hModule);
		break;
	case DLL_THREAD_DETACH:
		ShinkuHook::GetHook()->NotifyThreadDetach(hModule);
		break;
	case DLL_PROCESS_DETACH:
		ShinkuHook::GetHook()->UnInit(hModule);
		ml::MlUnInitialize();
		break;
	}
	return TRUE;
}

#pragma comment(linker, "/EXPORT:GetPrivateInfo=_GetPrivateInfo@8,PRIVATE")
EXTERN_C MY_DLL_EXPORT Void NTAPI GetPrivateInfo(PBYTE Buffer, ULONG Size)
{
	static Byte Info[8] = { 0xCF, 0x12, 0x48, 0xF1, 0x1A, 0x22, 0x1A, 0xC9 };

	RtlCopyMemory(Buffer, Info, Size);
}

#pragma comment(linker, "/EXPORT:V2Link=_V2Link@4,PRIVATE")
EXTERN_C MY_DLL_EXPORT HRESULT NTAPI V2Link(iTVPFunctionExporter *exporter)
{
	TVPInitImportStub(exporter);

	tTJSVariant val;

	iTJSDispatch2 * global = TVPGetScriptDispatch();

	{
		iTJSDispatch2 * tjsclass = TVPCreateNativeClass_KAGParser();
		val = tTJSVariant(tjsclass);
		tjsclass->Release();
		global->PropSet(TJS_MEMBERENSURE, TJS_W("KAGParser"), NULL, &val, global);
	}

	global->Release();
	val.Clear();

	GlobalRefCountAtInit = TVPPluginGlobalRefCount;

	return S_OK;
}

#pragma comment(linker, "/EXPORT:V2Unlink=_V2Unlink@0,PRIVATE")
EXTERN_C MY_DLL_EXPORT HRESULT _stdcall V2Unlink()
{
	if (TVPPluginGlobalRefCount > GlobalRefCountAtInit) return E_FAIL;

	iTJSDispatch2 * global = TVPGetScriptDispatch();

	if (global)
	{
		global->DeleteMember(0, TJS_W("KAGParser"), NULL, global);
	}

	if (global) global->Release();

	TVPUninitImportStub();

	return S_OK;
}

