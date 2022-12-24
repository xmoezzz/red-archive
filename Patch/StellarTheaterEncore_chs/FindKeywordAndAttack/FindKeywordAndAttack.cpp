// FindKeywordAndAttack.cpp : 定义控制台应用程序的入口点。
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
	LPCTSTR lpFileName = L".\\*.*";	//指定搜索目录和文件类型，如搜索d盘的音频文件可以是"D:\\*.mp3"
	WIN32_FIND_DATA pNextInfo;	//搜索得到的文件信息将储存在pNextInfo中;
	hFile = FindFirstFile(lpFileName, &pNextInfo);//请注意是 &pNextInfo , 不是 pNextInfo;
	if (hFile == INVALID_HANDLE_VALUE)
	{
		return;
	}

	while (FindNextFile(hFile, &pNextInfo))
	{
		if (pNextInfo.cFileName[0] == '.')//过滤.和..
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

