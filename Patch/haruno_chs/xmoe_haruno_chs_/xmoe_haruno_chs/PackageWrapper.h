#ifndef _Package_Wrapper_
#define _Package_Wrapper_

#include <Windows.h>
#include <map>
#include <string>

using std::map;
using std::wstring;

int DecodePackageIndex();
int DecodeFileName();

class PackageManager
{
private:
	static PackageManager* Handle;
	PackageManager();

	map<DWORD, wstring> HashMap;
	map<DWORD, wstring>::iterator itr;

	bool inited;
	void MakeHash();

public:
	~PackageManager();
	static PackageManager* GetPackage();

	bool HasScript(const wchar_t* name);
	char* GetScript(const wchar_t* name);
};

#endif
