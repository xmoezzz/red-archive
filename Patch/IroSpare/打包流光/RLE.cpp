// The simplest of known compressions.
// RLE stands for "Run Length" compression.
//
// Implemented by Arkadi Kagan
//
#include "RLE.h"
#include <memory.h>

void CRLE::Encode(BYTE *target, long &tlen, BYTE *source, long slen, int escape)
{
	long s, t, i, count;
	BYTE last;
	t = s = 0;

	((DWORD*)target)[0] = slen;
	t += sizeof(DWORD);
	if (escape >= 0)
		target[t] = (BYTE)escape;
	else	target[t] = (BYTE)(escape = FindEscape(source, slen));
	t++;

	while (s < slen)
	{
		count = 0;
		last = source[s];
		while ((s + count < slen) && (source[s + count] == last) && (count < 255))
			count++;
		if ((count > 3) || (last == escape))
		{
			target[t] = (BYTE)escape;
			target[t + 1] = (BYTE)count;
			target[t + 2] = last;
			t += 3;
		}
		else
		{
			for (i = 0; i < count; i++)
				target[t + i] = last;
			t += count;
		}
		s += count;
	}

	tlen = t;
}
long CRLE::Decode(BYTE *target, long &tlen, BYTE *source, long)
{
	long i, s, t;
	BYTE escape;
	s = t = 0;

	tlen = ((DWORD*)source)[0];
	s += sizeof(DWORD);
	escape = source[s];
	s++;

	while (t < tlen)
	{
		if (source[s] == escape)
		{
			for (i = 0; i < source[s + 1]; i++)
				target[t++] = source[s + 2];
			s += 3;
		}
		while ((source[s] != escape) && (t < tlen))
			target[t++] = source[s++];
	}

	return s;
}

long CRLE::GetMaxEncoded(long len)
{
	return len * 3 / 2 + sizeof(BYTE) + sizeof(DWORD);
}
long CRLE::GetMaxDecoded(BYTE *source)
{
	return ((DWORD*)source)[0];
}

BYTE CRLE::FindEscape(BYTE *buf, long len)
{
	BYTE ret;
	int i;
	long m, tab[256];
	memset(tab, 0, 256 * sizeof(long));
	for (i = 0; i < len; i++)
		tab[buf[i]]++;

	ret = 0;
	m = tab[0];
	for (i = 1; i < 256; i++)
		if (tab[i] < m)
		{
			ret = (BYTE)i;
			m = tab[i];
		}
	return ret;
}
