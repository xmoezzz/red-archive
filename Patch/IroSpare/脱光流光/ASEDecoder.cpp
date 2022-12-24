#include "BaseDecoder.h"
#include "ASE.h"
#include "my.h"
#include "GlobalMap.h"

#define ASE_BaseKey_Mask TAG4('A542')

void ASEDecoder(PBYTE Buffer, ULONG_PTR Size, DWORD Key)
{
	PBYTE ASEBaseKey;
	PULONG CurKey;

	if (Size % 16)
		Ps::ExitProcess(0);

	ASEBaseKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(ASE_BaseKey_Mask);
	if (!ASEBaseKey)
		Ps::ExitProcess(0);

	CurKey = (PULONG)AllocStack(128 * sizeof(DWORD));
	

	for (ULONG_PTR i = 0; i < Size; i += 16)
	{
		aes_decrypt(Buffer + i, Buffer + i, CurKey, 128);
	}
	RtlZeroMemory(CurKey, 128 * sizeof(DWORD));
}

