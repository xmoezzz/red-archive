#ifndef _xKAG_
#define _xKAG_

#include <Windows.h>

#pragma pack (1)

struct struct_v29
{
	DWORD dword1;
	DWORD dword2;
	DWORD dword3;
	DWORD dword4;
};

struct xKAG
{
	struct_v29 struct_v290;
	BYTE f10[2320];
	DWORD dword920;
	BYTE f924[24];
	BYTE byte93C;
	BYTE f93D[3];
	DWORD dword940;
};
#pragma pack ()

#endif
