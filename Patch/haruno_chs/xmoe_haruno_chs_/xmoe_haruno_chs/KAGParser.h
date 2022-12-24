#ifndef _KAGParser_
#define _KAGParser_

#include <Windows.h>
#include "CharHolder.h"
#include <vector>
#include "tHash.h"
#include "ttstr.h" //跳过前两个DWORD,只管wchar_t数据段

typedef int tjs_int;

class tTVPScenarioCacheItem
{
public:
	struct tLine
	{
		const tjs_char *Start;
		tjs_int Length;
	};

private:
	tTVPCharHolder Buffer;
	tLine *Lines;
	tjs_int LineCount;

public:
	struct tLabelCacheData
	{
		tjs_int Line;
		tjs_int Count;
		tLabelCacheData(tjs_int line, tjs_int count)
		{
			Line = line;
			Count = count;
		}
	};

public:
	typedef tTJSHashTable<ttstr, tLabelCacheData> tLabelCacheHash;
private:
	tLabelCacheHash LabelCache; // Label cache
	std::vector<ttstr> LabelAliases;
	bool LabelCached; // whether the label is cached

	tjs_int RefCount;

public:
	tTVPScenarioCacheItem(const ttstr & name, bool istring);
protected:
	~tTVPScenarioCacheItem();
public:
	void AddRef();
	void Release();
//private:
	//Rewrite for hooking
	void LoadScenario(const wchar_t* name, bool isstring);
	// load file or string to buffer
public:
	const ttstr & GetLabelAliasFromLine(tjs_int line) const
	{
		return LabelAliases[line];
	}
	void EnsureLabelCache();

	tLine * GetLines() const { return Lines; }
	tjs_int GetLineCount() const { return LineCount; }
	const tLabelCacheHash & GetLabelCache() const { return LabelCache; }
};

#endif
