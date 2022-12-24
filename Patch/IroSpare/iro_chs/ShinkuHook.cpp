#include "ShinkuHook.h"
#include "MyHook.h"
#include "twofish.h"
#include "DummyChecker.h"
#include "Base64.h"
#include "MyUtf8.h"
#include "AES.h"
#include "IStreamExXP3.h"
#include <string>

using std::wstring;

ShinkuHook* ShinkuHook::m_Inst = NULL;

ShinkuHook::ShinkuHook() :
	m_SelfModule(NULL),
	OldLdrLoadDll(NULL),
	Inited(FALSE),
	StubV2Link(NULL),
	TVPFunctionExporter(NULL),
	StubCreateIStream(NULL),
	DebugPort(FALSE)
{
}

ShinkuHook::~ShinkuHook()
{
}

NTSTATUS ShinkuHook::LoadDirectShowFilter()
{
	PVOID Module;

	LOOP_ONCE
	{
		Module = Nt_LoadLibrary(L"XmoeSplitter.ax");
		if (!Module)
			break;

		Module = Nt_LoadLibrary(L"XmoeAudio.ax");
		if (!Module)
			break;

		Module = Nt_LoadLibrary(L"XmoeVideo.ax");
		if (!Module)
			break;
	}
	return Module ? STATUS_SUCCESS : STATUS_UNSUCCESSFUL;
}

unsigned long long MurmurHash64B(const void * key, int len, unsigned int seed = 0xEE6B27EB)
{
	const unsigned int m = 0x5bd1e995;
	const int r = 24;

	unsigned int h1 = seed ^ len;
	unsigned int h2 = 0;

	const unsigned int * data = (const unsigned int *)key;

	while (len >= 8)
	{
		unsigned int k1 = *data++;
		k1 *= m; k1 ^= k1 >> r; k1 *= m;
		h1 *= m; h1 ^= k1;
		len -= 4;

		unsigned int k2 = *data++;
		k2 *= m; k2 ^= k2 >> r; k2 *= m;
		h2 *= m; h2 ^= k2;
		len -= 4;
	}

	if (len >= 4)
	{
		unsigned int k1 = *data++;
		k1 *= m; k1 ^= k1 >> r; k1 *= m;
		h1 *= m; h1 ^= k1;
		len -= 4;
	}

	switch (len)
	{
	case 3: h2 ^= ((unsigned char*)data)[2] << 16;
	case 2: h2 ^= ((unsigned char*)data)[1] << 8;
	case 1: h2 ^= ((unsigned char*)data)[0];
		h2 *= m;
	};

	h1 ^= h2 >> 18; h1 *= m;
	h2 ^= h1 >> 22; h2 *= m;
	h1 ^= h2 >> 17; h1 *= m;
	h2 ^= h1 >> 19; h2 *= m;

	unsigned long long h = h1;

	h = (h << 32) | h2;

	return h;
}


wstring FileNameToLower(LPWSTR FileName)
{
	wstring Result;

	for (ULONG i = 0; i < StrLengthW(FileName); i++)
	{
		if (FileName[i] <= 'Z' && FileName[i] >= 'A')
			Result += towlower(FileName[i]);
		else
			Result += FileName[i];
	}
	return Result;
}


ShinkuHook* ShinkuHook::GetHook()
{
	if (!m_Inst)
		m_Inst = new ShinkuHook;

	return m_Inst;
}


NTSTATUS ShinkuHook::NotifyThreadAttach(HMODULE hModule)
{
	NTSTATUS Status;

	LOOP_ONCE
	{
		Status = STATUS_SUCCESS;
	}
	return Status;
}

NTSTATUS ShinkuHook::NotifyThreadDetach(HMODULE hMoudle)
{
	NTSTATUS    Status;

	LOOP_ONCE
	{
		Status = STATUS_SUCCESS;
	}
	return Status;
}


std::wstring GetKrkrFileName(LPCWSTR Name)
{
	std::wstring Info(Name);

	if (Info.find_last_of(L">") != std::wstring::npos)
		Info = Info.substr(Info.find_last_of(L">") + 1, std::wstring::npos);

	if (Info.find_last_of(L"/") != std::wstring::npos)
		Info = Info.substr(Info.find_last_of(L"/") + 1, std::wstring::npos);

	return Info;
}

