#pragma once

#include <Windows.h>
#include <string>
#include <vector>

using std::wstring;
using std::vector;

class GetSysInfo
{
public:
	GetSysInfo(void);
	~GetSysInfo(void);

public:
	/********��ȡ����ϵͳ�汾��Service pack�汾��ϵͳ����************/
	void GetOSVersion(wstring &strOSVersion, wstring &strServiceVersion);
	BOOL IsWow64();//�ж��Ƿ�Ϊ64λ����ϵͳ  

	/***********��ȡ������Ŀ������***********/
	int  GetInterFaceCount();
	void GetInterFaceName(wstring &InterfaceName, int pNum);

	/***��ȡ�����ڴ�������ڴ��С***/
	void GetMemoryInfo(wstring &dwTotalPhys, wstring &dwTotalVirtual);

	/****��ȡCPU���ơ��ں���Ŀ����Ƶ*******/
	void GetCpuInfo(wstring &chProcessorName, wstring &chProcessorType, DWORD &dwNum, DWORD &dwMaxClockSpeed);

	/****��ȡӲ����Ϣ****/
	void GetDiskInfo(DWORD &dwNum, wstring chDriveInfo[]);

	/****��ȡ�Կ���Ϣ*****/
	void GetDisplayCardInfo(DWORD &dwNum, wstring chCardName[]);

private:
	vector<wstring> Interfaces;                       //������������������  
	vector<DWORD>       Bandwidths;   //�������Ĵ���  
	vector<DWORD>       TotalTraffics;    //��������������  
};

