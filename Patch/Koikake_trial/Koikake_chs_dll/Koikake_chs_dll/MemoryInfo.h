#ifndef _MemoryInfo_
#define _MemoryInfo_

#include <Windows.h>

void WINAPI VirtualMemoryCopy(void* dest, void*src, size_t size);
void WINAPI SetNopCode(BYTE* pnop, size_t size);


#endif
