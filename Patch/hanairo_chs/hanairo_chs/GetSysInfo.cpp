#include "GetSysInfo.h"  
#include "float.h"  
#include "winperf.h"
#include "atlbase.h"

GetSysInfo::GetSysInfo(void)
{
}

GetSysInfo::~GetSysInfo(void)
{
}

void GetSysInfo::GetOSVersion(wstring &strOSVersion, wstring &strServiceVersion)
{
	wstring str;
	OSVERSIONINFOEX osvi;
	SYSTEM_INFO si;
	BOOL bOsVersionInfoEx;

	ZeroMemory(&si, sizeof(SYSTEM_INFO));
	ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));

	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	if (!(bOsVersionInfoEx = GetVersionEx((OSVERSIONINFO *)&osvi)))
	{
		osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		GetVersionEx((OSVERSIONINFO *)&osvi);
	}


	GetProcAddress(GetModuleHandleW(TEXT("kernel32.dll")),
		"GetNativeSystemInfo");

	GetSystemInfo(&si);
	switch (osvi.dwPlatformId)
	{
	case VER_PLATFORM_WIN32_NT:
		if (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 0)
		{
			if (osvi.wProductType == VER_NT_WORKSTATION)
			{
				str = TEXT("Windows Vista ");
			}
			else
			{
				str = TEXT("Windows Server \"Longhorn\" ");
			}
		}
		if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 2)
		{
			if (GetSystemMetrics(SM_SERVERR2))
			{
				str = TEXT("Microsoft Windows Server 2003 \"R2\" ");
			}
			else if (osvi.wProductType == VER_NT_WORKSTATION &&
				si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64)
			{
				str = TEXT("Microsoft Windows XP Professional x64 Edition ");
			}
			else
			{
				str = TEXT("Microsoft Windows Server 2003, ");
			}
		}

		if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1)
		{
			str = TEXT("Microsoft Windows XP ");
		}

		if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0)
			str = TEXT("Microsoft Windows 2000 ");

		if (osvi.dwMajorVersion <= 4)
		{
			str = TEXT("Microsoft Windows NT ");
		}

		// Test for specific product on Windows NT 4.0 SP6 and later.  
		if (bOsVersionInfoEx)
		{

			//��Service Pack �汾����
			strServiceVersion.reserve(MAX_PATH);
			strServiceVersion.clear();
			wsprintfW(&strServiceVersion[0], TEXT("Service Pack %d"), osvi.wServicePackMajor);

			// Test for the workstation type.  
			if (osvi.wProductType == VER_NT_WORKSTATION &&
				si.wProcessorArchitecture != PROCESSOR_ARCHITECTURE_AMD64)
			{
				if (osvi.dwMajorVersion == 4)
					str = str + _T("Workstation 4.0");
				else if (osvi.wSuiteMask & VER_SUITE_PERSONAL)
					str = str + _T("Home Edition");
				else str = str + _T("Professional");
			}

			// Test for the server type.  
			else if (osvi.wProductType == VER_NT_SERVER ||
				osvi.wProductType == VER_NT_DOMAIN_CONTROLLER)
			{
				if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 2)
				{
					if (si.wProcessorArchitecture ==
						PROCESSOR_ARCHITECTURE_IA64)
					{
						if (osvi.wSuiteMask & VER_SUITE_DATACENTER)
							str = str + _T("Datacenter Edition for Itanium-based Systems");
						else if (osvi.wSuiteMask & VER_SUITE_ENTERPRISE)
							str = str + _T("Enterprise Edition for Itanium-based Systems");
					}

					else if (si.wProcessorArchitecture ==
						PROCESSOR_ARCHITECTURE_AMD64)
					{
						if (osvi.wSuiteMask & VER_SUITE_DATACENTER)
							str = str + _T("Datacenter x64 Edition ");
						else if (osvi.wSuiteMask & VER_SUITE_ENTERPRISE)
							str = str + _T("Enterprise x64 Edition ");
						else str = str + _T("Standard x64 Edition ");
					}

					else
					{
						if (osvi.wSuiteMask & VER_SUITE_DATACENTER)
							str = str + _T("Datacenter Edition ");
						else if (osvi.wSuiteMask & VER_SUITE_ENTERPRISE)
							str = str + _T("Enterprise Edition ");
						else if (osvi.wSuiteMask & VER_SUITE_BLADE)
							str = str + _T("Web Edition ");
						else str = str + _T("Standard Edition ");
					}
				}
				else if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0)
				{
					if (osvi.wSuiteMask & VER_SUITE_DATACENTER)
						str = str + _T("Datacenter Server ");
					else if (osvi.wSuiteMask & VER_SUITE_ENTERPRISE)
						str = str + _T("Advanced Server ");
					else str = str + _T("Server ");
				}
				else  // Windows NT 4.0   
				{
					if (osvi.wSuiteMask & VER_SUITE_ENTERPRISE)
						str = str + _T("Server 4.0, Enterprise Edition ");
					else str = str + _T("Server 4.0 ");
				}
			}
		}
		// Test for specific product on Windows NT 4.0 SP5 and earlier  
		else
		{
			HKEY hKey;
			TCHAR szProductType[256];
			DWORD dwBufLen = 256 * sizeof(TCHAR);
			LONG lRet;

			lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
				_T("SYSTEM\\CurrentControlSet\\Control\\ProductOptions"), 0, KEY_QUERY_VALUE, &hKey);
			if (lRet != ERROR_SUCCESS)
				strOSVersion = str;
			return;

			lRet = RegQueryValueEx(hKey, TEXT("ProductType"),
				NULL, NULL, (LPBYTE)szProductType, &dwBufLen);
			RegCloseKey(hKey);

			if ((lRet != ERROR_SUCCESS) ||
				(dwBufLen > 256 * sizeof(TCHAR)))
				strOSVersion = str;
			return;

			if (lstrcmpi(TEXT("WINNT"), szProductType) == 0)
				str = str + _T("Workstation ");
			if (lstrcmpi(TEXT("LANMANNT"), szProductType) == 0)
				str = str + _T("Server ");
			if (lstrcmpi(TEXT("SERVERNT"), szProductType) == 0)
				str = str + _T("Advanced Server ");

			str.resize(MAX_PATH);
			str.clear();
			wsprintfW(&str[0], TEXT("%d.%d "), osvi.dwMajorVersion, osvi.dwMinorVersion);
		}

		// Display service pack (if any) and build number.  

		if (osvi.dwMajorVersion == 4 &&
			lstrcmpi(osvi.szCSDVersion, TEXT("Service Pack 6")) == 0)
		{
			HKEY hKey;
			LONG lRet;

			// Test for SP6 versus SP6a.  
			lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
				TEXT("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Hotfix\\Q246009"), 0, KEY_QUERY_VALUE, &hKey);
			if (lRet == ERROR_SUCCESS)
			{
				str.resize(MAX_PATH);
				str.clear();
				wsprintfW(&str[0], _T("Service Pack 6a (Build %d)\n"),
					osvi.dwBuildNumber & 0xFFFF);
			}
			else // Windows NT 4.0 prior to SP6a  
			{
				wprintf_s(TEXT("%s (Build %d)\n"),
					osvi.szCSDVersion,
					osvi.dwBuildNumber & 0xFFFF);
			}

			RegCloseKey(hKey);
		}
		else // not Windows NT 4.0   
		{
			wprintf_s(TEXT("%s (Build %d)\n"),
				osvi.szCSDVersion,
				osvi.dwBuildNumber & 0xFFFF);
		}

		break;

		// Test for the Windows Me/98/95.  
	case VER_PLATFORM_WIN32_WINDOWS:

		if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 0)
		{
			str = TEXT("Microsoft Windows 95 ");
			if (osvi.szCSDVersion[1] == 'C' || osvi.szCSDVersion[1] == 'B')
				str = str + TEXT("OSR2 ");
		}

		if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 10)
		{
			str = TEXT("Microsoft Windows 98 ");
			if (osvi.szCSDVersion[1] == 'A' || osvi.szCSDVersion[1] == 'B')
				str = str + TEXT("SE ");
		}
		if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 90)
		{
			str = TEXT("Microsoft Windows Millennium Edition\n");
		}
		break;

	case VER_PLATFORM_WIN32s:
		str = TEXT("Microsoft Win32s\n");
		break;
	default:
		break;
	}

	strOSVersion = str;
}

