#pragma once

#include "my.h"
#include "BaseStream.h"

class StreamHolderXP3
{
private:
	DWORD         mPreDecodeBufferSize;
	PBYTE         mPreDecodeBuffer;
	DWORD         mRtcBufferSize;
	ULONG64       Hash;
	PBYTE         mRtcBuffer;
	BOOL          mInitFlag;
	MemoryStream* mStream;

public:
	StreamHolderXP3(LPVOID Buffer, ULONG BufferSize, ULONG64 Hash);
	~StreamHolderXP3();

	ULONG64 WINAPI Seek(LONG64 Offset, LONG  Whence);
	ULONG64 WINAPI Read(LPVOID ReadBuffer, ULONG ReadSize);
	ULONG64 WINAPI Write(LPVOID buffer, ULONG WriteSize);
	ULONG64 WINAPI GetSize();

//private:
	HRESULT WINAPI DoDecryption();
	VOID  Destroy();
};


