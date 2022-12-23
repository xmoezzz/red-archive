#include "../my.h"
#include "def.h"
#include "psb_compiler_center.h"
using namespace std;



NTSTATUS CompilePSBFile(PBYTE& Buffer, ULONG& Size, Json::Value& DecompiledResult, Json::Value& ResourceResult)
{
	pcc.require_compile(DecompiledResult);

	if(!pcc.compile())
	{
		return STATUS_UNSUCCESSFUL;
	}

	if(!pcc.link())
	{
		return STATUS_UNSUCCESSFUL;
	}
	
	Size = pcc._link.length();
	Buffer = (PBYTE)AllocateMemoryP(Size);
	if (!Size)
		return STATUS_NO_MEMORY;

	RtlCopyMemory(Buffer, pcc._link.data(), Size);
	return STATUS_SUCCESS;
}
