#include "KaresekaHook.h"
#include "MyHook.h"
#include "tp_stub.h"
#include <string>
#include <algorithm>
#include "ThemidaSDK.h"

using std::wstring;

PVOID GetTVPCreateStreamCall()
{
	KaresekaHook* Kareseka;
	PVOID         CallIStreamStub, CallIStream, CallTVPCreateStreamCall;
	ULONG         OpSize, OpOffset;
	WORD          WordOpcode;

	static char funcname[] = "IStream * ::TVPCreateIStream(const ttstr &,tjs_uint32)";

	Kareseka = GetKareseka();

	LOOP_ONCE
	{
		CallTVPCreateStreamCall = NULL;

		CallIStreamStub = TVPGetImportFuncPtr(funcname);
		if (!CallIStreamStub)
			break;

		CallIStream = NULL;
		OpOffset = 0;

		LOOP_FOREVER
		{
			if (((PBYTE)CallIStreamStub + OpOffset)[0] == 0xCC)
			break;

			WordOpcode = *(PWORD)((ULONG_PTR)CallIStreamStub + OpOffset);
			//mov edx,dword ptr [ebp+0xC]
			if (WordOpcode == 0x558B)
			{
				OpOffset += 2;
				if (((PBYTE)CallIStreamStub + OpOffset)[0] == 0xC)
				{
					OpOffset++;
					WordOpcode = *(PWORD)((ULONG_PTR)CallIStreamStub + OpOffset);
					//mov edx,dword ptr [ebp+0x8]
					if (WordOpcode == 0x4D8B)
					{
						OpOffset += 2;
						if (((PBYTE)CallIStreamStub + OpOffset)[0] == 0x8)
						{
							OpOffset++;
							if (((PBYTE)CallIStreamStub + OpOffset)[0] == CALL)
							{
								CallIStream = (PVOID)GetCallDestination(((ULONG_PTR)CallIStreamStub + OpOffset));
								OpOffset += 5;
								break;
							}
						}
					}
				}
			}
			//the next opcode
			OpSize = GetOpCodeSize32(((PBYTE)CallIStreamStub + OpOffset));
			OpOffset += OpSize;
		}

			if (!CallIStream)
				break;

		OpOffset = 0;
		LOOP_FOREVER
		{
			if (((PBYTE)CallIStream + OpOffset)[0] == 0xC3)
			break;

			//find the first call
			if (((PBYTE)CallIStream + OpOffset)[0] == CALL)
			{
				CallTVPCreateStreamCall = (PVOID)GetCallDestination(((ULONG_PTR)CallIStream + OpOffset));
				OpOffset += 5;
				break;
			}

			//the next opcode
			OpSize = GetOpCodeSize32(((PBYTE)CallIStream + OpOffset));
			OpOffset += OpSize;
		}

			LOOP_FOREVER
		{
			if (((PBYTE)CallIStream + OpOffset)[0] == 0xC3)
			break;

			if (((PBYTE)CallIStream + OpOffset)[0] == CALL)
			{
				//push 0xC
				//call HostAlloc
				//add esp, 0x4
				if (((PBYTE)CallIStream + OpOffset - 2)[0] == 0x6A &&
					((PBYTE)CallIStream + OpOffset - 2)[1] == 0x0C)
				{
					Kareseka->StubHostAlloc = (FuncHostAlloc)GetCallDestination(((ULONG_PTR)CallIStream + OpOffset));
					OpOffset += 5;
				}
				break;
			}

			//the next opcode
			OpSize = GetOpCodeSize32(((PBYTE)CallIStream + OpOffset));
			OpOffset += OpSize;
		}

			LOOP_FOREVER
		{
			if (((PBYTE)CallIStream + OpOffset)[0] == 0xC3)
			break;

			//mov eax, mem.offset
			if (((PBYTE)CallIStream + OpOffset)[0] == 0xC7 &&
				((PBYTE)CallIStream + OpOffset)[1] == 0x00)
			{
				OpOffset += 2;
				Kareseka->IStreamAdapterVtable = *(PULONG_PTR)((PBYTE)CallIStream + OpOffset);
				OpOffset += 4;
				break;
			}

			//the next opcode
			OpSize = GetOpCodeSize32(((PBYTE)CallIStream + OpOffset));
			OpOffset += OpSize;
		}
	}


	//Find virtual table offset
	//IStreamAdapter

	if (Kareseka->StubHostAlloc && Kareseka->IStreamAdapterVtable)
		return CallTVPCreateStreamCall;
	else
		return NULL;
}


