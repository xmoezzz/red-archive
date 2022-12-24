#ifndef _StreamHolder_
#define _StreamHolder_

#include <Windows.h>
#include "CMem.h"
#include "BaseStream.h"
//#include "KeyTable.h"
#include "lzss.h"
#include "DebugTool.h"
#include "WinFile.h"
#include "lz4.h"

#pragma comment(lib, "LZ4.lib")

static ULONG StaticMagic = 0x91F2ED00;

typedef struct SWEncodeHeader
{
	ULONG Magic;
	ULONG DecompressLength;

}SWEncodeHeader, *pSWEncodeHeader;

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

	ULONG64 WINAPI Seek(LONG64 offset,       LONG  whence);
	ULONG64 WINAPI Read(LPVOID ReadBuffer,   ULONG ReadSize);
	ULONG64 WINAPI Write(const void *buffer, ULONG write_size);
	ULONG64 WINAPI GetSize();

private:
	HRESULT WINAPI DoDecryption();
};


#endif
