#pragma once

#include "my.h"
#include "BaseStream.h"
#include "WinFile.h"

#pragma pack(push, 1)
typedef struct SWEncodeHeader
{
	DWORD EncodeInfo;
	DWORD FileKey;
	DWORD NtBaseDecodeKey;
	DWORD DecompressSize;
}SWEncodeHeader, *pSWEncodeHeader;
#pragma pack(pop)

class StreamHolder
{
private:
	DWORD         mPreDecodeBufferSize;
	PUCHAR        mPreDecodeBuffer;
	DWORD         mRtcBufferSize;
	PUCHAR        mRtcBuffer;
	BOOL          mInitFlag;
	MemoryStream* mStream;

public:
	StreamHolder(LPVOID Buffer, ULONG BufferSize);
	~StreamHolder();

	ULONG64 WINAPI Seek(LONG64 Offset, LONG  Whence);
	ULONG64 WINAPI Read(LPVOID ReadBuffer, ULONG ReadSize);
	ULONG64 WINAPI Write(LPVOID buffer, ULONG WriteSize);
	ULONG64 WINAPI GetSize();

private:
	HRESULT WINAPI DoDecryption();
};


