///white decoding and encoding
//X'moe Project, The WhiteEternity Chinesization Project 
//xmoe.project@gmail.com

#include "stdafx.h"

#include <cstdio>
#include <windows.h>
#include <cstring>

#include "zlib128\zlib-1.2.8\zlib.h"

#pragma comment(lib,"zlib1.lib")

char *pFile = 0;
char XORTable_Real[1024];

typedef struct xHeader
{
	DWORD OriginalLength;
	DWORD CompressedLength;
};


unsigned char TextXorOuter[256] = 
{
	0xae, 0xae, 0xd4, 0x22, 0x6c, 0xa1, 0xb7, 0x85, 0x5d, 0xbd, 0xcf, 0x34, 0xa1, 0x38, 0xcd, 0x34, 
	0x00, 0x45, 0x63, 0x74, 0x74, 0x8c, 0xf0, 0xed, 0xe7, 0xd7, 0x5f, 0xed, 0x3d, 0x0e, 0x4c, 0xb0, 
	0x95, 0x65, 0x69, 0x31, 0xcc, 0xd4, 0x21, 0x3f, 0x43, 0xc8, 0x7b, 0xd3, 0xe3, 0x18, 0x40, 0xa1, 
	0x5e, 0x3a, 0x60, 0xed, 0xb7, 0xb7, 0x92, 0xa0, 0x04, 0xdc, 0x3b, 0x9b, 0x73, 0xb2, 0x91, 0x4b, 
	0x8b, 0x31, 0x00, 0x7e, 0xb4, 0xb1, 0xcd, 0x75, 0xfa, 0xa1, 0xf9, 0x3a, 0x0e, 0x7b, 0x68, 0x34, 
	0x8d, 0xf7, 0x42, 0xf8, 0x85, 0x80, 0x9a, 0x62, 0x36, 0xe4, 0x4d, 0xe5, 0x15, 0x4f, 0x2d, 0x1f, 
	0x17, 0x7a, 0x5f, 0xb0, 0x2b, 0x20, 0x02, 0x4d, 0x08, 0xb2, 0x11, 0x10, 0x2a, 0x4b, 0x8b, 0x12, 
	0x17, 0xe6, 0xd1, 0x3c, 0xe7, 0xcf, 0x4f, 0x5a, 0x03, 0x57, 0x5e, 0x71, 0x2d, 0xcc, 0x69, 0x53, 
	0xc1, 0xa8, 0x4f, 0x70, 0x3a, 0x09, 0x09, 0xef, 0xf7, 0x61, 0x8d, 0xfe, 0x40, 0x6f, 0xf2, 0x66, 
	0x84, 0x6e, 0xd4, 0x62, 0xe4, 0x8c, 0xfa, 0xb1, 0xf6, 0x9d, 0x36, 0xea, 0xc2, 0x11, 0x8d, 0x10, 
	0x11, 0x23, 0x98, 0x66, 0xe8, 0x54, 0x2a, 0x84, 0x4f, 0x18, 0x34, 0xab, 0x56, 0xcf, 0xe5, 0x56, 
	0x5a, 0xf6, 0x15, 0x11, 0x85, 0x9f, 0xe3, 0x8e, 0x94, 0x1e, 0x9d, 0xf6, 0xdd, 0x06, 0xe1, 0x7d, 
	0x90, 0x54, 0x02, 0x39, 0x3d, 0xe9, 0xad, 0x34, 0x96, 0x3e, 0xcd, 0xc0, 0x77, 0x53, 0xab, 0x0a, 
	0x23, 0xe8, 0x5a, 0xf2, 0xd1, 0xf0, 0x52, 0x1a, 0x67, 0x43, 0x5b, 0x3d, 0x85, 0x93, 0xad, 0xc3, 
	0xc5, 0xa1, 0x55, 0x92, 0x41, 0xb0, 0xda, 0x26, 0x56, 0x3a, 0x21, 0xe4, 0xa8, 0xe3, 0x8e, 0xab, 
	0x66, 0xab, 0x6c, 0xad, 0xd0, 0x67, 0x8f, 0x7d, 0xf6, 0x72, 0x37, 0x68, 0xc2, 0xa0, 0x38, 0x09

};

//terminated by zero 
//in order to protect memory, i have to use this single decoding
char TextXorReal[256];
extern char* pFileBuffer;

//PushString-->decoding
void __stdcall EncodeText(DWORD in /*, const char *out*/)
{	
	/*
	if(pFileBuffer==NULL || in==0)
		return;//empty string
	unsigned char len = *(in + pFileBuffer +1);
	for(unsigned int i = 0; i < len - 1; i++)
	{
		*(char*)(pFileBuffer + in + i) ^= TextXorReal[i];
	}
	*/

	
	FILE *fin = NULL;
	fopen_s(&fin, "FVPString.txt", "a");
	try{
		fprintf_s(fin, "[%08x]%s\n", in, in + pFileBuffer);
	}
	catch(...)
	{
		char msg[MAX_PATH];
		memset(msg, 0, sizeof(msg));
		sprintf(msg, "Error : %08x", GetLastError());
		MessageBoxA(NULL, msg, "Debugger", MB_OK);
	}
	fclose(fin);
	
}