tTJSBinaryStream* FASTCALL CallTVPCreateStream(const ttstr& FilePath)
{
	tTJSBinaryStream* Stream;
	KaresekaHook*     Handle;

	Handle = GetKareseka();

	if (Handle->StubTVPCreateStream == NULL)
		Handle->StubTVPCreateStream = (FuncCreateStream)GetTVPCreateStreamCall();

	Stream = NULL;

	if (Handle->StubTVPCreateStream == NULL)
		return Stream;

	return Handle->StubTVPCreateStream(FilePath, TJS_BS_READ);
}

IStream* FASTCALL ConvertBStreamToIStream(tTJSBinaryStream* BStream)
{
	KaresekaHook* Kareseka;
	IStream*      Stream;
	PVOID         CallHostAlloc;
	ULONG_PTR     IStreamAdapterVTableOffset;


	Kareseka = GetKareseka();

	CallHostAlloc = Kareseka->StubHostAlloc;
	IStreamAdapterVTableOffset = Kareseka->IStreamAdapterVtable;
	Stream = NULL;

	INLINE_ASM
	{
		push 0xC;
		call CallHostAlloc;
		add  esp, 0x4;
		test eax, eax;
		jz   NO_CREATE_STREAM;
		mov  esi, IStreamAdapterVTableOffset;
		mov  dword ptr[eax], esi; //Vtable 
		mov  esi, BStream;
		mov  dword ptr[eax + 4], esi; //StreamHolder
		mov  dword ptr[eax + 8], 1;   //ReferCount
		mov  Stream, eax;

	NO_CREATE_STREAM:
	}

	return Stream;
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


KaresekaHook* KaresekaHook::Handle = NULL;

KaresekaHook::KaresekaHook()
{
	StubTVPCreateStream = NULL;
	StubHostAlloc = NULL;
	StubV2Link = NULL;
	IStreamAdapterVtable = NULL;
	TVPFunctionExporter = NULL;
	m_SelfModule = NULL;
	Inited = FALSE;
	FileSystemInited = FALSE;
}

KaresekaHook* FASTCALL GetKareseka()
{
	if (KaresekaHook::Handle == NULL)
		KaresekaHook::Handle = new KaresekaHook();

	return KaresekaHook::Handle;
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

static tTJSCriticalSection LocalCreateStreamCS;

IStream* KaresekaHook::CreateLocalStream(LPCWSTR lpFileName)
{

	tTJSCriticalSectionHolder CSHolder(LocalCreateStreamCS);


	ULONG                 FileSize;
	PBYTE                 FileBuffer;
	ULONG64               Hash;
	std::wstring          FileName;
	StreamHolderXP3*      Holder;
	IStreamAdapterXP3*    StreamAdapter;

	FileName = GetKrkrFileName(lpFileName);

	if (GetKareseka()->IsBIG5)
		FileName += L".cht";

	LOOP_ONCE
	{
		StreamAdapter = nullptr;
		QueryFileXP3(&FileName[0], FileBuffer, FileSize, Hash);
		//QueryFile(NULL, &FileName[0], FileBuffer, FileSize, Hash);

		if (!FileBuffer || !FileSize)
			break;

		Holder = new StreamHolderXP3(FileBuffer, FileSize, Hash);
		StreamAdapter = new IStreamAdapterXP3(Holder);
	}
	return StreamAdapter;
}


tTJSBinaryStream* FASTCALL HookTVPCreateStream(const ttstr & _name, tjs_uint32 flags)
{
	KaresekaHook*      Kareseka;
	tTJSBinaryStream*  Stream;
	IStream*           IStream;

	Kareseka = GetKareseka();

	LOOP_ONCE
	{
		Stream = NULL;
		IStream = NULL;

		if (flags == TJS_BS_READ)
			IStream = Kareseka->CreateLocalStream(_name.c_str());

		if (IStream)
		{

			Stream = TVPCreateBinaryStreamAdapter(IStream);
			if (Stream)
				break;
		}

		Stream = Kareseka->StubTVPCreateStream(_name, flags);
	}
	return Stream;
}


/*

CPU Disasm
Address                           Hex dump                       Command                                                Comments
0027E0A0                          /.  55                         push ebp
0027E0A1                          |.  8BEC                       mov ebp,esp
0027E0A3                          |.  8B55 0C                    mov edx,dword ptr [ebp+0C]
0027E0A6                          |.  8B4D 08                    mov ecx,dword ptr [ebp+8]
0027E0A9                          |.  E8 C2800000                call 00286170
0027E0AE                          |.  5D                         pop ebp
0027E0AF                          \.  C2 0800                    retn 8

*/

static BYTE MyCreateIStream[] = { 0x55, 0x8B, 0xEC, 0x8B, 0x55, 0x0C, 0x8B, 0x4D, 0x08, 0xE8, 0xC2, 0x80, 0x00, 0x00, 0x5D, 0xC2, 0x08, 0x00 };


PVOID TVPCreateStream = NULL;

NAKED Void MyCreateIStream2()
{
	INLINE_ASM
	{
		push ebp;
		mov ebp, esp;
		mov edx, dword ptr[ebp + 0xC];
		mov ecx, dword ptr[ebp + 8];
		call TVPCreateStream;
		pop ebp;
		retn 8;
	}
}


PVOID FVPStream = NULL;


#include <set>

inline PLDR_MODULE GetModuleLdr2(PVOID ModuleBase)
{
	LDR_MODULE *Ldr, *FirstLdr;

	Ldr = FIELD_BASE(Nt_CurrentPeb()->Ldr->InInitializationOrderModuleList.Flink, LDR_MODULE, InInitializationOrderLinks);
	FirstLdr = Ldr;

	do
	{
		Ldr = FIELD_BASE(Ldr->InInitializationOrderLinks.Flink, LDR_MODULE, InInitializationOrderLinks);
		if (Ldr->BaseDllName.Buffer == NULL)
			continue;

		if (Ldr->DllBase != ModuleBase)
			continue;

		return Ldr;

	} while (FirstLdr != Ldr);

	return NULL;
}

BOOL IsValidDllWorkerThread2(PVOID ThreadStart)
{
	ULONG ExeModule;
	std::set<PLDR_MODULE> LAddressSet;


	static LPCWSTR DllSet[] = 
	{
		L"AlphaMovie.dll",
		L"extNagano.dll",
		L"extrans.dll",
		L"getSample.dll",
		L"k2compat.dll",
		L"kagexopt.dll",
		L"KAGParserEx.dll",
		L"krmovie.dll",
		L"kztouch.dll",
		L"layerExDraw.dll",
		L"lzfs.dll",
		L"menu.dll",
		L"multiimage.dll",
		L"PackinOne.dll",
		L"pkutil.dll",
		L"psbfile.dll",
		L"psd.dll",
		L"textrender.dll",
		L"win32dialog.dll",
		L"win32ole.dll",
		L"windowEx.dll",
		L"wuopus.dll",
		L"wuvorbis.dll"
	};

	for (ULONG i = 0; i < countof(DllSet); i++)
	{
		PVOID Handle = Nt_GetModuleHandle(DllSet[i]);
		if (Handle)
		{
			PLDR_MODULE Ldr = GetModuleLdr2(Handle);
			if (Ldr)
				LAddressSet.insert(Ldr);
		}
	}

	ExeModule = (ULONG)Nt_GetExeModuleHandle();

	if ((ULONG)ThreadStart >= ExeModule || (ULONG)ThreadStart <= (ExeModule + GetModuleLdr2(Nt_GetExeModuleHandle())->SizeOfImage))
		return TRUE;

	if (LAddressSet.size() == 0)
		return FALSE;

	for (auto Ldr : LAddressSet)
	{
		if (ThreadStart >= Ldr->DllBase || ThreadStart <= ((PBYTE)Ldr->DllBase + Ldr->SizeOfImage))
			return TRUE;
	}
	return FALSE;
}

void SetBreakPoint(PBYTE Address)
{
	DWORD   OldProtect;
	BYTE    DebugIns;
	SIZE_T  Bytes;

	DebugIns = 0xCC;
	VirtualProtect(Address, 1, PAGE_EXECUTE_READWRITE, &OldProtect);
	WriteProcessMemory(GetCurrentProcess(), Address, &DebugIns, 1, &Bytes);
	VirtualProtect(Address, 1, OldProtect, &OldProtect);
}


void RemoveBreakPoint(PBYTE Address)
{
	DWORD   OldProtect;
	BYTE    DebugIns;
	SIZE_T  Bytes;

	DebugIns = 0x55;
	VirtualProtect(Address, 1, PAGE_EXECUTE_READWRITE, &OldProtect);
	WriteProcessMemory(GetCurrentProcess(), Address, &DebugIns, 1, &Bytes);
	VirtualProtect(Address, 1, OldProtect, &OldProtect);
}

void RemoveBreakPointNext(PBYTE Address)
{
	DWORD   OldProtect;
	BYTE    DebugIns;
	SIZE_T  Bytes;

	DebugIns = 0x8B;
	VirtualProtect(Address, 1, PAGE_EXECUTE_READWRITE, &OldProtect);
	WriteProcessMemory(GetCurrentProcess(), Address, &DebugIns, 1, &Bytes);
	VirtualProtect(Address, 1, OldProtect, &OldProtect);
}

LONG NTAPI FirstVEHandler(
	EXCEPTION_POINTERS *ExceptionInfo
	)
{
	DWORD ReturnAddress;
	if (ExceptionInfo->ContextRecord->Eip == (DWORD)FVPStream)
	{
		ReturnAddress = *(PDWORD)(ExceptionInfo->ContextRecord->Esp);

		if (IsValidDllWorkerThread2((PVOID)ReturnAddress))
		{
			RemoveBreakPoint((PBYTE) ExceptionInfo->ContextRecord->Eip);
			SetBreakPoint((PBYTE)(ExceptionInfo->ContextRecord->Eip + 1));
			//ExceptionInfo->ContextRecord->Eip = (DWORD)MyCreateIStream2;
			return EXCEPTION_CONTINUE_EXECUTION;
		}
	}
	else if (ExceptionInfo->ContextRecord->Eip == (((DWORD)FVPStream) + 1))
	{
		RemoveBreakPointNext((PBYTE)ExceptionInfo->ContextRecord->Eip);
		SetBreakPoint((PBYTE)(ExceptionInfo->ContextRecord->Eip - 1));
		return EXCEPTION_CONTINUE_EXECUTION;
	}
	return EXCEPTION_CONTINUE_SEARCH;
}


HRESULT WINAPI HookV2Link(iTVPFunctionExporter *exporter)
{
	NTSTATUS       Status;
	KaresekaHook*  Kareseka;

	Kareseka = GetKareseka();
	TVPInitImportStub(exporter);
	Kareseka->TVPFunctionExporter = exporter;
	Kareseka->StubTVPCreateStream = (FuncCreateStream)GetTVPCreateStreamCall();
	TVPCreateStream = Kareseka->StubTVPCreateStream;

	static char funcname[] = "IStream * ::TVPCreateIStream(const ttstr &,tjs_uint32)";
	FVPStream = TVPGetImportFuncPtr(funcname);

	//0x55, 0x8B, 0xEC, 0x8B
	if (*(PDWORD)FVPStream != 0x8BEC8B55)
	{
		MessageBoxW(NULL, L"文件损坏", L"X'moe-Core", MB_OK | MB_ICONERROR);
		Ps::ExitProcess(0);
	}

	SetBreakPoint((PBYTE)FVPStream);
	AddVectoredExceptionHandler(0, FirstVEHandler);

	INLINE_PATCH_DATA f[] =
	{
		{ Kareseka->StubTVPCreateStream, HookTVPCreateStream, (PVOID*)&(Kareseka->StubTVPCreateStream) },
	};

	Status = InlinePatchMemory(f, countof(f));

	return Kareseka->StubV2Link(exporter);
}


int WINAPI HookMultiByteToWideChar(
	UINT   CodePage,
	DWORD  dwFlags,
	LPCSTR lpMultiByteStr,
	int    cbMultiByte,
	LPWSTR lpWideCharStr,
	int    cchWideChar
	)
{
	switch (CodePage)
	{
	case CP_ACP:
	case CP_OEMCP:
	case CP_THREAD_ACP:
		CodePage = 932;
		break;

	default:
		break;
	}

	return
		MultiByteToWideChar(
		CodePage,
		dwFlags,
		lpMultiByteStr,
		cbMultiByte,
		lpWideCharStr,
		cchWideChar
		);
}



NTSTATUS KaresekaHook::QueryFile(LPCWSTR QueryPathName, LPCWSTR FileName, PBYTE& FileBuffer, ULONG& FileSize, ULONG64& Hash)
{
	NtFileDisk File;
	NTSTATUS   Status;
	WCHAR      FullFileName[MAX_PATH];

#if 0

	if (!FileName)
		return STATUS_NO_SUCH_FILE;

	FileBuffer = NULL;
	FileSize = 0;
	Hash = 0;

	if (!FileSystemInited)
	{
		LOOP_ONCE
		{
			FormatStringW(FullFileName, L"ProjectDir\\%s", FileName);
			Status = File.Open(FullFileName);
			if (NT_FAILED(Status))
				break;

			FileSize = File.GetSize32();
			FileBuffer = (PBYTE)AllocateMemoryP(FileSize);
			if (!FileBuffer)
			{
				FileSize = 0;
				Status = STATUS_NO_MEMORY;
				break;
			}

			File.Read(FileBuffer, FileSize);
		}
		File.Close();
		return Status;
	}
	else
	{
		return STATUS_NO_SUCH_DEVICE;
	}
#else
	return STATUS_SUCCESS;
#endif
}


NTSTATUS KaresekaHook::InitKrkrHook(LPCWSTR lpFileName, PVOID Module)
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


HMODULE WINAPI HookLoadLibraryW(LPCWSTR lpLibFileName)
{
	ULONG_PTR     LengthOfName;
	WCHAR         SelfModuleName[MAX_PATH];
	HMODULE       Module;
	KaresekaHook*  Kareseka;


	Kareseka = GetKareseka();
	LengthOfName = StrLengthW(lpLibFileName);

#if 0
	if (LengthOfName >= 11 &&
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
		Kareseka->InitKrkrHook(lpLibFileName, Module);
		return Module;
	}
#else

	Module = LoadLibraryW(lpLibFileName);
	Kareseka->InitKrkrHook(lpLibFileName, Module);
	return Module;

#endif
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

NTSTATUS KaresekaHook::QueryFileXP3(LPWSTR FileName, PBYTE& Buffer, ULONG& Size, ULONG64& Hash)
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
		iEnd = std::upper_bound(XP3FileList.begin(), XP3FileList.end(), Info);

		if (iBegin == XP3FileList.end() && iEnd == XP3FileList.end())
			break;

		for (auto it = iBegin; it != iEnd; it++)
		{
			if (!StrCompareW(FileNameLower.c_str(), it->FileName.c_str()))
			{
				//PrintConsole(L"%s\n", FileName);
				XP3PackFile.Seek(it->Offset, FILE_BEGIN);

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

NTSTATUS KaresekaHook::InitFileSystemXP3()
{
	NTSTATUS         Status;
	DWORD            ChunkOffset, FileSize, ChunkSize, iPos;
	PBYTE            ChunkData;
	FileInfoChunkXP3 Info;

	VMStart();

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
		Status = XP3PackFile.Open(L"momo.bin");
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
	FileSystemInited = TRUE;
	VMEnd();
	return Status;
}


#include "mt64.h"


VOID SeedInit(HMODULE hModule)
{
	ULONG64  Seeds[4];
	WCHAR    Path[MAX_PATH];

	RtlZeroMemory(Path, countof(Path) * 2);
	Nt_GetExeDirectory(Path, MAX_PATH);

	Seeds[0] = MakeQword(Nt_GetCurrentProcessId(), Nt_GetCurrentThreadId());
	Seeds[1] = MakeQword(Nt_CurrentPeb()->ProcessHeap, Nt_CurrentTeb()->EnvironmentPointer);
	Seeds[2] = MurmurHash64B(Path, StrLengthW(Path) * 2);
	Seeds[3] = MakeQword(hModule, Nt_GetExeModuleHandle());

	init_by_array64(Seeds, countof(Seeds));
}


BOOL KaresekaHook::Init(HMODULE hModule)
{
	NTSTATUS      Status;
	LPCWSTR       Message;
	PVOID         FakeCompiler;
	PVOID         ExeModule;
	INT           Argc;
	PWSTR*        Args;
	DWORD         OldProtect;

	m_SelfModule = hModule;
	ExeModule = Nt_GetExeModuleHandle();

	SeedInit(hModule);
	//AllocConsole();
	IAT_PATCH_DATA f[] =
	{
		{ ExeModule, LoadLibraryW,        HookLoadLibraryW,        "Kernel32.dll" },
		{ ExeModule, CreateFileW,         HookCreateFileW,         "Kernel32.dll" },
		{ ExeModule, MultiByteToWideChar, HookMultiByteToWideChar, "Kernel32.dll" }
	};

	LOOP_ONCE
	{
		Status = IATPatchMemory(f, countof(f));
		if (NT_FAILED(Status))
		{
			MessageBoxW(NULL, L"第一部分启动失败", L"补丁启动失败", MB_OK | MB_ICONERROR);
			Ps::ExitProcess(0);
			break;
		}

		IsBIG5 = FALSE;

		Args = CommandLineToArgvW(::GetCommandLineW(), &Argc);

		for (LONG i = 0; i < Argc; i++)
		{
			if (!StrCompareW(Args[i], L"-Xmoe::CodePage::950"))
			{
				IsBIG5 = TRUE;
				break;
			}
		}

		VirtualProtect(MyCreateIStream, countof(MyCreateIStream), PAGE_EXECUTE_READWRITE, &OldProtect);
		
		Status = InitFileSystemXP3();
		if (NT_FAILED(Status))
		{
			MessageBoxW(NULL, L"无法运行补丁", L"补丁启动失败", MB_OK | MB_ICONERROR);
			Ps::ExitProcess(0);
			break;
		}
		
	}
	
	return NT_SUCCESS(Status);
}

BOOL KaresekaHook::UnInit()
{
	return TRUE;
}

