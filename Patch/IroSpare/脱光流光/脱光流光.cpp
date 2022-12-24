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


FORCEINLINE BOOL VerifyMask(DWORD Mask)
{
	return (Mask & 0x0000000Fu) <= 0xFu &&
		((Mask >> 4) & 0x0000000F) <= 0xFu;
}

/*
8bit : Vector-based shell code mask key
8Bit : Vector-based shell code selector
8Bit : 2nd Encode
4Bit : Compress
4Bit : 1st Encode
*/

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



Void 初始化脱光流光()
{
	ECRYPT_init();
	PhelixInit();
	PyECRYPT_init();
	InitAll1StKey();
	InitAll2ndKey();
}

int wmain(int argc, WCHAR* argv[])
{
	pSWEncodeHeader   PtrHeader;
	ULONG             CompressSize;
	PBYTE             DecodeBuffer;
	INT32             DecompResult;


	typedef union CombineResult
	{
		size_t tResult;
		SIZE_T TResult;
		ULONG  UResult;
		LONG   LResult;
	}CombineResult;

	PBYTE    mPreDecodeBuffer;
	ULONG    mPreDecodeBufferSize;
	WinFile  File;

	if (argc != 2)
		return 0;

	ml::MlInitialize();
	srand((int)time(NULL));
	//init global map!!!

	初始化脱光流光();

	if (File.Open(argv[1], WinFile::FileRead) != S_OK)
	{
		printf("open file failed\n");
		return 0;
	}

	mPreDecodeBufferSize = File.GetSize32();
	mPreDecodeBuffer = (PBYTE)AllocateMemoryP(mPreDecodeBufferSize, HEAP_ZERO_MEMORY);
	if (!mPreDecodeBuffer)
	{
		printf("mem failed(1)\n");
		return 0;
	}

	File.Read(mPreDecodeBuffer, mPreDecodeBufferSize);
	File.Release();

	DecodeHeaderInfo(mPreDecodeBuffer + sizeof(DWORD), *(PDWORD)mPreDecodeBuffer);

	PtrHeader = (pSWEncodeHeader)mPreDecodeBuffer;
	CompressSize = mPreDecodeBufferSize - sizeof(SWEncodeHeader);

	if (!VerifyMask(PtrHeader->EncodeInfo))
	{
		printf("invalid...\n");
		Ps::ExitProcess(-1);
	}
	ULONG mRtcBufferSize = DecodeCompressLength(PtrHeader->DecompressSize, PtrHeader->NtBaseDecodeKey);

	printf("stream 1...(%d)\n", PtrHeader->EncodeInfo & 0x0000000Fu);
	switch (PtrHeader->EncodeInfo & 0x0000000Fu)
	//switch (15)
	{
	case 0:
		CxdecDecoder(PtrHeader->FileKey, 0, mPreDecodeBuffer + sizeof(SWEncodeHeader), CompressSize);
		break;

	case 1:
		RC4Decoder(mPreDecodeBuffer + sizeof(SWEncodeHeader), CompressSize, PtrHeader->FileKey);
		break;

	case 2:
		BaseDecoder3(mPreDecodeBuffer + sizeof(SWEncodeHeader), CompressSize, PtrHeader->FileKey);
		HC128Decoder(mPreDecodeBuffer + sizeof(SWEncodeHeader), CompressSize, PtrHeader->FileKey);
		break;

	case 3:
		BaseDecoder3(mPreDecodeBuffer + sizeof(SWEncodeHeader), CompressSize, PtrHeader->FileKey);
		break;

	case 4:
		VMPCDecoder(mPreDecodeBuffer + sizeof(SWEncodeHeader), CompressSize, PtrHeader->FileKey);
		break;

	case 5:
		HC128Decoder(mPreDecodeBuffer + sizeof(SWEncodeHeader), CompressSize, PtrHeader->FileKey);
		break;

	case 6:
		Salsa20Decoder(mPreDecodeBuffer + sizeof(SWEncodeHeader), CompressSize, PtrHeader->FileKey);
		break;

	case 7:
		SosemanukDecoder(mPreDecodeBuffer + sizeof(SWEncodeHeader), CompressSize, PtrHeader->FileKey);
		break;

	case 8:
		RabbitDecoder(mPreDecodeBuffer + sizeof(SWEncodeHeader), CompressSize, PtrHeader->FileKey);
		break;

	case 9:
		//NekoXcodeDecoder(mPreDecodeBuffer + sizeof(SWEncodeHeader), CompressSize, PtrHeader->FileKey);
		SosemanukDecoder(mPreDecodeBuffer + sizeof(SWEncodeHeader), CompressSize, PtrHeader->FileKey);
		ChachaDecoder(mPreDecodeBuffer + sizeof(SWEncodeHeader), CompressSize, PtrHeader->FileKey);
		break;

	case 0xA:
		RabbitDecoder(mPreDecodeBuffer + sizeof(SWEncodeHeader), CompressSize, PtrHeader->FileKey);
		BaseDecoder3(mPreDecodeBuffer + sizeof(SWEncodeHeader), CompressSize, PtrHeader->FileKey);
		
		break;

	case 0xB:
		PanamaDecoder(mPreDecodeBuffer + sizeof(SWEncodeHeader), CompressSize, PtrHeader->FileKey);
		break;

	case 0xC:
		//PhelixDecoder(mPreDecodeBuffer + sizeof(SWEncodeHeader), CompressSize, PtrHeader->FileKey);
		Salsa20Decoder(mPreDecodeBuffer + sizeof(SWEncodeHeader), CompressSize, PtrHeader->FileKey);
		VMPCDecoder(mPreDecodeBuffer + sizeof(SWEncodeHeader), CompressSize, PtrHeader->FileKey);
		break;

	case 0xD:
		ChachaDecoder(mPreDecodeBuffer + sizeof(SWEncodeHeader), CompressSize, PtrHeader->FileKey);
		break;

	case 0xE:
		PyDecoder(mPreDecodeBuffer + sizeof(SWEncodeHeader), CompressSize, PtrHeader->FileKey);
		break;

	case 0xF:
		BaseDecoder3(mPreDecodeBuffer + sizeof(SWEncodeHeader), CompressSize, PtrHeader->FileKey);
		PanamaDecoder(mPreDecodeBuffer + sizeof(SWEncodeHeader), CompressSize, PtrHeader->FileKey);
		break;
	}
	printf("stream 1 ok...\n");

	//已经是解码好的了
	CombineResult ByteTransfered;
	ByteTransfered.UResult = mRtcBufferSize;

	PBYTE mRtcBuffer = (PBYTE)AllocateMemoryP(mRtcBufferSize);
	if (mRtcBuffer == NULL)
	{
		printf("alloc mem failed(2)\n");
		return 0;
	}


	if (mRtcBufferSize & 0xF)
	{
		printf("padding size : %08x, padded size : %08x\n", mRtcBufferSize & 0xF, CompressSize);
		CompressSize -= mRtcBufferSize & 0xF;
		mRtcBufferSize &= 0xFFFFFFF0UL;
		printf("fixed size : %08x\n", CompressSize);
	}
	printf("decompression...(%d)\n", (PtrHeader->EncodeInfo >> 4) & 0x0000000Fu);
	//Decompression

	switch ((PtrHeader->EncodeInfo >> 4) & 0x0000000Fu)
	//switch (14)
	{
	case 0:
		DecompResult = LZ4_uncompress((char*)(mPreDecodeBuffer + sizeof(SWEncodeHeader)),
			(char*)(mRtcBuffer), mRtcBufferSize);
		break;

	case 1:
		ByteTransfered.UResult = mRtcBufferSize;
		DecompResult = uncompress(mRtcBuffer, &ByteTransfered.UResult, mPreDecodeBuffer + sizeof(SWEncodeHeader), CompressSize);
		if (DecompResult != Z_OK)
		{
			printf("zlib : %d\n", DecompResult);
			//Ps::ExitProcess(0);
			RtlCopyMemory(mRtcBuffer, mPreDecodeBuffer + sizeof(SWEncodeHeader), ByteTransfered.UResult);
		}
		break;

	case 2:
		ByteTransfered.tResult = mRtcBufferSize;
		DecompResult = snappy_uncompress((PCHAR)mPreDecodeBuffer + sizeof(SWEncodeHeader), CompressSize, (PCHAR)mRtcBuffer, &ByteTransfered.tResult);
		if (DecompResult != SNAPPY_OK)
		{
			printf("snappy : %d\n", DecompResult);
			//Ps::ExitProcess(0);
			RtlCopyMemory(mRtcBuffer, mPreDecodeBuffer + sizeof(SWEncodeHeader), ByteTransfered.UResult);
		}
		break;

	case 3:
		DecompResult = ZSTD_decompress(mRtcBuffer, mRtcBufferSize, mPreDecodeBuffer + sizeof(SWEncodeHeader), CompressSize);
		if (ZSTD_isError(DecompResult))
		{
			printf("ZSTD : %d\n", DecompResult);
			//Ps::ExitProcess(0);
			RtlCopyMemory(mRtcBuffer, mPreDecodeBuffer + sizeof(SWEncodeHeader), ByteTransfered.UResult);
		}
		break;

	case 4:
		UCL_NRV2E_DecompressASMFast32(mPreDecodeBuffer + sizeof(SWEncodeHeader), mRtcBuffer);
		break;

	case 5:
	{
		qlz_state_decompress S[1];
		ByteTransfered.tResult = qlz_decompress((PCHAR)mPreDecodeBuffer + sizeof(SWEncodeHeader), mRtcBuffer, S);
	}
	break;

	case 6:
	{
		CRLE Rle;
		Rle.Decode(mRtcBuffer, ByteTransfered.LResult, mPreDecodeBuffer + sizeof(SWEncodeHeader), CompressSize);
	}
	break;

	case 7:
		ByteTransfered.tResult = mRtcBufferSize;
		DecompResult = lzjb_decompress(mPreDecodeBuffer + sizeof(SWEncodeHeader), mRtcBuffer, CompressSize, &ByteTransfered.tResult);
		if (DecompResult != LZJB_OK)
		{
			printf("lzjb_decompress %d\n", DecompResult);
			//Ps::ExitProcess(0);
			RtlCopyMemory(mRtcBuffer, mPreDecodeBuffer + sizeof(SWEncodeHeader), ByteTransfered.UResult);
		}
		break;

	case 8:
		fastlz_decompress(mPreDecodeBuffer + sizeof(SWEncodeHeader), CompressSize, mRtcBuffer, mRtcBufferSize);
		break;

	case 9:
	{
		ByteTransfered.UResult = mRtcBufferSize;
		PVOID WorkMem = AllocateMemoryP(ByteTransfered.UResult * 4);
		if (!WorkMem)
		{
			printf("lzo mem\n");
			Ps::ExitProcess(0);
		}
		lzo1x_decompress(mPreDecodeBuffer + sizeof(SWEncodeHeader), CompressSize,
			mRtcBuffer, &ByteTransfered.UResult, WorkMem);

		FreeMemoryP(WorkMem);
	}
		break;

	case 0xA:
#if 0
		ByteTransfered.UResult = mRtcBufferSize;
		lzrw1_decompress(mPreDecodeBuffer + sizeof(SWEncodeHeader), CompressSize,
			mRtcBuffer, &ByteTransfered.UResult);

		printf("%d\n", ByteTransfered.UResult);
#else
		ByteTransfered.tResult = mRtcBufferSize;
		//XorMemory(mPreDecodeBuffer + sizeof(SWEncodeHeader), PtrHeader->FileKey, CompressSize);
		DecompResult = lzjb_decompress(mPreDecodeBuffer + sizeof(SWEncodeHeader), mRtcBuffer, CompressSize, &ByteTransfered.tResult);
		if (DecompResult != LZJB_OK)
		{
			printf("lzjb_decompress %d\n", DecompResult);
			//Ps::ExitProcess(0);
			RtlCopyMemory(mRtcBuffer, mPreDecodeBuffer + sizeof(SWEncodeHeader), ByteTransfered.UResult);
		}
#endif
		break;

	case 0xB:
		blz_depack_safe(mPreDecodeBuffer + sizeof(SWEncodeHeader), CompressSize,
			mRtcBuffer, mRtcBufferSize);
		break;

	case 0xC:
	{
		lzfse_decoder_state S[1];
		lzfse_decode_buffer(mRtcBuffer, mRtcBufferSize,
			mPreDecodeBuffer + sizeof(SWEncodeHeader), CompressSize,
			S);
	}
	break;


	default:
		ByteTransfered.tResult = blosclz_decompress(mPreDecodeBuffer + sizeof(SWEncodeHeader), CompressSize, mRtcBuffer, mRtcBufferSize);

		if (ByteTransfered.tResult == 0)
		{
			//Ps::ExitProcess(0);
			RtlCopyMemory(mRtcBuffer, mPreDecodeBuffer + sizeof(SWEncodeHeader), ByteTransfered.UResult);
		}
		break;

	case 0xD:
	case 0xE:
		memcpy(mRtcBuffer, mPreDecodeBuffer + sizeof(SWEncodeHeader), CompressSize);
		break; 

	case 0xF:
		ps2_uncompress(mRtcBuffer, mRtcBufferSize, mPreDecodeBuffer + sizeof(SWEncodeHeader), CompressSize);
		break;
	}

	printf("decompression ok...\n");


	ULONG NextKey = PtrHeader->FileKey * (BYTE)((PtrHeader->EncodeInfo >> 8) & 0xFF);

	DragonDecoder(mRtcBuffer, mRtcBufferSize, NextKey, (PtrHeader->EncodeInfo >> 16) & 0xFF);
	NextKey = 0;

	FinalDecoder(mRtcBuffer, mRtcBufferSize, (PtrHeader->EncodeInfo >> 24) & 0xFF);

	FreeMemoryP(mPreDecodeBuffer);
	mPreDecodeBuffer = nullptr;
	mPreDecodeBufferSize = 0;

	WinFile OutFile;
	wstring OutFileName(argv[1]);
	OutFileName += L".txt";
	
	if (OutFile.Open(OutFileName.c_str(), WinFile::FileWrite) != S_OK)
	{
		printf("opeb output file failed\n");
		return 0;
	}

	ULONG ZeroByte = 0;
	ULONG SaveSize = mRtcBufferSize - 1;
	while (SaveSize > 0)
	{
		if (mRtcBuffer[SaveSize] == NULL)
		{
			ZeroByte++;
			SaveSize--;
		}
		else
		{
			break;
		}
	}

	OutFile.Write(mRtcBuffer, mRtcBufferSize - ZeroByte);
	
	OutFile.Release();

	FreeMemoryP(mRtcBuffer);
	ml::MlUnInitialize();
	return 0;
}

