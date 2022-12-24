#include "StreamHolder.h"

VOID WINAPI ReferError()
{
	MessageBoxW(NULL, L"Bad Reference - StreamError", L"X'moe CoreLib", MB_OK);
	ExitProcess(-1);
}


unsigned char KeyInfo[1024] =
{
	0xe8, 0x27, 0xc8, 0x92, 0x5a, 0x15, 0x71,
	0x40, 0x3f, 0xf5, 0xc8, 0x77, 0x96, 0xc4, 0xae,
	0x25, 0xc4, 0x03, 0xab, 0xfc, 0x0f, 0x36, 0x66,
	0xf3, 0x24, 0x66, 0x2e, 0xc6, 0x92, 0xa0, 0x75,
	0x71, 0x4d, 0x74, 0x50, 0xb9, 0x2b, 0x77, 0xf8,
	0xa3, 0x6a, 0xc1, 0x4b, 0xfb, 0x29, 0x70, 0x4c,
	0xce, 0xeb, 0x23, 0x9f, 0xf0, 0x17, 0x80, 0x0e,
	0x77, 0x7a, 0xae, 0x06, 0x3e, 0xc3, 0xdb, 0x1a,
	0x64, 0x06, 0xb7, 0x80, 0xca, 0x3a, 0xf8, 0x8f,
	0x98, 0xba, 0xd4, 0x47, 0xb9, 0xc8, 0x8a, 0xc9,
	0x5a, 0x06, 0xd9, 0xdb, 0x6f, 0xfb, 0x87, 0x65,
	0x2e, 0x94, 0xdc, 0xf6, 0x91, 0xa0, 0x24, 0x40,
	0xd8, 0x53, 0x31, 0x98, 0x06, 0xc4, 0xd6, 0x77,
	0x60, 0x6f, 0x6d, 0xfc, 0xf1, 0xb3, 0x52, 0x67,
	0x08, 0x55, 0x66, 0x9f, 0xb7, 0xfc, 0x8d, 0xad,
	0x56, 0xb3, 0x30, 0x3f, 0xff, 0x6a, 0xbc, 0x26,
	0x10, 0x74, 0x21, 0xd8, 0xac, 0x0a, 0x54, 0xef,
	0x39, 0xc7, 0xcd, 0xa8, 0xe3, 0x2b, 0x09, 0x65,
	0x18, 0x19, 0x09, 0x2b, 0x0a, 0x58, 0xd1, 0x25,
	0x31, 0x15, 0xeb, 0x1f, 0xc6, 0x5f, 0xe2, 0x0c,
	0x49, 0xaa, 0xc7, 0x81, 0xfc, 0x4d, 0xaf, 0x37,
	0x65, 0x04, 0x32, 0x8d, 0xd0, 0x6f, 0xee, 0x03,
	0xca, 0x91, 0x02, 0xc0, 0xa8, 0x51, 0x94, 0x0d,
	0xfe, 0xfc, 0x4c, 0xd8, 0x28, 0xc1, 0xd6, 0x32,
	0xc4, 0x34, 0x63, 0xd2, 0x36, 0xcc, 0x29, 0x90,
	0x23, 0x65, 0xde, 0xea, 0xf7, 0xbf, 0x41, 0x82,
	0x5f, 0xfd, 0x92, 0x9d, 0xcf, 0x26, 0x15, 0xa6,
	0xfd, 0xa8, 0x93, 0xa9, 0x64, 0xd0, 0xd8, 0xd9,
	0xc2, 0x53, 0x36, 0x0b, 0x9b, 0xc8, 0x00, 0x38,
	0xb4, 0x2b, 0x10, 0xff, 0x98, 0x5b, 0x43, 0x21,
	0x16, 0x9d, 0xf7, 0x03, 0xc1, 0x18, 0x94, 0x2f,
	0x6f, 0x57, 0x00, 0xd3, 0xba, 0xca, 0x29, 0x40,
	0x82, 0x45, 0x7f, 0x6c, 0x69, 0x7f, 0x76, 0x71,
	0x56, 0x94, 0x09, 0x0c, 0xf3, 0x84, 0x32, 0x20,
	0x2f, 0xb2, 0x73, 0x30, 0xbd, 0x66, 0x51, 0xe8,
	0x92, 0x4b, 0xd3, 0x94, 0x6b, 0xf1, 0x07, 0xa7,
	0x45, 0x4c, 0x7e, 0x36, 0xe2, 0x33, 0xcb, 0x7a,
	0x4b, 0xe2, 0x07, 0x52, 0x49, 0x79, 0x50, 0xbf,
	0xeb, 0x7b, 0x46, 0x66, 0x03, 0x50, 0x8c, 0x11,
	0xa8, 0xc3, 0x4d, 0x2e, 0xb6, 0x84, 0xb4, 0x4f,
	0x49, 0xa7, 0x73, 0xa8, 0x46, 0x23, 0x3d, 0x94,
	0xd2, 0x54, 0x4c, 0x10, 0xd9, 0x7a, 0xdc, 0x3e,
	0x88, 0x38, 0xae, 0xe4, 0xd3, 0x16, 0x86, 0xeb,
	0xf1, 0xff, 0xad, 0xe0, 0xdb, 0xc3, 0x70, 0x76,
	0xd0, 0x96, 0x9f, 0x02, 0xd3, 0x8f, 0x0e, 0xfe,
	0x2c, 0x2b, 0x17, 0x86, 0xe3, 0xc7, 0x17, 0xde,
	0x48, 0x29, 0xed, 0xea, 0x6e, 0xf8, 0x7e, 0xb5,
	0xab, 0x3f, 0x33, 0xeb, 0x1a, 0xef, 0x79, 0x5e,
	0x19, 0x5a, 0x40, 0x85, 0xcb, 0xb8, 0x7d, 0xf8,
	0x97, 0xa5, 0xa9, 0xf5, 0xa7, 0xa1, 0x3f, 0xde,
	0x6a, 0x8f, 0x42, 0xb9, 0x13, 0x37, 0xb4, 0xaf,
	0x18, 0xc4, 0x20, 0x8d, 0xb3, 0x46, 0x11, 0x46,
	0x64, 0x31, 0x99, 0x6f, 0x6d, 0xdd, 0xcb, 0xc2,
	0x55, 0x04, 0x41, 0x9c, 0x66, 0x47, 0x96, 0x7e,
	0x2f, 0xa9, 0xed, 0x90, 0x02, 0x12, 0x69, 0x19,
	0x77, 0xcd, 0xb3, 0x08, 0xe7, 0x0a, 0x77, 0x6f,
	0xf2, 0x5e, 0xe7, 0x02, 0xfa, 0x3e, 0x36, 0x9c,
	0xa5, 0x87, 0x1e, 0xbb, 0x5f, 0xf9, 0x5c, 0xff,
	0xd6, 0xb7, 0x2e, 0xaf, 0x7c, 0xc9, 0xdc, 0x34,
	0x08, 0x9a, 0x2c, 0x9b, 0xf5, 0x7b, 0xeb, 0x17,
	0x02, 0x1e, 0x6b, 0x7d, 0xb0, 0x1b, 0x00, 0xc7,
	0xc8, 0x6f, 0x83, 0x92, 0xd2, 0xf7, 0xcf, 0xa0,
	0x9e, 0xfa, 0x46, 0x56, 0xbf, 0x9c, 0x4d, 0x3e,
	0x0c, 0x6c, 0xcb, 0x87, 0x1d, 0xd7, 0xae, 0x80,
	0xd4, 0xb2, 0x66, 0x21, 0xd1, 0xb5, 0x68, 0x82,
	0xfc, 0xfa, 0xad, 0x61, 0xff, 0x82, 0x31, 0xa1,
	0xca, 0xb0, 0x74, 0xc6, 0x0d, 0xcc, 0xfc, 0x7a,
	0xc1, 0x82, 0xd0, 0x0a, 0xa0, 0x60, 0xff, 0xe9,
	0xa8, 0x5b, 0x17, 0x2c, 0x9c, 0x4b, 0xaf, 0x0d,
	0x83, 0x6a, 0xde, 0x69, 0x27, 0xd9, 0xc1, 0x42,
	0x97, 0x1b, 0xf8, 0x3d, 0xa6, 0x98, 0x29, 0x25,
	0x69, 0x1b, 0x7c, 0x66, 0xbd, 0x55, 0x1e, 0x93,
	0xbf, 0x58, 0xbf, 0xe0, 0x52, 0x1d, 0x13, 0xa9,
	0x9c, 0xfe, 0x54, 0xe9, 0x89, 0x3c, 0xbf, 0xc4,
	0x47, 0x7a, 0x13, 0xfd, 0xc8, 0x40, 0x15, 0x80,
	0x44, 0x7a, 0x0e, 0xda, 0xb4, 0xf6, 0x4b, 0xbc,
	0x57, 0xe9, 0x9c, 0x7d, 0x31, 0x6b, 0xd6, 0x94,
	0x87, 0xf6, 0x52, 0x21, 0x65, 0xeb, 0x6b, 0x65,
	0x19, 0x0e, 0x03, 0x46, 0xb5, 0x05, 0xff, 0xcc,
	0x90, 0xdc, 0xc6, 0xa7, 0xc5, 0x84, 0xc7, 0xa6,
	0xb3, 0x4f, 0xf0, 0x41, 0x7b, 0x76, 0x37, 0x10,
	0x85, 0x93, 0x15, 0x52, 0xfc, 0x27, 0x06, 0x67,
	0x4d, 0x15, 0x0a, 0x56, 0xac, 0x25, 0x27, 0x48,
	0x8f, 0x83, 0xe5, 0x0b, 0x31, 0x3d, 0xd0, 0x90,
	0x10, 0xc9, 0xfa, 0x6e, 0x6f, 0x7c, 0x77, 0x5c,
	0xd5, 0x14, 0xdf, 0xbb, 0x8c, 0x2f, 0xcf, 0x0a,
	0x23, 0xd1, 0x67, 0x6f, 0xed, 0xe2, 0xce, 0x35,
	0x80, 0xae, 0xaa, 0x48, 0x36, 0x63, 0xa8, 0xbb,
	0xaf, 0x97, 0xfa, 0x43, 0x4d, 0xbf, 0xd4, 0xb9,
	0xb7, 0xb9, 0xee, 0x9c, 0x57, 0x43, 0x06, 0x8c,
	0xdc, 0x81, 0x5b, 0xd0, 0xb8, 0x7c, 0x32, 0xd1,
	0xa3, 0x9d, 0x55, 0x9d, 0x16, 0x36, 0x8f, 0x65,
	0xd1, 0xf9, 0x31, 0x00, 0x56, 0x7f, 0x90, 0x66,
	0x6b, 0xc2, 0x85, 0x35, 0x9c, 0xa3, 0xeb, 0x2f,
	0xb7, 0x66, 0x25, 0xb9, 0x4e, 0x31, 0x95, 0x5e,
	0x39, 0x91, 0x26, 0x4b, 0x11, 0xf4, 0xc3, 0xd0,
	0xb5, 0x30, 0xde, 0xe5, 0xc9, 0xfa, 0xea, 0xa3,
	0x32, 0x70, 0xe1, 0xc6, 0x9c, 0x90, 0xbf, 0x32,
	0xf4, 0xbe, 0x05, 0x6b, 0xee, 0x43, 0x37, 0x1b,
	0x80, 0xc8, 0x5e, 0x90, 0x65, 0xdf, 0x86, 0x3b,
	0x9c, 0x7a, 0x41, 0x33, 0xe6, 0x72, 0x23, 0xb0,
	0x4b, 0x02, 0x44, 0x90, 0x95, 0x49, 0xc1, 0xd5,
	0xd4, 0xcb, 0x3b, 0x25, 0xd8, 0xf1, 0x56, 0x49,
	0xba, 0x84, 0x3b, 0xae, 0x54, 0x36, 0x17, 0xe7,
	0xc4, 0x19, 0x9a, 0x29, 0xed, 0x26, 0x79, 0xcd,
	0xf6, 0xb8, 0xed, 0xd2, 0xc9, 0x0e, 0x31, 0x59,
	0x95, 0xcd, 0x07, 0x27, 0x4d, 0x7b, 0x34, 0x26,
	0x27, 0x05, 0xff, 0xe4, 0x1d, 0x39, 0xb6, 0x13,
	0x6f, 0x4d, 0x2a, 0x07, 0x1f, 0x56, 0x2e, 0x3b,
	0x73, 0xd2, 0x1c, 0xcc, 0x77, 0x1f, 0x4f, 0xfc,
	0x79, 0x02, 0xaa, 0xb1, 0x8c, 0x21, 0x10, 0xf4,
	0x05, 0x89, 0xea, 0x72, 0x00, 0x29, 0xa4, 0xfe,
	0xdc, 0x54, 0x30, 0x0d, 0xbb, 0x43, 0x81, 0x39,
	0x03, 0x91, 0x11, 0xbf, 0xe0, 0xbd, 0x5c, 0x00,
	0xbf, 0xac, 0x63, 0x04, 0xd4, 0x24, 0x2a, 0xf2,
	0x96, 0x52, 0x3a, 0x99, 0x3e, 0x45, 0x20, 0xeb,
	0x4b, 0x70, 0xec, 0x7c, 0x01, 0x2c, 0xb2, 0x07,
	0xe5, 0x34, 0x0d, 0xea, 0x42, 0x27, 0x97, 0xa5,
	0xa8, 0x0a, 0x72, 0x5e, 0x68, 0xc3, 0xc3, 0x60,
	0x19, 0x9f, 0x31, 0x98, 0x16, 0xcd, 0x6a, 0x17,
	0xfd, 0xe0, 0x9e, 0x93, 0x31, 0x51, 0x02, 0xe6,
	0x59, 0xfb, 0x4f, 0x8c, 0xdf, 0x9d, 0x40, 0x29,
	0x72, 0x5c, 0x18, 0x02, 0x85, 0x3e, 0x19, 0x7f,
	0xce, 0xb0, 0x0e, 0xaf, 0xc8, 0x01, 0xc2, 0xc4,
	0x31, 0xe4, 0x87, 0x92, 0x8c, 0xf2, 0xb0, 0x14,
	0x9f, 0x26, 0x17, 0xe8, 0xf6, 0x60, 0x98, 0xce,
	0x5f, 0xe2, 0x93, 0x2d, 0x6d, 0xd6, 0x6e, 0x8e,
	0xf5, 0xc5, 0x11, 0x1e, 0x94, 0x21, 0x69, 0x31,
	0x27
};