NTSTATUS ShinkuHook::QueryFileXP3(LPWSTR FileName, PBYTE& Buffer, ULONG& Size, ULONG64& Hash)
{
	NTSTATUS                           Status;
	vector<FileInfoChunkXP3>::iterator iBegin, iEnd;
	FileInfoChunkXP3                   Info;
	wstring                            FileNameLower;

	LOOP_ONCE
	{
		Status = STATUS_UNSUCCESSFUL;
		Buffer = NULL;
		Size = 0;
		if (XP3PackFile.GetHandle() == 0 || XP3PackFile.GetHandle() == INVALID_HANDLE_VALUE)
			break;

		FileNameLower = FileNameToLower(FileName);
		Info.Hash = MurmurHash64B(FileNameLower.c_str(), StrLengthW(FileNameLower.c_str()) * 2);

		iBegin = std::lower_bound(XP3FileList.begin(), XP3FileList.end(), Info);
		iEnd   = std::upper_bound(XP3FileList.begin(), XP3FileList.end(), Info);

		if (iBegin == XP3FileList.end() && iEnd == XP3FileList.end())
			break;

		for (auto it = iBegin; it != iEnd; it++)
		{
			if (!StrCompareW(FileNameLower.c_str(), it->FileName.c_str()))
			{
				XP3PackFile.Seek(it->Offset, FILE_BEGIN);

				//PrintConsoleW(L"local %s\n", FileNameLower.c_str());

				Buffer = (PBYTE)AllocateMemoryP(it->Size);
				if (!Buffer)
					break;

				Size = it->Size;
				Hash = it->Hash;
				XP3PackFile.Read(Buffer, Size);

				Status = STATUS_SUCCESS;
				break;
			}
		}

	}
	return Status;
}


class tTJSCriticalSection
{
	CRITICAL_SECTION CS;
public:
	tTJSCriticalSection()  { InitializeCriticalSection(&CS); }
	~tTJSCriticalSection() { DeleteCriticalSection(&CS); }

	void Enter() { EnterCriticalSection(&CS); }
	void Leave() { LeaveCriticalSection(&CS); }
};

class tTJSCriticalSectionHolder
{
	tTJSCriticalSection *Section;
public:
	tTJSCriticalSectionHolder(tTJSCriticalSection &cs)
	{
		Section = &cs;
		Section->Enter();
	}

	~tTJSCriticalSectionHolder()
	{
		Section->Leave();
	}

};


static tTJSCriticalSection LocalCreateStreamCS;

IStream* ShinkuHook::CreateLocalStream(LPCWSTR lpFileName)
{

	tTJSCriticalSectionHolder CSHolder(LocalCreateStreamCS);


	ULONG                 FileSize;
	PBYTE                 FileBuffer;
	ULONG64               Hash;
	std::wstring          FileName;
	StreamHolderXP3*      Holder;
	IStreamAdapterXP3*    StreamAdapter;

	FileName = GetKrkrFileName(lpFileName);

	LOOP_ONCE
	{
		StreamAdapter = nullptr;
		QueryFileXP3(&FileName[0], FileBuffer, FileSize, Hash);

		if (!FileBuffer || !FileSize)
			break;

		Holder        = new StreamHolderXP3  (FileBuffer, FileSize, Hash);
		StreamAdapter = new IStreamAdapterXP3(Holder);
	}
	return StreamAdapter;
}

#if 0
IStream* NTAPI HookTVPCreateIStream(const ttstr & _name, tjs_uint32 flags)
{
	IStream*  Stream;

	LOOP_ONCE
	{
		Stream = NULL;
		Stream = ShinkuHook::GetHook()->CreateLocalStream(_name.c_str());
		if (Stream)
			break;
		
		Stream = ShinkuHook::GetHook()->StubCreateIStream(_name, flags);
	}
	return Stream;
}
#endif

/*
exe::TVPCreateIStream.stub
CPU Disasm
Address                 Hex dump                       Command                                                    Comments
0134D830                /. /55                         push ebp
0134D831                |. |8BEC                       mov ebp,esp
0134D833                |. |8B55 0C                    mov edx,dword ptr [ebp+0C]
0134D836                |. |8B4D 08                    mov ecx,dword ptr [ebp+8]
0134D839                |. |E8 E2800000                call 01355920   exe::TVPCreateIStream
0134D83E                |. |5D                         pop ebp
0134D83F                \. |C2 0800                    retn 8

exe::TVPCreateIStream fastcall
01355920                /$  55                         push ebp
01355921                |.  8BEC                       mov ebp,esp
01355923                |.  6A FF                      push -1
01355925                |.  68 BB405C01                push 015C40BB                                              ; Entry point
0135592A                |.  64:A1 00000000             mov eax,dword ptr fs:[0]
01355930                |.  50                         push eax
01355931                |.  83EC 0C                    sub esp,0C
01355934                |.  53                         push ebx
01355935                |.  56                         push esi
01355936                |.  57                         push edi
01355937                |.  A1 B0486901                mov eax,dword ptr [16948B0]
0135593C                |.  33C5                       xor eax,ebp
0135593E                |.  50                         push eax
0135593F                |.  8D45 F4                    lea eax,[ebp-0C]
01355942                |.  64:A3 00000000             mov dword ptr fs:[0],eax
01355948                |.  8965 F0                    mov dword ptr [ebp-10],esp
0135594B                |.  C745 EC 00000000           mov dword ptr [ebp-14],0
01355952                |.  C745 FC 00000000           mov dword ptr [ebp-4],0
01355959                |.  E8 8296FEFF                call 0133EFE0   exe::TVPCreateStream(fastcall)
*/

