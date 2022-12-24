#ifndef _PathSaver_
#define _PathSaver_

#include <string>

class PathSaver
{
private:

	PathSaver();
	static PathSaver* Handle;

public:

	~PathSaver();
	static PathSaver* GetHandle();
	void SetPath(std::wstring& p);
	std::wstring tPath;
};

#endif
