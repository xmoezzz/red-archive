#include <Windows.h>
#include <ntstatus.h>
#include "my.h"
#include "SenaHook.h"

#pragma comment(lib, "Psapi.lib")



LONG NTAPI XmoeTopEvent(_In_ struct _EXCEPTION_POINTERS *ExceptionInfo)
{
	NtFileDisk    File;
	NTSTATUS      Status;
	LARGE_INTEGER BytesTranferred;

	LOOP_ONCE
	{
		Status = File.Create(L"Log.txt");
		if (NT_FAILED(Status))
			break;

		File.Print(&BytesTranferred, L"[Exception]\r\n");
		File.Print(&BytesTranferred, L"Address : 0x%08x\r\n", ExceptionInfo->ExceptionRecord->ExceptionAddress);
		File.Print(&BytesTranferred, L"Code    : 0x%08x\r\n", ExceptionInfo->ExceptionRecord->ExceptionCode);
		File.Print(&BytesTranferred, L"[Context]\r\n");
		File.Print(&BytesTranferred, L"DR0     : 0x%08x\r\n", ExceptionInfo->ContextRecord->Dr0);
		File.Print(&BytesTranferred, L"DR1     : 0x%08x\r\n", ExceptionInfo->ContextRecord->Dr1);
		File.Print(&BytesTranferred, L"DR2     : 0x%08x\r\n", ExceptionInfo->ContextRecord->Dr2);
		File.Print(&BytesTranferred, L"DR3     : 0x%08x\r\n", ExceptionInfo->ContextRecord->Dr3);
		File.Print(&BytesTranferred, L"DR6     : 0x%08x\r\n", ExceptionInfo->ContextRecord->Dr6);
		File.Print(&BytesTranferred, L"DR7     : 0x%08x\r\n", ExceptionInfo->ContextRecord->Dr7);
		File.Print(&BytesTranferred, L"Eax     : 0x%08x\r\n", ExceptionInfo->ContextRecord->Eax);
		File.Print(&BytesTranferred, L"Ebp     : 0x%08x\r\n", ExceptionInfo->ContextRecord->Ebp);
		File.Print(&BytesTranferred, L"Ebx     : 0x%08x\r\n", ExceptionInfo->ContextRecord->Ebx);
		File.Print(&BytesTranferred, L"Ecx     : 0x%08x\r\n", ExceptionInfo->ContextRecord->Ecx);
		File.Print(&BytesTranferred, L"Edi     : 0x%08x\r\n", ExceptionInfo->ContextRecord->Edi);
		File.Print(&BytesTranferred, L"Edx     : 0x%08x\r\n", ExceptionInfo->ContextRecord->Edx);
		File.Print(&BytesTranferred, L"Eip     : 0x%08x\r\n", ExceptionInfo->ContextRecord->Eip);
		File.Print(&BytesTranferred, L"Esi     : 0x%08x\r\n", ExceptionInfo->ContextRecord->Esi);
		File.Print(&BytesTranferred, L"Esp     : 0x%08x\r\n", ExceptionInfo->ContextRecord->Esp);
		File.Print(&BytesTranferred, L"%s\r\n", Nt_IsWow64Process() ? L"64Bit System" : L"32Bit System");

		File.Close();
	}

	MessageBoxW(NULL, L"±§Ç¸,ÄãÐÄ°®µÄ»ÆÓÍ²¹¶¡ÒÑ¾­±ÀÀ£", L"X'moe CoreLib", MB_OK | MB_ICONERROR);
	Ps::ExitProcess(TAG4('exce'));
	return EXCEPTION_CONTINUE_EXECUTION;
}


NTSTATUS NTAPI Initialization(HMODULE hModule)
{
	RtlSetUnhandledExceptionFilter(XmoeTopEvent);
	SenaHook::GetSenaHook()->SetSelfModule(hModule);
	SenaHook::GetSenaHook()->Init();
	return STATUS_SUCCESS;
}

NTSTATUS NTAPI UnInitialization(HMODULE hModule)
{
	UNREFERENCED_PARAMETER(hModule);
	SenaHook::GetSenaHook()->UnInit();
	return STATUS_SUCCESS;
}

MY_DLL_EXPORT PWCHAR WINAPI XmoeLinkProc()
{
	return L"X'moe Sofpal Universal Patch V2.0";
}


BOOL NTAPI DllMain(HMODULE hModule, DWORD Reason, LPVOID lpReserved)
{
	switch (Reason)
	{
	case DLL_PROCESS_ATTACH:
		return IsStatusSuccess(Initialization(hModule)) ? TRUE : FALSE;

	case DLL_PROCESS_DETACH:
		return IsStatusSuccess(UnInitialization(hModule)) ? TRUE : FALSE;
	}
	return TRUE;
}
