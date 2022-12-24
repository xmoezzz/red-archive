#include "stdafx.h"
#include "KAGParser.h"
#include <string>

using std::wstring;

tTVPScenarioCacheItem::tTVPScenarioCacheItem(const ttstr & name, bool isstring)
{
	/*
	RefCount = 1;
	Lines = NULL;
	LineCount = 0;
	LabelCached = false;
	try
	{
		LoadScenario(name, isstring);
	}
	catch (...)
	{
		if (Lines) delete[] Lines;
		throw;
	}
	*/
}
//---------------------------------------------------------------------------
tTVPScenarioCacheItem::~tTVPScenarioCacheItem()
{
	if (Lines) delete[] Lines;
}
//---------------------------------------------------------------------------
void tTVPScenarioCacheItem::AddRef()
{
	RefCount++;
}
//---------------------------------------------------------------------------
void tTVPScenarioCacheItem::Release()
{
	if (RefCount == 1)
		delete this;
	else
		RefCount--;
}
//---------------------------------------------------------------------------
void tTVPScenarioCacheItem::LoadScenario(const wchar_t* name, bool isstring)
{
	// load scenario from file or string to buffer
	FILE* fin = nullptr;
	_wfopen_s(&fin, name, L"rb");
	if (fin == nullptr)
	{
		MessageBoxW(NULL, wstring(L"无法打开脚本 ： " + wstring(name)).c_str(), L"Error", MB_OK);
		ExitProcess(-1);
	}
	fseek(fin, 0, SEEK_END);
	size_t size = ftell(fin);
	rewind(fin);
	char* tBuffer = new char[size];
	fread(tBuffer, 1, size, fin);
	fclose(fin);

	Buffer.BufferSize = size;
	Buffer.Buffer = tBuffer;

	tjs_char *buffer_p = Buffer;

	// pass1: count lines
	tjs_int count = 0;
	tjs_char *ls = buffer_p;
	tjs_char *p = buffer_p;
	while (*p)
	{
		if (*p == L'\r' || *p == L'\n')
		{
			count++;
			if (*p == L'\r' && p[1] == L'\n') p++;
			p++;
			ls = p;
		}
		else
		{
			p++;
		}
	}

	if (p != ls)
	{
		count++;
	}

	Lines = new tLine[count];
	LineCount = count;

	// pass2: split lines
	count = 0;
	ls = buffer_p;
	while (*ls == '\t') ls++; // skip leading tabs
	p = ls;
	while (*p)
	{
		if (*p == L'\r' || *p == L'\n')
		{
			Lines[count].Start = ls;
			Lines[count].Length = p - ls;
			count++;
			tjs_char *rp = p;
			if (*p == L'\r' && p[1] == L'\n') p++;
			p++;
			ls = p;
			while (*ls == '\t') ls++;  // skip leading tabs
			p = ls;
			*rp = 0; // end line with null terminater
		}
		else
		{
			p++;
		}
	}

	if (p != ls)
	{
		Lines[count].Start = ls;
		Lines[count].Length = p - ls;
		count++;
	}

	LineCount = count;
	// tab-only last line will not be counted in pass2, thus makes
	// pass2 counted lines are lesser than pass1 lines.
}
//---------------------------------------------------------------------------
void tTVPScenarioCacheItem::EnsureLabelCache()
{
	//dummy
}