BOOL GetSysInfo::IsWow64()
{
	typedef BOOL(WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);
	LPFN_ISWOW64PROCESS fnIsWow64Process;
	BOOL bIsWow64 = FALSE;
	fnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(GetModuleHandle(_T("kernel32")), "IsWow64Process");
	if (NULL != fnIsWow64Process)
	{
		fnIsWow64Process(GetCurrentProcess(), &bIsWow64);
	}
	return bIsWow64;
}

void GetSysInfo::GetCpuInfo(wstring &chProcessorName, wstring &chProcessorType, DWORD &dwNum, DWORD &dwMaxClockSpeed)
{

	wstring strPath = TEXT("HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0");//ע����Ӽ�·��  
	CRegKey regkey;//����ע��������  
	LONG lResult;//LONG�ͱ�������Ӧ���  
	lResult = regkey.Open(HKEY_LOCAL_MACHINE, LPCTSTR(strPath.c_str()), KEY_ALL_ACCESS); //��ע����  
	if (lResult != ERROR_SUCCESS)
	{
		return;
	}
	WCHAR chCPUName[50] = { 0 };
	DWORD dwSize = 50;

	//��ȡProcessorNameString�ֶ�ֵ  
	if (ERROR_SUCCESS == regkey.QueryStringValue(_T("ProcessorNameString"), chCPUName, &dwSize))
	{
		chProcessorName = chCPUName;
	}

	//��ѯCPU��Ƶ  
	DWORD dwValue;
	if (ERROR_SUCCESS == regkey.QueryDWORDValue(_T("~MHz"), dwValue))
	{
		dwMaxClockSpeed = dwValue;
	}
	regkey.Close();//�ر�ע���  
	//UpdateData(FALSE);  

	//��ȡCPU������Ŀ  
	SYSTEM_INFO si;
	memset(&si, 0, sizeof(SYSTEM_INFO));
	GetSystemInfo(&si);
	dwNum = si.dwNumberOfProcessors;

	switch (si.dwProcessorType)
	{
	case PROCESSOR_INTEL_386:
	{
		chProcessorType = TEXT("Intel 386 processor");
	}
	break;
	case PROCESSOR_INTEL_486:
	{
		chProcessorType = TEXT("Intel 486 Processor");
	}
	break;
	case PROCESSOR_INTEL_PENTIUM:
	{
		chProcessorType = TEXT("Intel Pentium Processor");
	}
	break;
	case PROCESSOR_INTEL_IA64:
	{
		chProcessorType = TEXT("Intel IA64 Processor");
	}
	break;
	case PROCESSOR_AMD_X8664:
	{
		chProcessorType = TEXT("AMD X8664 Processor");
	}
	break;
	default:
		chProcessorType = TEXT("δ֪");
		break;
	}

	//GetDisplayName()  
}