//133EFE0
tTJSBinaryStream* FASTCALL HookTVPCreateStream(const ttstr & _name, tjs_uint32 flags)
{
	tTJSBinaryStream*  Stream;
	IStream*           IStream;

	LOOP_ONCE
	{
		Stream  = NULL;
		IStream = NULL;
		
		if (flags == TJS_BS_READ)
			IStream = ShinkuHook::GetHook()->CreateLocalStream(_name.c_str());

		if (IStream)
		{
			Stream = TVPCreateBinaryStreamAdapter(IStream);
			if (Stream)
				break;
		}

		Stream = ShinkuHook::GetHook()->StubCreateIStream(_name, flags);
	}
	return Stream;
}

void FASTCALL HookTVPAddLog(const ttstr &line, bool appendtoimportant)
{
	if(ShinkuHook::GetHook()->DebugPort) PrintConsoleW(L"%s%s\n", appendtoimportant ? L"[Warn]" : L"[Info]", line.c_str());
}

HRESULT WINAPI HookV2Link(iTVPFunctionExporter *exporter)
{
	NTSTATUS       Status;
	ShinkuHook*    Handle;
	PVOID          TVPAddLogFastCall;
	

	Handle = ShinkuHook::GetHook();
	TVPInitImportStub(exporter);
	Handle->TVPFunctionExporter = exporter;
	Handle->StubCreateIStream = (FuncCreateStream)((ULONG_PTR)Nt_GetExeModuleHandle() + 0xEFE0);

	TVPAddLogFastCall = (PVOID)((ULONG_PTR)Nt_GetExeModuleHandle() + 0xDBB70);

	INLINE_PATCH_DATA f[] =
	{
		{ Handle->StubCreateIStream, HookTVPCreateStream, (PVOID*)&(Handle->StubCreateIStream) },
		{ TVPAddLogFastCall,         HookTVPAddLog,       NULL                                 }
	};

	Status = InlinePatchMemory(f, countof(f));

	return Handle->StubV2Link(exporter);
}


NTSTATUS ShinkuHook::InitKrkrHook(LPCWSTR lpFileName, PVOID Module)
{
	NTSTATUS    Status;
	ULONG_PTR   Length;
	ULONG64     Extension;
	DWORD       ThreadId;
	PVOID       pV2Link;

	LOOP_ONCE
	{
		Status = STATUS_ALREADY_REGISTERED;
		if (Inited == TRUE)
			break;

		Status = STATUS_UNSUCCESSFUL;
		if (Module == NULL)
			break;

		Length = StrLengthW(lpFileName);
		if (Length <= 4)
			break;

		Extension = *(PULONG64)&lpFileName[Length - 4];

		if (Extension != TAG4W('.dll') && Extension != TAG4W('.tpm'))
			break;

		pV2Link = Nt_GetProcAddress(Module, "V2Link");
		if (pV2Link == NULL)
			break;

		INLINE_PATCH_DATA f[] =
		{
			{ pV2Link, HookV2Link, (PVOID*)&StubV2Link }
		};
		
		Status = InlinePatchMemory(f, countof(f));
		Inited = TRUE;
	}
	return Status;
}

//KAGParser.dll
HMODULE WINAPI HookLoadLibraryW(LPCWSTR lpLibFileName)
{
	ULONG_PTR     LengthOfName;
	WCHAR         SelfModuleName[MAX_PATH];
	HMODULE       Module;

	LengthOfName = StrLengthW(lpLibFileName);

	if (LengthOfName >= 12 &&
		*(PULONG64)&lpLibFileName[LengthOfName - 0xC] == TAG4W('AGPa') &&
		*(PULONG64)&lpLibFileName[LengthOfName - 0x8] == TAG4W('rser') &&
		*(PULONG64)&lpLibFileName[LengthOfName - 0x4] == TAG4W('.dll'))
	{
		Nt_GetModuleFileName(ShinkuHook::GetHook()->m_SelfModule, SelfModuleName, countof(SelfModuleName)); 
		for (ULONG_PTR i = 0; i < 3; i++)
		{
			//avoid global release
			Nt_LoadLibrary(SelfModuleName);
		}
		return ShinkuHook::GetHook()->m_SelfModule;
	}
	else if (LengthOfName >= 11 && 
		     *(PULONG64)&lpLibFileName[LengthOfName - 0xB] == TAG4W('krmo') &&
		     *(PULONG64)&lpLibFileName[LengthOfName - 0x7] == TAG4W('vie.') &&
		     *(PULONG64)&lpLibFileName[LengthOfName - 0x4] == TAG4W('.dll'))
	{
		Module = (HMODULE)Nt_LoadLibrary(L"KrMovieEx.dll");
		return Module;
	}
	else
	{
		Module = LoadLibraryW(lpLibFileName); 
		ShinkuHook::GetHook()->InitKrkrHook(lpLibFileName, Module);
		return Module;
	}
}


