#ifndef _Hash_
#define _Hash_

#include <Windows.h>

template<class T>
size_t GetHash(const T *name)
{
	register size_t hash = 0;
	while (size_t ch = (size_t)*name++)
	{
		hash = hash * 131 + ch;
	}
	return hash;
}

#endif
