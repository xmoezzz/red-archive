// VideoEncoder.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <Windows.h>
#include <my.h>
#include <stdint.h>
#include <string>

#pragma comment(lib, "MyLibrary_x86_static.lib")

#define ROTL(a,b) (((a) << (b)) | ((a) >> (32 - (b))))
#define QR(a, b, c, d)(		\
	b ^= ROTL(a + d, 7),	\
	c ^= ROTL(b + a, 9),	\
	d ^= ROTL(c + b,13),	\
	a ^= ROTL(d + c,18))
#define ROUNDS 20

__forceinline void salsa20_block(uint32_t out[16], uint32_t const in[16])
{
	int i;
	uint32_t x[16];

	for (i = 0; i < 16; ++i)
		x[i] = in[i];
	// 10 loops × 2 rounds/loop = 20 rounds
	for (int i = 0; i < ROUNDS; i += 2) {
		// Odd round
		QR(x[0], x[4], x[8], x[12]);	// column 1
		QR(x[5], x[9], x[13], x[1]);	// column 2
		QR(x[10], x[14], x[2], x[6]);	// column 3
		QR(x[15], x[3], x[7], x[11]);	// column 4
		// Even round
		QR(x[0], x[1], x[2], x[3]);	// row 1
		QR(x[5], x[6], x[7], x[4]);	// row 2
		QR(x[10], x[11], x[8], x[9]);	// row 3
		QR(x[15], x[12], x[13], x[14]);	// row 4
	}
	for (i = 0; i < 16; ++i)
		out[i] = x[i] + in[i];
}




	typedef struct {
		uint32_t buf[16];
		uint32_t hash[8];
		uint32_t len[2];
	} sha256_context;

#define RL(x,n)   (((x) << n) | ((x) >> (32 - n)))
#define RR(x,n)   (((x) >> n) | ((x) << (32 - n)))

#define S0(x)  (RR((x), 2) ^ RR((x),13) ^ RR((x),22))
#define S1(x)  (RR((x), 6) ^ RR((x),11) ^ RR((x),25))
#define G0(x)  (RR((x), 7) ^ RR((x),18) ^ ((x) >> 3))
#define G1(x)  (RR((x),17) ^ RR((x),19) ^ ((x) >> 10))

#ifdef SWAP_BYTES
#define BSWP(x,y)  _bswapw((uint32_t *)(x), (uint32_t)(y))
#else
#define BSWP(p,n)
#endif
#ifdef USE_STD_MEMCPY
#define MEMCP(x,y,z) memcpy((x),(y),(z))
#else
#define MEMCP(x,y,z) _memcp((x),(y),(z))
#endif

#ifndef __cdecl
#define __cdecl
#endif

	static const uint32_t K[64] = {
		0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
		0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
		0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
		0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
		0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
		0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
		0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
		0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
		0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
		0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
		0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
		0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
		0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
		0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
		0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
		0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
	};

	/* -------------------------------------------------------------------------- */
	static void _bswapw(uint32_t *p, uint32_t i)
	{
		while (i--) p[i] = (RR(p[i], 24) & 0x00ff00ff) | (RR(p[i], 8) & 0xff00ff00);

	} /* _bswapw */

	/* -------------------------------------------------------------------------- */
#ifndef USE_STD_MEMCPY
	void * __cdecl _memcp(void *d, const void *s, uint32_t sz)
	{
		void *rv = d;

		while (sz--) *(char *)d = *(char *)s, d = (char *)d + 1, s = (char *)s + 1;

		return(rv);
	} /* _memcp */
