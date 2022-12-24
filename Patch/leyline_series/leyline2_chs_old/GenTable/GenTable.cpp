// GenTable.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <cstdio> 
#include <cstdlib>
#include<time.h> 


int _tmain(int argc, _TCHAR* argv[])
{
	FILE* fin = fopen("key.txt", "wb");
	srand((int)time(NULL));
	for (int i = 0; i < 1024; i++)
	{
		if ((i + 1) % 8 == 0)
		{
			fprintf(fin, "\r\n\t");
		}
		unsigned long a = rand();
		unsigned long b = rand();
		//fprintf(fin, "0x%08x, ", a | (b<<16));
		fprintf(fin, "0x%02x, ", (unsigned char)a);
	}
	fclose(fin);
	return 0;
}