void __stdcall CalculateTextXorTable() 
{
	unsigned char TextXorInner[256] = 
	{
		0x08, 0x31, 0xb6, 0xd9, 0x4d, 0x8a, 0x77, 0x71, 0x32, 0x0e, 0x91, 0x71, 0x82, 0x6f, 0xbe, 0x41, 
		0x54, 0x77, 0x6b, 0x23, 0x2f, 0x9f, 0x04, 0x68, 0x72, 0xde, 0x71, 0xe4, 0xf3, 0xbf, 0xe1, 0x9f, 
		0xe7, 0xb6, 0xc6, 0xcd, 0x02, 0xb4, 0x20, 0x01, 0xdf, 0xf9, 0xff, 0x9d, 0xa1, 0x2a, 0x43, 0x8d, 
		0xdb, 0xa3, 0xa7, 0x35, 0xad, 0x0e, 0xfe, 0xa7, 0x30, 0x34, 0xbc, 0x1a, 0x14, 0x14, 0xb7, 0x98, 
		0x88, 0x32, 0x2f, 0xf6, 0x5a, 0x32, 0x0c, 0x09, 0x60, 0xa5, 0x68, 0x17, 0x16, 0x23, 0x4d, 0x8c, 
		0x88, 0x9a, 0xc0, 0xee, 0x73, 0xe5, 0xfe, 0x13, 0xa7, 0xa1, 0x05, 0x92, 0xaf, 0x3c, 0x56, 0x77, 
		0xb4, 0x4e, 0xfa, 0x3b, 0x9f, 0x2d, 0xc3, 0xf3, 0x7e, 0xbc, 0xd4, 0xc7, 0x29, 0x83, 0x63, 0xa6, 
		0x25, 0x05, 0xbe, 0x38, 0xc8, 0x4d, 0x8c, 0x14, 0x9e, 0xcb, 0x56, 0x34, 0x0c, 0x5e, 0x46, 0xa5, 
		0x33, 0xb2, 0x2e, 0x83, 0x17, 0xcc, 0xcb, 0x25, 0x01, 0xe3, 0x4c, 0x95, 0x22, 0x72, 0x0e, 0x43, 
		0x78, 0x8b, 0xaa, 0xfa, 0xf6, 0x6e, 0x31, 0x12, 0xdf, 0x5a, 0xb6, 0xe8, 0x73, 0xa3, 0x0d, 0x8b, 
		0xcd, 0x05, 0xd3, 0xb8, 0x0c, 0x38, 0xae, 0x09, 0xb1, 0xc4, 0xd6, 0x69, 0x48, 0x17, 0xd5, 0xcb, 
		0x4b, 0xd5, 0x8b, 0x1c, 0x43, 0x6f, 0x74, 0x75, 0x30, 0xf7, 0x2e, 0x96, 0x2b, 0x33, 0x35, 0x90, 
		0x4a, 0xf0, 0xf2, 0xc1, 0xc4, 0x98, 0xf4, 0x05, 0x55, 0x07, 0x7d, 0x2b, 0xe4, 0x9b, 0x40, 0xa6, 
		0x64, 0x8a, 0x69, 0x85, 0xf8, 0x79, 0xde, 0xa6, 0x5a, 0x4a, 0xc4, 0x26, 0x7d, 0x35, 0x46, 0x1b, 
		0x71, 0x1a, 0x91, 0x86, 0x88, 0x15, 0x23, 0x83, 0xb7, 0x54, 0x46, 0xc3, 0x3e, 0x25, 0xd7, 0x3c, 
		0x8b, 0x53, 0x4c, 0x1f, 0x5c, 0xb3, 0xf6, 0x0b, 0x25, 0xfa, 0x82, 0x80, 0xb1, 0xd1, 0xc6, 0x96
	};
	
	unsigned char TextSeed[8] = {0x51, 0x85, 0xf4, 0x56, 0x1a, 0x52, 0x61, 0xd1};
	
	DWORD iPos = 0;
	DWORD zPos = 0;
	
	while(iPos < 256)
	{
		TextXorReal[iPos] = TextXorInner[iPos] ^ TextXorOuter[iPos];
		TextXorReal[iPos] ^= TextSeed[zPos];
		iPos ++;
		zPos ++;
		
		zPos %= 8;
	}
}

void __stdcall FirstDecode(char *in, unsigned int in_len)
{
	if(!in)
	{
		MessageBoxA(NULL, "No Data! Please Re-install your game\n", "X'moe", MB_OK);
		exit(-1);
		//return 0;
	}
	CalculateXORTable();
	for(unsigned int iPos = 0; iPos < in_len; iPos++)
	{
		in[iPos] ^= XORTable_Real[iPos];
		iPos %= 1024;
	}
}

void __stdcall QuitAndShowMess(const char *in)
{
	if(!in || strlen(in) == 0)
		exit(-1);

	MessageBoxA(NULL, in, "X'moe", MB_OK);
	exit(-1);
}