void  GetSysInfo::GetMemoryInfo(wstring &dwTotalPhys, wstring &dwTotalVirtual)
{
	//   TODO:     Add   extra   initialization   here   
	MEMORYSTATUS   Mem;
	//   get   the   memory   status   
	GlobalMemoryStatus(&Mem);

	DWORD dwSize = (DWORD)Mem.dwTotalPhys / (1024 * 1024);
	DWORD dwVirtSize = (DWORD)Mem.dwTotalVirtual / (1024 * 1024);

	dwTotalPhys.resize(MAX_PATH);
	dwTotalVirtual.resize(MAX_PATH);
	dwTotalPhys.clear();
	dwTotalVirtual.clear();

	wsprintfW(&dwTotalPhys[0], TEXT("�����ڴ�:%ld MB"), dwSize);
	wsprintfW(&dwTotalVirtual[0], TEXT("�����ڴ�:%ld MB"), dwVirtSize);
}

int GetSysInfo::GetInterFaceCount()
{
	/*CGetNetData pNet;
	DWORD pCount = pNet.GetNetworkInterfacesCount();
	return pCount;*/


	try
	{
#define DEFAULT_BUFFER_SIZE 40960L  

		unsigned char *data = (unsigned char*)malloc(DEFAULT_BUFFER_SIZE);
		DWORD type;
		DWORD size = DEFAULT_BUFFER_SIZE;
		DWORD ret;

		WCHAR s_key[4096] = { 0 };
		wsprintfW(s_key, L"510");
		//RegQueryValueEx�Ĺ̶����ø�ʽ          
		wstring str(s_key);

		//���RegQueryValueEx����ִ��ʧ�������ѭ��  
		while ((ret = RegQueryValueEx(HKEY_PERFORMANCE_DATA, str.c_str(), 0, &type, data, &size)) != ERROR_SUCCESS)
		{
			Sleep(10);
			//���RegQueryValueEx�ķ���ֵΪERROR_MORE_DATA(������ڴ���data̫С����������RegQueryValueEx���ص�����)  
			if (ret == ERROR_MORE_DATA)
			{
				Sleep(10);
				size += DEFAULT_BUFFER_SIZE;
				data = (unsigned char*)realloc(data, size);//���·����㹻����ڴ�  

				ret = RegQueryValueEx(HKEY_PERFORMANCE_DATA, str.c_str(), 0, &type, data, &size);//����ִ��RegQueryValueEx����  
			}
			//���RegQueryValueEx����ֵ�Ծ�δ�ɹ���������.....(ע���ڴ�й¶��free������~~~)��  
			//���if��֤�����whileֻ�ܽ���һ��~~~������ѭ��  
			if (ret != ERROR_SUCCESS)
			{
				if (NULL != data)
				{
					free(data);
					data = NULL;
				}
				return 0;//0���ӿ�  
			}
		}

		//����ִ�гɹ�֮����ǶԷ��ص�data�ڴ������ݵĽ����ˣ��������ȥ�鿴MSDN�й�RegQueryValueEx�����������ݽṹ��˵��  
		//�õ����ݿ�       
		PERF_DATA_BLOCK  *dataBlockPtr = (PERF_DATA_BLOCK *)data;
		//�õ���һ������  
		PERF_OBJECT_TYPE *objectPtr = (PERF_OBJECT_TYPE *)((BYTE *)dataBlockPtr + dataBlockPtr->HeaderLength);

		for (int a = 0; a<(int)dataBlockPtr->NumObjectTypes; a++)
		{
			char nameBuffer[255] = { 0 };
			if (objectPtr->ObjectNameTitleIndex == 510)
			{
				DWORD processIdOffset = ULONG_MAX;
				PERF_COUNTER_DEFINITION *counterPtr = (PERF_COUNTER_DEFINITION *)((BYTE *)objectPtr + objectPtr->HeaderLength);

				for (int b = 0; b<(int)objectPtr->NumCounters; b++)
				{
					if (counterPtr->CounterNameTitleIndex == 520)
						processIdOffset = counterPtr->CounterOffset;

					counterPtr = (PERF_COUNTER_DEFINITION *)((BYTE *)counterPtr + counterPtr->ByteLength);
				}

				if (processIdOffset == ULONG_MAX)
				{
					if (data != NULL)
					{
						free(data);
						data = NULL;
					}
					return 0;
				}

				PERF_INSTANCE_DEFINITION *instancePtr = (PERF_INSTANCE_DEFINITION *)((BYTE *)objectPtr + objectPtr->DefinitionLength);

				for (int b = 0; b<objectPtr->NumInstances; b++)
				{
					wchar_t *namePtr = (wchar_t *)((BYTE *)instancePtr + instancePtr->NameOffset);
					PERF_COUNTER_BLOCK *counterBlockPtr = (PERF_COUNTER_BLOCK *)((BYTE *)instancePtr + instancePtr->ByteLength);

					DWORD bandwith = *((DWORD *)((BYTE *)counterBlockPtr + processIdOffset));
					DWORD tottraff = 0;

					Interfaces.push_back(wstring(namePtr)); //������������  
					Bandwidths.push_back(bandwith);       //����  
					TotalTraffics.push_back(tottraff);    // ������ʼ��Ϊ0  

					PERF_COUNTER_BLOCK  *pCtrBlk = (PERF_COUNTER_BLOCK *)((BYTE *)instancePtr + instancePtr->ByteLength);


					instancePtr = (PERF_INSTANCE_DEFINITION *)((BYTE *)instancePtr + instancePtr->ByteLength + pCtrBlk->ByteLength);
				}
			}
			objectPtr = (PERF_OBJECT_TYPE *)((BYTE *)objectPtr + objectPtr->TotalByteLength);
		}
		if (data != NULL)
		{
			free(data);
			data = NULL;
		}
	}
	catch (...)
	{
		return 0;
	}
	return Interfaces.size();
}

