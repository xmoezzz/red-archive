#pragma once

#include <Windows.h>

struct MemoryStream
{
	UCHAR *Base;
	UCHAR *Here;
	UCHAR *Stop;

	MemoryStream(LPVOID cfxBase, LPVOID cfxHere, LPVOID cfxStop) :
		Base(nullptr),
		Here(nullptr),
		Stop(nullptr)
	{
		Base = (UCHAR*)cfxBase;
		Here = (UCHAR*)cfxHere;
		Stop = (UCHAR*)cfxStop;
	}

	MemoryStream() :
		Base(nullptr),
		Here(nullptr),
		Stop(nullptr)
	{
	}

	~MemoryStream()
	{
		Base = nullptr;
		Here = nullptr;
		Stop = nullptr;
	}

	HRESULT WINAPI Init(LPVOID pMem, ULONG Size)
	{
		if (!pMem || !Size)
		{
			return S_FALSE;
		}

		Base = (UCHAR *)pMem;
		Here = Base;
		Stop = Base + Size;
		return S_OK;
	}

	ULONG64 WINAPI Write(const LPVOID Ptr, ULONG64 Size, ULONG64 Num)
	{
		if ((Here + (Num * Size)) > Stop)
		{
			Num = (Stop - Here) / Size;
		}
		RtlCopyMemory(Here, Ptr, (size_t)Num * (size_t)Size);
		Here += Num * Size;
		return Num;
	}

	ULONG64 WINAPI GetMemorySize()
	{
		return (ULONG64)(Stop - Base);
	}

	ULONG64 WINAPI Seek(LONG64 Offset, LONG Whence)
	{
		UCHAR *NewPos;

		switch (Whence)
		{
		case FILE_BEGIN:
			NewPos = Base + Offset;
			break;
		case FILE_CURRENT:
			NewPos = Here + Offset;
			break;
		case FILE_END:
			NewPos = Stop + Offset;
			break;

		default:
			MessageBoxW(NULL, L"Seek Error", L"X'moe CoreLib", MB_OK);
			ExitProcess(-1);
		}
		if (NewPos < Base)
		{
			NewPos = Base;
		}
		if (NewPos > Stop)
		{
			NewPos = Stop;
		}
		Here = NewPos;
		return (LONG64)(Here - Base);
	}

	ULONG64 WINAPI Read(LPVOID Ptr, ULONG64 Size, ULONG Num)
	{
		size_t total_bytes;
		size_t mem_available;

		total_bytes = ((size_t)Num * (size_t)Size);
		if ((Num <= 0) || (Size <= 0) || ((total_bytes / Num) != (size_t)Size))
		{
			return 0;
		}

		mem_available = (Stop - Here);
		if (total_bytes > mem_available)
		{
			total_bytes = mem_available;
		}

		RtlCopyMemory(Ptr, Here, total_bytes);
		Here += total_bytes;
		return (total_bytes / Size);
	}

};

class StreamHolder
{
private:
	DWORD         mPreDecodeBufferSize;
	PUCHAR        mPreDecodeBuffer;
	MemoryStream* mStream;

public:
	StreamHolder(LPVOID Buffer, ULONG BufferSize);
	~StreamHolder();

	ULONG64 WINAPI Seek(LONG64   offset,     LONG  whence);
	ULONG64 WINAPI Read(LPVOID   ReadBuffer, ULONG ReadSize);
	ULONG64 WINAPI Write(LPCVOID buffer,     ULONG write_size);
	ULONG64 WINAPI GetSize();
};