unsigned char XORTable_1[1024] = 
{
	0x8d, 0x19, 0xa0, 0x34, 0xeb, 0xa0, 0x63, 0x24, 0xb5, 0xb2, 0x1f, 0x5c, 0x0b, 0xfc, 0x9c, 0x15, 
	0x8c, 0xb3, 0x08, 0x56, 0x76, 0x2b, 0x3d, 0x23, 0x0e, 0xa6, 0x43, 0xf2, 0x43, 0x6a, 0x16, 0x5e, 
	0x8e, 0x16, 0xa9, 0x1f, 0x1c, 0x35, 0x2a, 0x3a, 0x2f, 0x14, 0x88, 0x75, 0xc2, 0xd1, 0x33, 0x0e, 
	0x6c, 0xb5, 0x23, 0xac, 0x86, 0xc3, 0x1b, 0x96, 0x90, 0x91, 0x2f, 0x23, 0xd1, 0x57, 0x84, 0x72, 
	0x3e, 0x46, 0x57, 0x5a, 0x9d, 0x1b, 0x40, 0xa4, 0xea, 0xf3, 0xb8, 0x78, 0xfa, 0x61, 0xda, 0x16, 
	0x5e, 0xbe, 0x66, 0xc7, 0x8a, 0xc0, 0x0b, 0x12, 0x37, 0x4e, 0xe6, 0x32, 0x05, 0x93, 0x46, 0xc8, 
	0x65, 0x52, 0xb2, 0xce, 0xb6, 0x79, 0x2d, 0xcc, 0xae, 0xf8, 0xb8, 0x4e, 0xfc, 0xd3, 0x19, 0x95, 
	0x2b, 0x76, 0xdb, 0x8d, 0xca, 0x4a, 0x96, 0xff, 0xca, 0x85, 0x70, 0x08, 0x27, 0x46, 0xe4, 0xca, 
	0xcb, 0xe1, 0xc2, 0x61, 0xaf, 0x77, 0x77, 0x18, 0x43, 0xcb, 0x8f, 0xdd, 0x10, 0x50, 0x78, 0xf3, 
	0x9b, 0x87, 0x89, 0xe7, 0x8d, 0x87, 0x42, 0xc5, 0x12, 0xdd, 0xd6, 0x8b, 0x7f, 0x97, 0xe6, 0xde, 
	0x37, 0x9c, 0x90, 0xfc, 0xcf, 0x3f, 0xa8, 0xf2, 0x70, 0x13, 0x45, 0x0f, 0x7d, 0x00, 0x80, 0x98, 
	0x76, 0x96, 0x78, 0xbd, 0x1d, 0xa2, 0x9a, 0xcc, 0xd7, 0xff, 0x1f, 0xa5, 0x54, 0xaf, 0xd5, 0x6e, 
	0x72, 0x2a, 0x23, 0x88, 0x5f, 0xf6, 0x47, 0xc1, 0xfe, 0x78, 0xe3, 0xcb, 0x8d, 0x0a, 0xb8, 0xed, 
	0x84, 0x4e, 0xb1, 0xf8, 0xc0, 0xc1, 0x23, 0x7d, 0xe0, 0x93, 0x53, 0x3d, 0xf0, 0xb6, 0x39, 0xe1, 
	0x45, 0x35, 0x83, 0xeb, 0xa7, 0xc7, 0xdd, 0xed, 0xb6, 0xa4, 0x70, 0xf8, 0x86, 0x97, 0xa8, 0x58, 
	0x8d, 0x54, 0x3b, 0x7e, 0xbf, 0x0d, 0x67, 0x3e, 0xf7, 0x40, 0x7b, 0x3b, 0x99, 0xd3, 0x98, 0x9f, 
	0x75, 0x62, 0xb9, 0x0f, 0xef, 0xd8, 0xf1, 0xde, 0x5d, 0x3c, 0xf4, 0x80, 0xb1, 0xce, 0xd9, 0x42, 
	0x58, 0x53, 0x1e, 0x39, 0x62, 0xad, 0xed, 0x78, 0xe2, 0xae, 0x9d, 0x86, 0x98, 0x2e, 0x7c, 0x10, 
	0xcd, 0x5b, 0xcc, 0xda, 0x7f, 0x51, 0x0b, 0xfb, 0xbe, 0xeb, 0x78, 0x4a, 0x56, 0xd8, 0xd2, 0x14, 
	0xae, 0xf1, 0x63, 0x10, 0xf0, 0xc9, 0x3d, 0x94, 0x6a, 0x86, 0xc4, 0x08, 0x35, 0xf0, 0x6c, 0x9c, 
	0x14, 0xc8, 0xc4, 0x36, 0x9e, 0x5b, 0xb4, 0xae, 0xa0, 0x57, 0x02, 0x3e, 0xbd, 0xdc, 0x1b, 0x34, 
	0x57, 0xd6, 0x11, 0xea, 0xb3, 0x8b, 0xe0, 0xf8, 0x57, 0x70, 0xf5, 0xa8, 0xb7, 0x41, 0xf1, 0xab, 
	0x11, 0x50, 0xaa, 0x0a, 0x96, 0x1e, 0x72, 0x5f, 0xca, 0x28, 0x9d, 0x44, 0x2d, 0x03, 0x3d, 0x0c, 
	0x1b, 0xab, 0x30, 0xb1, 0xf1, 0x19, 0x5d, 0x0e, 0x71, 0x13, 0x3a, 0x4e, 0x68, 0x48, 0x92, 0xa5, 
	0x8d, 0x9c, 0x85, 0x3e, 0xad, 0xc1, 0xd0, 0x74, 0x05, 0x07, 0x4e, 0x44, 0xf0, 0x75, 0xbf, 0x02, 
	0xc2, 0x18, 0xc9, 0x4c, 0xf3, 0x9b, 0x3c, 0x3d, 0x80, 0x18, 0x9a, 0xe2, 0x8e, 0x2e, 0xd7, 0xf2, 
	0x50, 0x54, 0x5d, 0xb9, 0x2c, 0x6c, 0x53, 0x56, 0x19, 0x9c, 0x1f, 0x26, 0x4c, 0x59, 0x29, 0x80, 
	0x13, 0xc4, 0xe2, 0xa3, 0x01, 0x3a, 0x06, 0xec, 0x4b, 0x26, 0x1e, 0x4d, 0x72, 0x1a, 0x48, 0xfa, 
	0x22, 0x1f, 0x3a, 0x65, 0x5b, 0x49, 0x85, 0x6d, 0xce, 0x8e, 0x17, 0xd2, 0x8a, 0xd7, 0x04, 0xed, 
	0xd7, 0x58, 0x86, 0x9e, 0x62, 0x1d, 0x42, 0x85, 0x9b, 0xe6, 0xcc, 0x75, 0x5c, 0x35, 0x6e, 0x25, 
	0xcb, 0xa5, 0x25, 0x29, 0x81, 0x7d, 0xed, 0x21, 0xec, 0x85, 0x3e, 0x31, 0xf1, 0x19, 0xd7, 0xb1, 
	0xd6, 0x7b, 0xba, 0x25, 0x60, 0x6e, 0x78, 0x6f, 0x38, 0x00, 0xae, 0x43, 0x93, 0xa7, 0xd0, 0xdc, 
	0x12, 0x8f, 0x25, 0xed, 0xe7, 0x33, 0x13, 0xdb, 0x3a, 0x2b, 0x9d, 0x29, 0xcb, 0x44, 0x2a, 0x34, 
	0xd8, 0xd6, 0x88, 0x20, 0x41, 0x52, 0x30, 0x12, 0xea, 0x1b, 0xcc, 0x9f, 0x61, 0x97, 0xf6, 0x85, 
	0xc1, 0x85, 0x43, 0x99, 0xd6, 0x91, 0x80, 0x01, 0x81, 0x25, 0x3b, 0xa3, 0x5e, 0x83, 0x86, 0xdd, 
	0xa5, 0x11, 0xf7, 0x77, 0x4e, 0xf3, 0xf3, 0xd6, 0x79, 0xdf, 0x2c, 0x72, 0x0c, 0x2d, 0x69, 0x8a, 
	0x9e, 0x2e, 0x86, 0x16, 0x94, 0xbf, 0xbb, 0xfd, 0x89, 0x1e, 0x21, 0x88, 0xf4, 0xfc, 0x71, 0x16, 
	0x04, 0xd3, 0x0f, 0x12, 0xcf, 0x79, 0x48, 0x23, 0xac, 0xf6, 0xd8, 0xa2, 0xdd, 0x93, 0xb0, 0x51, 
	0x72, 0x33, 0xf6, 0x4a, 0x69, 0xe6, 0x4c, 0x35, 0x1a, 0xbc, 0x55, 0xbe, 0xd3, 0xd7, 0x75, 0x47, 
	0xbf, 0xc4, 0xd9, 0xd9, 0x0c, 0x0c, 0xb8, 0x61, 0x4c, 0x05, 0xd8, 0x19, 0x1c, 0xee, 0x53, 0x44, 
	0x04, 0x3b, 0x9b, 0x1e, 0x9f, 0x2e, 0xbc, 0x13, 0xfc, 0xa7, 0xe1, 0x2f, 0x43, 0x3d, 0x1a, 0xd6, 
	0x9c, 0x8d, 0x5c, 0xb5, 0x4c, 0xd2, 0xca, 0xf8, 0x21, 0xb7, 0x32, 0xbd, 0x11, 0x69, 0xdb, 0xc9, 
	0x1e, 0xef, 0x7e, 0x7a, 0x7d, 0xbe, 0x92, 0xfd, 0xf6, 0x88, 0xcc, 0xc1, 0x8e, 0x56, 0xe7, 0x2c, 
	0x64, 0xd6, 0xa1, 0x8c, 0xd9, 0xf6, 0x06, 0x50, 0xf3, 0xb1, 0xf0, 0x78, 0x04, 0x29, 0xcf, 0x4a, 
	0x86, 0xf7, 0xa6, 0x47, 0x4a, 0xbf, 0x56, 0x5d, 0xd2, 0x07, 0x1f, 0x5e, 0xfb, 0x49, 0x64, 0xb1, 
	0xde, 0x46, 0xae, 0x48, 0xf9, 0x9e, 0xf4, 0xd1, 0x8a, 0x9e, 0x1a, 0x31, 0x3c, 0x59, 0xb7, 0x2e, 
	0x05, 0xfa, 0x1b, 0x6b, 0x4f, 0x58, 0x91, 0x9a, 0x56, 0xcb, 0xe1, 0xed, 0xd1, 0x3f, 0x19, 0xcd, 
	0xd4, 0x86, 0x8d, 0xcf, 0xf5, 0xf3, 0x1d, 0xe4, 0xae, 0x24, 0xb6, 0xd0, 0x02, 0x1f, 0x1c, 0xdd, 
	0x63, 0xa1, 0xe6, 0xd0, 0xd4, 0xb2, 0xca, 0x1c, 0x4b, 0x7d, 0x1a, 0x56, 0x59, 0x5f, 0x8f, 0xe9, 
	0x0c, 0x3e, 0x46, 0x0b, 0x15, 0x1c, 0x08, 0xef, 0x26, 0xeb, 0xce, 0x3c, 0x9e, 0xa3, 0x84, 0xbf, 
	0x68, 0x93, 0x0e, 0x5c, 0x20, 0xf4, 0x89, 0x4b, 0x78, 0xc4, 0xd3, 0x81, 0xdb, 0xd2, 0x4d, 0x6b, 
	0x50, 0x15, 0xdf, 0xe2, 0xa0, 0x41, 0x3d, 0x5c, 0xbb, 0x9c, 0x69, 0x5f, 0x58, 0x0e, 0x7a, 0x3c, 
	0xdc, 0x78, 0x9b, 0xf9, 0x7d, 0x47, 0x56, 0x8f, 0xa7, 0x49, 0x13, 0x56, 0x9e, 0xbf, 0xdb, 0xbd, 
	0x66, 0xb3, 0x62, 0x3e, 0xe0, 0x8c, 0x45, 0x91, 0x35, 0xdf, 0x90, 0x20, 0x77, 0x88, 0x83, 0xbc, 
	0x86, 0xfa, 0x96, 0x8e, 0x31, 0xd3, 0xba, 0x50, 0x9e, 0xb4, 0xe2, 0xbd, 0xec, 0x4f, 0xc2, 0x45, 
	0x16, 0xc1, 0xd6, 0x06, 0x1b, 0x23, 0xa7, 0xf8, 0x5c, 0x5b, 0x4a, 0x67, 0x45, 0x38, 0x29, 0xa7, 
	0x2f, 0xbf, 0x06, 0x03, 0x85, 0xbf, 0x3c, 0xf6, 0x26, 0xac, 0x48, 0x9e, 0x0b, 0xa9, 0x89, 0x6d, 
	0x2a, 0xe7, 0x44, 0x22, 0x9a, 0x2e, 0xeb, 0xf7, 0xf7, 0xb9, 0x9f, 0x1c, 0x08, 0x47, 0xf3, 0x65, 
	0x9f, 0x6f, 0xf3, 0x40, 0xc1, 0x34, 0x65, 0xe9, 0x07, 0xd9, 0x4e, 0xe1, 0x44, 0xf7, 0xb8, 0x9c, 
	0x68, 0xcc, 0xb3, 0x79, 0xa5, 0xd6, 0x9a, 0xf8, 0xd0, 0xa0, 0x97, 0x28, 0x09, 0xdd, 0x69, 0x5e, 
	0x9e, 0xb3, 0x65, 0x2c, 0x2d, 0x59, 0xbb, 0x91, 0x09, 0xe4, 0xfb, 0x6e, 0xdf, 0x5e, 0xd8, 0x39, 
	0x9a, 0x19, 0x2b, 0xf5, 0x83, 0x42, 0x3b, 0x62, 0xad, 0xb9, 0x3b, 0x71, 0x90, 0x20, 0x14, 0xfa, 
	0xf4, 0x33, 0x65, 0xb1, 0x11, 0x57, 0xc8, 0x57, 0xf4, 0x75, 0x58, 0x2d, 0x24, 0x08, 0x70, 0xae, 
	0x86, 0x75, 0xb5, 0x7d, 0x7e, 0x9c, 0x56, 0x9d, 0x57, 0xac, 0x92, 0xe0, 0xe5, 0x3b, 0x7b, 0xa2

};