/****************************************************************/
StreamHolder::StreamHolder(LPVOID Buffer, ULONG BufferSize):
	mRtcBufferSize(0),
	mRtcBuffer(nullptr),
	mInitFlag(FALSE),
	mStream(nullptr)
{
	if (Buffer == nullptr || BufferSize == 0)
	{
#ifdef DebugMode
		DebugInfo(L"On Init");
		MessageBoxW(NULL, L"On Init", NULL, MB_OK);
#endif
		ReferError();
	}
	else
	{
		mPreDecodeBuffer     = (PUCHAR)Buffer;
		mPreDecodeBufferSize = BufferSize;
	}
}

StreamHolder::~StreamHolder()
{
	if (mPreDecodeBuffer)
	{
		CMem::Free(mPreDecodeBuffer);
	}
	mPreDecodeBuffer     = nullptr;
	mPreDecodeBufferSize = 0;

	if (mRtcBuffer)
	{
		CMem::Free(mRtcBuffer);
	}
	if (mStream)
	{
		CMem::Free(mStream);
	}
	mRtcBuffer     = nullptr;
	mRtcBufferSize = 0;
}

ULONG64 WINAPI StreamHolder::Seek(LONG64 offset, LONG  whence)
{
	if (!mInitFlag)
	{
		DoDecryption();
	}
	if (mRtcBuffer == nullptr || mRtcBufferSize == 0 || mStream == nullptr)
	{
#ifdef DebugMode
		MessageBoxW(NULL, L"On Seek", NULL, MB_OK);
		DebugInfo(L"On Seek");
#endif
		ReferError();
	}
	return mStream->Seek(offset, whence);
}


