#include "StreamHolder.h"
#include "ShinkuDef.h"

#include "VMProtectSDK.h"

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

#include "StreamDecoder.h"
#include "SecondStreamDecoder.h"
#include "AES.h"

#pragma comment(lib, "zstdlib_x86.lib")
#pragma comment(lib, "zlibstat_sse.lib")
#pragma comment(lib, "brotli_dec.lib")
#pragma comment(lib, "brotli_common.lib")

FORCEINLINE BOOL VerifyMask(DWORD Mask)
{
	return (Mask & 0x0000000Fu) <= 0xFu &&
		((Mask >> 4) & 0x0000000F) <= 0xFu;
}


FORCEINLINE DWORD DecodeCompressLength(DWORD FakeLength, DWORD NtKey)
{
	return ((NtKey * 0x23333333u) ^ (NtKey >> 16) * 0xAB561274) ^ FakeLength;
}


ForceInline Void ErrorExit()
{
	MessageBoxW(NULL, L"醒醒吧，补丁被艹坏了", L"你的黄油都不爱你了", MB_OK | MB_ICONERROR);
	Ps::ExitProcess(0x96);
}

/****************************************************************/
StreamHolder::StreamHolder(LPVOID Buffer, ULONG BufferSize) :
mRtcBufferSize(0),
mRtcBuffer(nullptr),
mInitFlag(FALSE),
mStream(nullptr)
{
	if (Buffer == nullptr || BufferSize == 0)
		ErrorExit();
	else
	{
		mPreDecodeBuffer = (PUCHAR)Buffer;
		mPreDecodeBufferSize = BufferSize;
	}
}

StreamHolder::~StreamHolder()
{
	if (mPreDecodeBuffer)
	{
		FreeMemoryP(mPreDecodeBuffer);
	}
	mPreDecodeBuffer     = nullptr;
	mPreDecodeBufferSize = 0;

	if (mRtcBuffer)
		FreeMemoryP(mRtcBuffer);
	
	if (mStream)
		FreeMemoryP(mStream);
	
	mRtcBuffer = nullptr;
	mRtcBufferSize = 0;
}

ULONG64 WINAPI StreamHolder::Seek(LONG64 offset, LONG  whence)
{
	if (!mInitFlag)
		DoDecryption();
	
	if (mRtcBuffer == nullptr || mRtcBufferSize == 0 || mStream == nullptr)
		ErrorExit();

	return mStream->Seek(offset, whence);
}


ULONG64 WINAPI StreamHolder::Read(LPVOID ReadBuffer, ULONG ReadSize)
{
	if (!mInitFlag)
		DoDecryption();

	if (mRtcBuffer == nullptr || mRtcBufferSize == 0 || mStream == nullptr)
		ErrorExit();
	
	return mStream->Read(ReadBuffer, 1, ReadSize);
}

ULONG64 WINAPI StreamHolder::Write(LPVOID Buffer, ULONG WriteSize)
{
	if (!mInitFlag)
		DoDecryption();
	
	if (mRtcBuffer == nullptr || mRtcBufferSize == 0 || mStream == nullptr)
		ErrorExit();

	return mStream->Write(Buffer, 1, WriteSize);
}

