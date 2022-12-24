#ifndef _CharHolder_
#define _CharHolder_

#include <Windows.h>

#define TJS_strlen strlen

typedef char tjs_char;


class tTVPCharHolder
{
public:

	tjs_char *Buffer;
	size_t BufferSize;
public:
	tTVPCharHolder() : Buffer(NULL), BufferSize(0)
	{
	}
	~tTVPCharHolder()
	{
		Clear();
	}

	tTVPCharHolder(const tTVPCharHolder &ref) : Buffer(NULL), BufferSize(0)
	{
		operator =(ref);
	}

	void Clear()
	{
		if (Buffer) delete[] Buffer, Buffer = NULL;
		BufferSize = 0;
	}

	void operator =(const tTVPCharHolder &ref)
	{
		Clear();
		BufferSize = ref.BufferSize;
		Buffer = new tjs_char[BufferSize];
		memcpy(Buffer, ref.Buffer, BufferSize *sizeof(tjs_char));
	}

	void operator =(const tjs_char *ref)
	{
		Clear();
		if (ref)
		{
			BufferSize = TJS_strlen(ref) + 1;
			Buffer = new tjs_char[BufferSize];
			memcpy(Buffer, ref, BufferSize*sizeof(tjs_char));
		}
	}

	operator const tjs_char *() const
	{
		return Buffer;
	}

	operator tjs_char *()
	{
		return Buffer;
	}
};


class xmoeCharHolder
{
public:

	tTVPCharHolder o;

	//For Hooking
	void ToCharHolder(void *Addr)
	{
		tTVPCharHolder *ref = (tTVPCharHolder*)Addr;
		o.BufferSize = ref->BufferSize;;
		o.Buffer = ref->Buffer;
		//由系统自动回收
	}
};

#endif


/*
Hold不是指针，
需要先获取地址= = 

#include <cstdio>
#include <windows.h>

class A
{
public:
A(): a(0), b(0){}
int a;
int b;
};

void Proc(void* in)
{
A* b = (A*)in;
b->a = 1;
b->b = 2;
}

int main()
{
A p;
A* pp = &p;
Proc(pp);
printf("%d %d\n", p.a, p.b);
return 0;
}
*/