void GetSysInfo::GetInterFaceName(wstring &InterfaceName, int pNum)
{
	UNREFERENCED_PARAMETER(InterfaceName);
	UNREFERENCED_PARAMETER(pNum);
	/*
	POSITION pos = Interfaces(pNum);
	if (pos == NULL)
	return;

	InterfaceName = Interfaces.GetAt(pos);
	pos = Bandwidths.FindIndex(pNum);
	if (pos == NULL)
	return;
	DWORD dwBandwidth = Bandwidths.GetAt(pos);

	wstring str;
	str.resize(MAX_PATH);
	str.clear();
	wsprintfW(&str[0], _T("%d"), dwBandwidth);

	InterfaceName = InterfaceName + str;
	*/
}

void GetSysInfo::GetDiskInfo(DWORD &dwNum, wstring chDriveInfo[])
{
	DWORD DiskCount = 0;

	//����GetLogicalDrives()�������Ի�ȡϵͳ���߼����������������������ص���һ��32λ�޷����������ݡ�  
	DWORD DiskInfo = GetLogicalDrives();

	//ͨ��ѭ�������鿴ÿһλ�����Ƿ�Ϊ1�����Ϊ1�����Ϊ��,���Ϊ0����̲����ڡ�  
	while (DiskInfo)
	{
		//ͨ��λ������߼���������ж��Ƿ�Ϊ1  
		Sleep(10);
		if (DiskInfo & 1)
		{
			DiskCount++;
		}
		DiskInfo = DiskInfo >> 1;//ͨ��λ��������Ʋ�����֤ÿѭ��һ��������λ�������ƶ�һλ��*/  
	}

	if (dwNum < DiskCount)
	{
		return;//ʵ�ʵĴ�����Ŀ����dwNum  
	}
	dwNum = DiskCount;//�����̷�����������  


	//-------------------------------------------------------------------//  
	//ͨ��GetLogicalDriveStrings()������ȡ�����������ַ�����Ϣ����  
	int DSLength = GetLogicalDriveStrings(0, NULL);

	WCHAR* DStr = new WCHAR[DSLength];
	memset(DStr, 0, DSLength);

	//ͨ��GetLogicalDriveStrings���ַ�����Ϣ���Ƶ�����������,���б�������������������Ϣ��  
	GetLogicalDriveStrings(DSLength, DStr);

	int DType;
	int si = 0;
	BOOL fResult;
	unsigned _int64 i64FreeBytesToCaller;
	unsigned _int64 i64TotalBytes;
	unsigned _int64 i64FreeBytes;

	//��ȡ����������Ϣ������DStr�ڲ����ݸ�ʽ��A:\NULLB:\NULLC:\NULL������DSLength/4���Ի�þ����ѭ����Χ  
	for (int i = 0; i<DSLength / 4; ++i)
	{
		Sleep(10);
		wstring strdriver = DStr + i * 4;
		wstring strTmp, strTotalBytes, strFreeBytes;
		DType = GetDriveType(strdriver.c_str());//GetDriveType���������Ի�ȡ���������ͣ�����Ϊ�������ĸ�Ŀ¼  
		switch (DType)
		{
		case DRIVE_FIXED:
		{
			strTmp = TEXT("���ش���");
		}
		break;
		case DRIVE_CDROM:
		{
			strTmp = TEXT("DVD������");
		}
		break;
		case DRIVE_REMOVABLE:
		{
			strTmp = TEXT("���ƶ�����");
		}
		break;
		case DRIVE_REMOTE:
		{
			strTmp = TEXT("�������");
		}
		break;
		case DRIVE_RAMDISK:
		{
			strTmp = TEXT("����RAM����");
		}
		break;
		case DRIVE_UNKNOWN:
		{
			strTmp = TEXT("����RAMδ֪�豸");
		}
		break;
		default:
			strTmp = TEXT("δ֪�豸");
			break;
		}

		//GetDiskFreeSpaceEx���������Ի�ȡ���������̵Ŀռ�״̬,�������ص��Ǹ�BOOL��������  
		fResult = GetDiskFreeSpaceEx(strdriver.c_str(),
			(PULARGE_INTEGER)&i64FreeBytesToCaller,
			(PULARGE_INTEGER)&i64TotalBytes,
			(PULARGE_INTEGER)&i64FreeBytes);

		if (fResult)
		{
			strTotalBytes.resize(MAX_PATH);
			strFreeBytes.resize(MAX_PATH);
			strTotalBytes.clear();
			strFreeBytes.clear();

			wsprintfW(&strTotalBytes[0], TEXT("����������%fMB"), (float)i64TotalBytes / 1024 / 1024);
			wsprintfW(&strFreeBytes[0], TEXT("����ʣ��ռ�%fMB"), (float)i64FreeBytesToCaller / 1024 / 1024);
		}
		else
		{
			strTotalBytes = TEXT("");
			strFreeBytes = TEXT("");
		}
		chDriveInfo[i] = strTmp + TEXT("(") + strdriver + TEXT("):") + strTotalBytes + strFreeBytes;
		si += 4;
	}
}

