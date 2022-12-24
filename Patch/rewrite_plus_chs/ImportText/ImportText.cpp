#include <Windows.h>
#include <stdio.h>
#include <fstream>
#include <string>
#include <map>
#include <vector>

using std::map;
using std::string;
using std::wstring;
using std::fstream;
using std::vector;

ULONG DummyNode = 0;

struct StringInfo
{
	string CNLine;
	string JPLine;
	BOOL   Referred;

	StringInfo()
	{
		Referred = FALSE;
	}

	StringInfo& operator = (const StringInfo& o)
	{
		this->CNLine = o.CNLine;
		this->JPLine = o.JPLine;
		this->Referred = o.Referred;

		return *this;
	}
};


//¡ð
BYTE HollowMark[] = { 0xE2, 0x97, 0x8B };
//¡ñ
BYTE SolidMark[]  = { 0xE2, 0x97, 0x8F };


map<ULONG, StringInfo> OldTextHashTable;
map<ULONG, StringInfo> NewTextHashTable;


int main(int argc, char* argv[])
{
	if (argc != 3)
		return 0;

	setlocale(LC_ALL, ".ACP");

	string OriName = argv[1];
	string NewName = argv[2];
	string OutName = OriName + ".out";
	
	fstream OldText(OriName);
	if (!OldText)
	{
		printf("Failed to open %s\n", OriName.c_str());
		return 0;
	}

	printf("Parsing old....\n");
	string ReadLine;
	while (getline(OldText, ReadLine))
	{
		if (!memcmp(ReadLine.c_str(), HollowMark, 3))
		{
			CHAR Index[20] = { 0 };
			memcpy(Index, &ReadLine[3], 6);
			ReadLine = &ReadLine[12];

			StringInfo info;
			info.JPLine = ReadLine;
			ULONG IndexNo;
			sscanf(Index, "%d", &IndexNo);
			
			if (!getline(OldText, ReadLine))
			{
				printf("OldText : GetText Error...\n");
				return 0;
			}
			ReadLine = &ReadLine[12];
			info.CNLine = ReadLine;
			OldTextHashTable.insert(std::make_pair(IndexNo, info));
		}
	}

	OldText.close();


	fstream NewText(NewName);
	if (!NewText)
	{
		printf("Failed to open %s\n", NewName.c_str());
		return 0;
	}

	while (getline(NewText, ReadLine))
	{
		if (!memcmp(ReadLine.c_str(), HollowMark, 3))
		{
			CHAR Index[20] = { 0 };
			memcpy(Index, &ReadLine[3], 6);
			ReadLine = &ReadLine[12];

			StringInfo info;
			info.JPLine = ReadLine;
			ULONG IndexNo;
			sscanf(Index, "%d", &IndexNo);

			if (info.JPLine == "dummy")
				DummyNode = IndexNo;

			if (!getline(NewText, ReadLine))
			{
				printf("OldText : GetText Error...\n");
				return 0;
			}
			ReadLine = &ReadLine[12];
			info.CNLine = ReadLine;
			NewTextHashTable.insert(std::make_pair(IndexNo, info));
		}
	}

	NewText.close();

	for (auto it = NewTextHashTable.begin(); it != NewTextHashTable.end(); it++)
	{
		auto SubItem = OldTextHashTable.find(it->first - 65);

		if (SubItem != OldTextHashTable.end())
		{
			if (it->second.JPLine == SubItem->second.JPLine)
			{
				it->second.CNLine = SubItem->second.CNLine;
				it->second.Referred = TRUE;
			}
			else
			{
				WCHAR Info[0x2000] = { 0 };
				MultiByteToWideChar(CP_UTF8, 0, it->second.JPLine.c_str(), it->second.JPLine.length(), Info, 0x2000);
				wprintf(L"Item[%d] not found: %s\n", it->first, Info);
				return 0;
			}
		}
		else
		{
			it->second.CNLine = it->second.JPLine;
		}
	}

	FILE* file = fopen(OutName.c_str(), "wb");
	if (!file)
		return 0;

	for (auto it : NewTextHashTable)
	{
		fwrite(HollowMark, 1, 3, file);
		fprintf(file, "%06d", it.first);
		fwrite(HollowMark, 1, 3, file);
		fprintf(file, "%s\r\n", it.second.JPLine.c_str());

		fwrite(SolidMark, 1, 3, file);
		fprintf(file, "%06d", it.first);
		fwrite(SolidMark, 1, 3, file);
		fprintf(file, "%s\r\n\r\n", it.second.CNLine.c_str());
	}
	
	BOOL Result = TRUE;

	for (auto it : NewTextHashTable)
	{
		if (it.first <= DummyNode && it.second.Referred == FALSE)
		{
			WCHAR Info[0x1000] = { 0 };
			MultiByteToWideChar(CP_UTF8, 0, it.second.JPLine.c_str(), it.second.JPLine.length(), Info, 0x1000);
			wprintf(L"[%06d]%s\n", it.first, Info);
			Result = FALSE;
		}
	}

	if (Result)
	{
		printf("Ok!\n");
	}

	fclose(file);
	
	return 0;
}