unsigned char XORTable_2[1024] = 
{
	0xfe, 0x83, 0xfc, 0xe7, 0xd6, 0xe9, 0x3e, 0x06, 0x07, 0xe6, 0xd6, 0xcd, 0x48, 0xa3, 0x5f, 0x39, 
	0x2d, 0xb6, 0x20, 0x46, 0x56, 0x8d, 0xe1, 0x6c, 0xe7, 0x10, 0xfd, 0x6a, 0x37, 0x82, 0xb9, 0x42, 
	0xcd, 0xa8, 0xd4, 0x85, 0x88, 0x2e, 0x5f, 0xd0, 0x1d, 0x46, 0xcf, 0x97, 0xc7, 0x92, 0x51, 0x21, 
	0x39, 0x44, 0x5d, 0xd4, 0xd8, 0x84, 0x8e, 0xb3, 0x27, 0x12, 0xb1, 0x25, 0x82, 0x2c, 0x1c, 0xf8, 
	0x10, 0xb2, 0x41, 0xa5, 0xf3, 0x88, 0x82, 0xd6, 0xc1, 0x3c, 0x47, 0x25, 0x38, 0xe8, 0x4e, 0x27, 
	0x2d, 0x5a, 0x43, 0xa9, 0xc6, 0x73, 0x91, 0x39, 0xe9, 0xcd, 0x77, 0xe9, 0xf4, 0xa0, 0x5e, 0x4f, 
	0xaf, 0xe7, 0x6a, 0xd0, 0x7f, 0xbe, 0x4f, 0x1f, 0xdc, 0x0f, 0x66, 0x00, 0x04, 0x6d, 0x00, 0x52, 
	0xf1, 0x40, 0xfa, 0x4c, 0x89, 0x23, 0x91, 0x07, 0x16, 0x8a, 0x78, 0x3c, 0xf5, 0xa8, 0x28, 0x50, 
	0x92, 0x90, 0x78, 0x8d, 0x94, 0x9a, 0x6d, 0xb3, 0x54, 0x08, 0x53, 0xaf, 0x94, 0xe9, 0x0c, 0xaa, 
	0x6e, 0x3e, 0xa9, 0x45, 0x8a, 0x5c, 0x37, 0x23, 0x94, 0x91, 0xdb, 0xa9, 0xed, 0x0a, 0x22, 0x01, 
	0xa1, 0xf4, 0x92, 0x65, 0x9a, 0xe2, 0x85, 0x9a, 0x13, 0x6f, 0x37, 0xba, 0x4f, 0x24, 0x1d, 0x37, 
	0x8a, 0x9c, 0x78, 0x1d, 0x30, 0xe5, 0x2b, 0x97, 0x4d, 0x2a, 0xc9, 0xb5, 0x44, 0x8f, 0xf3, 0x6c, 
	0xc5, 0x5d, 0xe1, 0xdf, 0xf9, 0x5f, 0x3e, 0xdc, 0xff, 0x8c, 0x39, 0xaa, 0x9c, 0xe5, 0xd9, 0x02, 
	0x2e, 0xa1, 0x90, 0x5b, 0xe2, 0x88, 0x14, 0x6b, 0x27, 0x9d, 0x6a, 0xea, 0x63, 0xff, 0x43, 0x99, 
	0xe4, 0x11, 0x8c, 0x83, 0x19, 0xd9, 0x41, 0x82, 0x02, 0xa6, 0x82, 0x06, 0xe5, 0xf6, 0xe8, 0x12, 
	0x43, 0x96, 0x19, 0x87, 0x0a, 0x0b, 0x9a, 0xa5, 0x0c, 0x31, 0xe5, 0xcf, 0xb0, 0x22, 0xbc, 0x8e, 
	0xe7, 0x59, 0xbc, 0xd9, 0x62, 0x18, 0x35, 0x94, 0x03, 0x06, 0x39, 0x56, 0x91, 0x1d, 0xf3, 0x6f, 
	0xaf, 0xc3, 0x3a, 0x2a, 0x0f, 0x37, 0x66, 0x4f, 0xe3, 0x2f, 0x63, 0xec, 0x94, 0xc0, 0x04, 0x55, 
	0xb7, 0x7d, 0x99, 0x6a, 0x3d, 0xe3, 0xc3, 0x18, 0xe9, 0xf5, 0x87, 0x23, 0x08, 0x23, 0xa2, 0x22, 
	0x5c, 0x70, 0x1c, 0xcb, 0x59, 0xd4, 0x20, 0x70, 0x94, 0xdf, 0x0b, 0xca, 0x78, 0xa0, 0xc3, 0xf6, 
	0x3a, 0xc5, 0x4a, 0xbd, 0x10, 0x03, 0x93, 0x18, 0x9e, 0xb9, 0x94, 0xf3, 0xb1, 0xd0, 0x9c, 0x32, 
	0x30, 0xe5, 0xe7, 0xf2, 0x50, 0xaa, 0x70, 0x11, 0x06, 0x89, 0x06, 0xf0, 0xc2, 0x8b, 0xa2, 0x77, 
	0x5a, 0x79, 0xf8, 0x5a, 0x44, 0x41, 0x4c, 0x9b, 0x09, 0x9a, 0x87, 0x51, 0xf6, 0xeb, 0x8a, 0xa7, 
	0x15, 0x69, 0xc2, 0x27, 0x5c, 0x80, 0xfc, 0x39, 0x23, 0x75, 0x7b, 0xe6, 0xdc, 0x49, 0x48, 0xe2, 
	0xfe, 0xe0, 0xca, 0xca, 0x42, 0x63, 0x96, 0xaa, 0x12, 0xe2, 0x88, 0xc2, 0x3e, 0x3e, 0x13, 0x8a, 
	0xf2, 0x46, 0xd5, 0xf3, 0xe5, 0x20, 0x6f, 0xf0, 0xd2, 0xeb, 0x93, 0x35, 0x2c, 0xa2, 0x5f, 0x3e, 
	0x0e, 0x43, 0xe9, 0x94, 0x71, 0x31, 0x1b, 0x4c, 0xa1, 0xd7, 0xc1, 0xd0, 0xf2, 0x8f, 0xe0, 0xe2, 
	0xae, 0xc2, 0x49, 0xdd, 0x53, 0x50, 0x6f, 0x3e, 0xfb, 0x32, 0x76, 0x64, 0x1c, 0x5e, 0x8c, 0x94, 
	0x71, 0xea, 0x7c, 0x40, 0x38, 0x75, 0x80, 0x89, 0x9e, 0xc3, 0x57, 0x02, 0x78, 0xa7, 0x98, 0xb7, 
	0x33, 0x26, 0x46, 0x6e, 0x0e, 0xd9, 0xa4, 0x2c, 0x86, 0x93, 0x4b, 0xfb, 0x12, 0x44, 0x79, 0xeb, 
	0x11, 0x1d, 0xac, 0x57, 0x01, 0xf5, 0x70, 0x69, 0xf1, 0xec, 0x75, 0xe0, 0x39, 0x4e, 0xe4, 0x11, 
	0x68, 0xba, 0xf3, 0x2c, 0x7e, 0x83, 0xb8, 0xc1, 0x5c, 0x56, 0x3a, 0x83, 0x78, 0x1d, 0xce, 0x4b, 
	0xd5, 0x24, 0xa1, 0x60, 0x33, 0x7a, 0x91, 0xf5, 0x83, 0x9b, 0x41, 0xf3, 0x9d, 0x4b, 0x6c, 0xf9, 
	0x35, 0xc5, 0x79, 0xa1, 0x0b, 0x15, 0x50, 0x06, 0x63, 0xc3, 0x6d, 0x83, 0xb5, 0xb1, 0x32, 0xbd, 
	0xa5, 0x46, 0x82, 0xe3, 0x35, 0xcc, 0x8b, 0x34, 0x3a, 0x17, 0xe3, 0xc2, 0x0d, 0x68, 0xd7, 0x76, 
	0x82, 0x91, 0x00, 0x55, 0x1e, 0x59, 0x17, 0x01, 0x84, 0x21, 0x09, 0x83, 0x31, 0xc8, 0x4e, 0x47, 
	0x69, 0xcd, 0x77, 0x68, 0x71, 0xb3, 0x07, 0x2f, 0xff, 0xaa, 0x84, 0xd6, 0xef, 0x6b, 0xce, 0x91, 
	0x37, 0x64, 0xaf, 0xcf, 0x1d, 0x15, 0xb2, 0xbc, 0xa8, 0xba, 0x39, 0x0b, 0x54, 0x2a, 0xca, 0xf4, 
	0x09, 0xff, 0xaa, 0x78, 0x4e, 0xf7, 0xad, 0xec, 0xbb, 0x9a, 0x4c, 0xb5, 0xac, 0x1d, 0xf8, 0x51, 
	0x3d, 0x87, 0xae, 0x97, 0x72, 0x12, 0xcc, 0x3f, 0xb5, 0xd4, 0x23, 0xa4, 0x86, 0x9e, 0x4d, 0xc9, 
	0x6e, 0x25, 0x40, 0x9a, 0x34, 0x5f, 0x24, 0x75, 0x54, 0x30, 0x62, 0xe9, 0xad, 0x46, 0xfe, 0xbe, 
	0x7a, 0x42, 0x26, 0x35, 0x83, 0x18, 0x0b, 0x90, 0x95, 0xb8, 0xef, 0xd5, 0x2f, 0xee, 0x7f, 0xd0, 
	0x7e, 0x86, 0x64, 0x57, 0x8b, 0xb5, 0x15, 0xd2, 0xb4, 0xb4, 0xef, 0xfa, 0x59, 0xae, 0x87, 0xe0, 
	0xd7, 0xdc, 0x3f, 0x31, 0xba, 0xef, 0x17, 0xb9, 0x2e, 0xae, 0xc6, 0x27, 0xb7, 0xe0, 0x0a, 0x10, 
	0x22, 0x6c, 0x3c, 0x36, 0xbc, 0xbf, 0x27, 0x09, 0xc1, 0x6e, 0x1a, 0x6e, 0x17, 0x1c, 0x3c, 0xc0, 
	0x3c, 0x9f, 0x20, 0x14, 0x7e, 0x5e, 0x99, 0xc2, 0x6a, 0xfd, 0xd0, 0x21, 0x86, 0x3d, 0x93, 0x91, 
	0x43, 0x1d, 0xf0, 0xbf, 0x2d, 0x46, 0x03, 0x24, 0x65, 0xa5, 0x0c, 0xcf, 0x51, 0x5a, 0xc4, 0x65, 
	0x92, 0xd1, 0xf2, 0x66, 0x36, 0x2e, 0x39, 0xb1, 0x2f, 0xef, 0x34, 0x4b, 0x04, 0xcc, 0xc5, 0x5c, 
	0xc7, 0xe2, 0xa9, 0x7a, 0x47, 0x11, 0x50, 0x2a, 0x86, 0xa3, 0xec, 0xa4, 0x6e, 0x2e, 0xc9, 0xd7, 
	0xbf, 0xbb, 0xdc, 0xad, 0x4c, 0x27, 0x9e, 0x90, 0x67, 0xca, 0x1b, 0x2d, 0x9a, 0x57, 0x45, 0x78, 
	0x98, 0x03, 0x8f, 0xf0, 0x72, 0xea, 0xb7, 0x24, 0x0e, 0xae, 0xe3, 0x76, 0xd6, 0x61, 0xf0, 0x1f, 
	0xad, 0xa5, 0x07, 0x73, 0x27, 0x11, 0x71, 0x66, 0xf9, 0xd7, 0xac, 0x50, 0xae, 0xa4, 0xbe, 0xed, 
	0x9c, 0xc8, 0xc9, 0xa8, 0x17, 0x97, 0xe0, 0x19, 0xe4, 0x0f, 0x19, 0xcc, 0xf0, 0xbb, 0xe3, 0x44, 
	0x42, 0xd6, 0x9a, 0x3f, 0x2f, 0xb4, 0x59, 0x3c, 0xcd, 0x5e, 0x10, 0x3b, 0xaa, 0x7d, 0xd6, 0xc4, 
	0xbd, 0x79, 0x80, 0x2a, 0x9c, 0xe1, 0x72, 0x11, 0xf0, 0x0e, 0xb5, 0x2e, 0x26, 0x03, 0x4a, 0x4e, 
	0x68, 0x98, 0xbe, 0x9a, 0xcc, 0xd8, 0xff, 0x19, 0xcb, 0xa7, 0x6e, 0x76, 0xf4, 0xa8, 0x35, 0x04, 
	0xe1, 0x5d, 0xdb, 0xff, 0x6b, 0x90, 0x16, 0x15, 0x1a, 0xf2, 0xe0, 0x24, 0xdf, 0x03, 0xcc, 0x46, 
	0x06, 0x31, 0x9b, 0x0b, 0x66, 0x44, 0x0a, 0x05, 0xdb, 0xf9, 0xef, 0x8a, 0xf6, 0xed, 0x84, 0xb5, 
	0xf2, 0xbd, 0x03, 0xae, 0xea, 0x6c, 0x73, 0x2b, 0x4a, 0x05, 0xc1, 0x37, 0x84, 0x81, 0x12, 0x33, 
	0x04, 0xea, 0x58, 0x1a, 0x65, 0xc1, 0x23, 0x09, 0xe5, 0x9e, 0xbb, 0xfe, 0x16, 0x16, 0x6b, 0xdf, 
	0xd7, 0xe1, 0x1f, 0xbf, 0x83, 0x3c, 0x32, 0x5e, 0x68, 0x8d, 0x81, 0xee, 0x7b, 0x46, 0xc4, 0x1d, 
	0x49, 0x0b, 0x1e, 0x4f, 0x31, 0x17, 0xf2, 0x2b, 0xd1, 0xdc, 0xf9, 0x5a, 0xbe, 0xe9, 0x91, 0x8b, 
	0x78, 0x11, 0x58, 0xbb, 0x9d, 0xca, 0xfa, 0xb3, 0x5c, 0xd3, 0x47, 0xd2, 0x2d, 0x1a, 0x89, 0x0c, 
	0xbf, 0xdc, 0x14, 0x33, 0x32, 0x0e, 0x1e, 0x75, 0x87, 0xfc, 0xd1, 0x27, 0x55, 0x30, 0x9f, 0xc1

};

