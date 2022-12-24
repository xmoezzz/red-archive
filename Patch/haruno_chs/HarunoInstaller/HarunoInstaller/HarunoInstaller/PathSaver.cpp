#include "stdafx.h"
#include "PathSaver.h"

PathSaver* PathSaver::Handle = NULL;

PathSaver::PathSaver()
{
}

PathSaver::~PathSaver()
{
}

PathSaver* PathSaver::GetHandle()
{
	if(Handle == NULL)
	{
		try
		{
			Handle = new PathSaver;
		}
		catch(...)
		{
			if(Handle)
				delete Handle;
			Handle = NULL;
			return Handle;
		}
	}
	return Handle;
}

void PathSaver::SetPath(std::wstring& p)
{
	tPath = p;
}
