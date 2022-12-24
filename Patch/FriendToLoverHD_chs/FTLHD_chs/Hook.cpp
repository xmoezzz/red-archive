#include "Hook.h"
#include "MinHook.h"

#pragma comment(lib, "libMinHook.x86.lib")

void Nt_PatchMemory(MIN_MEMORY_FUNCTION_PATCH* p, ULONG Size)
{
	MH_Initialize();
	for (ULONG i = 0; i < Size; i++)
	{
		MH_CreateHook(p[i].pTarget, p[i].pTarget, p[i].pOri);
	}
	MH_EnableHook(MH_ALL_HOOKS);
}