HMODULE WINAPI HookLoadLibraryExW(LPCWSTR lpLibFileName, HANDLE hFile, DWORD dwFlags)
{
	ULONG_PTR     LengthOfName;
	WCHAR         SelfModuleName[MAX_PATH];
	HMODULE       Module;

	LengthOfName = StrLengthW(lpLibFileName);

	if (LengthOfName >= 12 &&
		*(PULONG64)&lpLibFileName[LengthOfName - 0xC] == TAG4W('AGPa') &&
		*(PULONG64)&lpLibFileName[LengthOfName - 0x8] == TAG4W('rser') &&
		*(PULONG64)&lpLibFileName[LengthOfName - 0x4] == TAG4W('.dll'))
	{
		Nt_GetModuleFileName(ShinkuHook::GetHook()->m_SelfModule, SelfModuleName, countof(SelfModuleName));
		for (ULONG_PTR i = 0; i < 3; i++)
		{
			//avoid global release
			Nt_LoadLibrary(SelfModuleName);
		}
		return ShinkuHook::GetHook()->m_SelfModule;
	}
	else if (LengthOfName >= 11 &&
		     *(PULONG64)&lpLibFileName[LengthOfName - 0xB] == TAG4W('krmo') &&
		     *(PULONG64)&lpLibFileName[LengthOfName - 0x7] == TAG4W('vie.') &&
		     *(PULONG64)&lpLibFileName[LengthOfName - 0x4] == TAG4W('.dll'))
	{
		Module = (HMODULE)Nt_LoadLibrary(L"KrMovieEx.dll");
		return Module;
	}
	else
	{
		Module = LoadLibraryExW(lpLibFileName, hFile, dwFlags);
		ShinkuHook::GetHook()->InitKrkrHook(lpLibFileName, Module);
		return Module;
	}
}


NTSTATUS NTAPI HookLdrLoadDll(
IN  PWSTR               PathToFile OPTIONAL,
IN  PULONG              DllCharacteristics OPTIONAL,
IN  PCUNICODE_STRING    ModuleFileName,
OUT PVOID*              DllHandle
)
{
	ULONG_PTR      LengthOfName;
	WCHAR          SelfModuleName[MAX_PATH];
	UNICODE_STRING SelfUnicodeName;
	PWSTR          lpLibFileName;

	LengthOfName  = ModuleFileName->Length;
	lpLibFileName = ModuleFileName->Buffer;

	if (LengthOfName >= 12 &&
		*(PULONG64)&lpLibFileName[LengthOfName - 0xC] == TAG4W('AGPa') &&
		*(PULONG64)&lpLibFileName[LengthOfName - 0x8] == TAG4W('rser') &&
		*(PULONG64)&lpLibFileName[LengthOfName - 0x4] == TAG4W('.dll'))
	{
		Nt_GetModuleFileName(ShinkuHook::GetHook()->m_SelfModule, SelfModuleName, countof(SelfModuleName));
		RtlInitUnicodeString(&SelfUnicodeName, SelfModuleName);
		for (ULONG_PTR i = 0; i < 3; i++)
		{
			//avoid global release
			ShinkuHook::GetHook()->OldLdrLoadDll(PathToFile, DllCharacteristics, &SelfUnicodeName, DllHandle);
		}

		*DllHandle = ShinkuHook::GetHook()->m_SelfModule;
		return STATUS_SUCCESS;
	}
	else
	{
		return ShinkuHook::GetHook()->OldLdrLoadDll(PathToFile, DllCharacteristics, ModuleFileName, DllHandle);
	}
}

/*
List of windows, item 2
Handle = 00721878
Text = 生命的备件
Parent = Topmost
WinProc =
ID/menu =
Type = UNICODE
Style = 16CA0000 WS_OVERLAPPED|WS_MINIMIZEBOX|WS_CAPTION|WS_SYSMENU|WS_VISIBLE|WS_CLIPSIBLINGS|WS_CLIPCHILDREN
ExtStyle = 00000110 WS_EX_WINDOWEDGE|WS_EX_ACCEPTFILES
Thread = Main
ClsProc = 013738D0
ClsName = TVPMainWindow
*/

