// Implementation of Arithmetic looseless compression.
//
// Implemented by Arkadi Kagan.
//
#include "Arithmetic.h"
#include <string.h>

#define BITS_CODE	16

CArithmetic::CArithmetic()
{
	msb = 1 << (BITS_CODE - 1);	// msb for BITS_CODE bits number
	msb2 = msb >> 1;			// msb for (BITS_CODE-1) bits number
	filler = msb - 1 + msb;		// mask with BITS_CODE one`s
	InitAll(1);
}
CArithmetic::~CArithmetic()
{
}

void CArithmetic::bitout(void *target, long &toffset, BYTE source)
{
	BYTE *t = (BYTE*)target;
	source = source ? (BYTE)1 : (BYTE)0;
	t[toffset / 8] |= source << (7 - (toffset % 8));
	toffset++;
}
BYTE CArithmetic::bitin(void *source, long &soffset)
{
	BYTE *s = (BYTE*)source;
	return (1 << (7 - (soffset % 8)))&s[soffset++ / 8] ? (BYTE)1 : (BYTE)0;
}

void CArithmetic::InitAll(BYTE filler)
{
	int i;
	sum = 0;
	for (i = 0; i < 256; i++)
	{
		let[i] = filler;
		acc[i] = sum;
		sum += filler;
	}
}
int CArithmetic::InitLet(BYTE *source, long length)
{
	long i, temp;
	sum = length;
	memset(let, 0, 256 * sizeof(DWORD));
	for (i = 0; i < length; i++)
		let[source[i]]++;

	// find maximum
	temp = 0;
	for (i = 0; i < 256; i++)
		if ((DWORD)temp < let[i])
			temp = let[i];

	if (temp <= 256)
	{
		UpdateAccByLet();
		return 0;
	}
	// scale probabilities to fit one byte
	sum = 0;
	for (i = 0; i < 256; i++)
	{
		let[i] = 255 * let[i] / temp;
		let[i] += let[i] ? 0 : 1;
		sum += let[i];
	}
	UpdateAccByLet();
	if (sum >= 0x3fff)
		return 1;
	return 0;
}
void CArithmetic::UpdateAccByLet()
{
	int i;
	DWORD sm = 0;
	for (i = 0; i < 256; i++)
	{
		acc[i] = sm;
		sm += let[i];
	}
}

BYTE CArithmetic::FindInRange(DWORD start, DWORD end, DWORD point)
{
	DWORD X = ((point - start + 1)*sum - 1) / (end - start + 1);
	int p1, p2, p3;
	if (X >= acc[255])
		return 255;
	p1 = 0; p2 = 255;
	while ((p2 - p1) / 2)
	{
		p3 = p1 + (p2 - p1) / 2;
		if (acc[p3] <= X)
			p1 = p3;
		else	p2 = p3;
	}
	return (BYTE)(p1 + (p2 - p1) / 2);
}
DWORD CArithmetic::GetStartPoint(DWORD start, DWORD end, BYTE leter)
{
	return acc[leter] * (end - start + 1) / sum + start;
}
DWORD CArithmetic::GetEndPoint(DWORD start, DWORD end, BYTE leter)
{
	return ((acc[leter] + let[leter])*(end - start + 1) / sum - 1) + start;
}

