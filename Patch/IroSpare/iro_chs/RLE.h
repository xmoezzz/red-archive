#ifndef _RLE_H
#define _RLE_H

#include <Windows.h>

class CRLE
{
public:
	void Encode(BYTE *target, long &tlen, BYTE *source, long slen, int escape = -1);
	long Decode(BYTE *target, long &tlen, BYTE *source, long slen);
	long GetMaxEncoded(long len);
	long GetMaxDecoded(BYTE *source);

	BYTE FindEscape(BYTE *buf, long len);
};

#endif
