#include <Windows.h>
#include <ntstatus.h>
#include "my.h"
#include "resource.h"
#include "SenaHook.h"
#include "BufferRenderer.h"

#include <ShellScalingAPI.h>

#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "Psapi.lib")
#pragma comment(lib, "STRMBASE.lib")


static const GUID CLSID_LavAudioDecoderXmoe =
{ 0xE8E73B6B, 0x4CB3, 0x44A4, { 0xBE, 0x99, 0x4F, 0x7B, 0xCB, 0x96, 0xE4, 0x92 } };

static const GUID CLSID_LavVideoDecoderXmoe =
{ 0xEE30215D, 0x164F, 0x4A92, { 0xA4, 0xEB, 0x9D, 0x4C, 0x13, 0x39, 0x0F, 0xA0 } };

//171252A0-8820-4AFE-9DF8-5C92B2D66B04
static const GUID CLSID_LavSplitter_SourceXmoe =
{ 0x171252A0, 0x8820, 0x4AFE, { 0x9D, 0xF8, 0x5C, 0x92, 0xB2, 0xD6, 0x6B, 0x05 } };



API_POINTER(DllGetClassObject) pfSplitterClass = nullptr;
API_POINTER(DllGetClassObject) pfVideoDecoderClass = nullptr;
API_POINTER(DllGetClassObject) pfAudioDecoderClass = nullptr;

API_POINTER(DllRegisterServer) pfInitSplitter = nullptr;
API_POINTER(DllRegisterServer) pfInitVideoDecoder = nullptr;
API_POINTER(DllRegisterServer) pfInitAudioDecoder = nullptr;

API_POINTER(DllUnregisterServer) pfUnInitSplitter = nullptr;
API_POINTER(DllUnregisterServer) pfUnInitVideoDecoder = nullptr;
API_POINTER(DllUnregisterServer) pfUnInitAudioDecoder = nullptr;

VOID WINAPI OnComError(const WCHAR* Info)
{
	MessageBoxW(NULL, Info, L"Error", MB_OK);
	Ps::ExitProcess(-1);
}

Void InitDirectShowFilter()
{
	HRESULT          hr;
	NTSTATUS         Status;
	IBaseFilter*     pLavSplitter = NULL;
	IBaseFilter*     pLavVideoDecoder = NULL;
	IBaseFilter*     pLavAudioDecoder = NULL;
	IClassFactory*   pClassFactorySplitter = NULL;
	IClassFactory*   pClassFactoryVideo = NULL;
	IClassFactory*   pClassFactoryAudio = NULL;
	PVOID            hLibSplitter = NULL;
	PVOID            hLibVideoDecoder = NULL;
	PVOID            hLibAudioDecoder = NULL;
	PVOID            hSelfModule, Buffer;
	HGLOBAL          BufferHandle;
	ULONG            BufferSize;
	HRSRC            ResourceHandle;
	UNICODE_STRING   DemuxerName, VideoName, AudioName;
	WCHAR            CurrentDir[MAX_PATH];
	WCHAR            FullPath[MAX_PATH];

	RtlZeroMemory(CurrentDir, MAX_PATH * 2);
	Nt_GetCurrentDirectory(MAX_PATH, CurrentDir);

	LOOP_ONCE
	{
		hSelfModule = SenaHook::GetSenaHook()->hSelfModule;

		ResourceHandle = ::FindResourceW((HMODULE)hSelfModule, MAKEINTRESOURCE(IDR_FILTER1), L"FILTER");
		if (!ResourceHandle)
			break;

		BufferSize = SizeofResource((HMODULE)hSelfModule, ResourceHandle);
		if (!BufferSize)
			break;

		BufferHandle = LoadResource((HMODULE)hSelfModule, ResourceHandle);
		Buffer = (LPSTR)LockResource(BufferHandle);
		if (!Buffer)
			break;

		RtlZeroMemory(FullPath, MAX_PATH * 2);
		StrCopyW(FullPath, CurrentDir);
		lstrcatW(FullPath, L"//XmoeSplitter.ax");

		RtlInitUnicodeString(&DemuxerName, FullPath);
		LoadDllFromMemory(Buffer, BufferSize, &DemuxerName, &hLibSplitter);
		FreeResource(BufferHandle);


		ResourceHandle = ::FindResourceW((HMODULE)hSelfModule, MAKEINTRESOURCE(IDR_FILTER2), L"FILTER");
		if (!ResourceHandle)
			break;

		BufferSize = SizeofResource((HMODULE)hSelfModule, ResourceHandle);
		if (!BufferSize)
			break;

		BufferHandle = LoadResource((HMODULE)hSelfModule, ResourceHandle);
		Buffer = (LPSTR)LockResource(BufferHandle);
		if (!Buffer)
			break;

		RtlZeroMemory(FullPath, MAX_PATH * 2);
		StrCopyW(FullPath, CurrentDir);
		lstrcatW(FullPath, L"//XmoeVideo.ax");

		RtlInitUnicodeString(&VideoName, FullPath);
		LoadDllFromMemory(Buffer, BufferSize, &VideoName, &hLibVideoDecoder);
		FreeResource(BufferHandle);



		ResourceHandle = ::FindResourceW((HMODULE)hSelfModule, MAKEINTRESOURCE(IDR_FILTER3), L"FILTER");
		if (!ResourceHandle)
			break;

		BufferSize = SizeofResource((HMODULE)hSelfModule, ResourceHandle);
		if (!BufferSize)
			break;

		BufferHandle = LoadResource((HMODULE)hSelfModule, ResourceHandle);
		Buffer = (LPSTR)LockResource(BufferHandle);
		if (!Buffer)
			break;

		RtlZeroMemory(FullPath, MAX_PATH * 2);
		StrCopyW(FullPath, CurrentDir);
		lstrcatW(FullPath, L"//XmoeAudio.ax");

		RtlInitUnicodeString(&AudioName, FullPath);
		LoadDllFromMemory(Buffer, BufferSize, &AudioName, &hLibAudioDecoder);
		FreeResource(BufferHandle);

		hr = E_FAIL;
		if (!hLibAudioDecoder || !hLibSplitter || !hLibVideoDecoder)
		{
			MessageBoxW(NULL, L"加载失败,但是不影响游戏运行", L"X'moe-CoreLib(GPL)", MB_OK);
			break;
		}

		hr = E_FAIL;
		if (!pfSplitterClass)
			break;

		hr = pfSplitterClass(CLSID_LavSplitter_SourceXmoe, IID_IClassFactory, (void**)&pClassFactorySplitter);
		if (FAILED(hr))
			break;


		hr = E_FAIL;
		if (!pfVideoDecoderClass)
			break;

		hr = pfVideoDecoderClass(CLSID_LavVideoDecoderXmoe, IID_IClassFactory, (void**)&pClassFactoryVideo);
		if (FAILED(hr))
			break;


		hr = E_FAIL;
		if (!pfAudioDecoderClass)
			break;

		hr = pfAudioDecoderClass(CLSID_LavAudioDecoderXmoe, IID_IClassFactory, (void**)&pClassFactoryAudio);
		if (FAILED(hr))
			break;
	}

}


