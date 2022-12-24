#include "FriendToLoverHook.h"
#include "GetSysInfo.h"

#define AppCPUInfoIntel L"Intel"
#define AppCPUInfoAMD   L"AMD"

#define AppGPUInfoIntel0  L"Intel:�޷�����"
#define AppGPUInfoIntel1  L"Intel:Ӳ������"
#define AppGPUInfoNvidia0 L"NVIDIA:�޷�����"
#define AppGPUInfoNvidia1 L"NVIDIA:Ӳ������"
#define AppGPUInfoAMD0    L"AMD-ATI:���Ǻܶ�����AMD"
#define AppGPUInfoAMD1    L"AMD-ATI:���Ǻܶ�����AMD"
#define AppGPUInfoATI0    L"AMD-ATI:���Ǻܶ�����AMD"
#define AppGPUInfoATI1    L"AMD-ATI:���Ǻܶ�����AMD"
#define AppGPUInfoUnk     L"δ֪�Կ�:�޷�����"
#define AppDynamic0       L"��̬����:�ر�"
#define AppDynamic1       L"��̬����:����"

#define AccErrorReason0 L"(ָ��:OK)"
#define AccErrorReason1 L"(�޷�ʶ���ָ����)"
#define AccErrorReason2 L"(�����ݵ�CPU)"
#define AccErrorReason3 L"(�����ڲ��쳣)"

BOOL WINAPI GetIntelInternalSupport()
{
	HMODULE hMod = GetModuleHandleW(L"IntelQuickDecode.dll");
	if (!hMod)
		return FALSE;

	return TRUE;
}

BOOL WINAPI GetNvidiaInternalSupport()
{
	return TRUE;
}

BOOL WINAPI GetAMDInternalSupport()
{
	return FALSE;
}

HRESULT WINAPI FriendToLoverHook::SetAppName()
{

#if 1
	int Argc = 0;
	BOOL ReadyForRelease = TRUE;
	PWCHAR* Args = CommandLineToArgvW(GetCommandLineW(), &Argc);

	for (int i = 0; i < Argc; i++)
	{
		if (!lstrcmpiW(Args[i], L"--xmoe_debug"))
		{
			ReadyForRelease = FALSE;
			break;
		}
	}

	if (ReadyForRelease)
	{
		AppName = BaseAppName;
		return S_OK;
	}
#endif

	wstring CpuName, CpuType;
	DWORD CpuCount, CpuSpeed;
	GetSysInfo SystemInfo;
	SystemInfo.GetCpuInfo(CpuName, CpuType, CpuCount, CpuSpeed);

	AppName = BaseAppName;

	//AppName += AccErrorReason0;
	AppName += L"(Locale=936)";
	AppName += L"(Info : ";
	if (wcsstr(CpuName.c_str(), AppCPUInfoIntel))
	{
		AppName += L"Intelָ�����:����, ";
	}
	else
	{
		AppName += L"Intelָ�����:�ر�, ";
	}

	DISPLAY_DEVICEW dd = { 0 };
	dd.cb = sizeof(dd);
	vector<wstring> GraphCardList;
	int i = 0;
	HRESULT InfoResult = S_FALSE;

	AppName += L"����ģʽ:";

	while (EnumDisplayDevices(NULL, i, &dd, 0))
	{
		//wprintf(TEXT("Device Name: %s Device String: %s\n"), dd.DeviceName, dd.DeviceString);
		if (wcsstr(dd.DeviceString, L"Intel"))
		{
			InfoResult = S_OK;
			AppName += AppGPUInfoIntel0;
			break;
		}
		else if (wcsstr(dd.DeviceString, L"AMD") || wcsstr(dd.DeviceString, L"ATI"))
		{
			InfoResult = S_OK;
			if (GetAMDInternalSupport())
				AppName += AppGPUInfoAMD1;
			else
				AppName += AppGPUInfoAMD0;
			break;
		}
		else if (wcsstr(dd.DeviceString, L"NVIDIA"))
		{
			InfoResult = S_OK;
			if (GetNvidiaInternalSupport())
				AppName += AppGPUInfoNvidia1;
			else
				AppName += AppGPUInfoNvidia0;
			break;
		}

		if (EnumDisplayDevices(dd.DeviceName, 0, &dd, 0))
		{
			//wprintf(TEXT("Monitor Name: %s Monitor String: %s\n"), dd.DeviceName, dd.DeviceString);
		}
		i++;
	}

	if (FAILED(InfoResult))
		AppName += AppGPUInfoUnk;

	AppName += L", ";
	AppName += AppDynamic0;

	AppName += L")";
	return S_OK;
}

HRESULT WINAPI FriendToLoverHook::GetAppName(wstring& Name)
{
	Name = this->AppName;
	return S_OK;
}

