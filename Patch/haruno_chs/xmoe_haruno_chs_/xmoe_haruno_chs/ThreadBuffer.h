#ifndef _ThreadBuffer_
#define _ThreadBuffer_

#include <Windows.h>

struct ThreadBuffer
{
	DWORD ThreadId;
	char* buffer;
	size_t size;

	ThreadBuffer() : ThreadId(0), buffer(0), size(0){}
};

#endif
