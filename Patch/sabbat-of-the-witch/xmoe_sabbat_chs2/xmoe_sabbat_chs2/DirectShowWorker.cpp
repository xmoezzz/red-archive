#include "DirectShowWorker.h"

//2015/12/12 A dynamic method to load directshow filters
//http://blog.sina.com.cn/s/blog_7c0ccd76010134yg.html

/*
typedef int (WINAPI *PROC_DllGetClassObject)(REFCLSID rclsid, REFIID riid, LPVOID FAR* ppv);
PROC_DllGetClassObject g_PROC_DllGetClassObject = NULL;
IBaseFilter *pFFDshow = NULL;
IClassFactory *pClassFactory = NULL;
HMODULE   hInstLibrary = LoadLibrary(L"ffdshow.ax");
if (hInstLibrary )
{
	g_PROC_DllGetClassObject = (PROC_DllGetClassObject)GetProcAddress (hInstLibrary,"DllGetClassObject");
	if(g_PROC_DllGetClassObject)
	{
		g_PROC_DllGetClassObject(CLSID_FFDshow,IID_IClassFactory,(void**)&pClassFactory);
		pClassFactory->CreateInstance(0,IID_IBaseFilter, reinterpret_cast<void**> (&pFFDshow));
		m_FilterGraph->AddFilter(pFFDshow,L"ffdshow video decode");
		SAFE_RELEASE(pFFDshow);
	}
}
*/


//#define Mono


static const GUID CLSID_LavAudioDecoder =
{ 0xE8E73B6B, 0x4CB3, 0x44A4, { 0xBE, 0x99, 0x4F, 0x7B, 0xCB, 0x96, 0xE4, 0x91 } };

static const GUID CLSID_LavVideoDecoder =
{ 0xEE30215D, 0x164F, 0x4A92, { 0xA4, 0xEB, 0x9D, 0x4C, 0x13, 0x39, 0x0F, 0x9F } };

//171252A0-8820-4AFE-9DF8-5C92B2D66B04
static const GUID CLSID_LavSplitter_Source =
{ 0x171252A0, 0x8820, 0x4AFE, { 0x9D, 0xF8, 0x5C, 0x92, 0xB2, 0xD6, 0x6B, 0x04 } };


static const GUID CLSID_MonogramAACDecoder =
{ 0x3fc3dbbf, 0x9d37, 0x4ce0, { 0x86, 0x89, 0x65, 0x3f, 0xe8, 0xba, 0xb9, 0xb3 } };



//from mpc
//使用
static const GUID GUID_LAVSplitter = { 0x171252A0, 0x8820, 0x4AFE, { 0x9D, 0xF8, 0x5C, 0x92, 0xB2, 0xD6, 0x6B, 0x04 } };

//无视
static const GUID GUID_LAVSplitterSource = { 0xB98D13E7, 0x55DB, 0x4385, {0xA3, 0x3D, 0x09, 0xFD, 0x1B, 0xA2, 0x63, 0x38 }};

//使用
static const GUID GUID_LAVVideo = { 0xEE30215D, 0x164F, 0x4A92, {0xA4, 0xEB, 0x9D, 0x4C, 0x13, 0x39, 0x0F, 0x9F }};
static const GUID GUID_LAVAudio = { 0xE8E73B6B, 0x4CB3, 0x44A4,{ 0xBE, 0x99, 0x4F, 0x7B, 0xCB, 0x96, 0xE4, 0x91 }};


typedef HRESULT (WINAPI *StubDllGetClassObject)(REFCLSID rclsid, REFIID riid, LPVOID FAR* ppv);
typedef HRESULT (WINAPI *StubDllRegisterServer)();
typedef HRESULT (WINAPI *StubDllUnregisterServer)();

StubDllGetClassObject pfSplitterClass = nullptr;
StubDllGetClassObject pfVideoDecoderClass = nullptr;
StubDllGetClassObject pfAudioDecoderClass = nullptr;

StubDllRegisterServer pfInitSplitter = nullptr;
StubDllRegisterServer pfInitVideoDecoder = nullptr;
StubDllRegisterServer pfInitAudioDecoder = nullptr;

StubDllUnregisterServer pfUnInitSplitter = nullptr;
StubDllUnregisterServer pfUnInitVideoDecoder = nullptr;
StubDllUnregisterServer pfUnInitAudioDecoder = nullptr;

VOID WINAPI OnComError(const WCHAR* Info)
{
	MessageBoxW(GetActiveWindow(), Info, L"Error", MB_OK);
	ExitProcess(-1);
}

