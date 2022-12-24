// FVPCHSEncode.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#include "Common.h"
#include "Encode.h"


#include "zlib128\zlib-1.2.8\zlib.h"

#pragma comment(lib,"zlib1.lib")

int main(int argc, char **argv)
{
	if(argc != 2)
	{
		printf("Usage : %s *.hcb\n", argv[0]);
		return -1;
	}
	DWORD iPos = 0;

	FILE* fin = fopen(argv[1], "rb");
	fseek(fin,0,SEEK_END);
    unsigned __int32 FileSize=ftell(fin);
    rewind(fin);
    char *oFile = new char[FileSize];
   	fread(oFile,FileSize,1,fin);
	fclose(fin);

	char *pFile = new char[FileSize*2];

	//EncodeHeader(oFile);
	xHeader header;
	header.OriginalLength = FileSize;
	header.CompressedLength = 0;
	char *tmp = new char[FileSize];
	DWORD size;
	compress((BYTE*)tmp, &size,(PBYTE)oFile, FileSize);

	memcpy(pFile+iPos, &(header.OriginalLength), sizeof(DWORD));
	iPos += 4;
	memcpy(pFile+iPos, &(header.CompressedLength), sizeof(DWORD));
	iPos += 4;

	memcpy(pFile, &(header.OriginalLength), 4);
	memcpy(pFile+4, &size, 4);
	memcpy(pFile+8, tmp, size);
	///pFile--OK

	//CalculateXORTable();
	//extern char XORTable_Real[1024];
	//EncodeHeader(pFile);
	/*
	iPos = 0;
	char *ppFile = (pFile + 8);
	for(unsigned int i = 0; i < size; i++)
	{
		ppFile[i] ^= XORTable_Real[1024];
		iPos++;
		iPos %= 1024;
	}
	*/
	FILE *fout = fopen("a.pck", "wb");
	fwrite(pFile, 1, 8 + size, fout);
	fclose(fout);


	//getchar();
	return 0;
}

