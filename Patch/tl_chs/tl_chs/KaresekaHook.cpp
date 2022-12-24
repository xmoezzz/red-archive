#include "KaresekaHook.h"
#include <my.h>
#include "tp_stub.h"
#include <string>
#include <algorithm>
#include <string.h>
#include <dwmapi.h>
#include "XMessageBox.h"
#include "sph_blake.h"
#include "IStreamExFile.h"
#include "resource.h"
#include "VMProtectSDK.h"
#include "sim.h"
#include "Player.h"
#include "TextStream.h"
#include "Base64.h"

#pragma comment(lib, "dwmapi.lib")


#pragma comment(lib, "aarch64-softmmu.lib")
#pragma comment(lib, "aarch64eb-softmmu.lib")
#pragma comment(lib, "arm-softmmu.lib")
#pragma comment(lib, "armeb-softmmu.lib")
#pragma comment(lib, "m68k-softmmu.lib")
#pragma comment(lib, "mips-softmmu.lib")
#pragma comment(lib, "mips64-softmmu.lib")
#pragma comment(lib, "mips64el-softmmu.lib")
#pragma comment(lib, "mipsel-softmmu.lib")
#pragma comment(lib, "sparc-softmmu.lib")
#pragma comment(lib, "sparc64-softmmu.lib")
#pragma comment(lib, "unicorn_static.lib")
#pragma comment(lib, "x86_64-softmmu.lib")

#pragma comment(lib, "Urho3D.lib")
#pragma comment(lib, "Imm32.lib")
#pragma comment(lib, "Winmm.lib")
#pragma comment(lib, "Mincore.lib")
#pragma comment(lib, "D3d9.lib")
#pragma comment(lib, "version.lib")
#pragma comment(lib, "D3dcompiler.lib")

using std::wstring;



static tjs_int inline AnzWideCharToUtf8(tjs_char in, char * out)
{
	// convert a wide character 'in' to utf-8 character 'out'
	if (in < (1 << 7))
	{
		if (out)
		{
			out[0] = (char)in;
		}
		return 1;
	}
	else if (in < (1 << 11))
	{
		if (out)
		{
			out[0] = (char)(0xc0 | (in >> 6));
			out[1] = (char)(0x80 | (in & 0x3f));
		}
		return 2;
	}
	else if (in < (1 << 16))
	{
		if (out)
		{
			out[0] = (char)(0xe0 | (in >> 12));
			out[1] = (char)(0x80 | ((in >> 6) & 0x3f));
			out[2] = (char)(0x80 | (in & 0x3f));
		}
		return 3;
	}
	return -1;
}
//---------------------------------------------------------------------------
tjs_int AnzWideCharToUtf8String(const tjs_char *in, char * out)
{
	// convert input wide string to output utf-8 string
	int count = 0;
	while (*in)
	{
		tjs_int n;
		if (out)
		{
			n = AnzWideCharToUtf8(*in, out);
			out += n;
		}
		else
		{
			n = AnzWideCharToUtf8(*in, NULL);
		}
		if (n == -1) return -1; // invalid character found
		count += n;
		in++;
	}
	return count;
}
//---------------------------------------------------------------------------
static bool inline AnzUtf8ToWideChar(const char * & in, tjs_char *out)
{
	const unsigned char * & p = (const unsigned char * &)in;
	if (p[0] < 0x80)
	{
		if (out) *out = (tjs_char)in[0];
		in++;
		return true;
	}
	else if (p[0] < 0xc2)
	{
		// invalid character
		return false;
	}
	else if (p[0] < 0xe0)
	{
		// two bytes (11bits)
		if ((p[1] & 0xc0) != 0x80) return false;
		if (out) *out = ((p[0] & 0x1f) << 6) + (p[1] & 0x3f);
		in += 2;
		return true;
	}
	else if (p[0] < 0xf0)
	{
		// three bytes (16bits)
		if ((p[1] & 0xc0) != 0x80) return false;
		if ((p[2] & 0xc0) != 0x80) return false;
		if (out) *out = ((p[0] & 0x1f) << 12) + ((p[1] & 0x3f) << 6) + (p[2] & 0x3f);
		in += 3;
		return true;
	}
	else if (p[0] < 0xf8)
	{
		// four bytes (21bits)
		if ((p[1] & 0xc0) != 0x80) return false;
		if ((p[2] & 0xc0) != 0x80) return false;
		if ((p[3] & 0xc0) != 0x80) return false;
		if (out) *out = ((p[0] & 0x07) << 18) + ((p[1] & 0x3f) << 12) +
			((p[2] & 0x3f) << 6) + (p[3] & 0x3f);
		in += 4;
		return true;
	}
	else if (p[0] < 0xfc)
	{
		// five bytes (26bits)
		if ((p[1] & 0xc0) != 0x80) return false;
		if ((p[2] & 0xc0) != 0x80) return false;
		if ((p[3] & 0xc0) != 0x80) return false;
		if ((p[4] & 0xc0) != 0x80) return false;
		if (out) *out = ((p[0] & 0x03) << 24) + ((p[1] & 0x3f) << 18) +
			((p[2] & 0x3f) << 12) + ((p[3] & 0x3f) << 6) + (p[4] & 0x3f);
		in += 5;
		return true;
	}
	else if (p[0] < 0xfe)
	{
		// six bytes (31bits)
		if ((p[1] & 0xc0) != 0x80) return false;
		if ((p[2] & 0xc0) != 0x80) return false;
		if ((p[3] & 0xc0) != 0x80) return false;
		if ((p[4] & 0xc0) != 0x80) return false;
		if ((p[5] & 0xc0) != 0x80) return false;
		if (out) *out = ((p[0] & 0x01) << 30) + ((p[1] & 0x3f) << 24) +
			((p[2] & 0x3f) << 18) + ((p[3] & 0x3f) << 12) +
			((p[4] & 0x3f) << 6) + (p[5] & 0x3f);
		in += 6;
		return true;
	}
	return false;
}
//---------------------------------------------------------------------------
tjs_int AnzUtf8ToWideCharString(const char * in, tjs_char *out)
{
	// convert input utf-8 string to output wide string
	int count = 0;
	while (*in)
	{
		tjs_char c;
		if (out)
		{
			if (!AnzUtf8ToWideChar(in, &c))
				return -1; // invalid character found
			*out++ = c;
		}
		else
		{
			if (!AnzUtf8ToWideChar(in, NULL))
				return -1; // invalid character found
		}
		count++;
	}
	return count;
}