BOOL NTAPI HookSetWindowTextW(HWND hWnd, LPCWSTR lpString)
{
	WCHAR  ClassName[MAX_PATH];
	
	RtlZeroMemory(ClassName, countof(ClassName) * sizeof(ClassName[0])); 
	GetClassNameW(hWnd, ClassName, MAX_PATH);

	if (!StrCompareW(ClassName, L"TVPMainWindow"))
		return SetWindowTextW(hWnd, L"生命的备件[最终版]");
	else
		return SetWindowTextW(hWnd, lpString);
}


HWND
WINAPI
HookCreateWindowExW(
_In_ DWORD dwExStyle,
_In_opt_ LPCWSTR lpClassName,
_In_opt_ LPCWSTR lpWindowName,
_In_ DWORD dwStyle,
_In_ int X,
_In_ int Y,
_In_ int nWidth,
_In_ int nHeight,
_In_opt_ HWND hWndParent,
_In_opt_ HMENU hMenu,
_In_opt_ HINSTANCE hInstance,
_In_opt_ LPVOID lpParam)
{
	if (!StrCompareW(lpClassName, L"TVPMainWindow"))
		lpWindowName = L"生命的备件[最终版]";

	return CreateWindowExW(
		dwExStyle,
		lpClassName,
		lpWindowName,
		dwStyle,
		X,
		Y,
		nWidth,
		nHeight,
		hWndParent,
		hMenu,
		hInstance,
		lpParam);
}


HANDLE
WINAPI
HookCreateFileW(
_In_ LPCWSTR lpFileName,
_In_ DWORD dwDesiredAccess,
_In_ DWORD dwShareMode,
_In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes,
_In_ DWORD dwCreationDisposition,
_In_ DWORD dwFlagsAndAttributes,
_In_opt_ HANDLE hTemplateFile
)
{
	wstring   FileName(lpFileName);
	ULONG_PTR Cur;

	Cur = FileName.find_first_of(L"\\");
	if (Cur != wstring::npos)
		FileName = FileName.substr(Cur + 1, wstring::npos);

	Cur = FileName.find_first_of(L"/");
	if (Cur != wstring::npos)
		FileName = FileName.substr(Cur + 1, wstring::npos);

	if (!StrICompareW(FileName.c_str(), L"patch2.xp3", StrCmp_ToLower))
		return INVALID_HANDLE_VALUE;
	else
		return CreateFileW(
		lpFileName,
		dwDesiredAccess,
		dwShareMode,
		lpSecurityAttributes,
		dwCreationDisposition,
		dwFlagsAndAttributes,
		hTemplateFile);
}


HFONT WINAPI HookCreateFontIndirectW(CONST LOGFONTW *lplf)
{
	LOGFONTW  FontInfo;

	static WCHAR FontName[] = L"黑体";

	RtlZeroMemory(&FontInfo, sizeof(FontInfo));
	CopyStruct(&FontInfo, lplf, sizeof(FontInfo));
	RtlCopyMemory(FontInfo.lfFaceName, FontName, (countof(FontName) + 1) * 2);
	FontInfo.lfCharSet = GB2312_CHARSET;

	return CreateFontIndirectW(&FontInfo);
}