void __stdcall CalculateXORTable()
{
	int i = 0;
	int j = 1023;
	while(i < 1024)
	{
		XORTable_Real[i] = XORTable_1[i] ^ XORTable_2[j];
		XORTable_Real[i] ^= 0x52;
		i++;
		j--;
	}
}

void __stdcall DecodeHeader(char *cHeader)
{
	if(!cHeader)
	{
		QuitAndShowMess("Invalid File!");
	}
	unsigned char seed[8] = {0x55, 0x14, 0x71, 0xf1, 0x31, 0x41, 0xa1, 0x91};
	for(unsigned int i = 0; i <= 7; i++)
	{
		cHeader[i] ^= seed[i];
	}
}




/*hcb File*/
/*
00438F33  |. E8 44F40000    CALL WhiteEte.0044837C                   ; \WhiteEte.0044837C
00438F38  |> 50             PUSH EAX                                 ; /hSearch  //hcb�����ɹ��� ֱ����ת������ر�����handle
00438F39  |. FF15 B8D04500  CALL DWORD PTR DS:[<&KERNEL32.FindClose>>; \FindClose
00438F3F  |. 8D5424 48      LEA EDX,DWORD PTR SS:[ESP+48]
00438F43  |. 8DB7 ACDF6800  LEA ESI,DWORD PTR DS:[EDI+68DFAC]
00438F49  |. 52             PUSH EDX    //LPCSTR filename
00438F4A  |. 8BCE           MOV ECX,ESI
00438F4C  |. E8 6FB5FFFF    CALL WhiteEte.004344C0    //��ʼCreateFile���ж�ȡ,�����һ�������Եĺ���,�����ļ���ȡ���Ǵ������ȡ��

*/