void CArithmetic::FlashEncoder(DWORD code, BYTE *target, long &toffset)
{
	int i, point = msb;
	bitout(target, toffset, !!(code&msb));
	while (underflow)
	{
		bitout(target, toffset, !(code&msb));
		underflow--;
	}
	for (i = 1; i < BITS_CODE; i++)
	{
		point >>= 1;
		bitout(target, toffset, !!(code&point));
	}
}
long CArithmetic::EncodeOnce(DWORD &start, DWORD &end, BYTE *target, long &toffset, BYTE source)
{
	DWORD	point = GetStartPoint(start, end, source);

	end = GetEndPoint(start, end, source);
	start = point;
	start &= filler;
	end &= filler;

	while (true)
	{
		if ((start&msb) == (end&msb))
		{
			bitout(target, toffset, !!(start&msb));
			while (underflow)
			{
				bitout(target, toffset, !(start&msb));
				underflow--;
			}
		}
		else	if ((start&msb2) && !(end&msb2))
		{
			underflow++;
			start ^= msb + msb2;
			end |= msb2;
		}
		else break;
		start <<= 1;
		end <<= 1;	end++;
		start &= filler;
		end &= filler;
	}
	return 1;
}
long CArithmetic::DecodeOnce(DWORD &start, DWORD &end, BYTE *target, BYTE *source, long &soffset, DWORD &code)
{
	DWORD	point;
	BYTE	leter;

	leter = FindInRange(start, end, code);
	target[0] = leter;

	point = GetStartPoint(start, end, leter);
	end = GetEndPoint(start, end, leter);
	start = point;
	start &= filler;		// may be not needed
	end &= filler;

	while (true)
	{
		if ((start&msb) == (end&msb))
		{
		}
		else	if ((start&msb2) && !(end&msb2))
		{
			code ^= msb2;
			start ^= msb + msb2;
			end |= msb2;
		}
		else break;
		code = (code << 1) + bitin(source, soffset);
		start <<= 1;
		end <<= 1;	end++;
		code &= filler;
		start &= filler;
		end &= filler;
	}
	return 1;
}

long CArithmetic::GetMaxEncoded(long len)
{
	return len + 256 * sizeof(BYTE) + sizeof(DWORD);
}
long CArithmetic::GetMaxDecoded(BYTE *source)
{
	return ((DWORD*)source)[0];
}

DWORD CArithmetic::LoadFirstCode(BYTE *source, long &offset)
{
	int i;
	DWORD code = 0;
	for (i = 0; i < BITS_CODE; i++)
		code = (code << 1) + bitin(source, offset);
	return code;
}
void CArithmetic::LoadTable(BYTE *source, long &offset)
{
	int i;
	sum = 0;
	for (i = 0; i < 256; i++)
	{
		let[i] = source[i];
		sum += let[i];
	}
	offset += 256 * sizeof(BYTE) * 8;
	UpdateAccByLet();
}
void CArithmetic::SaveTable(BYTE *target, long &offset)
{
	int i;
	for (i = 0; i < 256; i++)
		target[i] = (BYTE)let[i];
	offset += 256 * sizeof(BYTE) * 8;
}
void CArithmetic::Encode(BYTE *target, long &tlen, BYTE *source, long slen)
{
	DWORD start = 0, end = (1L << (BITS_CODE - 1)) - 1L + (1L << (BITS_CODE - 1));
	long offset = 0;
	long i;
	underflow = 0;
	memset(target, 0, tlen);
	tlen = sizeof(DWORD);
	((DWORD*)target)[0] = slen;
	target += sizeof(DWORD);

	if (InitLet(source, slen))
	{
		tlen = 0;
		return;
	}
	SaveTable(target, offset);

	for (i = 0; i < slen; i++)
	{
		EncodeOnce(start, end, target, offset, source[i]);
		if (offset / 8 >= slen)
		{
			tlen = 0;
			return;
		}
		OnStep();
	}
	FlashEncoder(start, target, offset);

	tlen += offset / 8 + 1;
}
long CArithmetic::Decode(BYTE *target, long &tlen, BYTE *source, long)
{
	DWORD start = 0, end = (1 << (BITS_CODE - 1)) - 1 + (1 << (BITS_CODE - 1));
	DWORD code;
	long offset = 0;
	long i;
	underflow = 0;
	tlen = GetMaxDecoded(source);
	source += sizeof(DWORD);
	LoadTable(source, offset);
	code = LoadFirstCode(source, offset);

	for (i = 0; i < tlen; i++)
	{
		DecodeOnce(start, end, target + i, source, offset, code);
		OnStep();
	}
	return offset / 8 + 1 + sizeof(DWORD);
}