Void UnInitDirectShowFilter()
{
	if (pfUnInitSplitter)
		pfUnInitSplitter();
	if (pfUnInitVideoDecoder)
		pfUnInitVideoDecoder();
	if (pfUnInitAudioDecoder)
		pfUnInitAudioDecoder();
}


Void NTAPI AdjustDpi()
{
	PPEB_BASE                           Peb;
	PVOID                               Shcore;
	API_POINTER(SetProcessDpiAwareness) StubSetProcessDpiAwareness;

	Peb = Nt_CurrentPeb();
	Shcore = Nt_LoadLibrary(L"Shcore.dll");

	if (Shcore)
		StubSetProcessDpiAwareness = (API_POINTER(SetProcessDpiAwareness))Nt_GetProcAddress(Shcore, "SetProcessDpiAwareness");
	else
		StubSetProcessDpiAwareness = NULL;

	switch (Peb->OSMajorVersion)
	{
	case 6:
		switch (Peb->OSMinorVersion)
		{
		case 3: //win8.1
			if (StubSetProcessDpiAwareness)
				StubSetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
			else
				SetProcessDPIAware();
			break;

		default: //win7 vista win 8
			SetProcessDPIAware();
			break;
		}
		break;

	case 10:
		if (StubSetProcessDpiAwareness)
			StubSetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
		else
			SetProcessDPIAware();
		break;

	default:
		SetProcessDPIAware();
		break;
	}
}


LONG WINAPI SenaFilter(_EXCEPTION_POINTERS *ExceptionInfo)
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

	if (ExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_FLT_DIVIDE_BY_ZERO)
	{
		MessageBoxW(NULL, L"抱歉,你心爱的黄油补丁已经崩溃", L"X'moe CoreLib", MB_OK | MB_ICONERROR);
		Ps::ExitProcess(TAG4('exce'));
	}
	return EXCEPTION_CONTINUE_EXECUTION;
}

NTSTATUS NTAPI Initialization(HMODULE hModule)
{
	//AllocConsole();
	AdjustDpi();
	ml::MlInitialize();
	RtlSetUnhandledExceptionFilter(SenaFilter);
	SenaHook::GetSenaHook()->SetSelfModule(hModule);
	//InitDirectShowFilter();
	SenaHook::GetSenaHook()->Init();
	return STATUS_SUCCESS;
}

NTSTATUS NTAPI UnInitialization(HMODULE hModule)
{
	UNREFERENCED_PARAMETER(hModule);

	UnInitDirectShowFilter();
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