NTSTATUS ShinkuHook::Init(HMODULE hModule)
{
	NTSTATUS      Status;
	LPCWSTR       Message;
	PVOID         FakeCompiler;
	PVOID         ExeModule;

	m_SelfModule = hModule;
	ExeModule = Nt_GetExeModuleHandle();

	if (Nt_GetModuleHandle(L"LocaleEmulator.dll") || Nt_GetModuleHandle(L"ntleai.dll"))
	{
		MessageBoxW(NULL, L"請勿開啟轉區程式...", L">_<", MB_OK | MB_ICONWARNING);
		Ps::ExitProcess(0);
	}

	Nt_LoadLibrary(L"USER32.DLL");

	IAT_PATCH_DATA f[] =
	{
		{ ExeModule, LoadLibraryW,        HookLoadLibraryW,        "Kernel32.dll" },
		{ ExeModule, LoadLibraryExW,      HookLoadLibraryExW,      "Kernel32.dll" },
		{ ExeModule, CreateFileW,         HookCreateFileW,         "Kernel32.dll" },
		{ ExeModule, SetWindowTextW,      HookSetWindowTextW,      "User32.dll"   },
		{ ExeModule, CreateWindowExW,     HookCreateWindowExW,     "User32.dll"   },
		{ ExeModule, CreateFontIndirectW, HookCreateFontIndirectW, "Gdi32.dll"    }
	};
	
	INLINE_PATCH_DATA p[] = 
	{
		{ LdrLoadDll, HookLdrLoadDll, (PVOID*)&OldLdrLoadDll }
	};

	LOOP_ONCE
	{
		Status = LaunchAntiLiveWorker();
		if (NT_FAILED(Status))
		{
			MessageBoxW(NULL, L"你的黄油补丁坏掉了QAQ", L"是的，连黄油补丁都不爱你了", MB_OK | MB_ICONERROR);
			Ps::ExitProcess(0x44);
		}

		FakeCompiler = PtrXor(CheckPatch2, CheckPatch3);

		Status = IATPatchMemory(f, countof(f));
		//Status = InlinePatchMemory(p, countof(p));
		if (NT_FAILED(Status))
		{
			switch (GetACP())
			{
			case 936:
				Message = L"第一部分启动失败";
				break;

			default:
				Message = L"第一部分啟動失敗";
				break;
			}
			MessageBoxW(NULL, Message, L"X'moe Core Lib", MB_OK | MB_ICONERROR);
			break;
		}

		Status = InitAddtion();
		if (NT_FAILED(Status))
		{
			switch (GetACP())
			{
			case 936:
				Message = L"内核初始化失败";
				break;

			default:
				Message = L"內核初始化失敗";
				break;
			}
			MessageBoxW(NULL, Message, L"X'moe Core Lib", MB_OK | MB_ICONERROR);
			break;
		}

		Status = LoadDirectShowFilter();
		if (NT_FAILED(Status))
		{
			MessageBoxW(NULL, L"启动失败", L"X'moe CoreLib", MB_OK | MB_ICONERROR);
			Ps::ExitProcess(TAG4('filt'));
		}

		Status = InitFileSystem();
		if (NT_FAILED(Status))
		{
			MessageBoxW(NULL, L"启动失败", L"X'moe CoreLib", MB_OK | MB_ICONERROR);
			Ps::ExitProcess(TAG4('fuck'));
		}

		Status = InitFileSystemXP3();
		if (NT_FAILED(Status))
		{
			MessageBoxW(NULL, L"启动失败", L"X'moe CoreLib", MB_OK | MB_ICONERROR);
			Ps::ExitProcess(TAG4('f@ck'));
		}

	}

	return Status;
}


NTSTATUS ShinkuHook::InitFileSystemXP3()
{
	NTSTATUS         Status;
	DWORD            ChunkOffset, FileSize, ChunkSize, iPos;
	PBYTE            ChunkData;
	FileInfoChunkXP3 Info;

	static BYTE ChunkKey[256] =
	{
		0x1c, 0x06, 0x4b, 0x7e, 0xac, 0x4f, 0xe4,
		0xf6, 0xed, 0xdb, 0x19, 0x79, 0x53, 0xbe, 0xa8,
		0xb5, 0x07, 0x62, 0x45, 0xee, 0x64, 0x04, 0x15,
		0x7a, 0x7b, 0x34, 0x32, 0x45, 0x90, 0x58, 0x2b,
		0x4f, 0x9b, 0x29, 0x81, 0x29, 0xc8, 0x34, 0x2a,
		0x7c, 0xf6, 0x5b, 0x11, 0x81, 0x3b, 0x52, 0x94,
		0x8a, 0x5f, 0x23, 0x05, 0x78, 0x5d, 0xaa, 0x2a,
		0x43, 0xe6, 0x19, 0xbe, 0x76, 0xdd, 0x75, 0xed,
		0xaf, 0xdb, 0x17, 0xdb, 0x23, 0xad, 0x2d, 0x1e,
		0x18, 0xd1, 0x36, 0x3f, 0x69, 0xfd, 0x89, 0x3e,
		0x06, 0x98, 0xcf, 0x0b, 0x72, 0x3f, 0x84, 0x0d,
		0x43, 0x41, 0x7a, 0x9e, 0xa5, 0x23, 0x56, 0x8d,
		0xd7, 0x1d, 0x11, 0x9b, 0xac, 0x9b, 0x78, 0xff,
		0x0c, 0xbd, 0xad, 0xe3, 0x71, 0xd8, 0xa3, 0xe4,
		0x6a, 0xf2, 0xa6, 0x95, 0x1b, 0x4a, 0xd1, 0xfd,
		0xba, 0xce, 0x96, 0x14, 0x14, 0xa2, 0x3a, 0x4a,
		0x06, 0xa0, 0x56, 0x00, 0x05, 0xd3, 0x57, 0x0d,
		0x97, 0xfa, 0xfe, 0x3b, 0xd7, 0x0c, 0xe1, 0xc8,
		0xf5, 0xae, 0xe8, 0xe5, 0xb3, 0xbe, 0xd1, 0x3a,
		0xe9, 0xcc, 0xad, 0x5f, 0x02, 0x9b, 0x61, 0x64,
		0x7d, 0xa4, 0x26, 0x4b, 0x6d, 0x94, 0x09, 0x89,
		0xf9, 0xc9, 0x6b, 0x89, 0xdd, 0xd9, 0x82, 0x28,
		0xe7, 0x0b, 0xd6, 0x3a, 0x7b, 0xdb, 0xc5, 0x04,
		0x0f, 0x7b, 0x00, 0xc0, 0xaf, 0x4d, 0x0b, 0x1c,
		0x7b, 0x6a, 0xc1, 0xbb, 0x24, 0x1e, 0xce, 0xb2,
		0x73, 0x69, 0x33, 0x0c, 0xc2, 0x7f, 0xc6, 0x47,
		0x80, 0x49, 0xaf, 0xd4, 0xb1, 0xe2, 0xec, 0x9b,
		0x6c, 0x1b, 0xcd, 0x75, 0x5c, 0xf8, 0x79, 0xb1,
		0x40, 0x30, 0x68, 0x8f, 0x6a, 0xb1, 0xe6, 0xc8,
		0x43, 0x1a, 0x97, 0x04, 0xc6, 0x3f, 0xed, 0x62,
		0x01, 0xa8, 0xb3, 0xf4, 0x97, 0x12, 0x86, 0x40,
		0x41, 0xed, 0x56, 0xc0, 0x48, 0xdc, 0xea, 0x63,
		0x0c
	};

	LOOP_ONCE
	{
		Status = XP3PackFile.Open(L"yayaya.bin");
		if (NT_FAILED(Status))
			break;

		XP3PackFile.Read(&ChunkOffset, 4);

		ChunkOffset ^= 0x84F2DA63;
		FileSize = XP3PackFile.GetSize32();

		XP3PackFile.Seek(ChunkOffset, FILE_BEGIN);

		ChunkSize = FileSize - ChunkOffset;
		ChunkData = (PBYTE)AllocateMemoryP(ChunkSize);

		XP3PackFile.Read(ChunkData, ChunkSize);


		for (DWORD i = 0; i < ChunkSize; i++)
			ChunkData[i] ^= ChunkKey[i % 256];

		iPos = 0;
		while (iPos < ChunkSize)
		{
			wstring FileName((LPCWSTR)(ChunkData + iPos));
			Info.FileName = FileName;

			iPos += (FileName.length() + 1) * 2;

			Info.Offset = *(PDWORD)(ChunkData + iPos);
			iPos += 4;
			Info.Size = *(PDWORD)(ChunkData + iPos);
			iPos += 4;
			Info.Hash = *(PULONG64)(ChunkData + iPos);
			iPos += 8;

			
			XP3FileList.push_back(Info);
		}

		FreeMemoryP(ChunkData);

		std::sort(XP3FileList.begin(), XP3FileList.end());
	}
	return Status;
}