ULONG64 WINAPI StreamHolder::Read(LPVOID ReadBuffer, ULONG ReadSize)
{
	if (!mInitFlag)
	{
		DoDecryption();
	}
	if (mRtcBuffer == nullptr || mRtcBufferSize == 0 || mStream == nullptr)
	{
#ifdef DebugMode
		MessageBoxW(NULL, L"On Read", NULL, MB_OK);
		DebugInfo(L"On Read");
#endif
		ReferError();
	}
	return mStream->Read(ReadBuffer, 1, ReadSize);
}

ULONG64 WINAPI StreamHolder::Write(const void *buffer, ULONG write_size)
{
	if (!mInitFlag)
	{
		DoDecryption();
	}
	if (mRtcBuffer == nullptr || mRtcBufferSize == 0 || mStream == nullptr)
	{
#ifdef DebugMode
		MessageBoxW(NULL, L"On Write", NULL, MB_OK);
		DebugInfo(L"On Write");
#endif
		ReferError();
	}
	return mStream->Write((const LPVOID)buffer, 1, write_size);
}

ULONG64 WINAPI StreamHolder::GetSize()
{
	if (!mInitFlag)
	{
		DoDecryption();
	}
	if (mRtcBuffer == nullptr || mRtcBufferSize == 0 || mStream == nullptr)
	{
#ifdef DebugMode
		MessageBoxW(NULL, L"On Size", NULL, MB_OK);
		DebugInfo(L"On GetSize");
#endif
		ReferError();
	}
	return mStream->GetMemorySize();
}

