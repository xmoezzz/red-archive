#include "PluginHook.h"
#include "ImageChecker.h"
#include "Charset.h"

typedef HRESULT(WINAPI *tlink)(iTVPFunctionExporter *);
PVOID pV2Link = NULL;


typedef tjs_int(__stdcall * FuncUtf8ToWide)(const char *, tjs_char *);
static FuncUtf8ToWide pfUtf8ToWide = nullptr;

static BYTE StreamHooked[5] = { 0 };
static BYTE Utf8ConvHooked[5] = { 0 };

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
	//pV2Link = DetourFindFunction(ModuleName, "V2Link");
	pV2Link = GetProcAddress(ImageBase, "V2Link");
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

	if ((!wcsncmp(_name.c_str(), Prefix, lstrlenW(Prefix))) && isOk && memcmp(pfCreateIStream, StreamHooked, 5) == 0)
	{
		//if (FAILED(GetImageCheck()))
		//{
		//	st = nullptr;
		//}
		//else
		//{
			//DebugInfo((wstring(L"Try to Create Stream : ") + _name.c_str()).c_str());
			st = XmoeCreateStream(_name, flags);
		//}
	}
	if (st == nullptr)
	{
		st = pfCreateIStream(_name, flags);
	}
	return st;
}


#define XmoeStringStart 0xFFFFFFFEUL
#define XmoeStringEnd   0xFFFFFFFFUL

typedef union DwordPack
{
	unsigned int  DwordPart;
	unsigned char BytePart[4];
}DwordPack;


template<class T>
T CPP_ROL(T n, const int bitN)
{
	const int BITLEN = sizeof(T) * 8;
	n = (n >> (BITLEN - bitN)) | (n << bitN);
	return n;
}

template<class T>
T CPP_ROR(T n, const int bitN)
{
	const int BITLEN = sizeof(T) * 8;
	n = (n << (BITLEN - bitN)) | (n >> bitN);
	return n;
}


unsigned char CPP_ROL_8(unsigned char n, const int bitN)
{
	const int BITLEN = sizeof(unsigned char) * 8;
	n = (n >> (BITLEN - bitN)) | (n << bitN);
	return n;
}


unsigned char CPP_ROR_8(unsigned char n, const int bitN)
{
	const int BITLEN = sizeof(unsigned char) * 8;
	n = (n << (BITLEN - bitN)) | (n >> bitN);
	return n;
}


void Decode(unsigned char* code, unsigned int size)
{
	for (unsigned int i = 0; i < size; i++)
	{
		code[i] = CPP_ROR_8(code[i], 2);
	}
}



string DecodeString(const char* code)
{
	if (code == nullptr || strlen(code) <= 4)
		return string(code);

	string PackStr;

	static unsigned char DecodeArea[2000];

	unsigned int iPos = 0;
	while (iPos < strlen(code))
	{
		if (*(unsigned int*)(code + iPos) == XmoeStringStart)
		{
			iPos += 4;
			unsigned int Ptr = 0;

			memset(DecodeArea, 0, sizeof(DecodeArea));
			while (iPos < strlen(code))
			{
				if (*(unsigned int*)(code + iPos) == XmoeStringEnd)
				{
					iPos += 4;
					break;
				}
				else
				{
					DecodeArea[Ptr] = code[iPos];
					iPos++;
					Ptr++;
				}
			}
			Decode(DecodeArea, strlen((char*)DecodeArea));
			PackStr += (char*)DecodeArea;
		}
		else
		{
			PackStr += code[iPos];
			iPos++;
		}
	}
	return PackStr;
}


tjs_int __stdcall MyUtf8ToWide(const char * in, tjs_char * out)
{
	ULONG ReturnAddr = (ULONG)_ReturnAddress();
	BOOL isOk = IsVaildArea(ReturnAddr);

	string DecStr = DecodeString(in);

	if (DecStr.length() != strlen(in))
	{
		if (!isOk || memcmp(pfUtf8ToWide, Utf8ConvHooked, 5))
		{
			return AnzUtf8ToWideCharString("", out);
		}
	}
	return AnzUtf8ToWideCharString(DecStr.c_str(), out);
}


LONG WINAPI FakeModuleFilter(_EXCEPTION_POINTERS* excp)
{
	MessageBoxW(NULL, L"Fake Module", L"X'moe CoreLib", MB_OK);
	__asm int 2;
	ExitProcess(0);
	return  EXCEPTION_EXECUTE_HANDLER;
}


HRESULT WINAPI PluginHook::HookCreateStream()
{
	if (XmoeTVPFunctionExporter == nullptr)
	{
		MessageBoxW(NULL, L"Bad Refer", L"X'moe CoreLib", MB_OK);
		ExitProcess(-1);
	}

	auto OldFilter = SetUnhandledExceptionFilter(FakeModuleFilter);

	static char szCreateIStream[] = "IStream * ::TVPCreateIStream(const ttstr &,tjs_uint32)";
	pfCreateIStream = (FuncCreateIStream)TVPGetImportFuncPtr(szCreateIStream);

	if (pfCreateIStream == nullptr)
	{
		MessageBoxW(NULL, L"Bad Refer", L"X'moe CoreLib", MB_OK);
		ExitProcess(-1);
	}
	else
	{
		if (((PBYTE)pfCreateIStream)[0] == 0xE9)
		{
			__asm int 2;
		}
	}

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach((void**)&pfCreateIStream, MyTVPCreateStream);
	DetourTransactionCommit();

	RtlCopyMemory(StreamHooked, pfCreateIStream, 5);

	static char szUtf8ToWide[] = "tjs_int ::TVPUtf8ToWideCharString(const char *,tjs_char *)";
	pfUtf8ToWide = (FuncUtf8ToWide)TVPGetImportFuncPtr(szUtf8ToWide);
	
	if (pfUtf8ToWide == nullptr)
	{
		MessageBoxW(NULL, L"Bad Refer", L"X'moe CoreLib", MB_OK);
		ExitProcess(-1);
	}
	else
	{
		if (((PBYTE)pfUtf8ToWide)[0] == 0xE9)
		{
			__asm int 2;
		}
	}

	SetUnhandledExceptionFilter(OldFilter);

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach((void**)&pfUtf8ToWide, MyUtf8ToWide);
	DetourTransactionCommit();
	
	RtlCopyMemory(Utf8ConvHooked, pfUtf8ToWide, 5);

	DebugInfo(L"Inited Stream");

	return S_OK;
}