NTSTATUS ShinkuHook::InitFileSystem()
{
	NTSTATUS      Status;
	ULONG         ChunkSize, iPos, FileSize, ChunkOffset;
	PBYTE         ChunkData;
	Twofish_key   S[1];
	BYTE          Dec[16];
	string        UTF8Str;
	WCHAR         FileName[400];
	FileInfoChunk Info;

	static BYTE EndMark[]  = { 0xFF, 0xFF, 0xFF, 0xAA, 0xAA, 0xAA, 0xBB, 0xBB, 0xBB };
	static BYTE ChunkKey[] =
	{
		0xce, 0xd1, 0x08, 0xb1, 0x96, 0x8d, 0x0a,
		0x21, 0xa5, 0x52, 0x1a, 0x13, 0x95, 0x89, 0xfc,
		0x6d, 0x75, 0xb3, 0x28, 0x69, 0x46, 0xe1, 0x4a,
		0x12, 0x7f, 0x3c, 0xc2, 0xcb, 0x89, 0x7e, 0x24,
		0xc8, 0x43, 0x77, 0xb9, 0x93, 0x7f, 0x89, 0xfc,
		0x88, 0x83, 0x2c, 0x1e, 0x5a, 0x8a, 0x6b, 0x82,
		0x8c, 0x41, 0x64, 0x42, 0xf8, 0x4a, 0xcc, 0xa8,
		0x4b, 0xbb, 0x69, 0xb6, 0x87, 0xa0, 0x97, 0x9f,
		0x80, 0x75, 0xc4, 0x4b, 0x60, 0xae, 0xf4, 0xd7,
		0x23, 0x2f, 0x3d, 0x12, 0x1b, 0xd3, 0x4b, 0x01,
		0x6e, 0xe9, 0xdd, 0x5c, 0x93, 0xb3, 0x47, 0x0f,
		0xd8, 0xe6, 0xee, 0xbb, 0xdf, 0x2c, 0xcf, 0x32,
		0x1c, 0xa5, 0xf9, 0xfe, 0x58, 0x61, 0x0e, 0xdb,
		0x31, 0xe8, 0xc5, 0x38, 0x98, 0xb2, 0x6b, 0xba,
		0x52, 0xaf, 0x5e, 0xb8, 0x78, 0xc1, 0x90, 0xc0,
		0xf8, 0x3d, 0x0a, 0x11, 0x11, 0x6e, 0x67, 0x1f,
		0xda
	};

	LOOP_ONCE
	{
		Status = PackFile.Open(L"ino.bin");
		if (NT_FAILED(Status))
			break;

		PackFile.Read(&ChunkOffset, 4);

		ChunkOffset ^= 0x52128463;
		FileSize     = PackFile.GetSize32();

		PackFile.Seek(ChunkOffset, FILE_BEGIN);

		ChunkSize = FileSize - ChunkOffset;
		ChunkData = (PBYTE)AllocateMemoryP(ChunkSize);

		PackFile.Read(ChunkData, ChunkSize);
		Twofish_initialise();
		
		Twofish_prepare_key(ChunkKey, 32, S);

		for (ULONG i = 0; i < ChunkSize / 16; i++)
		{
			Twofish_decrypt(S, &ChunkData[i * 16], Dec);
			RtlCopyMemory  (&ChunkData[i * 16], Dec, 16);
		}

		RtlZeroMemory(S, sizeof(S[0]));

		iPos = 0;
		while (iPos < ChunkSize)
		{
			UTF8Str = (LPCSTR)(ChunkData + iPos);
			iPos += UTF8Str.length() + 1;

			UTF8Str = base64_decode(UTF8Str);

			RtlZeroMemory(FileName, countof(FileName) * sizeof(FileName[0]));
			AnzUtf8ToWideCharString(UTF8Str.c_str(), FileName);

			Info.FileName = FileNameToLower(FileName);

			Info.Offset = *(PDWORD)(ChunkData + iPos);
			iPos += 4;
			Info.Size = *(PDWORD)(ChunkData + iPos);
			iPos += 4;
			Info.KeySize = *(PDWORD)(ChunkData + iPos);
			iPos += 4;

			Info.Hash = MurmurHash64B(Info.FileName.c_str(), StrLengthW(Info.FileName.c_str()) * 2);

			FileList.push_back(Info);

			if (!memcmp((ChunkData + iPos), EndMark, sizeof(EndMark)))
				break;
		}

		FreeMemoryP(ChunkData);

		std::sort(FileList.begin(), FileList.end());
	}
	
	return Status;
}