HRESULT WINAPI InitDirectShowFilter()
{
	HRESULT          hr = S_OK;
	IBaseFilter*     pLavSplitter = NULL;
	IBaseFilter*     pLavVideoDecoder = NULL;
	IBaseFilter*     pLavAudioDecoder = NULL;
	IClassFactory*   pClassFactorySplitter = NULL;
	IClassFactory*   pClassFactoryVideo = NULL;
	IClassFactory*   pClassFactoryAudio = NULL;
	HMODULE          hLibSplitter = NULL;
	HMODULE          hLibVideoDecoder = NULL;
	HMODULE          hLibAudioDecoder = NULL;


#ifndef Mono
	hLibSplitter = LoadLibraryW(L"LAVSplitter.ax");
	hLibVideoDecoder = LoadLibraryW(L"LAVVideo.ax");
	hLibAudioDecoder = LoadLibraryW(L"LAVAudio.ax");

	if (hLibSplitter)
	{
		pfSplitterClass = (StubDllGetClassObject)GetProcAddress(hLibSplitter, "DllGetClassObject");
		pfInitSplitter = (StubDllRegisterServer)GetProcAddress(hLibSplitter, "DllRegisterServer");
		pfUnInitSplitter = (StubDllUnregisterServer)GetProcAddress(hLibSplitter, "DllUnregisterServer");
		if (pfSplitterClass)
		{

			hr = pfSplitterClass(GUID_LAVSplitter, IID_IClassFactory, (void**)&pClassFactorySplitter);
			if (FAILED(hr))
			{
				OnComError(L"Failed to load class[0]");
			}
			//pClassFactorySplitter->CreateInstance(0, IID_IBaseFilter, reinterpret_cast<void**> (&pLavSplitter));
		}
		/*
		if (pfInitSplitter)
		{
			hr = pfInitSplitter();
			if (FAILED(hr))
			{
				WCHAR Code[MAX_PATH] = {0};
				wsprintfW(Code, L" Code = 0x%08x", GetLastError());
				wstring Info(L"Failed to init filter[0]");
				Info += Code;
				OnComError(Info.c_str());
			}
		}
		*/
	}
	else
	{
		WCHAR Code[MAX_PATH] = { 0 };
		wsprintfW(Code, L" Code = 0x%08x", GetLastError());
		wstring Info(L"Failed to load filter[0]");
		Info += Code;
		OnComError(Info.c_str());
	}

	if (hLibVideoDecoder)
	{
		pfVideoDecoderClass = (StubDllGetClassObject)GetProcAddress(hLibVideoDecoder, "DllGetClassObject");
		pfInitVideoDecoder = (StubDllRegisterServer)GetProcAddress(hLibSplitter, "DllRegisterServer");
		pfUnInitVideoDecoder = (StubDllUnregisterServer)GetProcAddress(hLibSplitter, "DllUnregisterServer");
		if (pfVideoDecoderClass)
		{
			hr = pfVideoDecoderClass(GUID_LAVVideo, IID_IClassFactory, (void**)&pClassFactoryVideo);
			if (FAILED(hr))
			{
				OnComError(L"Failed to load class[1]");
			}
		}
		/*
		if (pfInitVideoDecoder)
		{
			hr = pfInitVideoDecoder();
			if (FAILED(hr))
			{
				OnComError(L"Failed to init filter[1]");
			}
		}
		*/
	}
	else
	{
		OnComError(L"Failed to load filter[1]");
	}


	if (hLibAudioDecoder)
	{
		pfAudioDecoderClass = (StubDllGetClassObject)GetProcAddress(hLibAudioDecoder, "DllGetClassObject");
		pfInitAudioDecoder = (StubDllRegisterServer)GetProcAddress(hLibAudioDecoder, "DllRegisterServer");
		pfUnInitAudioDecoder = (StubDllUnregisterServer)GetProcAddress(hLibAudioDecoder, "DllUnregisterServer");
		if (pfAudioDecoderClass)
		{
			hr = pfAudioDecoderClass(CLSID_LavAudioDecoder, IID_IClassFactory, (void**)&pClassFactoryAudio);
			if (FAILED(hr))
			{
				OnComError(L"Failed to load class[2]");
			}
			//pClassFactorySplitter->CreateInstance(0, IID_IBaseFilter, reinterpret_cast<void**> (&pLavSplitter));
		}
		/*
		if (pfInitAudioDecoder)
		{
			hr = pfInitAudioDecoder();
			if (FAILED(hr))
			{
				OnComError(L"Failed to init filter[2]");
			}
		}
		*/
	}
	else
	{
		OnComError(L"Failed to load filter[2]");
	}


#else
	hLibAudioDecoder = LoadLibraryW(L"aac_filter.ax");

	if (hLibAudioDecoder)
	{
		pfAudioDecoderClass = (StubDllGetClassObject)GetProcAddress(hLibAudioDecoder, "DllGetClassObject");
		pfInitAudioDecoder = (StubDllRegisterServer)GetProcAddress(hLibAudioDecoder, "DllRegisterServer");
		pfUnInitAudioDecoder = (StubDllUnregisterServer)GetProcAddress(hLibAudioDecoder, "DllUnregisterServer");
		if (pfAudioDecoderClass)
		{
			hr = pfAudioDecoderClass(CLSID_MonogramAACDecoder, IID_IClassFactory, (void**)&pClassFactoryAudio);
			if (FAILED(hr))
			{
				OnComError(L"Failed to load class[2]");
			}
			//pClassFactorySplitter->CreateInstance(0, IID_IBaseFilter, reinterpret_cast<void**> (&pLavSplitter));
		}
		if (pfInitAudioDecoder)
		{
			hr = pfInitAudioDecoder();
			if (FAILED(hr))
			{
				OnComError(L"Failed to init filter[2]");
			}
		}
	}
	else
	{
		OnComError(L"Failed to load filter[2]");
	}
#endif
	return hr;
}


HRESULT WINAPI UnInitDirectShowFilter()
{
	if (pfUnInitSplitter)
	{
		pfUnInitSplitter();
	}
	if (pfUnInitVideoDecoder)
	{
		pfUnInitVideoDecoder();
	}
	if (pfUnInitAudioDecoder)
	{
		pfUnInitAudioDecoder();
	}
	return S_OK;
}