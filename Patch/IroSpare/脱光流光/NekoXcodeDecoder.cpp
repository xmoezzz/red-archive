#include "NekoXcodeDecoder.h"
#include "StreamDecoder.h"

#define SWAP4(x)	(((x) & 0xff) << 24 | ((x) & 0xff00) << 8 | ((x) & 0xff0000) >> 8 | ((x) & 0xff000000) >> 24)

static void xcode_create(u32 key, u32 *xcode, DWORD xcode_len)
{
	unsigned int eax = 0;
	unsigned int ebx = 0;

	do {
		eax <<= 1;
		ebx ^= 1;
		eax = ((eax | ebx) << (key & 1)) | ebx;
		key >>= 1;
	} while (!(eax & 0x80000000));
	key = eax << 1;
	eax = key + SWAP4(key);
	BYTE cl = (BYTE)key;
	do {
		ebx = key ^ eax;
		eax = (ebx << 4) ^ (ebx >> 4) ^ (ebx << 3) ^ (ebx >> 3) ^ ebx;
		--cl;
	} while (cl);

	for (DWORD i = 0; i < 616 / 4; i++) {
		ebx = key ^ eax;
		eax = (ebx << 4) ^ (ebx >> 4) ^ (ebx << 3) ^ (ebx >> 3) ^ ebx;
		xcode[i] = eax;
	}
}


static void xcode_init(u32 key, BYTE *xcode, BYTE flag)
{
	int v4;
	BYTE byte_532310[14] = { 0xEF, 0xFC, 0xFD, 0xFE, 0xF8, 0xF9, 0xFA, 0xEF, 0xF8, 0xF9, 0xFA, 0xFC, 0xFD, 0xFE };
	BYTE byte_532324[6] = { 0xCA, 0xD3, 0xDC, 0xE5, 0xEE, 0xF1 };

	xcode[0] = 0x01;
	xcode[1] = 0xF1;
	xcode[2] = 0x0F;
	xcode[3] = 0x6F;
	xcode[4] = 0x06;
	xcode += 5;
	int v15 = key & 0xffff;
	int v16 = (key >> 16) & 0xfff;
	if (flag)
		v4 = 0;
	else
		v4 = 7;
	unsigned int v17 = v4 + (key >> 28);
	for (DWORD i = 0; i < 4; i++) {
		BYTE v6;
		int v11, v12;

		if (flag)
			v6 = (BYTE)i;
		else
			v6 = (BYTE)(3 - i);
		v11 = v15 >> (4 * v6);
		v12 = v16 >> (3 * v6);
		xcode[0] = 15;
		xcode[1] = byte_532310[(v11 + v17) % 0xE];
		xcode[2] = v12 % 6 - 63;
		xcode += 3;
	}
	xcode[0] = 0x0F;
	xcode[1] = 0x7F;
	xcode[2] = 0x07;
	xcode[3] = 0x83;
	xcode[4] = 0xC6;
	xcode[5] = 0x08;
	xcode[6] = 0x83;
	xcode[7] = 0xC7;
	xcode[8] = 0x08;
	xcode += 9;
	for (DWORD i = 0; i < 6; i++) {
		xcode[0] = 15;
		xcode[1] = -44;
		xcode[2] = byte_532324[(i + key) % 6];
		xcode += 3;
	}
	xcode[0] = 0x39;
	xcode[1] = 0xCE;
	xcode[2] = 0x72;
	xcode[3] = 0xD2;
	xcode[4] = 0xC3;
}



static void xcode_execute(u32 key, BYTE *xcode_base, BYTE *dst, BYTE *src, SIZE_T len)
{
	u64 mm_reg[6];
	void(*xcode_start)(void) = (void(*)(void))(xcode_base + xcode_base[0] + 320);

	for (DWORD i = 0; i < 6; i++) {
		mm_reg[i] = *(u64 *)&xcode_base[(key % 0x28) * 8];
		key /= 0x28;
	}

	__asm {
		movq    mm1, qword ptr mm_reg[0]
			movq    mm2, qword ptr mm_reg[8]
			movq    mm3, qword ptr mm_reg[16]
			movq    mm4, qword ptr mm_reg[24]
			movq    mm5, qword ptr mm_reg[32]
			movq    mm6, qword ptr mm_reg[40]
			mov		esi, src
			mov		edi, dst
			mov		ecx, len
	}

	/*
	debug045:01AD01B5 add     ecx, esi
	debug045:01AD01B7 loc_1AD01B7:
	debug045:01AD01B7 movq    mm0, qword ptr [esi]
	debug045:01AD01BA psubd   mm0, mm1
	debug045:01AD01BD pxor    mm0, mm4
	debug045:01AD01C0 paddw   mm0, mm1
	debug045:01AD01C3 paddd   mm0, mm2
	debug045:01AD01C6 movq    qword ptr [edi], mm0
	debug045:01AD01C9 add     esi, 8
	debug045:01AD01CC add     edi, 8
	debug045:01AD01CF paddq   mm6, mm1
	debug045:01AD01D2 paddq   mm1, mm2
	debug045:01AD01D5 paddq   mm2, mm3
	debug045:01AD01D8 paddq   mm3, mm4
	debug045:01AD01DB paddq   mm4, mm5
	debug045:01AD01DE paddq   mm5, mm6
	debug045:01AD01E1 cmp     esi, ecx
	debug045:01AD01E3 jb      short loc_1AD01B7
	debug045:01AD01E5 retn
	*/
	(*xcode_start)();

	__asm { emms }
}

static BYTE *xcode = NULL;

static void decrypt_release(void)
{
	if (xcode) {
		VirtualFree(xcode, 0, MEM_RELEASE);
		xcode = NULL;
	}
}

static int decrypt_init(u32 init_key)
{
	DWORD old_protection;

	xcode = (BYTE *)VirtualAlloc(NULL, 616, MEM_COMMIT, PAGE_READWRITE);
	if (!xcode)
		Ps::ExitProcess(0);

	xcode_create(init_key, (u32 *)xcode, 616);
	xcode_init(init_key, xcode + xcode[0] + 320, 0);
	if (!VirtualProtect(xcode, 616, PAGE_EXECUTE_READ, &old_protection)) {
		decrypt_release();
		Ps::ExitProcess(0);
	}

	return 0;
}

void decrypt(u32 exec_key, BYTE *dec, BYTE *enc, DWORD len)
{
	if (xcode)
		xcode_execute(exec_key, xcode, dec, enc, len);
	else
		memcpy(dec, enc, len);
}


Void NTAPI NekoXcodeDecoder(PBYTE Buffer, ULONG Length, DWORD Key)
{
	decrypt_init(Key * 0x5128475);

	PBYTE OutputMem = (PBYTE)AllocateMemoryP(Length);

	decrypt(Key, OutputMem, Buffer, Length);
	decrypt_release();

	RtlCopyMemory(Buffer, OutputMem, Length);
	RtlFillMemory(OutputMem, Length, 0xCC);
	FreeMemoryP(OutputMem);
}