void GetSysInfo::GetDisplayCardInfo(DWORD &dwNum, wstring chCardName[])
{
	HKEY keyServ;
	HKEY keyEnum;
	HKEY key;
	HKEY key2;
	LONG lResult;//LONG�ͱ��������溯������ֵ  

	//��ѯ"SYSTEM\\CurrentControlSet\\Services"�µ������Ӽ����浽keyServ  
	lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("SYSTEM\\CurrentControlSet\\Services"), 0, KEY_READ, &keyServ);
	if (ERROR_SUCCESS != lResult)
		return;


	//��ѯ"SYSTEM\\CurrentControlSet\\Enum"�µ������Ӽ����浽keyEnum  
	lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("SYSTEM\\CurrentControlSet\\Enum"), 0, KEY_READ, &keyEnum);
	if (ERROR_SUCCESS != lResult)
		return;

	int i = 0, count = 0;
	DWORD size = 0, type = 0;
	for (;; ++i)
	{
		Sleep(5);
		size = 512;
		TCHAR name[512] = { 0 };//����keyServ�¸�������ֶ�����  

		//���ö��keyServ�µĸ������ֶα��浽name��  
		lResult = RegEnumKeyEx(keyServ, i, name, &size, NULL, NULL, NULL, NULL);

		//Ҫ��ȡ��������ڣ���keyServ������ȫ��������ʱ����ѭ��  
		if (lResult == ERROR_NO_MORE_ITEMS)
			break;

		//��keyServ�������ֶ�Ϊname����ʶ���ֶε�ֵ���浽key  
		lResult = RegOpenKeyEx(keyServ, name, 0, KEY_READ, &key);
		if (lResult != ERROR_SUCCESS)
		{
			RegCloseKey(keyServ);
			return;
		}


		size = 512;
		//��ѯkey�µ��ֶ�ΪGroup���Ӽ��ֶ������浽name  
		lResult = RegQueryValueEx(key, TEXT("Group"), 0, &type, (LPBYTE)name, &size);
		if (lResult == ERROR_FILE_NOT_FOUND)
		{
			//?��������  
			RegCloseKey(key);
			continue;
		};



		//�����ѯ����name����Video��˵���ü������Կ�������  
		if (lstrcmp(TEXT("Video"), name) != 0)
		{
			RegCloseKey(key);
			continue;     //����forѭ��  
		};

		//��������������ִ�еĻ�˵���Ѿ��鵽���й��Կ�����Ϣ������������Ĵ���ִ����֮��Ҫbreak��һ��forѭ������������  
		lResult = RegOpenKeyEx(key, TEXT("Enum"), 0, KEY_READ, &key2);
		RegCloseKey(key);
		key = key2;
		size = sizeof(count);
		lResult = RegQueryValueEx(key, TEXT("Count"), 0, &type, (LPBYTE)&count, &size);//��ѯCount�ֶΣ��Կ���Ŀ��  

		dwNum = count;//�����Կ���Ŀ  
		for (int j = 0; j <count; ++j)
		{
			TCHAR sz[512] = { 0 };
			TCHAR name[64] = { 0 };
			wsprintf(name, TEXT("%d"), j);
			size = sizeof(sz);
			lResult = RegQueryValueEx(key, name, 0, &type, (LPBYTE)sz, &size);


			lResult = RegOpenKeyEx(keyEnum, sz, 0, KEY_READ, &key2);
			if (ERROR_SUCCESS)
			{
				RegCloseKey(keyEnum);
				return;
			}


			size = sizeof(sz);
			lResult = RegQueryValueEx(key2, TEXT("FriendlyName"), 0, &type, (LPBYTE)sz, &size);
			if (lResult == ERROR_FILE_NOT_FOUND)
			{
				size = sizeof(sz);
				lResult = RegQueryValueEx(key2, TEXT("DeviceDesc"), 0, &type, (LPBYTE)sz, &size);
				chCardName[j] = sz;//�����Կ�����  
			};
			RegCloseKey(key2);
			key2 = NULL;
		};
		RegCloseKey(key);
		key = NULL;
		break;
	}
}

