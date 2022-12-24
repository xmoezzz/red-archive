#ifndef _DummyAPI_
#define _DummyAPI_


//Buffer Read, Buffer SetPointer
#include <Windows.h>
#include "SDL\SDL.h"

#pragma comment(lib, "SDL2.lib ")


class DummyMemory
{
	unsigned char* Buffer;
	DWORD size;
	DWORD offset;
	SDL_RWops* file;

public:
	DummyMemory() : Buffer(0), size(0), offset(0), file(0){}
	~DummyMemory()
	{
		if (Buffer)
		{
			//delete[] Buffer;
		}
		size = 0;
		SDL_RWclose(file);
	}

	void SetBuffer(void *pBuffer, size_t pSize)
	{
		Buffer = (unsigned char*)pBuffer;
		size = pSize;
		file = SDL_RWFromMem(pBuffer, pSize);
	}

	BOOL WINAPI DummyReadFile(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead,
		LPOVERLAPPED lpOverlapped);

	DWORD WINAPI SetFilePointer( HANDLE hFile, LONG lDistanceToMove, PLONG lpDistanceToMoveHigh, DWORD dwMoveMethod);
};



#endif
