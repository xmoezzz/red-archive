#include "my.h"

#pragma comment(linker, "/ENTRY:DllEntryMain")
#pragma comment(linker, "/SECTION:.text,ERW /MERGE:.rdata=.text /MERGE:.data=.text")
#pragma comment(linker, "/SECTION:.Xmoe,ERW /MERGE:.text=.Xmoe")

#pragma comment(lib, "ntdll.lib")

#pragma comment(linker, "/EXPORT:XmoeGetFile=_XmoeGetFile@16,PRIVATE")
EXTERN_C MY_DLL_EXPORT
NTSTATUS NTAPI XmoeGetFile(LPWSTR FileName, HANDLE Heap, PBYTE* Buffer, PULONG Size)
{

#if 0
	NTSTATUS   Status;
	NtFileDisk File;

	LOOP_ONCE
	{
		*Buffer = NULL;
		*Size   = 0;
		Status = File.Open(FileName);
		if (NT_FAILED(Status))
			break;

		*Size  = File.GetSize32();
		Status = STATUS_UNSUCCESSFUL;
		if (!File.GetSize32())
			break;

		Status  = STATUS_INSUFFICIENT_RESOURCES;
		*Buffer = (PBYTE)RtlAllocateHeap(Heap, HEAP_ZERO_MEMORY, *Size);
		if (!*Buffer)
			break;


		File.Read(*Buffer, *Size);
		Status = STATUS_SUCCESS;
	}
	File.Close();
	return Status;
#else
	return STATUS_SUCCESS;
#endif
}




BOOL FASTCALL InitExtraHook()
{
	ml::MlInitialize();

}


BOOL NTAPI DllEntryMain(HMODULE hModule, DWORD Reason, LPVOID lpReserved)
{
	if (Reason == DLL_PROCESS_ATTACH)
		return InitExtraHook();

	return TRUE;
}
