// FindKeywordAndAttack.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include <vector>
#include <string>
#include <fstream>
#include <Windows.h>

using std::vector;
using std::fstream;
using std::string;


vector<std::wstring> FilesList;

void GetFileList()
{
	HANDLE hFile;
	LPCTSTR lpFileName = L".\\*.*";	//ָ������Ŀ¼���ļ����ͣ�������d�̵���Ƶ�ļ�������"D:\\*.mp3"
	WIN32_FIND_DATA pNextInfo;	//�����õ����ļ���Ϣ��������pNextInfo��;
	hFile = FindFirstFile(lpFileName, &pNextInfo);//��ע���� &pNextInfo , ���� pNextInfo;
	if (hFile == INVALID_HANDLE_VALUE)
	{
		return;
	}

	while (FindNextFile(hFile, &pNextInfo))
	{
		if (pNextInfo.cFileName[0] == '.')//����.��..
			continue;

		FilesList.push_back(pNextInfo.cFileName);
	}

}



int _tmain(int argc, _TCHAR* argv[])
{
	GetFileList();

	FILE* AttackFile = fopen("Attack.log", "wb");
	FILE* KeywordFile = fopen("Keyword.log", "wb");

	for (auto& FileName : FilesList)
	{
		fstream fin(FileName);
		string ReadLine;

		while (getline(fin, ReadLine))
		{
			if (strstr(ReadLine.c_str(), ".keyword"))
				fprintf(KeywordFile, "%s\r\n", ReadLine.c_str());

			if (strstr(ReadLine.c_str(), ".attack"))
			{
				getline(fin, ReadLine);
				if (ReadLine[0] != '.')
					fprintf(AttackFile, "%s\r\n", ReadLine.c_str());
			}
		}

		fin.close();
	}

	fclose(AttackFile);
	fclose(KeywordFile);
	return 0;
}

