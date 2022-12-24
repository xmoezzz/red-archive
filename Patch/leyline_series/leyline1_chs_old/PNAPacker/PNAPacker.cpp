#include "stdafx.h"

#pragma pack(1)
struct PNAPHDR 
{
	char signature[4]; // "PNAP"
	long unknown1;
	long width;
	long height;
	long entry_count;
};

struct PNAPENTRY 
{
	long unknown1;
	long index;
	long offset_x;
	long offset_y;
	long width;
	long height;
	long unknown2;
	long unknown3;
	long unknown4;
	long length;
};
#pragma pack()


int _tmain(int argc, _TCHAR* argv[])
{
	return 0;
}

