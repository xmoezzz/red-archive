#include "stdafx.h"
#include <time.h>
#include <stdlib.h>

int _tmain(int argc, _TCHAR* argv[])
{
	FILE* fin = fopen("key.txt", "wb");
	srand((int)time(NULL));
	for (int i = 0; i < 256; i++)
	{
		fprintf(fin, "#define SecondTable_%03d TAG4('%c%c%c%c')\r\n", i,
			(rand() * 51) % 26 + 'A',
			(rand() * 51) % 26 + 'A',
			(rand() * 51) % 10 + '0',
			(rand() * 51) % 10 + '0');
	}
	for (int index = 0; index < 256; index++)
	{
		fprintf(fin, "BYTE SecondTableProc_%03d[] = \r\n{\r\n", index);
		bool first = true;
		for (int i = 1; i <= 128; i++)
		{
			if (first)
			{
				first = false;
				fprintf(fin, "\t");
			}
			if (i % 8 == 0)
			{
				fprintf(fin, "\r\n\t");
			}
			unsigned long a = rand();
			unsigned long b = rand();
			//fprintf(fin, "0x%08x, ", a | (b<<16));
			if (i != 128)
				fprintf(fin, "0x%02x, ", (unsigned char)a);
			else
				fprintf(fin, "0x%02x", (unsigned char)a);
		}
		fprintf(fin, "\r\n};\r\n", index);
	}

	for (int i = 0; i < 256; i++)
	{
		fprintf(fin, "g_GlobalMap->InsertNode(SecondTable_%03d, SecondTableProc_%03d);\r\n", i, i);
	}

	fprintf(fin, "switch(Selector)\r\n");
	fprintf(fin, "{\r\n");
	for (int i = 0; i < 256; i++)
	{
		fprintf(fin, "\tcase %d:\r\n", i);
		fprintf(fin, "\tInitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_%03d);\r\n", i);
		fprintf(fin, "\tbreak;\r\n\r\n");
	}
	fprintf(fin, "}\r\n");
	fclose(fin);
	return 0;
}

