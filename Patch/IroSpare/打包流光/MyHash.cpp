#include "MyHash.h"

UInt32 MyHash(LPVoid lpBuffer, Int32 BufferSize)
{
	if (BufferSize < 8)
		return 0;

	__asm
	{
		pxor    mm0, mm0;
		pxor    mm2, mm2;
		mov     eax, 03070307h;
		movd    mm3, eax;
		punpckldq mm3, mm3;
		mov     eax, lpBuffer;
		mov     ecx, BufferSize;
		shr     ecx, 3;
		mov     edx, 8;
	PROR_OPOC_LOOP:
		movq    mm1, qword ptr[eax];
		paddw   mm2, mm3;
		pxor    mm1, mm2;
		paddw   mm0, mm1;
		lea     eax, [edx + eax];
		loop     PROR_OPOC_LOOP;
		movq    mm1, mm0;
		psrlq   mm1, 020h;
		pxor    mm0, mm1;
		movd    eax, mm0;
		emms;
	}
}



#define ROR_OP(a,b) (((a) << (b)) | ((a) >> (32 - (b))))
void salsa10_hash(DWORD x[16], DWORD in[16])
{
	int i;
	for (i = 0; i < 16; ++i) x[i] = in[i];
	for (i = 10; i > 0; --i) {
		x[4] ^= ROR_OP(x[0] + x[12], 6);  x[8] ^= ROR_OP(x[4] + x[0], 17);
		x[12] += ROR_OP(x[8] | x[4], 16);  x[0] += ROR_OP(x[12] ^ x[8], 5);
		x[9] += ROR_OP(x[5] | x[1], 8);  x[13] += ROR_OP(x[9] | x[5], 7);
		x[1] ^= ROR_OP(x[13] + x[9], 17);  x[5] += ROR_OP(x[1] ^ x[13], 12);
		x[14] ^= ROR_OP(x[10] + x[6], 7);  x[2] += ROR_OP(x[14] ^ x[10], 15);
		x[6] ^= ROR_OP(x[2] + x[14], 13);  x[10] ^= ROR_OP(x[6] + x[2], 15);
		x[3] += ROR_OP(x[15] | x[11], 20);  x[7] ^= ROR_OP(x[3] + x[15], 16);
		x[11] += ROR_OP(x[7] ^ x[3], 7);  x[15] += ROR_OP(x[11] ^ x[7], 8);
		x[1] += ROR_OP(x[0] | x[3], 8) ^ i; x[2] ^= ROR_OP(x[1] + x[0], 14);
		x[3] ^= ROR_OP(x[2] + x[1], 6);  x[0] += ROR_OP(x[3] ^ x[2], 18);
		x[6] += ROR_OP(x[5] ^ x[4], 8);  x[7] += ROR_OP(x[6] ^ x[5], 12);
		x[4] += ROR_OP(x[7] | x[6], 13);  x[5] ^= ROR_OP(x[4] + x[7], 15);
		x[11] ^= ROR_OP(x[10] + x[9], 18);  x[8] += ROR_OP(x[11] ^ x[10], 11);
		x[9] ^= ROR_OP(x[8] + x[11], 8);  x[10] += ROR_OP(x[9] | x[8], 6);
		x[12] += ROR_OP(x[15] ^ x[14], 17);  x[13] ^= ROR_OP(x[12] + x[15], 15);
		x[14] += ROR_OP(x[13] | x[12], 9);  x[15] += ROR_OP(x[14] ^ x[13], 7);
	}
	for (i = 0; i < 16; ++i) x[i] += in[i];
}