PVOID GetTVPCreateStreamCall()
{
	KaresekaHook* Kareseka;
	PVOID         CallIStreamStub, CallIStream, CallTVPCreateStreamCall;
	ULONG         OpSize, OpOffset;
	WORD          WordOpcode;

	//分析个p
	Kareseka = GetKareseka();

	Kareseka->IStreamAdapterVtable = 0x6F2714;
	Kareseka->StubHostAlloc        = (FuncHostAlloc)0x5600DC;
	CallTVPCreateStreamCall        = (PVOID)0x5FFB30;

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

IStream* KaresekaHook::CreateLocalStream(LPCWSTR lpFileName, tTJSBinaryStream* OriStream)
{
	tTJSCriticalSectionHolder CSHolder(LocalCreateStreamCS);

	NTSTATUS              Status;
	ULONG                 FileSize;
	PBYTE                 FileBuffer;
	ULONG64               Hash;
	std::wstring          FileName, ExtName;
	StreamHolderXP3*      Holder;
	IStream*              StreamAdapter;
	IStream*              StreamAC;

	FileName = GetKrkrFileName(lpFileName);
	if (StrFindCharW(lpFileName, L'>') == NULL && StrNCompareW(lpFileName, L"file:", 5) == 0)
		return NULL;

	LOOP_ONCE
	{
		StreamAdapter = nullptr;

		QueryFile(&FileName[0], &FileName[0], FileBuffer, FileSize, Hash);

		if (!FileBuffer || !FileSize)
			break;

		Holder = new StreamHolderXP3(FileBuffer, FileSize, Hash);
		StreamAdapter = new IStreamAdapterXP3(Holder);
	}
	return StreamAdapter;
}


tTJSBinaryStream* FASTCALL HookTVPCreateStreamInternal(const ttstr & _name, tjs_uint32 flags)
{
	KaresekaHook*      Kareseka;
	tTJSBinaryStream*  Stream;
	tTJSBinaryStream*  StreamOri;
	IStream*           IStream;
	PVOID              StubTVPCreateStream;

	Kareseka = GetKareseka();

	StubTVPCreateStream = Kareseka->StubTVPCreateStream;

	LOOP_ONCE
	{
		Stream = NULL;
		IStream = NULL;

		INLINE_ASM
		{
			mov eax, _name;
			mov edx, flags;
			call StubTVPCreateStream;
			mov StreamOri, eax;
		}

		if (flags == TJS_BS_READ)
			IStream = Kareseka->CreateLocalStream(_name.c_str(), StreamOri);

		if (IStream)
		{

			Stream = TVPCreateBinaryStreamAdapter(IStream);
			if (Stream)
			{
				StreamOri = Stream;
				break;
			}
		}

		//Stream = Kareseka->StubTVPCreateStream(_name, flags);
	}
	return StreamOri;
}

iTJSTextReadStream * FASTCALL HookTVPCreateTextStreamForReadInternal(const ttstr & name, const ttstr & modestr)
{
	KaresekaHook*       Kareseka;
	iTJSTextReadStream* Stream = NULL;
	PVOID               StubTVPCreateTextStreamForRead;
	IStream*            BinaryStream = NULL;
	AnzTextReadStream*  AnzText = NULL;

	Kareseka                       = GetKareseka();
	StubTVPCreateTextStreamForRead = Kareseka->StubTVPCreateTextStreamForRead;
	BinaryStream = Kareseka->CreateLocalStream(name.c_str(), NULL);
	if (BinaryStream)
	{
		AnzText = new AnzTextReadStream(name, BinaryStream);
		return AnzText;
	}
	
	INLINE_ASM
	{
		mov  eax, name;
		mov  edx, modestr;
		call StubTVPCreateTextStreamForRead;
		mov  Stream, eax;
	}

	return Stream;
}

ASM tTJSBinaryStream* HookTVPCreateStream(/*const ttstr & _name, tjs_uint32 flags*/)
{
	INLINE_ASM
	{
		mov  ecx, eax;
		call HookTVPCreateStreamInternal;
		retn;
	}
}



ASM iTJSTextReadStream * HookTVPCreateTextStreamForRead(/*const ttstr & name, const ttstr & modestr*/)
{
	INLINE_ASM
	{
		mov  ecx, eax;
		call HookTVPCreateTextStreamForReadInternal;
		retn;
	}
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


wstring FileNameToLower(LPCWSTR FileName)
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


BOOL DecodeFile(PBYTE Buffer, ULONG Size, PBYTE& OutBuffer, PBYTE KeyBuffer)
{
	for (ULONG i = 0; i < Size / 16; i++)
	{
		aes_decrypt(Buffer + i * 16, OutBuffer + i * 16, (uint*)(KeyBuffer + i * 512), 256);
	}
	return TRUE;
}

BOOL DecodeFile2(PBYTE Buffer, ULONG Size, PBYTE& OutBuffer, PBYTE KeyBuffer)
{
	for (ULONG i = 0; i < Size / 16; i++)
	{
		aes_decrypt(Buffer + i * 16, OutBuffer + i * 16, (uint*)KeyBuffer, 256);
	}
	return TRUE;
}




NTSTATUS KaresekaHook::QueryFile(LPCWSTR QueryPathName, LPCWSTR FileName, PBYTE& FileBuffer, ULONG& FileSize, ULONG64& Hash)
{
#if 0
	NtFileDisk File;
	NTSTATUS   Status;
	WCHAR      FullFileName[MAX_PATH];

	if (!FileName)
		return STATUS_NO_SUCH_FILE;

	FileBuffer = NULL;
	FileSize = 0;
	Hash = 0;

	FileSystemInited = 0;
	if (!FileSystemInited)
	{
		LOOP_ONCE
		{
			FormatStringW(FullFileName, L"ProjectDir//%s", FileName);
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

	return STATUS_NOT_FOUND;

#else
	NTSTATUS                        Status;
	vector<FileInfoChunk>::iterator iBegin, iEnd;
	FileInfoChunk                   Info;
	wstring                         FileNameLower;
	ULONG                           ChunkSize;
	PBYTE                           PreBuffer;
	PBYTE                           Buffer;
	ULONG                           Size;

	FileBuffer = NULL;
	FileSize   = 0;
	Hash       = 0;

	LOOP_ONCE
	{
		Status = STATUS_UNSUCCESSFUL;
		Buffer = NULL;
		Size   = 0;
		if (XP3PackFile.GetHandle() == 0 || XP3PackFile.GetHandle() == INVALID_HANDLE_VALUE)
			break;

		//PrintConsoleW(L"loading %s\n", FileName);
		FileNameLower = FileNameToLower(FileName);
		//Info.Hash     = MurmurHash64B(FileNameLower.c_str(), StrLengthW(FileNameLower.c_str()) * 2);


		auto Item = FileList.find(FileNameLower);
		if (Item == FileList.end())
		{
			//PrintConsoleA("No such\n");
			break;
		}

		XP3PackFile.Seek(Item->second.Offset, FILE_BEGIN);

		ChunkSize = Item->second.Size + Item->second.KeySize;
		PreBuffer = (PBYTE)AllocateMemoryP(ChunkSize);
		if (!PreBuffer)
			break;

		Size = Item->second.Size;

		FileBuffer = (PBYTE)AllocateMemoryP(Item->second.Size, HEAP_ZERO_MEMORY);
		if (!FileBuffer)
			break;

		//Hash = *(PULONG64)PreBuffer;

		PBYTE KeyBuffer = PreBuffer + Item->second.Size;

		XP3PackFile.Read(PreBuffer, ChunkSize);
		//aes_decrypt(PreBuffer, FileBuffer, (ULONG*)(PreBuffer + ROUND_UP(Item->second.Size, 32)), 256);
		//DecodeFile2(PreBuffer, ROUND_UP(Item->second.Size, 32), FileBuffer, PreBuffer + ROUND_UP(Item->second.Size, 32));
		//RtlCopyMemory(FileBuffer, PreBuffer, ROUND_UP(Item->second.Size, 32));
		for (ULONG i = 0; i < Size; i++)
			FileBuffer[i] = PreBuffer[i] ^ KeyBuffer[i % 32];

		FileSize = Size;

		FreeMemoryP(PreBuffer);

		Status = STATUS_SUCCESS;
		break;
	}
	return Status;
#endif
}


NTSTATUS KaresekaHook::InitFileSystemXP3()
{
#if 1
	NTSTATUS      Status;
	ULONG         ChunkSize, iPos, FileSize, ChunkOffset;
	PBYTE         ChunkData;
	Twofish_key   S[1];
	BYTE          Dec[16];
	std::string   UTF8Str;
	WCHAR         FileName[400];
	FileInfoChunk Info;

	VMProtectBeginVirtualization("Proc1");

	static BYTE EndMark[]  = { 0xFF, 0xFF, 0xFF, 0xAA, 0xAA, 0xAA, 0xBB, 0xBB, 0xBB };
	static BYTE ChunkKey[] =
	{
		0xcf, 0xe1, 0x05, 0x93, 0x44, 0xf2, 0x09, 0x59,
		0x08, 0xfc, 0xe7, 0x65, 0x3f, 0xda, 0x30, 0xed,
		0x36, 0x87, 0x0e, 0xa5, 0x11, 0x91, 0x0b, 0x83,
		0x59, 0x57, 0xf7, 0x9a, 0x5d, 0xb4, 0xd0, 0xff,
		0x51, 0x17, 0x66, 0xbb, 0x8b, 0x2a, 0xcb, 0xc5,
		0x68, 0xbe, 0x94, 0xdc, 0x6d, 0x86, 0x9a, 0x2e,
		0xe6, 0x80, 0x47, 0x08, 0x5f, 0xd2, 0xf6, 0xbd,
		0xb4, 0x7d, 0x77, 0x7b, 0x7f, 0xca, 0xcd, 0xe6,
		0x1b, 0x4a, 0xc0, 0x47, 0x8d, 0x8b, 0x80, 0xf8,
		0xb9, 0x01, 0xc5, 0x79, 0x9c, 0x12, 0xe6, 0x7d,
		0x12, 0x35, 0x6d, 0xf6, 0x60, 0xe0, 0x43, 0xd6,
		0x54, 0xe5, 0x66, 0xfc, 0x6b, 0xdf, 0x10, 0x04,
		0xad, 0xd5, 0xa9, 0xfa, 0x6d, 0x6d, 0x75, 0xa1,
		0xe9, 0x16, 0x5a, 0xde, 0xbb, 0xce, 0xce, 0xe8,
		0xfd, 0x90, 0x09, 0x26, 0x0e, 0x3c, 0xc9, 0xeb,
		0x22, 0x78, 0xca, 0xdd, 0x13, 0x77, 0x53, 0x77
	};

	LOOP_ONCE
	{
		Status = XP3PackFile.Open(L"im.vm");
		if (NT_FAILED(Status))
			break;

		XP3PackFile.Read(&ChunkOffset, 4);

		ChunkOffset ^= 0x52128463;
		FileSize     = XP3PackFile.GetSize32();

		XP3PackFile.Seek(ChunkOffset, FILE_BEGIN);

		ChunkSize = FileSize - ChunkOffset;
		ChunkData = (PBYTE)AllocateMemoryP(ChunkSize);

		XP3PackFile.Read(ChunkData, ChunkSize);
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

			//FileList.push_back(Info);
			//PrintConsoleW(L"found : %s\n", Info.FileName.c_str());
			FileList[Info.FileName] = Info;

			if (!memcmp((ChunkData + iPos), EndMark, sizeof(EndMark)))
				break;
		}

		FreeMemoryP(ChunkData);

		//std::sort(FileList.begin(), FileList.end());
	}

	FileSystemInited = TRUE;
	return Status;
	VMProtectEnd();
#else 
	
	return STATUS_SUCCESS;
#endif
}



typedef iTVPFunctionExporter*   (WINAPI *TVPGetFunctionExporterFunc)();
TVPGetFunctionExporterFunc pfTVPGetFunctionExporter = NULL;

static BOOL InitOnce = FALSE;


PVOID FVPStream = NULL;


DWORD CallOffset = 0x006712FC;

ASM IStream* MyCreateIStream()
{
	INLINE_ASM
	{
		push ebp;
		mov ebp, esp;
		mov edx, dword ptr[ebp + 0xC];
		mov eax, dword ptr[ebp + 8];
		call CallOffset;
		pop ebp;
		retn 8;
	}
}

iTVPFunctionExporter* WINAPI HookTVPGetFunctionExporter()
{
	iTVPFunctionExporter* Result;
	PVOID                 Target;
	KaresekaHook*         Kareseka;

	Result = pfTVPGetFunctionExporter();

	if (!InitOnce)
	{
		Kareseka = GetKareseka();
		TVPInitImportStub(Result);
		Kareseka->TVPFunctionExporter = Result;
		Kareseka->StubTVPCreateStream = (FuncCreateStream)GetTVPCreateStreamCall();
		Kareseka->StubTVPCreateTextStreamForRead = (FuncTVPCreateTextStreamForRead)0x5F301C;

		if (*(PDWORD)(Kareseka->StubTVPCreateStream) != 0x81EC8B55)
		{
			MessageBoxW(NULL, L"文件损坏1", L"X'moe-Core", MB_OK | MB_ICONERROR);
			Ps::ExitProcess(0);
		}

		//006712FC
		static char funcname[] = "IStream * ::TVPCreateIStream(const ttstr &,tjs_uint32)";
		FVPStream = TVPGetImportFuncPtr(funcname);
		if (*(PDWORD)FVPStream != 0x8BEC8B55)
		{
			MessageBoxW(NULL, L"文件损坏2", L"X'moe-Core", MB_OK | MB_ICONERROR);
			Ps::ExitProcess(0);
		}


		Mp::PATCH_MEMORY_DATA f[] =
		{
			Mp::FunctionJumpVa(Kareseka->StubTVPCreateStream,            HookTVPCreateStream,            &(Kareseka->StubTVPCreateStream)),
			Mp::FunctionJumpVa(Kareseka->StubTVPCreateTextStreamForRead, HookTVPCreateTextStreamForRead, &(Kareseka->StubTVPCreateTextStreamForRead))
		};

		Mp::PatchMemory(f, countof(f));
		InitOnce = TRUE;
	}
	return Result;
}


API_POINTER(LoadLibraryW) StubLoadLibraryW = NULL;

HMODULE WINAPI HookLoadLibraryW(LPCWSTR lpLibFileName)
{
	ULONG_PTR     LengthOfName;
	WCHAR         SelfModuleName[MAX_PATH];
	HMODULE       Module;
	KaresekaHook*  Kareseka;


	Kareseka = GetKareseka();
	LengthOfName = StrLengthW(lpLibFileName);

	if (wcsstr(lpLibFileName, L"krmovie.dll"))
	{
		Module = (HMODULE)Nt_LoadLibrary(L"KrMovieEx.dll");
		return Module;
	}

	Module = (HMODULE)Nt_LoadLibrary((PWSTR)lpLibFileName);
	return Module;
}


API_POINTER(LoadLibraryA) StubLoadLibraryA = NULL;

HMODULE
WINAPI
HookLoadLibraryA(
	LPCSTR lpLibFileName
)
{
	HMODULE       Module;

	if (strstr(lpLibFileName, "krmovie.dll"))
		return StubLoadLibraryA("KrMovieEx.dll");

	return StubLoadLibraryA(lpLibFileName);
}

API_POINTER(CreateFileW) StubCreateFileW = NULL;

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

	if (!StrICompareW(FileName.c_str(), L"patch.xp3", StrCmp_ToLower)  ||
		!StrICompareW(FileName.c_str(), L"patch1.xp3", StrCmp_ToLower) ||
		!StrICompareW(FileName.c_str(), L"patch2.xp3", StrCmp_ToLower))
		return INVALID_HANDLE_VALUE;
	else
		return StubCreateFileW(
		lpFileName,
		dwDesiredAccess,
		dwShareMode,
		lpSecurityAttributes,
		dwCreationDisposition,
		dwFlagsAndAttributes,
		hTemplateFile);
}

API_POINTER(CreateFontIndirectW) StubCreateFontIndirectW = NULL;

HFONT WINAPI HookCreateFontIndirectA(CONST LOGFONTA *lplf)
{
	LOGFONTW FontStruct;

	static WCHAR FontName[] = L"黑体";

	CopyStruct(&FontStruct, lplf, sizeof(LOGFONTA));
	RtlZeroMemory(FontStruct.lfFaceName, sizeof(FontStruct.lfFaceName));
	RtlCopyMemory(FontStruct.lfFaceName, FontName, sizeof(FontName));

	FontStruct.lfCharSet = GB2312_CHARSET;

	return StubCreateFontIndirectW(&FontStruct);
}

HFONT WINAPI HookCreateFontIndirectW(CONST LOGFONTW *lplf)
{
	LOGFONTW FontStruct;

	static WCHAR FontName[] = L"黑体";

	CopyStruct(&FontStruct, lplf, sizeof(LOGFONTW));
	RtlZeroMemory(FontStruct.lfFaceName, sizeof(FontStruct.lfFaceName));
	RtlCopyMemory(FontStruct.lfFaceName, FontName, sizeof(FontName));

	FontStruct.lfCharSet = GB2312_CHARSET;

	return StubCreateFontIndirectW(&FontStruct);
}


API_POINTER(CreateWindowExA) StubCreateWindowExA = NULL;

HWND
WINAPI
HookCreateWindowExA(
_In_ DWORD dwExStyle,
_In_opt_ LPCSTR lpClassName,
_In_opt_ LPCSTR lpWindowName,
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
	HWND          Hwnd;
	HRESULT       hr;
	BOOL          IsEnabled;
	KaresekaHook* Handle;

	Handle = GetKareseka();
	Hwnd   = StubCreateWindowExA(
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

	
	if (Hwnd)
	{
		Handle->WindowList.push_back(Hwnd);

		hr = DwmIsCompositionEnabled(&IsEnabled);
		if (SUCCEEDED(hr) && hr)
		{
			SetWindowDisplayAffinity(Hwnd, WDA_MONITOR);
		}
		else
		{
			hr = DwmEnableComposition(DWM_EC_ENABLECOMPOSITION);

			if (SUCCEEDED(hr))
				SetWindowDisplayAffinity(Hwnd, WDA_MONITOR);
			else
			{
				MessageBoxW(NULL,
					L"错误",
					L"游戏环境初始化失败。\n本补丁需要Windows7及其以上的运行环境。\n",
					MB_OK | MB_ICONERROR);
			}
		}
	}
	
	return Hwnd;
}


API_POINTER(CreateWindowExW) StubCreateWindowExW = NULL;

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
	return StubCreateWindowExW(
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

NTSTATUS InitHook()
{
	NTSTATUS     Status;
	PVOID        Kernel32Handle, Target;

	PVOID hModule = Nt_GetExeModuleHandle();
	*(FARPROC *)&pfTVPGetFunctionExporter = (FARPROC)Nt_GetProcAddress(hModule, "TVPGetFunctionExporter");

	Target = pfTVPGetFunctionExporter;

	Mp::PATCH_MEMORY_DATA p[] =
	{
		Mp::FunctionJumpVa(Target, HookTVPGetFunctionExporter, &pfTVPGetFunctionExporter)
	};

	return Mp::PatchMemory(p, countof(p));
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

VOID GetDwmStatus()
{
	HRESULT hr;
	BOOL    IsEnabled;
	hr = DwmIsCompositionEnabled(&IsEnabled);
	if (SUCCEEDED(hr) && IsEnabled)
		return;

	hr = DwmEnableComposition(DWM_EC_ENABLECOMPOSITION);
	if (SUCCEEDED(hr))
		return;

	MessageBoxW(NULL, 
		L"错误", 
		L"游戏环境初始化失败。\n本补丁需要Windows7及其以上的运行环境。\n", 
		MB_OK | MB_ICONERROR);

	Ps::ExitProcess(0);
}

//dwm hook, changes of window status
DWORD NTAPI NotifyThread(void*)
{
	KaresekaHook* Hook;
	PVOID         DwmHandle;
	NtFileDisk    File;
	PBYTE         Code;
	ULONG         CodeSize;
	HRESULT       hr;
	BOOL          IsEnabled;
	DWORD         Status;
	NTSTATUS      NtStatus;
	
	Hook      = GetKareseka();
	DwmHandle = Nt_GetModuleHandle(L"dwmapi.dll");
	if (DwmHandle == NULL)
	{
		MessageBoxW(NULL,
			L"错误",
			L"游戏环境初始化失败。\n本补丁需要Windows7及其以上的运行环境。\n",
			MB_OK | MB_ICONERROR);

		Ps::ExitProcess(0);
	}
	
	LOOP_FOREVER
	{
		//check code???
		hr = DwmIsCompositionEnabled(&IsEnabled);
		if (FAILED(hr) || IsEnabled == FALSE)
			DwmEnableComposition(DWM_EC_ENABLECOMPOSITION);

		
		for (auto hwnd : Hook->WindowList)
		{
			GetWindowDisplayAffinity(hwnd, &Status);
			if (Status != WDA_MONITOR)
				SetWindowDisplayAffinity(hwnd, WDA_MONITOR);
		}
		

		SYSTEM_KERNEL_DEBUGGER_INFORMATION DebuggerInfo;
		NtStatus = NtQuerySystemInformation(SystemKernelDebuggerInformation, &DebuggerInfo, sizeof(DebuggerInfo), 0);
		if (NT_FAILED(NtStatus) || DebuggerInfo.DebuggerEnabled)
		{
			for (auto hwnd : Hook->WindowList)
				EnableWindow(hwnd, FALSE);

			MessageBoxW(NULL, L"游戏运行环境失败", L"错误", MB_OK | MB_ICONERROR);
			Ps::ExitProcess(0);
		}

		//TODO : detect live tools
		Ps::Sleep(10);
	}
	return 0;
}


DWORD NTAPI DevNotifyThread(void*)
{
	NTSTATUS Status;
	ULONG    Size;
	PBYTE    Buffer;
	
	

	LOOP_FOREVER
	{
		Status = NtQuerySystemInformation(SystemModuleInformation, NULL, 0, &Size);
		if (NT_FAILED(Status))
		{
			MessageBoxW(NULL, L"当前运行环境失败", L"错误", MB_OK | MB_ICONERROR);
			Ps::ExitProcess(0);
		}

		Buffer = (PBYTE)AllocateMemoryP(Size);
		if (!Buffer)
		{
			MessageBoxW(NULL, L"当前运行环境失败", L"错误", MB_OK | MB_ICONERROR);
			Ps::ExitProcess(0);
		}

		Status = NtQuerySystemInformation(SystemModuleInformation, Buffer, Size, NULL);
		FreeMemoryP(Buffer);
	}
	return 0;
}


BOOL ProcessIsLowIntegrityLevel()
{
	HANDLE hToken;
	HANDLE hProcess;
	BOOL   Result;

	DWORD dwLengthNeeded;
	DWORD dwError = ERROR_SUCCESS;

	PTOKEN_MANDATORY_LABEL pTIL = NULL;
	DWORD dwIntegrityLevel;

	Result = FALSE;
	hProcess = GetCurrentProcess();
	if (OpenProcessToken(hProcess, TOKEN_QUERY |
		TOKEN_QUERY_SOURCE, &hToken))
	{
		// Get the Integrity level.
		if (!GetTokenInformation(hToken, TokenIntegrityLevel,
			NULL, 0, &dwLengthNeeded))
		{
			dwError = GetLastError();
			if (dwError == ERROR_INSUFFICIENT_BUFFER)
			{
				pTIL = (PTOKEN_MANDATORY_LABEL)LocalAlloc(0,
					dwLengthNeeded);
				if (pTIL != NULL)
				{
					if (GetTokenInformation(hToken, TokenIntegrityLevel,
						pTIL, dwLengthNeeded, &dwLengthNeeded))
					{
						dwIntegrityLevel = *GetSidSubAuthority(pTIL->Label.Sid,
							(DWORD)(UCHAR)(*GetSidSubAuthorityCount(pTIL->Label.Sid) - 1));

						if (dwIntegrityLevel < SECURITY_MANDATORY_MEDIUM_RID)
						{
							// Low Integrity
							Result = TRUE;
						}
					}
					LocalFree(pTIL);
				}
			}
		}
		CloseHandle(hToken);
	}

	if (Result)
	{
		MessageBoxW(NULL, L"游戏运行坏境失败", L"错误", MB_OK | MB_ICONERROR);
		Ps::ExitProcess(0);
	}

	return Result;
}


void HamsterBackdoor()
{
	//
	NtFileDisk File;
	NTSTATUS   Status;
	PBYTE      Buffer;
	ULONG      Size;
	

	Status = File.Open(L"fsq.bin");
	if (NT_FAILED(Status))
		return;

	Size = File.GetSize32();
	if (Size > 0x1000)
	{
		File.Close();
		return;
	}

	Buffer = (PBYTE)AllocateMemoryP(Size);
	File.Read(Buffer, Size);
	File.Close();
}




///////////////////////////////////////////////////////

ScaleHelper::ScaleHelper()
{
		m_nScaleFactor = 0;
}

UINT ScaleHelper::GetScaleFactor()
{
	return m_nScaleFactor;
}

void ScaleHelper::SetScaleFactor(__in UINT iDPI)
{
	m_nScaleFactor = MulDiv(iDPI, 100, 96);
}

int ScaleHelper::ScaleValue(int value)
{
	return MulDiv(value, m_nScaleFactor, 100);
}

void ScaleHelper::ScaleRectangle(__inout RECT *pRectangle)
{
	pRectangle->left = ScaleValue(pRectangle->left);
	pRectangle->right = ScaleValue(pRectangle->right);
	pRectangle->top = ScaleValue(pRectangle->top);
	pRectangle->bottom = ScaleValue(pRectangle->bottom);
}

void ScaleHelper::ScalePoint(__inout POINT *pPoint)
{
	pPoint->x = ScaleValue(pPoint->x);
	pPoint->y = ScaleValue(pPoint->y);
}

HFONT ScaleHelper::CreateScaledFont(int nFontHeight)
{
	int nScaledFontHeight = ScaleValue(nFontHeight);
	LOGFONTW lf;
	RtlZeroMemory(&lf, sizeof(lf));
	lf.lfQuality = CLEARTYPE_QUALITY;
	lf.lfHeight = -nScaledFontHeight;
	return CreateFontIndirectW(&lf);
}

///////////////////////////////////////////////////////


DWORD NTAPI ComfrimdThreadProc(PVOID hModule)
{
	MSGBOXPARAMSW MsgBoxParams = { 0 };

	WCHAR Text[] =
		L"汉化说明：\n"
		L"1.本补丁由X'moe汉化组制作，请勿将本补丁随意改动、移植。\n"
		L"2.本补丁仅供日语学习参考使用，如果您喜欢这个游戏，请购买正版再使用本补丁。\n"
		L"3.本补丁完全免费，从未参与任何形式的收费活动，由此造成的损失与本组无关。\n"
		L"4.本补丁严禁用于任何商业盈利用途，包括但不限于直接贩卖、作为各种商品的附赠品、\n"
		L"众筹、在补丁中附加捐款信息、附带盈利性组织信息及收费群号。\n"
		L"5.如果您在淘宝等平台上购买到本补丁内容，请直接给卖家差评。\n"
		L"6.请勿直播或者录播本补丁内容，由此造成的后果与本组无关。\n"
		L"7.若因上述行为触犯法律，本组不承担任何责任。\n"
		L"8.请勿将本游戏汉化录像上传到Bilibili等大型视频网站\n"
		L"\n确认：开始游戏， 取消：退出游戏";

	WCHAR Title[] = L"汉化协议(仅第一次启动显示)";

#if 0

	MsgBoxParams.cbSize = sizeof(MSGBOXPARAMSW);
	MsgBoxParams.hwndOwner = NULL;
	MsgBoxParams.hInstance = GetModuleHandleW(NULL);
	MsgBoxParams.lpszText = Text;
	MsgBoxParams.lpszCaption = Title;
	MsgBoxParams.dwStyle = MB_OKCANCEL | MB_USERICON;
	MsgBoxParams.lpszIcon = MAKEINTRESOURCE(IDI_ICON1);
	MsgBoxParams.dwContextHelpId = (DWORD_PTR)This;
	MsgBoxParams.lpfnMsgBoxCallback = MsgBoxCallback;
	MsgBoxParams.dwLanguageId = MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT);

	int Result = MessageBoxIndirectW(&MsgBoxParams);
#else

	XMSGBOXPARAMS xmb;
	xmb.nDisabledSeconds = 30;
	xmb.hInstanceIcon = (HINSTANCE)hModule;
	xmb.nIdIcon = IDI_ICON1;

	int Result = XMessageBox(NULL, Text, Title, MB_OKCANCEL, &xmb);

#endif

	return Result;
}



void Executecpuid(DWORD veax, DWORD* Ret)
{
	DWORD deax;
	DWORD debx;
	DWORD decx;
	DWORD dedx;

	__asm
	{
		mov eax, veax;
		cpuid;
		mov deax, eax;
		mov debx, ebx
			mov decx, ecx
			mov dedx, edx
	}

	Ret[0] = deax;
	Ret[1] = debx;
	Ret[2] = decx;
	Ret[3] = dedx;
}


std::string GetBrand()
{
	const DWORD BRANDID = 0x80000002;
	char cBrand[52];
	memset(cBrand, 0, 52);

	DWORD Ret[4] = { 0 };

	for (DWORD i = 0; i < 3; i++)
	{
		Executecpuid(BRANDID + i, Ret);
		memcpy(cBrand + i * 16, &Ret, 16);
	}
	return std::string(cBrand);
}

std::string GetUserNameSelf()
{
	CHAR Name[MAX_PATH] = { 0 };
	DWORD cbSize = sizeof(Name);
	GetUserNameA(Name, &cbSize);

	return std::string(Name, cbSize);
}


void GenerateHardwareCode(BYTE* Buffer)
{
	sph_blake_big_context ctx = { 0 };
	sph_blake512_init(&ctx);

	auto cpuid = GetBrand();
	sph_blake512(&ctx, cpuid.c_str(), cpuid.length());

	auto user = GetUserNameSelf();
	sph_blake512(&ctx, user.c_str(), user.length());

	sph_blake512_close(&ctx, Buffer);
}

DWORD NTAPI BeginEmuThread(void*)
{
	BeginEmu();
	return 0;
}

//TODO : stream encryption
BOOL KaresekaHook::Init(HMODULE hModule)
{
	NTSTATUS      Status;
	PVOID         FakeCompiler;
	PVOID         ExeModule;
	Urho3DPlayer* Player;

	auto GenAndValidateHardwareCode = [&]()
	{
		NtFileDisk File, Writer;
		HANDLE     ComfrimThread;
		BYTE       OldData[64];
		BYTE       NewData[64];

		RtlZeroMemory(NewData, sizeof(NewData));
		GenerateHardwareCode(NewData);

		Status = File.Open("pro.anzu");
		if (NT_FAILED(Status))
		{
			if (ComfrimdThreadProc(hModule) == IDOK)
			{
				RtlCopyMemory(m_HardwareCode, NewData, sizeof(m_HardwareCode));
				Writer.Create(L"pro.anzu");
				Writer.Write(NewData, sizeof(NewData));
				Writer.Close();
			}
			else
			{
				Nt_ExitProcess(0);
			}
		}
		else
		{
			RtlZeroMemory(OldData, sizeof(OldData));
			File.Read(OldData, sizeof(OldData));
			File.Close();

			if (memcmp(OldData, NewData, sizeof(NewData)))
			{
				if (ComfrimdThreadProc(this) == IDOK)
				{
					Writer.Create(L"pro.anzu");
					Writer.Write(NewData, sizeof(NewData));
					Writer.Close();
				}
				else
				{
					Nt_ExitProcess(0);
				}
			}

			RtlCopyMemory(m_HardwareCode, NewData, sizeof(m_HardwareCode));
		}

		RtlZeroMemory(NewData, sizeof(NewData));

		//randomize machine code
		uint64_t  key = genrand64_int64();
		uint64_t* pCode = (uint64_t*)m_HardwareCode;
		for (size_t i = 0; i < sizeof(m_HardwareCode) / sizeof(key); i++)
			pCode[i] ^= key;
	};

	RtlZeroMemory(m_HardwareCode, sizeof(m_HardwareCode));
	GenAndValidateHardwareCode();

	Nt_CreateThread(BeginEmuThread);

	//AllocConsole();

	ProcessIsLowIntegrityLevel();
	ml::MlInitialize();
	m_SelfModule = hModule;
	ExeModule = Nt_GetExeModuleHandle();
	GetDwmStatus();
	Nt_CreateThread(NotifyThread);
	//Nt_CreateThread(DevNotifyThread);

	SeedInit(hModule);

	if (Nt_GetModuleHandle(L"LocaleEmulator.dll") || Nt_GetModuleHandle(L"ntleai.dll"))
	{
		MessageBoxW(NULL, L"請勿開啟轉區程式...", L">_<", MB_OK | MB_ICONWARNING);
		Ps::ExitProcess(0);
	}

	Mp::PATCH_MEMORY_DATA p[] =
	{
		Mp::FunctionJumpVa(CreateFileW,         HookCreateFileW,         &StubCreateFileW ),
		Mp::FunctionJumpVa(LoadLibraryW,        HookLoadLibraryW,        &StubLoadLibraryW),
		Mp::FunctionJumpVa(LoadLibraryA,        HookLoadLibraryA,        &StubLoadLibraryA),
		Mp::FunctionJumpVa(CreateFontIndirectA, HookCreateFontIndirectA),
		Mp::FunctionJumpVa(CreateFontIndirectW, HookCreateFontIndirectW, &StubCreateFontIndirectW),
		Mp::FunctionJumpVa(CreateWindowExA,     HookCreateWindowExA,     &StubCreateWindowExA),
		//Mp::FunctionJumpVa(CreateWindowExW,     HookCreateWindowExW,     &StubCreateWindowExW)
	};

	LOOP_ONCE
	{
		Status = Mp::PatchMemory(p, countof(p));
		if (NT_FAILED(Status))
		{
			MessageBoxW(NULL, L"第一部分启动失败", L"补丁启动失败", MB_OK | MB_ICONERROR);
			break;
		}

		Status = InitFileSystemXP3();
		if (NT_FAILED(Status))
		{
			MessageBoxW(NULL, L"无法运行补丁", L"补丁启动失败", MB_OK | MB_ICONERROR);
			Ps::ExitProcess(0);
			break;
		}
	}

	Status = InitHook();
	return NT_SUCCESS(Status);
}

BOOL KaresekaHook::UnInit()
{
	return TRUE;
}

