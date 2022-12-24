#include "my.h"

#include "ShinkuDef.h"

#include "lz4.h"
#include "QuickLZ.h"
#include "zlib.h"
#include "zstd.h"
#include "my.h"
#include "snappy-C.h"
#include "zstd.h"
#include "RLE.h"
#include "LZJB.h"
#include "FastLZ.h"
#include "minilzo.h"
#include "lzrw1.h"
#include "brieflz.h"
#include "lzfse.h"
#include "lzfse_internal.h"
#include "brotli_decoder.h"
#include "blosclz.h"
#include "lzss.h"
#include "encode.h"

#include "StreamDecoder.h"
#include "SecondStreamDecoder.h"
#include "ASE.h"

#include "GlobalMap.h"
#include "Init2ndKey.h"
#include "Init1stKey.h"

#include "ecrypt-sync.h"
#include "py.h"
#include "Init2ndKey.h"
extern "C"
{
#include "phelix.h"
}

#include <WinFile.h>

#include <string>
#include <time.h>
#include <stdlib.h>

using std::wstring;

#pragma comment(lib, "zstdlib_x86.lib")
#pragma comment(lib, "zlibstat_sse.lib")
#pragma comment(lib, "brotli_enc.lib")
#pragma comment(lib, "brotli_dec.lib")
#pragma comment(lib, "brotli_common.lib")
#pragma comment(lib, "Psapi.lib")


FORCEINLINE DWORD DecodeCompressLength(DWORD FakeLength, DWORD NtKey)
{
	return ((NtKey * 0x23333333u) ^ (NtKey >> 16) * 0xAB561274) ^ FakeLength;
}

FORCEINLINE BYTE ByteFilter(register BYTE b)
{
	b = (BYTE)((b * 0x0802LU & 0x22110LU) | (b * 0x8020LU & 0x88440LU)) * 0x10101LU >> 16;
	return b;
}

VOID DecodeHeaderInfo(PBYTE Header, DWORD Mask)
{
	BYTE InitVector = (Mask >> 24) & 0xFFu;
	BYTE KeyStream;

	for (ULONG Index = 0; Index < 12; Index++)
	{
		KeyStream = InitVector * 0x561;
		InitVector = ByteFilter(InitVector) * 0x45238471;
		Header[Index] ^= KeyStream;
	}
	KeyStream = 0;
}


/*
8bit : Vector-based shell code mask key
8Bit : Vector-based shell code selector
8Bit : 2nd Encode
4Bit : Compress
4Bit : 1st Encode
*/


Void 初始化打包流光()
{
	ECRYPT_init();
	PhelixInit();
	PyECRYPT_init();
	InitAll1StKey();
	InitAll2ndKey();
}


#pragma pack(push, 1)
typedef struct SWEncodeHeader
{
	DWORD EncodeInfo;
	DWORD FileKey;
	DWORD NtBaseDecodeKey;
	DWORD DecompressSize;
}SWEncodeHeader, *pSWEncodeHeader;
#pragma pack(pop)



Void FilledWithKey(PBYTE Buffer, ULONG RawSize, ULONG PaddedSize, BYTE Key)
{
	if (RawSize == PaddedSize)
		return;

	for (ULONG i = RawSize; i < PaddedSize; i++)
	{
		Buffer[i] = Key;
	}
}

