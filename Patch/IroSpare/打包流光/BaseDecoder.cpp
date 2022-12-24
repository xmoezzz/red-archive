#include "StreamDecoder.h"
#include "GlobalMap.h"
#include "my.h"
#include "NodeKeyDef.h"


Void NTAPI BaseDecoder1(PBYTE buf, DWORD len, DWORD key)
{
	DWORD shifts;
	DWORD decode_key[16];
	int   align;
	DWORD i, k;
	PBYTE DecoderInfo;

	DecoderInfo = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(BaseDecoder1_Mask);
	if (!DecoderInfo)
		Ps::ExitProcess(0);

	for (DWORD i = 0; i < 16; ++i)
	{
		decode_key[i] = *(PDWORD)&DecoderInfo[i * 4] + key;
	}

	shifts = key;
	for (i = 0; i < 7; i++)
		shifts = (shifts >> 4) ^ key;

	shifts ^= 0xFFFFFFFD;
	shifts &= 0x0f;
	shifts += 8;

	i = 3;
	for (k = 0; k < len / 4; k++) 
	{
		buf[k] = _lrotr((decode_key[i++] ^ buf[k]) + 0x6E58A5C2, shifts);
		i &= 0x0f;
	}

	align = len & 3;
	for (int n = align; n > 0; n--)
	{
		*((BYTE *)&buf[k] + align - n) = (*((BYTE *)&buf[k] + align - n) ^ (BYTE)(decode_key[i++] >> (n * 4))) + 0x52;
		i &= 0x0f;
	}
	RtlZeroMemory(decode_key, sizeof(decode_key));
}



Void NTAPI BaseDecoder2(PBYTE buf, DWORD len, DWORD offset, DWORD hash)
{
	BYTE shift, xor;

	shift = (BYTE)hash;
	xor = (BYTE)((hash >> 8) & 0xff);

	if (!shift)
		shift = 15;
	if (!xor)
		xor = 0xf0;

	shift &= 7;
	for (DWORD i = offset; i < offset + len; i++)
	{
		buf[i] ^= xor;
		buf[i] = (buf[i] << shift) | (buf[i] >> (8 - shift));
	}
}


Bool DecryptWorker(LPVoid lpBuffer, Int32 BufferSize, UInt32 Key, Bool bDecrypt)
{
	Key = (Key + BufferSize) ^ 0xFEC9753E;
	BufferSize /= 8;
	if (BufferSize <= 0)
		return False;

	PUInt64 pBuffer;
	Large_Integer Seed1, Seed2, Key64;

	pBuffer = (PUInt64)lpBuffer;
	Seed1.LowPart = 0xCE24F523;
	Seed1.HighPart = Seed1.LowPart;     // mm6
	Seed2.LowPart = 0xA73C5F9D;
	Seed2.HighPart = Seed2.LowPart;     // mm7
	Key64.LowPart = Key;
	Key64.HighPart = Key64.LowPart;     // mm5

	if (bDecrypt)
	{
		while (BufferSize--)
		{
			Seed2.LowPart += Seed1.LowPart;
			Seed2.HighPart += Seed1.HighPart;
			Seed2.QuadPart ^= Key64.QuadPart;
			Key64.QuadPart = *pBuffer ^ Seed2.QuadPart;
			*pBuffer++ = Key64.QuadPart;
		}
	}
	else
	{
		while (BufferSize--)
		{
			Seed2.LowPart += Seed1.LowPart;
			Seed2.HighPart += Seed1.HighPart;
			Seed2.QuadPart ^= Key64.QuadPart;
			Key64.QuadPart = *pBuffer;
			*pBuffer++ ^= Seed2.QuadPart;
		}
	}

	return True;

	__asm
	{
		mov       eax, 0CE24F523h;      // seed1
		movd      mm6, eax;
		punpckldq mm6, mm6;
		mov       eax, 0A73C5F9Dh;      // seed2
		movd      mm7, eax;
		punpckldq mm7, mm7;
		mov       eax, Key;             // key
		movd      mm5, eax;
		punpckldq mm5, mm5;
		mov       ecx, bDecrypt;
		mov       edx, BufferSize;
		mov       eax, lpBuffer;
	DECRYPT_LOOP:
		paddd     mm7, mm6;
		pxor      mm7, mm5;
		movq      mm0, qword ptr[eax];
		jecxz     ENCRYPT;
		pxor      mm0, mm7;
		movq      mm5, mm0;
		jmp       MODE_END;
	ENCRYPT:
		movq      mm5, mm0;
		pxor      mm0, mm7;
	MODE_END:
		movq      qword ptr[eax], mm0;
		add       eax, 8;
		dec       edx;
		jnz       DECRYPT_LOOP;
		emms
	}

	return True;
}


Bool Encrypt(LPVoid lpBuffer, Int32 BufferSize, UInt32 Key)
{
	return DecryptWorker(lpBuffer, BufferSize, Key, False);
}

Bool Decrypt(LPVoid lpBuffer, Int32 BufferSize, UInt32 Key)
{
	return DecryptWorker(lpBuffer, BufferSize, Key, True);
}

Void NTAPI BaseDecoder3(PBYTE buf, DWORD len, DWORD key)
{
	Decrypt(buf, len, key);
}


Void NTAPI BaseDecoder3_Encode(PBYTE buf, DWORD len, DWORD key)
{
	Encrypt(buf, len, key);
}