/*
004344C0  /$ 51             PUSH ECX
004344C1  |. 53             PUSH EBX
004344C2  |. 8B5C24 0C      MOV EBX,DWORD PTR SS:[ESP+C]
004344C6  |. 56             PUSH ESI
004344C7  |. 57             PUSH EDI
004344C8  |. 6A 00          PUSH 0                                   ; /hTemplateFile = NULL
004344CA  |. 68 00000008    PUSH 8000000                             ; |Attributes = SEQUENTIAL_SCAN
004344CF  |. 6A 03          PUSH 3                                   ; |Mode = OPEN_EXISTING
004344D1  |. 6A 00          PUSH 0                                   ; |pSecurity = NULL
004344D3  |. 6A 01          PUSH 1                                   ; |ShareMode = FILE_SHARE_READ
004344D5  |. 68 00000080    PUSH 80000000                            ; |Access = GENERIC_READ
004344DA  |. 53             PUSH EBX                                 ; |FileName
004344DB  |. 8BF1           MOV ESI,ECX                              ; |
004344DD  |. FF15 70D04500  CALL DWORD PTR DS:[<&KERNEL32.CreateFile>; \CreateFileA
004344E3  |. 8BF8           MOV EDI,EAX
004344E5  |. 83FF FF        CMP EDI,-1
004344E8  |. 75 37          JNZ SHORT WhiteEte.00434521
004344EA  |. 68 80000000    PUSH 80
004344EF  |. E8 11370100    CALL WhiteEte.00447C05
004344F4  |. 83C4 04        ADD ESP,4
004344F7  |. 85C0           TEST EAX,EAX
004344F9  |. 74 11          JE SHORT WhiteEte.0043450C
004344FB  |. 53             PUSH EBX
004344FC  |. 68 68D94500    PUSH WhiteEte.0045D968
00434501  |. 50             PUSH EAX
00434502  |. E8 E913FFFF    CALL WhiteEte.004258F0
00434507  |. 83C4 0C        ADD ESP,0C
0043450A  |. EB 02          JMP SHORT WhiteEte.0043450E
0043450C  |> 33C0           XOR EAX,EAX
0043450E  |> 894424 14      MOV DWORD PTR SS:[ESP+14],EAX
00434512  |. 68 F8BD4700    PUSH WhiteEte.0047BDF8                   ; /Arg2 = 0047BDF8
00434517  |. 8D4424 18      LEA EAX,DWORD PTR SS:[ESP+18]            ; |
0043451B  |. 50             PUSH EAX                                 ; |Arg1
0043451C  |. E8 5B3E0100    CALL WhiteEte.0044837C                   ; \WhiteEte.0044837C
00434521  |> 8B06           MOV EAX,DWORD PTR DS:[ESI]+
00434523  |. 85C0           TEST EAX,EAX
00434525  |. 74 16          JE SHORT WhiteEte.0043453D
00434527  |. 50             PUSH EAX                                 ; /pMemory
00434528  |. 6A 00          PUSH 0                                   ; |Flags = 0
0043452A  |. FF15 84D04500  CALL DWORD PTR DS:[<&KERNEL32.GetProcess>; |[GetProcessHeap
00434530  |. 50             PUSH EAX                                 ; |hHeap
00434531  |. FF15 80D04500  CALL DWORD PTR DS:[<&KERNEL32.HeapFree>] ; \HeapFree
00434537  |. C706 00000000  MOV DWORD PTR DS:[ESI],0
0043453D  |> 6A 00          PUSH 0                                   ; /pFileSizeHigh = NULL
0043453F  |. 57             PUSH EDI                                 ; |hFile
00434540  |. C746 04 000000>MOV DWORD PTR DS:[ESI+4],0               ; |
00434547  |. FF15 90D04500  CALL DWORD PTR DS:[<&KERNEL32.GetFileSiz>; \GetFileSize 
0043454D  |. 50             PUSH EAX      //�ļ���С@eax
0043454E  |. 8BCE           MOV ECX,ESI
00434550  |. E8 0BFBFFFF    CALL WhiteEte.00434060 //HeapReAllocate
00434555  |. 8B56 04        MOV EDX,DWORD PTR DS:[ESI+4]    //ԭʼ��EDI,Handle
00434558  |. 8B06           MOV EAX,DWORD PTR DS:[ESI]      //ԭʼ��EAX,pBuffer
0043455A  |. 6A 00          PUSH 0                                   ; /pOverlapped = NULL
0043455C  |. 8D4C24 10      LEA ECX,DWORD PTR SS:[ESP+10]            ; |
00434560  |. 51             PUSH ECX                                 ; |pBytesRead
00434561  |. 52             PUSH EDX                                 ; |BytesToRead
00434562  |. 50             PUSH EAX                                 ; |Buffer
00434563  |. 57             PUSH EDI                                 ; |hFile
00434564  |. FF15 8CD04500  CALL DWORD PTR DS:[<&KERNEL32.ReadFile>] ; \ReadFile
0043456A  |. 57             PUSH EDI                                 ; /hObject
0043456B  |. FF15 58D04500  CALL DWORD PTR DS:[<&KERNEL32.CloseHandl>; \CloseHandle //����ֵ,BOOL
00434571  |. 5F             POP EDI
00434572  |. 5E             POP ESI
00434573  |. 5B             POP EBX
00434574  |. 59             POP ECX
00434575  \. C2 0400        RETN 4
*/

/*
hcb������ɺ�:

00438F51  |. 8B46 08        MOV EAX,DWORD PTR DS:[ESI+8]   //EIP
00438F54  |. 8B0E           MOV ECX,DWORD PTR DS:[ESI]     //FileBuffer
00438F56  |. 8B0C08         MOV ECX,DWORD PTR DS:[EAX+ECX] //LPVOID (pBuffer + EIP), EAX = EIP?-->ECX = HeaderOffset
00438F59  |. 83C0 04        ADD EAX,4
*/