#pragma once

#include <Windows.h>
typedef struct _MIN_MEMORY_FUNCTION_PATCH
{
	PVOID  pTarget;
	PVOID  pDetour;
	PVOID* pOri;
} MIN_MEMORY_FUNCTION_PATCH;


void Nt_PatchMemory(MIN_MEMORY_FUNCTION_PATCH* p, ULONG Size);
