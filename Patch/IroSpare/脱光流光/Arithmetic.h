#ifndef __ARITHMETIC_H
#define __ARITHMETIC_H

#include <Windows.h>

class CArithmetic
{
private:
	DWORD	let[256];		// array of probabilities for every leter
	DWORD	acc[256];		// array of accumulative probabilities (acc[0] = 0)
	DWORD	sum;			// sum of contence in array let or acc[255]+let[255]
	long	underflow;		// counter for underflow - bits
	DWORD	msb;			// msb for BITS_CODE bits number
	DWORD	msb2;			// msb for (BITS_CODE-1) bits number
	DWORD	filler;			// mask with BITS_CODE one`s
private:
	// source can`t be more then 2^28 bytes (2^28 = 268435456)
	inline BYTE bitin(void *source, long &soffset);
	inline void bitout(void *target, long &toffset, BYTE source);

	void InitAll(BYTE filler);
	int InitLet(BYTE *source, long length);
	inline void UpdateAccByLet();

	inline BYTE FindInRange(DWORD start, DWORD end, DWORD point);
	inline DWORD GetStartPoint(DWORD start, DWORD end, BYTE leter);
	inline DWORD GetEndPoint(DWORD start, DWORD end, BYTE leter);

	void FlashEncoder(DWORD code, BYTE *target, long &toffset);
	void LoadTable(BYTE *source, long &offset);
	void SaveTable(BYTE *target, long &offset);
	DWORD LoadFirstCode(BYTE *source, long &offset);

	long EncodeOnce(DWORD &start, DWORD &end, BYTE *target, long &toffset, BYTE source);
	long DecodeOnce(DWORD &start, DWORD &end, BYTE *target, BYTE *source, long &soffset, DWORD &code);
public:
	CArithmetic();
	virtual ~CArithmetic();

	void Encode(BYTE *target, long &tlen, BYTE *source, long slen);
	long Decode(BYTE *target, long &tlen, BYTE *source, long slen);
	long GetMaxEncoded(long len);
	long GetMaxDecoded(BYTE *source);

	virtual void OnStep() = 0;
};

#endif
