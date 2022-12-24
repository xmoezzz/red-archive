#include "stdafx.h"
#include "PackageWrapper.h"

#pragma pack (1)
union FileNameKeyInfo
{
	unsigned char c_key[8];
	WORD          w_key[4];
	DWORD         d_key[2];
};
#pragma pack ()

int DecodeFileName(wchar_t* in_file, wchar_t* out_file)
{
	FileNameKeyInfo Info;
	memcpy(Info.c_key, ".d36st8f2dw%^-s)", 16);
	Info.d_key[0] ^= 0x36F51A97;
	Info.d_key[1] ^= 0x65DE7A20;
	
	int i;
	for (i = 0; i < 16; i++)
	{
		Info.c_key[i] *= 0x23;
	}

	for (i = 0; i < wcslen(out_file); i++)
	{
		out_file[i] = in_file[i] ^ Info.w_key[i % 4];
	}
	return i;
}


static DWORD CompressedKey    = 0x2d6f8de1;
static DWORD PreCompressedKey = 0x6184FA31;


union IndexAtomInfo
{
	DWORD          d_key;
	unsigned char  c_key[4];
};

int DecodePackageIndex(unsigned char* in_buff, unsigned long len, unsigned char* out_buff)
{
	
}

/*****************************/
static wchar_t* FileTable[] = 
{
	L""
};


/******************************/
PackageManager* PackageManager::Handle = NULL;

PackageManager* PackageManager::GetPackage()
{
	if (Handle == NULL)
	{
		Handle = new PackageManager;
	}
	return Handle;
}


PackageManager::PackageManager(): 
inited(false)
{

}

PackageManager::~PackageManager()
{
	
}

void PackageManager::MakeHash()
{
	
	inited = true;
}

bool PackageManager::HasScript(const wchar_t* name)
{
	if (inited == false)
	{
		MakeHash();
	}
}

char* PackageManager::GetScript(const wchar_t* name)
{

}
