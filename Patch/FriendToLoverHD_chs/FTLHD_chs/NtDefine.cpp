#include "NtDefine.h"


PLDR_MODULE Nt_FindLdrModuleByHandle(PVOID BaseAddress)
{
	PLDR_MODULE Ldr;
	PLIST_ENTRY LdrLink, NextLink;

	if (BaseAddress != NULL)
	{
		NTSTATUS Status;

		Status = LdrFindEntryForAddress(BaseAddress, &Ldr);
		return NT_SUCCESS(Status) ? Ldr : NULL;
	}

	LdrLink = &Nt_CurrentPeb()->Ldr->InLoadOrderModuleList;
	NextLink = LdrLink->Flink;

	return FIELD_BASE(NextLink, LDR_MODULE, InLoadOrderLinks);
}


NTSTATUS
Nt_WriteMemory(
HANDLE      ProcessHandle,
PVOID       BaseAddress,
PVOID       Buffer,
ULONG_PTR   Size,
PULONG_PTR  BytesWritten /* = NULL */
)
{
	return ZwWriteVirtualMemory(ProcessHandle, BaseAddress, Buffer, Size, BytesWritten);
}



NTSTATUS
Nt_FreeMemory(
HANDLE  ProcessHandle,
PVOID   BaseAddress
)
{
	SIZE_T Size = 0;

	return NtFreeVirtualMemory(ProcessHandle, &BaseAddress, &Size, MEM_RELEASE);
}


NTSTATUS
Nt_AllocateMemory(
HANDLE      ProcessHandle,
PVOID*      BaseAddress,
ULONG_PTR   Size,
ULONG       Protect,
ULONG       AllocationType
)
{
	return NtAllocateVirtualMemory(ProcessHandle,
		BaseAddress, 0, &Size, AllocationType, Protect);
}


PTEB_ACTIVE_FRAME Nt_FindThreadFrameByContext(ULONG_PTR Context)
{
	PTEB_ACTIVE_FRAME Frame;

	Frame = RtlGetFrame();
	while (Frame != NULL && Frame->Context != Context)
		Frame = Frame->Previous;

	return Frame; 
}