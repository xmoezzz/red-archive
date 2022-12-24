#include "PluginHook.h"
#include "ImageChecker.h"

typedef HRESULT(WINAPI *tlink)(iTVPFunctionExporter *);
PVOID pV2Link = NULL;


HRESULT WINAPI WarppedV2Link(iTVPFunctionExporter *exporter)
{
	TVPInitImportStub(exporter);
	XmoeTVPFunctionExporter = exporter;
	DebugInfo(L"Inited Linker");
	PluginHook::HookCreateStream();
	return ((tlink)pV2Link)(exporter);
}

ULONG GetFileLen(LPVOID pBaseaddr, LPVOID pReadBuf)
{
	LPBYTE pBase = (LPBYTE)pBaseaddr;
	PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)pReadBuf;
	ULONG uSize = PIMAGE_OPTIONAL_HEADER((pBase + pDosHeader->e_lfanew + 4 + 20))->SizeOfHeaders;
	PIMAGE_SECTION_HEADER    pSec = (PIMAGE_SECTION_HEADER)(pBase + pDosHeader->e_lfanew + sizeof(IMAGE_NT_HEADERS));
	for (int i = 0; i<PIMAGE_FILE_HEADER(pBase + pDosHeader->e_lfanew + 4)->NumberOfSections; ++i)
	{
		uSize += pSec[i].SizeOfRawData;
	}
	return uSize;
}

HRESULT WINAPI PluginHook::InitHook(const char* ModuleName, HMODULE ImageBase)
{
	if (strstr(ModuleName, "psbfile.dll"))
	{
		DWORD ImageStart = (DWORD)ImageBase;
		DWORD Len = GetFileLen(ImageBase, ImageBase);
		InitPsbDllArea(ImageStart, Len);
#ifdef DebugMode
		DebugInfo(L"Psb Info Inited!");
#endif
	}
	if (PluginHookInited == TRUE)
	{
		return S_OK;
	}
	if (ImageBase == NULL)
	{
		return S_FALSE;
	}
	ULONG Strlen = lstrlenA(ModuleName);
	if (Strlen <= 3)
	{
		return S_FALSE;
	}
	Strlen--;
	if ((ModuleName[Strlen] != 'm' || ModuleName[Strlen - 1] != 'p' || ModuleName[Strlen - 2] != 't') &&
		(ModuleName[Strlen] != 'l' || ModuleName[Strlen - 1] != 'l' || ModuleName[Strlen - 2] != 'd'))
	{
		return S_FALSE;
	}
	tlink v2link = (tlink)GetProcAddress(ImageBase, "V2Link");
	if (v2link == NULL)
	{
		return S_FALSE;
	}
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	pV2Link = DetourFindFunction(ModuleName, "V2Link");
	DetourAttach(&pV2Link, WarppedV2Link);
	DetourTransactionCommit();
	PluginHookInited = TRUE;

	DebugInfo(L"Inited Filter");
	return S_OK;
}

IStream* __stdcall MyTVPCreateStream(const ttstr & _name, tjs_uint32 flags)
{
	ULONG ReturnAddr = (ULONG)_ReturnAddress();
	BOOL isOk = IsVaildArea(ReturnAddr);

	static WCHAR* Prefix = L"archive://";
	IStream* st = nullptr;

#ifdef DebugMode
	if ((!wcsncmp(_name.c_str(), Prefix, lstrlenW(Prefix))))
	{
		DebugInfo((wstring(L"Try to Create Stream (No Check) : ") + _name.c_str()).c_str());
	}
#endif

	if ((!wcsncmp(_name.c_str(), Prefix, lstrlenW(Prefix))) && isOk)
	{
		if (GetImageCheck() != S_OK)
		{
			st = nullptr;
		}
		else
		{
			DebugInfo((wstring(L"Try to Create Stream : ") + _name.c_str()).c_str());
			st = XmoeCreateStream(_name, flags);
		}
	}
	if (st == nullptr)
	{
		st = pfCreateIStream(_name, flags);
	}
	return st;
}

HRESULT WINAPI PluginHook::HookCreateStream()
{
	if (XmoeTVPFunctionExporter == nullptr)
	{
		MessageBoxW(NULL, L"Bad Refer", L"X'moe CoreLib", MB_OK);
		ExitProcess(-1);
	}

	static char szCreateIStream[] = "IStream * ::TVPCreateIStream(const ttstr &,tjs_uint32)";
	pfCreateIStream = (FuncCreateIStream)TVPGetImportFuncPtr(szCreateIStream);

	if (pfCreateIStream == nullptr)
	{
		MessageBoxW(NULL, L"Bad Refer", L"X'moe CoreLib", MB_OK);
		ExitProcess(-1);
	}

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach((void**)&pfCreateIStream, MyTVPCreateStream);
	DetourTransactionCommit();

	DebugInfo(L"Inited Stream");

	return S_OK;
}