HRESULT WINAPI StreamHolder::DoDecryption()
{
	if (mInitFlag)
	{
		return S_FALSE;
	}
	if (!mPreDecodeBuffer || !mPreDecodeBufferSize)
	{
#ifdef DebugMode
		MessageBoxW(NULL, L"Proc 1", NULL, MB_OK);
		DebugInfo(L"Proc 1");
#endif
		ReferError();
		return S_FALSE;
	}
	
	ULONG ByteTransfered = 0;
	while (ByteTransfered < mPreDecodeBufferSize)
	{
		mPreDecodeBuffer[ByteTransfered] ^= KeyInfo[ByteTransfered % 1024];
		ByteTransfered++;
	}

	pSWEncodeHeader PtrHeader = (pSWEncodeHeader)mPreDecodeBuffer;
	ULONG CompressSize = mPreDecodeBufferSize - sizeof(SWEncodeHeader);
	if (PtrHeader->Magic != StaticMagic)
	{
#ifdef DebugMode
		WCHAR szErrorString[260] = { 0 };
		wsprintfW(szErrorString, L"magic %08x, static maigc %08x", PtrHeader->Magic, StaticMagic);
		DebugInfo(szErrorString);
		MessageBoxW(NULL, L"Proc 2", NULL, MB_OK);
		DebugInfo(L"Proc 2");
#endif
		ReferError();
	}
	
	//LZSS LzssCompression;
	mRtcBuffer = (PUCHAR)CMem::Alloc(PtrHeader->DecompressLength);
	if (mRtcBuffer == nullptr)
	{
#ifdef DebugMode
		MessageBoxW(NULL, L"Proc 3", NULL, MB_OK);
		DebugInfo(L"Proc 3");
#endif
		ReferError();
	}

	//mRtcBufferSize = LzssCompression.UnCompress(mPreDecodeBuffer + sizeof(SWEncodeHeader),
	//	mPreDecodeBufferSize - sizeof(SWEncodeHeader), mRtcBuffer);

	LZ4_uncompress((char*)(mPreDecodeBuffer + sizeof(SWEncodeHeader)),
		(char*)(mRtcBuffer), PtrHeader->DecompressLength);

	mRtcBufferSize = PtrHeader->DecompressLength;

	mStream = (pMemoryStream)CMem::Alloc(sizeof(MemoryStream));
	mStream->Init(mRtcBuffer, mRtcBufferSize);

	CMem::Free(mPreDecodeBuffer);
	mPreDecodeBuffer = nullptr;
	mPreDecodeBufferSize = 0;

	mInitFlag = TRUE;
	return S_OK;
}
