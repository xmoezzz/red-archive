#ifndef _CMEM_
#define _CMEM_

#include <Windows.h>

class CMem
{
public:
	static LPVOID WINAPI Alloc(ULONG Size)
	{
		return HeapAlloc(GetProcessHeap(), 0, Size);
	}

	static VOID WINAPI Free(LPVOID BufferStart)
	{
		HeapFree(GetProcessHeap(), 0, BufferStart);
	}
};

#endif
