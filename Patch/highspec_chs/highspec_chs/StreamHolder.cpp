#include "StreamHolder.h"

StreamHolder::StreamHolder(LPVOID Buffer, ULONG BufferSize) :
mStream(nullptr)
{
	mPreDecodeBuffer = (PUCHAR)Buffer;
	mPreDecodeBufferSize = BufferSize;

	mStream = new MemoryStream;
	mStream->Init(mPreDecodeBuffer, mPreDecodeBufferSize);
}

StreamHolder::~StreamHolder()
{
	if (mPreDecodeBuffer)
		HeapFree(GetProcessHeap(), 0, mPreDecodeBuffer);
	
	mPreDecodeBuffer = nullptr;
	mPreDecodeBufferSize = 0;

	if (mStream)
		delete mStream;
	
	mStream = nullptr;
}

ULONG64 WINAPI StreamHolder::Seek(LONG64 offset, LONG  whence)
{
	return mStream->Seek(offset, whence);
}

ULONG64 WINAPI StreamHolder::Read(LPVOID ReadBuffer, ULONG ReadSize)
{
	return mStream->Read(ReadBuffer, 1, ReadSize);
}

ULONG64 WINAPI StreamHolder::Write(const void *buffer, ULONG write_size)
{
	return mStream->Write((const LPVOID)buffer, 1, write_size);
}

ULONG64 WINAPI StreamHolder::GetSize()
{
	return mStream->GetMemorySize();
}
