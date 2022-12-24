#include "stdio.h"
#include "lzss.h"
#include <Windows.h>



#define N               4096
#define F               18
#define THRESHOLD       2
#define NIL             N



LZSS::LZSS()
{
	buffer = new unsigned char[N + F - 1];
	lson = new int[N + 1];
	rson = new int[N + 257];
	dad = new int[N + 1];
}

LZSS::~LZSS()
{
	delete buffer;
	delete lson;
	delete rson;
	delete dad;
}

//获取一个字节的数据
int LZSS::GetByte()
{
	if (InSize++ >= InDataSize)return(EOF);
	switch (InType)
	{
	case inMEM:return(*InData++);
	}
	return(EOF);
}

//写入一个字节的数据
void LZSS::PutByte(unsigned char c)
{
	OutSize++;
	switch (OutType)
	{
	case inMEM:*OutData++ = c; return;
	}
}

//初始化串表
void LZSS::InitTree()
{
	int i;
	for (i = N + 1; i <= N + 256; i++)
		rson[i] = NIL;
	for (i = 0; i<N; i++)
		dad[i] = NIL;
}

//插入一个表项
void LZSS::InsertNode(int r)
{
	int i, p, cmp;
	unsigned char *key;
	cmp = 1;
	key = &buffer[r];
	p = N + 1 + key[0];
	rson[r] = lson[r] = NIL; mlen = 0;
	while (1)
	{
		if (cmp >= 0)
		{
			if (rson[p] != NIL)p = rson[p]; else
			{
				rson[p] = r; dad[r] = p; return;
			}
		}
		else
		{
			if (lson[p] != NIL)
				p = lson[p];
			else
			{
				lson[p] = r;
				dad[r] = p;
				return;
			}
		}
		for (i = 1; i<F; i++)
		if ((cmp = key[i] - buffer[p + i]) != 0)	break;
		if (i>mlen)
		{
			mpos = p;
			if ((mlen = i) >= F)	break;
		}
	}
	dad[r] = dad[p];
	lson[r] = lson[p];
	rson[r] = rson[p];
	dad[lson[p]] = r;
	dad[rson[p]] = r;
	if (rson[dad[p]] == p)
		rson[dad[p]] = r;
	else
		lson[dad[p]] = r;
	dad[p] = NIL;
}
void LZSS::DeleteNode(int p)
{
	int q;
	if (dad[p] == NIL)return;
	if (rson[p] == NIL)q = lson[p];
	else if (lson[p] == NIL)
		q = rson[p];
	else
	{
		q = lson[p];
		if (rson[q] != NIL)
		{
			do
			{
				q = rson[q];
			} while (rson[q] != NIL);
			rson[dad[q]] = lson[q]; dad[lson[q]] = dad[q];
			lson[q] = lson[p]; dad[lson[p]] = q;
		}
		rson[q] = rson[p];
		dad[rson[p]] = q;
	}
	dad[q] = dad[p];
	if (rson[dad[p]] == p)
		rson[dad[p]] = q;
	else
		lson[dad[p]] = q;
	dad[p] = NIL;
}

void LZSS::Encode()
{
	int i, c, len, r, s, lml, cbp;
	unsigned char codebuf[17], mask;
	InitTree();
	codebuf[0] = 0;
	cbp = mask = 1;
	s = 0;
	r = N - F;
	memset(buffer, ' ', r);
	for (len = 0; len<F && (c = GetByte()) != EOF; len++)
		buffer[r + len] = c;
	if (len == 0)	return;
	for (i = 1; i <= F; i++)
		InsertNode(r - i);
	InsertNode(r);
	do
	{
		if (mlen>len)
			mlen = len;
		if (mlen <= THRESHOLD)
		{
			mlen = 1;
			codebuf[0] |= mask;
			codebuf[cbp++] = buffer[r];
		}
		else
		{
			codebuf[cbp++] = (unsigned char)mpos;
			codebuf[cbp++] = (unsigned char)(((mpos >> 4) & 0xF0) | (mlen - (THRESHOLD + 1)));
		}
		if ((mask <<= 1) == 0)
		{
			for (i = 0; i<cbp; i++)
				PutByte(codebuf[i]);
			codebuf[0] = 0;
			cbp = mask = 1;
		}
		lml = mlen;
		for (i = 0; i<lml && (c = GetByte()) != EOF; i++)
		{
			DeleteNode(s);
			buffer[s] = c;
			if (s<F - 1)
				buffer[s + N] = c;
			s = (s + 1)&(N - 1);
			r = (r + 1)&(N - 1);
			InsertNode(r);
		}
		while (i++<lml)
		{
			DeleteNode(s);
			s = (s + 1)&(N - 1);
			r = (r + 1)&(N - 1);
			if (--len)
				InsertNode(r);
		}
	} while (len>0);

	if (cbp>1)
	for (i = 0; i<cbp; i++)
		PutByte(codebuf[i]);
}

void LZSS::Decode()
{
	int i, j, k, r, c;
	unsigned int flags;
	for (i = 0; i<N - F; i++)
		buffer[i] = ' ';
	r = N - F;
	flags = 0;
	for (;;)
	{
		if (((flags >>= 1) & 256) == 0)
		{
			if ((c = GetByte()) == EOF)
				break;
			flags = c | 0xFF00;
		}
		if (flags & 1)
		{
			if ((c = GetByte()) == EOF)
				break;
			PutByte(c);
			buffer[r++] = c;
			r &= (N - 1);
		}
		else
		{
			if ((i = GetByte()) == EOF)
				break;
			if ((j = GetByte()) == EOF)
				break;
			i |= ((j & 0xF0) << 4);
			j = (j & 0x0F) + THRESHOLD;
			for (k = 0; k <= j; k++)
			{
				c = buffer[(i + k)&(N - 1)];
				PutByte(c);
				buffer[r++] = c;
				r &= (N - 1);
			}
		}
	}
}

unsigned long LZSS::Compress(unsigned char *in, unsigned long insize, unsigned char *out)
{
	InType = inMEM;
	InData = in;
	InDataSize = insize;
	InSize = 0;

	OutType = inMEM;
	OutData = out;
	OutSize = 0;
	Encode();
	return(OutSize);
}

unsigned long LZSS::UnCompress(unsigned char *in, unsigned long insize, unsigned char *out)
{
	InType = inMEM;
	InData = in;
	InDataSize = insize;
	InSize = 0;

	OutType = inMEM;
	OutData = out;
	OutSize = 0;
	Decode();
	return(OutSize);
}
