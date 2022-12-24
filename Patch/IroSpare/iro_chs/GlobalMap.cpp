#include "GlobalMap.h"

GlobalMap* GlobalMap::m_Inst = NULL;

GlobalMap::GlobalMap()
{

}

GlobalMap::~GlobalMap()
{
	m_Map.clear();
}

GlobalMap* GlobalMap::GetGlobalMap()
{
	if (m_Inst == NULL)
		m_Inst = new GlobalMap();

	return m_Inst;
}


PVOID GlobalMap::FindNode(ULONG_PTR Mask)
{
	auto it = m_Map.find(Mask);

	if (it != m_Map.end())
		return it->second;
	else
		return NULL;
}


BOOL GlobalMap::InsertNode(ULONG_PTR Mask, PVOID Node)
{
	m_Map.insert(std::make_pair(Mask, Node));
	return TRUE;
}