#endif

	/* -------------------------------------------------------------------------- */
	static void _rtrf(uint32_t *b, uint32_t *p, uint32_t i, uint32_t j)
	{
#define B(x, y) b[(x-y) & 7]
#define P(x, y) p[(x+y) & 15]

		B(7, i) += (j ? (p[i & 15] += G1(P(i, 14)) + P(i, 9) + G0(P(i, 1))) : p[i & 15])
			+ K[i + j] + S1(B(4, i))
			+ (B(6, i) ^ (B(4, i) & (B(5, i) ^ B(6, i))));
		B(3, i) += B(7, i);
		B(7, i) += S0(B(0, i)) + ((B(0, i) & B(1, i)) | (B(2, i) & (B(0, i) ^ B(1, i))));

#undef P
#undef B
	} /* _rtrf */

	/* -------------------------------------------------------------------------- */
	static void _hash(sha256_context *ctx)
	{
		uint32_t b[8], *p, j;

		b[0] = ctx->hash[0]; b[1] = ctx->hash[1]; b[2] = ctx->hash[2];
		b[3] = ctx->hash[3]; b[4] = ctx->hash[4]; b[5] = ctx->hash[5];
		b[6] = ctx->hash[6]; b[7] = ctx->hash[7];

		for (p = ctx->buf, j = 0; j < 64; j += 16)
			_rtrf(b, p, 0, j), _rtrf(b, p, 1, j), _rtrf(b, p, 2, j),
			_rtrf(b, p, 3, j), _rtrf(b, p, 4, j), _rtrf(b, p, 5, j),
			_rtrf(b, p, 6, j), _rtrf(b, p, 7, j), _rtrf(b, p, 8, j),
			_rtrf(b, p, 9, j), _rtrf(b, p, 10, j), _rtrf(b, p, 11, j),
			_rtrf(b, p, 12, j), _rtrf(b, p, 13, j), _rtrf(b, p, 14, j),
			_rtrf(b, p, 15, j);

		ctx->hash[0] += b[0]; ctx->hash[1] += b[1]; ctx->hash[2] += b[2];
		ctx->hash[3] += b[3]; ctx->hash[4] += b[4]; ctx->hash[5] += b[5];
		ctx->hash[6] += b[6]; ctx->hash[7] += b[7];

	} /* _hash */

	/* -------------------------------------------------------------------------- */
	__forceinline void sha256_init(sha256_context *ctx)
	{
		ctx->len[0] = ctx->len[1] = 0;
		ctx->hash[0] = 0x6a09e667; ctx->hash[1] = 0xbb67ae85;
		ctx->hash[2] = 0x3c6ef372; ctx->hash[3] = 0xa54ff53a;
		ctx->hash[4] = 0x510e527f; ctx->hash[5] = 0x9b05688c;
		ctx->hash[6] = 0x1f83d9ab; ctx->hash[7] = 0x5be0cd19;

	} /* sha256_init */

	/* -------------------------------------------------------------------------- */
	__forceinline void sha256_hash(sha256_context *ctx, uint8_t *dat, uint32_t sz)
	{
		register uint32_t i = ctx->len[0] & 63, l, j;

		if ((ctx->len[0] += sz) < sz)  ++(ctx->len[1]);

		for (j = 0, l = 64 - i; sz >= l; j += l, sz -= l, l = 64, i = 0)
		{
			MEMCP((char *)ctx->buf + i, &dat[j], l);
			BSWP(ctx->buf, 16);
			_hash(ctx);
		}
		MEMCP((char *)ctx->buf + i, &dat[j], sz);

	} /* _hash */

	/* -------------------------------------------------------------------------- */
	__forceinline void sha256_done(sha256_context *ctx, uint8_t *buf)
	{
		uint32_t i = (uint32_t)(ctx->len[0] & 63), j = ((~i) & 3) << 3;

		BSWP(ctx->buf, (i + 3) >> 2);

		ctx->buf[i >> 2] &= 0xffffff80 << j;  /* add padding */
		ctx->buf[i >> 2] |= 0x00000080 << j;

		if (i < 56) i = (i >> 2) + 1;
		else ctx->buf[15] ^= (i < 60) ? ctx->buf[15] : 0, _hash(ctx), i = 0;

		while (i < 14) ctx->buf[i++] = 0;

		ctx->buf[14] = (ctx->len[1] << 3) | (ctx->len[0] >> 29); /* add length */
		ctx->buf[15] = ctx->len[0] << 3;

		_hash(ctx);

		for (i = 0; i < 32; i++)
			ctx->buf[i % 16] = 0, /* may remove this line in case of a DIY cleanup */
			buf[i] = (uint8_t)(ctx->hash[i >> 2] >> ((~i & 3) << 3));

	} /* sha256_done */





BYTE Key[1024];


void StreamProxy(ULONG Size)
{
	uint32_t       iv[16];
	uint8_t        siv[32];
	sha256_context ctx;
	DWORD          PrimaryKey;
	STATSTG        ProxyStat;

	PrimaryKey = Size;
	//PrimaryKey = 0x41414141;
	for (ULONG i = 0; i < 16; i++)
	{
		sha256_init(&ctx);
		sha256_hash(&ctx, (uint8_t*)&PrimaryKey, sizeof(PrimaryKey));
		sha256_done(&ctx, siv);
		memcpy(iv, siv, sizeof(siv));
		sha256_init(&ctx);
		PrimaryKey *= 0x4141;
		sha256_hash(&ctx, (uint8_t*)&PrimaryKey, sizeof(PrimaryKey));
		sha256_done(&ctx, siv);
		memcpy(&iv[8], siv, sizeof(siv));
		PrimaryKey += 0x8373;
		salsa20_block((uint32_t*)(Key + 64 * i), iv);
	}

	memset(iv, 0, sizeof(iv));
	memset(iv, 0, sizeof(siv));
}


int _tmain(int argc, _TCHAR* argv[])
{
	NTSTATUS   Status;
	NtFileDisk File;
	PBYTE      Buffer;
	ULONG      Size;

	ml::MlInitialize();
	if (argc < 2)
		return 0;

	Status = File.Open(argv[1]);
	if (NT_FAILED(Status))
		return 0;

	Size = File.GetSize32();
	StreamProxy(Size);
	Buffer = new BYTE[Size];
	File.Read(Buffer, Size);
	File.Close();

	for (ULONG i = 0; i < Size; i++)
	{
		Buffer[i] ^= Key[i % sizeof(Key)];
		//Buffer[i] ^= 0x41;
	}

	std::wstring FileName = argv[1];
	FileName += L".out";
	File.Create(FileName.c_str());
	File.Write(Buffer, Size);
	File.Close();

	delete[] Buffer;
	
	return 0;
}