int wmain(int argc, WCHAR* argv[])
{
	if (argc != 2)
		return 0;

	WinFile File;
	PBYTE   Buffer;
	ULONG   Size;
	PBYTE   CompressBuffer;
	INT     Result;
	
	union
	{
		size_t tSize;
		SIZE_T TSize;
		ULONG  USize;
		LONG   LSize;
		INT    ISize;
	}CompressSize;

	ULONG PackedCompressSize;

	ml::MlInitialize();
	srand((int)time(NULL));
	//init global map!!!

	初始化打包流光();

	if (File.Open(argv[1], WinFile::FileRead) != S_OK)
		return 0;

	if (File.GetSize32() % 16)
		Size = File.GetSize32() + 16 - (File.GetSize32() % 16);
	else
		Size = File.GetSize32();

	Buffer = (PBYTE)AllocateMemoryP(Size, HEAP_ZERO_MEMORY);
	if (!Buffer)
	{
		printf("alloc failed(1)\n");
		return 0;
	}

	File.Read(Buffer, Size);
	File.Release();

	SWEncodeHeader Header;
	Header.EncodeInfo = (DWORD)(rand() | (rand() << 16));
	Header.FileKey = (DWORD)(rand() | (rand() << 16));
	Header.NtBaseDecodeKey = (DWORD)(rand() | (rand() << 16));
	
	FinalDecoder(Buffer, Size, (Header.EncodeInfo >> 24) & 0xFF);

	ULONG NextKey = Header.FileKey * (BYTE)((Header.EncodeInfo >> 8) & 0xFF);
	DragonDecoder(Buffer, Size, NextKey, (Header.EncodeInfo >> 16) & 0xFF);

	CompressBuffer = (PBYTE)AllocateMemoryP(Size * 4, HEAP_ZERO_MEMORY);
	if (!CompressBuffer)
	{
		printf("alloc failed(2)\n");
		return 0;
	}


	switch ((Header.EncodeInfo >> 4) & 0x0000000Fu)
	//switch (14)
	{
	case 0:
		CompressSize.ISize = LZ4_compress((char*)(Buffer), (char*)CompressBuffer, Size);
		if (CompressSize.ISize % 16)
			PackedCompressSize = CompressSize.ISize + 16 - (CompressSize.ISize % 16);
		else
			PackedCompressSize = CompressSize.ISize;
		break;

	case 1:
		CompressSize.USize = Size * 4;
		Result = compress(CompressBuffer, &CompressSize.USize, Buffer, Size);
		if (Result != Z_OK)
		{
			printf("zlib error\n");
			Ps::ExitProcess(0);
		}
		if (CompressSize.ISize % 16)
			PackedCompressSize = CompressSize.ISize + 16 - (CompressSize.ISize % 16);
		else
			PackedCompressSize = CompressSize.ISize;
		break;

	case 2:
		CompressSize.USize = Size * 4;
		Result = snappy_compress((PCHAR)Buffer, Size, (PCHAR)CompressBuffer, &CompressSize.tSize);
		if (Result != SNAPPY_OK)
		{
			printf("snappy error\n");
			Ps::ExitProcess(0);
		}
		if (CompressSize.ISize % 16)
			PackedCompressSize = CompressSize.ISize + 16 - (CompressSize.ISize % 16);
		else
			PackedCompressSize = CompressSize.ISize;
		break;

	case 3:
		CompressSize.USize = Size * 4;
		CompressSize.USize = ZSTD_compress(CompressBuffer, CompressSize.USize, Buffer, Size, 1);
		if (ZSTD_isError(CompressSize.USize))
		{
			printf("zstd error\n");
			Ps::ExitProcess(0);
		}
		if (CompressSize.ISize % 16)
			PackedCompressSize = CompressSize.ISize + 16 - (CompressSize.ISize % 16);
		else
			PackedCompressSize = CompressSize.ISize;
		break;

	case 4:
		UCL_NRV2E_Compress(Buffer, Size, CompressBuffer, &CompressSize.USize);
		if (CompressSize.ISize % 16)
			PackedCompressSize = CompressSize.ISize + 16 - (CompressSize.ISize % 16);
		else
			PackedCompressSize = CompressSize.ISize;
		break;

	case 5:
	{
		qlz_state_compress S[1];
		CompressSize.tSize = qlz_compress(Buffer, (PCHAR)CompressBuffer, Size, S);
		if (CompressSize.ISize % 16)
			PackedCompressSize = CompressSize.ISize + 16 - (CompressSize.ISize % 16);
		else
			PackedCompressSize = CompressSize.ISize;
	}
	break;

	case 6:
	{
		CRLE Rle;
		Rle.Encode(CompressBuffer, CompressSize.LSize, Buffer, Size);
		if (CompressSize.ISize % 16)
			PackedCompressSize = CompressSize.ISize + 16 - (CompressSize.ISize % 16);
		else
			PackedCompressSize = CompressSize.ISize;
	}
	break;

	case 7:
		CompressSize.tSize = lzjb_compress(Buffer, CompressBuffer, Size, Size * 2);
		if (CompressSize.ISize % 16)
			PackedCompressSize = CompressSize.ISize + 16 - (CompressSize.ISize % 16);
		else
			PackedCompressSize = CompressSize.ISize;
		break;

	case 8:
		CompressSize.ISize = fastlz_compress(Buffer, Size, CompressBuffer);
		if (CompressSize.ISize % 16)
			PackedCompressSize = CompressSize.ISize + 16 - (CompressSize.ISize % 16);
		else
			PackedCompressSize = CompressSize.ISize;
		break;

	case 9:
	{
		CompressSize.ISize = Size * 2;
		PVOID WorkMem = AllocateMemoryP(Size * 4);
		if (!WorkMem)
		{
			printf("lzo mem\n");
			Ps::ExitProcess(0);
		}
		Result = lzo1x_1_compress(Buffer, Size,
			CompressBuffer, &CompressSize.USize, WorkMem);
		if (Result != LZO_E_OK)
		{
			printf("lzo\n");
			Ps::ExitProcess(0);
		}
		if (CompressSize.ISize % 16)
			PackedCompressSize = CompressSize.ISize + 16 - (CompressSize.ISize % 16);
		else
			PackedCompressSize = CompressSize.ISize;

		FreeMemoryP(WorkMem);
	}
		break;

	case 0xA:
#if 0
		lzrw1_compress(Buffer, Size, CompressBuffer, &CompressSize.USize);
		if (CompressSize.ISize % 16)
			PackedCompressSize = CompressSize.ISize + 16 - (CompressSize.ISize % 16);
		else
			PackedCompressSize = CompressSize.ISize;
#else
		CompressSize.tSize = lzjb_compress(Buffer, CompressBuffer, Size, Size * 2);
		//XorMemory(CompressBuffer, Header.FileKey, CompressSize.tSize);
		if (CompressSize.ISize % 16)
			PackedCompressSize = CompressSize.ISize + 16 - (CompressSize.ISize % 16);
		else
			PackedCompressSize = CompressSize.ISize;
#endif
		break;

	case 0xB:
	{
		PVOID WorkMem = AllocateMemoryP(Size * 4);
		if (!WorkMem)
		{
			printf("blz mem\n");
			Ps::ExitProcess(0);
		}
		CompressSize.ISize = blz_pack(Buffer, CompressBuffer, Size, WorkMem);
		if (CompressSize.ISize % 16)
			PackedCompressSize = CompressSize.ISize + 16 - (CompressSize.ISize % 16);
		else
			PackedCompressSize = CompressSize.ISize;

		FreeMemoryP(WorkMem);
	}
		break;

	case 0xC:
	{
		lzfse_encoder_state S[1];
		CompressSize.ISize = lzfse_encode_buffer(CompressBuffer, Size * 2, Buffer, Size, S);

		if (CompressSize.ISize % 16)
			PackedCompressSize = CompressSize.ISize + 16 - (CompressSize.ISize % 16);
		else
			PackedCompressSize = CompressSize.ISize;
	}
	break;

	default:
		CompressSize.ISize = blosclz_compress(1, Buffer, Size, CompressBuffer, Size * 2, 1);

		if (CompressSize.ISize % 16)
			PackedCompressSize = CompressSize.ISize + 16 - (CompressSize.ISize % 16);
		else
			PackedCompressSize = CompressSize.ISize;
		break;

	case 0xD:
	case 0xE:
		memcpy(CompressBuffer, Buffer, Size);
		CompressSize.ISize = Size;
		if (CompressSize.ISize % 16)
			PackedCompressSize = CompressSize.ISize + 16 - (CompressSize.ISize % 16);
		else
			PackedCompressSize = CompressSize.ISize;
		break;
		
	case 0xF:
		CompressSize.tSize = lzss_encode(Buffer, Size, CompressBuffer, Size * 2);
		if (CompressSize.ISize % 16)
			PackedCompressSize = CompressSize.ISize + 16 - (CompressSize.ISize % 16);
		else
			PackedCompressSize = CompressSize.ISize;
		break;
	}

	//Generate a key
	BYTE FillKey = (BYTE)rand();

	FilledWithKey(CompressBuffer, CompressSize.USize, PackedCompressSize, FillKey);

	switch (Header.EncodeInfo & 0x0000000Fu)
	//switch (15)
	{
	case 0:
		CxdecDecoder(Header.FileKey, 0, CompressBuffer, PackedCompressSize);
		break;

	case 1:
		RC4Decoder(CompressBuffer, PackedCompressSize, Header.FileKey);
		break;

	case 2:
		HC128Decoder(CompressBuffer, PackedCompressSize, Header.FileKey);
		BaseDecoder3_Encode(CompressBuffer, PackedCompressSize, Header.FileKey);
		break;

	case 3:
		BaseDecoder3_Encode(CompressBuffer, PackedCompressSize, Header.FileKey);
		break;

	case 4:
		VMPCDecoder(CompressBuffer, PackedCompressSize, Header.FileKey);
		break;

	case 5:
		HC128Decoder(CompressBuffer, PackedCompressSize, Header.FileKey);
		break;

	case 6:
		Salsa20Decoder(CompressBuffer, PackedCompressSize, Header.FileKey);
		break;

	case 7:
		SosemanukDecoder(CompressBuffer, PackedCompressSize, Header.FileKey);
		break;

	case 8:
		RabbitDecoder(CompressBuffer, PackedCompressSize, Header.FileKey);
		break;

	case 9:
		//NekoXcodeDecoder(CompressBuffer, PackedCompressSize, Header.FileKey);
		ChachaDecoder(CompressBuffer, PackedCompressSize, Header.FileKey);
		SosemanukDecoder(CompressBuffer, PackedCompressSize, Header.FileKey);
		break;

	case 0xA:
		BaseDecoder3_Encode(CompressBuffer, PackedCompressSize, Header.FileKey);
		RabbitDecoder(CompressBuffer, PackedCompressSize, Header.FileKey);
		break;

	case 0xB:
		PanamaDecoder(CompressBuffer, PackedCompressSize, Header.FileKey);
		break;

	case 0xC:
		//PhelixDecoder(CompressBuffer, PackedCompressSize, Header.FileKey);
		VMPCDecoder(CompressBuffer, PackedCompressSize, Header.FileKey);
		Salsa20Decoder(CompressBuffer, PackedCompressSize, Header.FileKey);
		break;

	case 0xD:
		ChachaDecoder(CompressBuffer, PackedCompressSize, Header.FileKey);
		break;

	case 0xE:
		PyDecoder(CompressBuffer, PackedCompressSize, Header.FileKey);
		break;

	case 0xF:
		//Edon80Decoder(CompressBuffer, PackedCompressSize, Header.FileKey);
		PanamaDecoder(CompressBuffer, PackedCompressSize, Header.FileKey);
		BaseDecoder3_Encode(CompressBuffer, PackedCompressSize, Header.FileKey);
		break;
	}

	Header.DecompressSize = DecodeCompressLength(Size | (CompressSize.ISize & 0xF), Header.NtBaseDecodeKey);

	DecodeHeaderInfo((PBYTE)&Header + sizeof(DWORD), Header.EncodeInfo);

	wstring OutFileName(argv[1]);
	OutFileName += L".out";

	WinFile OutFile;
	if (OutFile.Open(OutFileName.c_str(), WinFile::FileWrite) != S_OK)
	{
		printf("outfile failed\n");
		return 0;
	}
	OutFile.Write((PBYTE)&Header, sizeof(Header));
	OutFile.Write(CompressBuffer, PackedCompressSize);
	OutFile.Release();

	FreeMemoryP(Buffer);
	FreeMemoryP(CompressBuffer);

	ml::MlUnInitialize();
	return 0;
}

