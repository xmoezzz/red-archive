#include "stdafx.h"
#include <vector>
#include <string>
#include <fstream>

using std::string;
using std::vector;
using std::fstream;

#define TextMagic "_TEXT_LIST__"

int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		return 0;
	}

	fstream fin(argv[1]);
	if (!fin)
	{
		return 0;
	}
	vector<string> TextPool;
	string ReadLine;
	while (getline(fin, ReadLine))
	{
		if (ReadLine.length())
		{
			TextPool.push_back(ReadLine);
		}
	}
	fin.close();

	FILE* out = fopen("TEXT_chs.DAT", "wb");
	unsigned Size = TextPool.size();
	fwrite(TextMagic, 1, strlen(TextMagic), out);
	fwrite(&Size, 1, 4, out);
	unsigned Index = 0;
	for (auto it : TextPool)
	{
		fwrite(&Index, 1, 4, out);
		fwrite(it.c_str(), 1, it.length() + 1, out);
		Index++;
	}
	fclose(out);
	return 0;
}

