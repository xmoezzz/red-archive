#pragma once

#include <Windows.h>
#include <map>
#include "ShinkuDef.h"

using std::map;

class GlobalMap
{
	static GlobalMap* m_Inst;
	GlobalMap();
public:

	~GlobalMap();

	static GlobalMap* GetGlobalMap();
	PVOID FindNode(ULONG_PTR Mask);
	BOOL  InsertNode(ULONG_PTR Mask, PVOID Node);

private:
	map<ULONG_PTR, PVOID> m_Map;
};

