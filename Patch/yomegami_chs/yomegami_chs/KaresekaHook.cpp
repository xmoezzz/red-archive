#include "KaresekaHook.h"
#include "MyHook.h"
#include "tp_stub.h"
#include <string>

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

	CallHostAlloc              = Kareseka->StubHostAlloc;
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
	StubTVPCreateStream  = NULL;
	StubHostAlloc        = NULL;
	StubV2Link           = NULL;
	IStreamAdapterVtable = NULL;
	TVPFunctionExporter  = NULL;
	m_SelfModule         = NULL;
	Inited               = FALSE;
	FileSystemInited     = FALSE;
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
	
	LOOP_ONCE
	{
		StreamAdapter = nullptr;
		QueryFile(lpFileName, &FileName[0], FileBuffer, FileSize, Hash);

		if (!FileBuffer || !FileSize)
			break;

		Holder        = new StreamHolderXP3(FileBuffer, FileSize, Hash);
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
		Stream  = NULL;
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

HRESULT WINAPI HookV2Link(iTVPFunctionExporter *exporter)
{
	NTSTATUS       Status;
	KaresekaHook*  Kareseka;

	Kareseka = GetKareseka();
	TVPInitImportStub(exporter);
	Kareseka->TVPFunctionExporter = exporter;
	Kareseka->StubTVPCreateStream = (FuncCreateStream)GetTVPCreateStreamCall();

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

#include "PSBTool.h"
#pragma comment(lib, "zlib.lib")
#pragma comment(lib, "jsoncpp.lib")

struct TextCompilerResult
{
	ULONG   Line;
	wstring Message;

	TextCompilerResult()
	{
		Line    = 0;
		Message = L"(未知)";
	}
};

#include <vector>

NTSTATUS FASTCALL MyReplaceText(LPCWSTR FileName, Json::Value& DecompiledResult, TextCompilerResult& Result)
{
	NTSTATUS            Status;
	ml::StringT<ULONG>  Text, TextFileName;
	PBYTE               Buffer;
	ULONG               Size, LineIndex;
	NtFileDisk          File;
	std::vector<string> TextPool;
	CHAR                OutString[2000];
	BOOL                ResultOfParse;


	auto ProcessLineXmoe = [](PSTR OutString, ULONG BufferSize, ml::StringT<ULONG>& Input)->BOOL
	{
		BOOL   Result;

		LOOP_ONCE
		{
			Result = FALSE;

			if (StrLengthW(Input) <= 13)
				break;

			if (Input[12] != ']')
				break;

			Input = Input.SubString(13);

			WideCharToMultiByte(CP_UTF8, 0, Input, StrLengthW(Input), OutString, BufferSize, 0, 0);
			Result = TRUE;
		}
		return Result;
	};

	LOOP_ONCE
	{
		Status    = STATUS_UNSUCCESSFUL;
		LineIndex = 0;

		TextFileName = L"Project\\";
		TextFileName += FileName;
		TextFileName += L".txt";

		Status = File.Open(TextFileName);
		if (NT_FAILED(Status))
		{
			Result.Message = L"无法打开对应文本：" + wstring(TextFileName);
			break;
		}

		Size = File.GetSize32();
		Buffer = (PBYTE)AllocateMemoryP(ROUND_UP(Size + 8, 4), HEAP_ZERO_MEMORY);
		if (!Buffer)
		{
			Status = STATUS_UNSUCCESSFUL;
			Result.Message = L"内存不足";
			break;
		}

		File.Seek(2, FILE_BEGIN);
		File.Read(Buffer, Size - 2);
		File.Close();

		Text = (PWSTR)Buffer;

		FreeMemoryP(Buffer);
		
		auto Lines = Text.SplitLines();

		if (Lines.GetSize() == 0)
		{
			Status = STATUS_UNSUCCESSFUL;
			Result.Message = L"空文本";
			break;
		}
		
		for (auto& Line : Lines)
		{
			if (StrLengthW(Line) == 0)
			{
				LineIndex++;
				continue;
			}

			if (Line[0] == L';')
			{
				RtlZeroMemory(OutString, CONST_STRSIZE(OutString));
				ResultOfParse = ProcessLineXmoe(OutString, CONST_STRSIZE(OutString), Line);

				if (ResultOfParse)
				{
					TextPool.push_back(OutString);
				}
				else
				{
					Result.Message = L"文本格式错误";
					Result.Line = LineIndex;
					return STATUS_UNSUCCESSFUL;
				}
			}
			LineIndex++;
		}

		PrintConsoleW(L"文本编译成功, Count = [%d]\n", TextPool.size());

		try
		{
			if (DecompiledResult["scenes"].isNull() && DecompiledResult["list"].isNull())
				return STATUS_UNSUCCESSFUL;

			LineIndex = 0;

			if (!DecompiledResult["scenes"].isNull())
			{
				auto& SceneInfo = DecompiledResult["scenes"];

				if (!SceneInfo["texts"].isNull())
				{
					for (auto& TextLine : SceneInfo["texts"])
					{
						for (ULONG i = 0; i < 3; i++)
						{
							if (TextLine[(int)i].type() == Json::stringValue)
								TextLine[(int)i] = TextPool[LineIndex];

							LineIndex++;
						}
					}
				}

				if (SceneInfo["selects"].isNull())
				{
					for (auto& TextLine : SceneInfo["selects"])
					{
						if (!TextLine["runLineStr"].isNull())
						{
							TextLine["runLineStr"] = TextPool[LineIndex];
							LineIndex++;
						}
					}
				}
			}

			if (!DecompiledResult["list"].isNull())
			{
				auto& ListInfo = DecompiledResult["list"];

				for (auto& TextLine : ListInfo["list"])
				{
					if (!TextLine["title"].isNull())
					{
						TextLine["title"] = TextPool[LineIndex];
						LineIndex++;
					}
				}

				for (auto& TextLine : ListInfo["list"])
				{
					if (!TextLine["selects"].isNull())
					{
						for (auto& AtomLine : TextLine["selects"])
						{
							if (!AtomLine["runLineStr"].isNull())
							{
								AtomLine["runLineStr"] = TextPool[LineIndex];
								LineIndex++;
							}
						}

						for (auto& AtomLine : TextLine["selects"])
						{
							if (!AtomLine["text"].isNull())
							{
								AtomLine["text"] = TextPool[LineIndex];
								LineIndex++;
							}
						}
					}
				}
			}
		}
		catch (std::exception& e)
		{
			PrintConsoleA("Fatal : %s\n", e.what());
		}
		Status = STATUS_SUCCESS;
	}
	PrintConsoleW(L"结束文本编译\n");
	return Status;
}

NTSTATUS NTAPI CompilePSBBinary(LPCWSTR QueryPathName, LPCWSTR FileName, PBYTE& FileBuffer, ULONG& FileSize)
{
	NTSTATUS           Status;
	tTJSBinaryStream*  Stream;
	IStream*           IStream;
	KaresekaHook*      Kareseka;
	PBYTE              PsbBuffer;
	ULONG              PsbSize;
	STATSTG            Stat;
	Json::Value        DecompiledResult;
	Json::Value        ResourceResult;
	TextCompilerResult Result;

	Kareseka = GetKareseka();

	LOOP_ONCE
	{
		Status = STATUS_UNSUCCESSFUL;

		if (!StrStrW(FileName, L".ks.scn"))
			break;

		Stream = Kareseka->StubTVPCreateStream(ttstr(QueryPathName), TJS_BS_READ);
		if (!Stream)
			break;

		IStream = ConvertBStreamToIStream(Stream);
		IStream->Stat(&Stat, STATFLAG_DEFAULT);

		PsbSize = Stat.cbSize.LowPart;
		PsbBuffer = (PBYTE)AllocateMemoryP(PsbSize);
		if (!PsbBuffer)
			break;

		IStream->Read(PsbBuffer, PsbSize, NULL);

		Status = DecompilePSBFile(PsbBuffer, PsbSize, FileName, DecompiledResult, ResourceResult);
		if (NT_FAILED(Status))
			break;

		PrintConsoleW(L"反编译成功:[%s]\n", FileName);

		Status = MyReplaceText(FileName, DecompiledResult, Result);
		if (NT_FAILED(Status))
		{
			WCHAR Info[500];
			RtlZeroMemory(Info, CONST_STRSIZE(Info));
			FormatStringW(Info, L"%s\n文本资源编译错误\nLine:%d\nReason: \n", FileName, Result.Line, Result.Message.c_str());
			//MessageBoxW(NULL, Info, L"汉化补丁编译错误", MB_OK | MB_ICONERROR);
			PrintConsoleW(Info);
			break;
		}

		PrintConsoleW(L"替换汉化文本成功:[%s]\n", FileName);

		Status = CompilePSBFile(FileBuffer, FileSize, DecompiledResult, ResourceResult);
		if (NT_SUCCESS(Status))
			PrintConsoleW(L"编译成功:[%s]\n", FileName);
	}
	return Status;
}


//int WINAPI ExtractPsbText(IStream* Stream, PBYTE& OutBuffer, ULONG& OutSize, wstring& TextName)
NTSTATUS NTAPI CompilePSBBinaryV2(LPCWSTR QueryPathName, LPCWSTR FileName, PBYTE& FileBuffer, ULONG& FileSize)
{
	NTSTATUS           Status;
	tTJSBinaryStream*  Stream;
	IStream*           IStream;
	KaresekaHook*      Kareseka;
	STATSTG            Stat;
	LARGE_INTEGER      Offset;

	Kareseka = GetKareseka();

	LOOP_ONCE
	{
		Status = STATUS_UNSUCCESSFUL;

		if (!StrStrW(FileName, L".ks.scn"))
			break;

		Stream = Kareseka->StubTVPCreateStream(ttstr(QueryPathName), TJS_BS_READ);
		if (!Stream)
			break;

		IStream = ConvertBStreamToIStream(Stream);
		
		if (ExtractPsbText(IStream, FileBuffer, FileSize, FileName) == 0)
		{
			PrintConsoleA("Compiling... Ok\n");
		}
		else
		{
			PrintConsoleA("Compiling... Failed\n");

			Offset.QuadPart = 0;
			IStream->Seek(Offset, FILE_BEGIN, NULL);
			IStream->Stat(&Stat, STATFLAG_DEFAULT);

			FileSize = Stat.cbSize.LowPart;
			FileBuffer = (PBYTE)AllocateMemoryP(FileSize);
			if (!FileBuffer)
				break;

			IStream->Read(FileBuffer, FileSize, NULL);
		}
		Status = STATUS_SUCCESS;
	}
	return Status;
}

NTSTATUS KaresekaHook::QueryFile(LPCWSTR QueryPathName, LPCWSTR FileName, PBYTE& FileBuffer, ULONG& FileSize, ULONG64& Hash)
{
	NtFileDisk File;
	NTSTATUS   Status;
	WCHAR      FullFileName[MAX_PATH];

	if (!FileName)
		return STATUS_NO_SUCH_FILE;

	FileBuffer = NULL;
	FileSize   = 0;
	Hash       = 0;


	if (!FileSystemInited)
	{
		LOOP_ONCE
		{
			Status = CompilePSBBinaryV2(QueryPathName, FileName, FileBuffer, FileSize);
			if (NT_SUCCESS(Status))
				return Status;

			FormatStringW(FullFileName, L"Project//%s", FileName);
			Status = File.Open(FullFileName);
			if (NT_FAILED(Status))
				break;


			FileSize = File.GetSize32();
			FileBuffer = (PBYTE)AllocateMemoryP(FileSize);
			if (!FileBuffer)
			{
				FileSize = 0;
				Status   = STATUS_NO_MEMORY;
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
	return Status;
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


	Kareseka     = GetKareseka();
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


BOOL KaresekaHook::Init(HMODULE hModule)
{
	NTSTATUS      Status;
	LPCWSTR       Message;
	PVOID         FakeCompiler;
	PVOID         ExeModule;

	m_SelfModule = hModule;
	ExeModule = Nt_GetExeModuleHandle();

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
			switch (GetACP())
			{
			case 936:
				Message = L"第一部分启动失败";
				break;

			default:
				Message = L"第一部分啟動失敗";
				break;
			}
			MessageBoxW(NULL, Message, L"错误", MB_OK | MB_ICONERROR);
			break;
		}
	}
	return NT_SUCCESS(Status);
}

BOOL KaresekaHook::UnInit()
{
	return TRUE;
}

