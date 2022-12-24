// TranslationTest.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <vector>
#include <string>
#include <fstream>
#include <Windows.h>
#include <set>
#include <map>

using std::vector;
using std::fstream;
using std::string;
using std::set;
using std::map;

vector<std::wstring> FilesList;
map<std::wstring, std::set<std::wstring>> Error;

void GetFileList()
{
	HANDLE hFile;
	LPCTSTR lpFileName = L".\\*.txt";	//指定搜索目录和文件类型，如搜索d盘的音频文件可以是"D:\\*.mp3"
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

int get_index(wchar_t* str)
{
	int len = wcslen(str);
	for (int i = 1; i<len; i++)
	{
		if (str[i] == L'>')
		{
			str[i] = 0;
			break;
		}
	}
	long index = wcstol(&str[1], 0, 10);

	return index;
}

typedef unsigned int ulong;

typedef struct string_table_s
{
	wchar_t* old_string;
	wchar_t* new_string;
}string_table_t;

typedef struct scene_file_info_s{
	ulong offset;
	ulong length;
} scene_file_info_t;

typedef struct header_pair_s{
	ulong offset;
	ulong count;
}header_pair_t;

typedef struct scene_header_s{
	ulong headerLength;
	header_pair_t v1;
	header_pair_t string_index_pair;
	header_pair_t string_data_pair;
	header_pair_t v4;
	header_pair_t v5;
	header_pair_t v6;
	header_pair_t v7;
	header_pair_t v8;
	header_pair_t v9;
	header_pair_t v10;
	header_pair_t v11;
	header_pair_t v12;
	header_pair_t v13;
	header_pair_t v14;
	header_pair_t v15;
	header_pair_t v16;
}scene_header_t;

void decrypt_string(wchar_t* str_buf, wchar_t* new_buf, int length, int key)
{
	key *= 0x7087;
	for (int i = 0; i<length; i++)
	{
		new_buf[i] = str_buf[i] ^ key;
	}
}


bool IsNonJapanese(LPWSTR lpString)
{
	bool res = true;
	for (int i = 0; i < wcslen(lpString); i++)
	{
		if (lpString[i] >= 0x80)
		{
			res = false;
			break;
		}
	}

	return res;
}

int main2(LPCWSTR TextName, LPCWSTR ScriptName, FILE* Writer)
{
	vector <string_table_t*> string_table;

	FILE* txt = _wfopen(TextName, L"rb");
	FILE* script = _wfopen(ScriptName, L"rb");
	fseek(script, 0, SEEK_END);
	size_t length = ftell(script);
	fseek(script, 0, SEEK_SET);

	byte* script_buf = new byte[length];
	fread(script_buf, length, 1, script);
	fclose(script);

	scene_header_t * header = (scene_header_t*)script_buf;

	scene_file_info_t* string_index = (scene_file_info_t*)&script_buf[header->string_index_pair.offset];
	wchar_t* string_data = (wchar_t*)&script_buf[header->string_data_pair.offset];
	printf("[stringpacker]decrypt string....\n");
	for (ulong x = 0; x<header->string_index_pair.count; x++)
	{
		scene_file_info_t* info = &string_index[x];
		wchar_t* info_str = &string_data[info->offset];
		wchar_t* new_str = new wchar_t[info->length + sizeof(wchar_t)];
		memset(new_str, 0, info->length * sizeof(wchar_t) + sizeof(wchar_t));

		decrypt_string(info_str, new_str, info->length, x);
		string_table_t *item = new string_table_t;
		item->old_string = new_str;
		item->new_string = 0;
		string_table.push_back(item);
	}
	printf("[stringpacker]decrypt string done....\n");

	fseek(txt, 2, SEEK_SET);
	union
	{
		wchar_t tmp_buffer[1024];
		byte cachebuffer[2048];
	};
	int index;

	while (fgetws(tmp_buffer, sizeof(tmp_buffer) / sizeof(wchar_t), txt))
	{
		if (tmp_buffer[0] == L'<')
		{
			index = get_index(tmp_buffer);
			continue;
		}
		if (tmp_buffer[0] == L'/')
			continue;
		if (tmp_buffer[0] == '\r' && tmp_buffer[1] == '\n')
			continue;
		int len = wcslen(tmp_buffer);
		for (int i = len; i != 0; i--)
		{
			if (tmp_buffer[i] == L'\r' && tmp_buffer[i + 1] == L'\n')
			{
				tmp_buffer[i] = 0;
				break;
			}
		}
		if (index<string_table.size())
		{
			string_table[index]->new_string = new wchar_t[wcslen(tmp_buffer) + 1];
			wcscpy(string_table[index]->new_string, tmp_buffer);
		}
		else
		{
			printf("[stringpacker]error index....????\n");
		}
	}
	//packer string to script files
	byte* buffer = (byte*)malloc(0);
	size_t offsets = 0;
	for (size_t x = 0; x<string_table.size(); x++)
	{
		scene_file_info_t* info = &string_index[x];
		if (string_table[x]->new_string)
		{
			wcscpy(tmp_buffer, string_table[x]->new_string);
		}
		else
		{
			wcscpy(tmp_buffer, string_table[x]->old_string);
		}
		size_t len = wcslen(tmp_buffer) * 2;
		string_index[x].offset = offsets / sizeof(wchar_t);
		string_index[x].length = len / 2;
		buffer = (byte*)realloc(buffer, offsets + len);
		decrypt_string(tmp_buffer, (wchar_t*)(&buffer[offsets]), len / 2, x);
		offsets += len;
	}


	set<std::wstring> ErrorHolder;

	for (size_t x = 0; x < string_table.size(); x++)
	{
		if (!string_table[x])
			continue;

		if (string_table[x]->old_string == NULL || string_table[x]->new_string == NULL)
			continue;

		if (!wcscmp(string_table[x]->old_string, string_table[x]->new_string))
		{
			if (!IsNonJapanese(string_table[x]->old_string))
			{
				ErrorHolder.insert(string_table[x]->old_string);
			}
		}
	}

	if (ErrorHolder.size())
	{
		fwprintf(Writer, L"File [%s]\r\n", TextName);

		for (auto& Item : ErrorHolder)
		{
			fwprintf(Writer, L"%s\r\n", Item.c_str());
		}
	}
	return 0;
}

std::wstring GetScnName(std::wstring Name)
{
	auto Index = Name.find_last_of(L'.');

	return Name.substr(0, Index);
}

int _tmain(int argc, _TCHAR* argv[])
{
	GetFileList();

	auto Writer = fopen("err.log", "wb");
	for (auto& FileName : FilesList)
	{
		main2(FileName.c_str(), GetScnName(FileName).c_str(), Writer);
	}
	fclose(Writer);
	return 0;
}