ULONG64 WINAPI StreamHolder::GetSize()
{
	if (!mInitFlag)
		DoDecryption();
	
	if (mRtcBuffer == nullptr || mRtcBufferSize == 0 || mStream == nullptr)
		ErrorExit();
	
	return mStream->GetMemorySize();
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

	for (ULONG Index = 0; Index < 12;  Index++)
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

HRESULT WINAPI StreamHolder::DoDecryption()
{
	VMProtectBeginVirtualization("001");

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

	if (mInitFlag)
		return E_FAIL;
	
	if (!mPreDecodeBuffer || !mPreDecodeBufferSize)
		return E_FAIL;

	DecodeHeaderInfo(mPreDecodeBuffer + sizeof(DWORD), *(PDWORD)mPreDecodeBuffer);

	PtrHeader    = (pSWEncodeHeader)mPreDecodeBuffer;
	CompressSize = mPreDecodeBufferSize - sizeof(SWEncodeHeader);

	if (!VerifyMask(PtrHeader->EncodeInfo))
		ErrorExit();

	mRtcBufferSize = DecodeCompressLength(PtrHeader->DecompressSize, PtrHeader->NtBaseDecodeKey);

	switch (PtrHeader->EncodeInfo & 0x0000000Fu)
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

	default:
		PhelixDecoder(mPreDecodeBuffer + sizeof(SWEncodeHeader), CompressSize, PtrHeader->FileKey);
		break;
	}

	//已经是解码好的了
	CombineResult ByteTransfered;
	ByteTransfered.UResult = mRtcBufferSize;

	mRtcBuffer = (PBYTE)AllocateMemoryP(mRtcBufferSize);
	if (mRtcBuffer == NULL)
		ErrorExit();

	if (mRtcBufferSize & 0xF)
	{
		CompressSize -= mRtcBufferSize & 0xF;
		mRtcBufferSize &= 0xFFFFFFF0UL;
	}

	//Decompression
	switch ((PtrHeader->EncodeInfo >> 4) & 0x0000000Fu)
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
			//Ps::ExitProcess(0);
			RtlCopyMemory(mRtcBuffer, mPreDecodeBuffer + sizeof(SWEncodeHeader), ByteTransfered.UResult);
		}
		break;

	case 2:
		ByteTransfered.tResult = mRtcBufferSize;
		DecompResult = snappy_uncompress((PCHAR)mPreDecodeBuffer + sizeof(SWEncodeHeader), CompressSize, (PCHAR)mRtcBuffer, &ByteTransfered.tResult);
		if (DecompResult != SNAPPY_OK)
		{
			//Ps::ExitProcess(0);
			RtlCopyMemory(mRtcBuffer, mPreDecodeBuffer + sizeof(SWEncodeHeader), ByteTransfered.UResult);
		}
		break;

	case 3:
		DecompResult = ZSTD_decompress(mRtcBuffer, mRtcBufferSize, mPreDecodeBuffer + sizeof(SWEncodeHeader), CompressSize);
		if (ZSTD_isError(DecompResult))
		{
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
			ErrorExit();
		}
		lzo1x_decompress(mPreDecodeBuffer + sizeof(SWEncodeHeader), CompressSize,
			mRtcBuffer, &ByteTransfered.UResult, WorkMem);

		FreeMemoryP(WorkMem);
	}
		break;

	case 0xA:
#if 0
		lzrw1_decompress(mPreDecodeBuffer + sizeof(SWEncodeHeader), CompressSize,
			mRtcBuffer, &ByteTransfered.UResult);
#endif
		ByteTransfered.tResult = mRtcBufferSize;
		//XorMemory(mPreDecodeBuffer + sizeof(SWEncodeHeader), PtrHeader->FileKey, CompressSize);
		DecompResult = lzjb_decompress(mPreDecodeBuffer + sizeof(SWEncodeHeader), mRtcBuffer, CompressSize, &ByteTransfered.tResult);
		if (DecompResult != LZJB_OK)
		{
			//Ps::ExitProcess(0);
			RtlCopyMemory(mRtcBuffer, mPreDecodeBuffer + sizeof(SWEncodeHeader), ByteTransfered.UResult);
		}
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

#if 0
	case 0xD:
	case 0xE:
		ByteTransfered.tResult = mRtcBufferSize;
		DecompResult = BrotliDecoderDecompress(
			CompressSize,
			mPreDecodeBuffer + sizeof(SWEncodeHeader),
			&ByteTransfered.tResult,
			mRtcBuffer);

		if (DecompResult != BROTLI_DECODER_RESULT_SUCCESS)
		{
			ErrorExit();
		}
		break;
#endif

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
		RtlCopyMemory(mRtcBuffer, mPreDecodeBuffer + sizeof(SWEncodeHeader), CompressSize);
		break;

	case 0xF:
		ps2_uncompress(mRtcBuffer, mRtcBufferSize, mPreDecodeBuffer + sizeof(SWEncodeHeader), CompressSize);
		break;
	}

	ULONG NextKey = PtrHeader->FileKey * (BYTE)((PtrHeader->EncodeInfo >> 8) & 0xFF);

	DragonDecoder(mRtcBuffer, mRtcBufferSize, NextKey, (PtrHeader->EncodeInfo >> 16) & 0xFF);
	NextKey = 0;

	FinalDecoder(mRtcBuffer, mRtcBufferSize, (PtrHeader->EncodeInfo >> 24) & 0xFF);

	FreeMemoryP(mPreDecodeBuffer);
	mPreDecodeBuffer = nullptr;
	mPreDecodeBufferSize = 0;

	mStream = (pMemoryStream)AllocateMemoryP(sizeof(MemoryStream));
	mStream->Init(mRtcBuffer, mRtcBufferSize);

	mInitFlag = TRUE;

	VMProtectEnd();
	return S_OK;
}