BOOL DecodeFile(PBYTE Buffer, ULONG Size, PBYTE& OutBuffer, PBYTE KeyBuffer)
{
	for (ULONG i = 0; i < Size / 16; i++)
	{
		aes_decrypt(Buffer + i * 16, OutBuffer + i * 16, (uint*)(KeyBuffer + i * 512), 256);
	}
	return TRUE;
}



NTSTATUS ShinkuHook::QueryFile(LPWSTR FileName, PBYTE& Buffer, ULONG& Size)
{
	NTSTATUS                        Status;
	vector<FileInfoChunk>::iterator iBegin, iEnd;
	FileInfoChunk                   Info;
	wstring                         FileNameLower;
	ULONG                           ChunkSize;
	PBYTE                           PreBuffer;
	
	LOOP_ONCE
	{
		Status = STATUS_UNSUCCESSFUL;
		Buffer = NULL;
		Size   = 0;
		if (PackFile.GetHandle() == 0 || PackFile.GetHandle() == INVALID_HANDLE_VALUE)
			break;

		FileNameLower = FileNameToLower(FileName);
		Info.Hash     = MurmurHash64B(FileNameLower.c_str(), StrLengthW(FileNameLower.c_str()) * 2);

		iBegin = std::lower_bound(FileList.begin(), FileList.end(), Info);
		iEnd   = std::upper_bound(FileList.begin(), FileList.end(), Info);

		if (iBegin == FileList.end() && iEnd == FileList.end())
			break;

		for (auto it = iBegin; it != iEnd; it++)
		{
			if (!StrCompareW(FileNameLower.c_str(), it->FileName.c_str()))
			{
				PackFile.Seek(it->Offset, FILE_BEGIN);

				ChunkSize = it->Size + it->KeySize;
				PreBuffer = (PBYTE)AllocateMemoryP(ChunkSize);
				if (!PreBuffer)
					break;

				Size = it->Size;

				Buffer = (PBYTE)AllocateMemoryP(it->Size);
				if (!Buffer)
					break;
				
				PackFile.Read(PreBuffer, ChunkSize);
				DecodeFile(PreBuffer, it->Size, Buffer, PreBuffer + it->Size);

				FreeMemoryP(PreBuffer);

				Status = STATUS_SUCCESS;
				break;
			}
		}

	}
	return Status;
}


NTSTATUS ShinkuHook::UnInit(HMODULE hModule)
{
	UNREFERENCED_PARAMETER(hModule);

	PackFile.Close();
	return STATUS_SUCCESS;
}






