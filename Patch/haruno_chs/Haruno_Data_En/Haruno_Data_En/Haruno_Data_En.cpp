// Haruno_Data_En.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <cstdio>
#include <string>

using std::string;

//data.xp3(data.xm3)的简单加密
int main(int argc, char **argv)
{
	unsigned long filelong = 0;
	if (argc != 2)
		return 0;

	FILE* fin = fopen(argv[1], "rb");
	fseek(fin, 0, SEEK_END);
	filelong = ftell(fin);
	rewind(fin);
	char *buffer = new char[filelong];
	fread(buffer, 1, filelong, fin);
	fclose(fin);
	unsigned long iPos = 0;
	while (iPos < filelong)
	{
		buffer[iPos] = (~buffer[iPos] & 0xFF);
		buffer[iPos] ^= 0x7F;
		iPos++;
	}

	FILE *out = fopen(string(string(argv[1]) + ".out").c_str(), "wb");

	fwrite(buffer, 1, filelong, out);
	fclose(out);
	return 0;
}
