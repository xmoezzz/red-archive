/**
* @file cryptmt.cpp
*
* @brief CryptMT ver. 3.0 Stream Cypher.
*
* oriented Fast Mersenne Twister(SFMT) pseudorandom
* number generator with jump function. This file includes common functions
* used in random number generation and jump.
*
* @author Makoto Matsumoto (Hiroshima University)
* @author Mutsuo Saito (Hiroshima University)
*
* Copyright (C) 2006 -- 2013 Mutsuo Saito, Makoto Matsumoto
* and Hiroshima University.
* All rights reserved.
*
* This software is patented.
* Free use for noncommercial or academic research.
* see LICENSE.txt
*/
#include <memory.h>
#include <cstdlib>
#include <stdint.h>
#include <iostream>
#include <iomanip>
#include <cerrno>
#include "cryptmt.h"

#define HAVE_SSE2

#if defined(HAVE_AVX)
#include <immintrin.h>
#define AVX_ALIGN 1
#define SIMD_HEADER
#if !defined(HAVE_SSE3)
#define HAVE_SSE3
#endif
#endif

#if defined(HAVE_SSE4)
#if !defined(SIMD_HEADER)
#include <smmintrin.h>
#define SIMD_HEADER
#endif
#if !defined(HAVE_SSE3)
#define HAVE_SSE3
#endif
#endif

#if defined(HAVE_SSE3)
#if !defined(SIMD_HEADER)
#include <pmmintrin.h>
#define SIMD_HEADER
#endif
#if !defined(HAVE_SSE2)
#define HAVE_SSE2
#endif
#endif

#if defined(HAVE_SSE2)
#if !defined(SIMD_HEADER)
#include <emmintrin.h>
#define SIMD_HEADER
#endif
#endif

#if defined(__APPLE__) ||                                               \
    (defined(__FreeBSD__) && __FreeBSD__ >= 3 && __FreeBSD__ <= 6)
#define MALLOC_OK 1
#elif defined(_POSIX_C_SOURCE)
#define POSIX_MEMALIGN 1
#elif defined(__GNUC__) && (__GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ >= 3))
#define MEMALIGN 1
#elif  defined(_MSC_VER) || defined(__BORLANDC__)
#define WIN_ALIGNED_MALLOC
#include <malloc.h>
#endif

#if defined(__GNUC__)
#define GCC_ALIGN __attribute__((aligned(16)))
#else
#define GCC_ALIGN
#endif
#if defined(__MSC_VER__)
#define MSC_ALIGN __declspec(align(16))
#else
#define MSC_ALIGN
#endif


#define POS1 108
#define SHIFT64 3
#define SR1 1
#define MSK1 UINT32_C(0xffdfafdf)
#define MSK2 UINT32_C(0xf5dabfff)
#define MSK3 UINT32_C(0xffdbffff)
#define MSK4 UINT32_C(0xef7bffff)
#define INIL UINT32_C(0x4d734e48)
//#define N 156
#define ARRAY_SIZE 156

#define RIGHT_ROT 0x39
#define LEFT_ROT 0x93
#define SHUFF2 0x8d
#define BOOT_ROT1 0x93
#define BOOT_ROT2 0x4b
#define BOOT_SL1 13
#define BOOT_SL2 11

/**
*@namespace cryptmt
*
*/
namespace cryptmt {
	using namespace std;
	static const int MAXKEYSIZE = 2048;
	static const int MAXIVSIZE = 2048;
	static const uint32_t BLOCKLENGTH = 624 * 2;
	/* =======================================
	* SFMT
	* ======================================= */
#if defined(HAVE_SSE2)
	typedef union {
		__m128i si;
		uint32_t ar[4];
	} simd_t;
#else // defined(HAVE_SSE2)
	typedef struct {
		uint32_t ar[4];
	} simd_t;
#endif //defined(HAVE_SSE2)
	struct SFMT_T {
		simd_t sfmt[ARRAY_SIZE];
	};
	typedef struct SFMT_T sfmt_t;

#if defined(HAVE_SSE2)
	static inline __m128i
		simd_recursion(const __m128i *x,
		const __m128i *y,
		__m128i z,
		const __m128i mask)
	{
		__m128i a, b, c, d GCC_ALIGN;

		a = _mm_load_si128(x);
		a = _mm_shuffle_epi32(a, RIGHT_ROT);
		b = _mm_load_si128(y);
		d = _mm_shuffle_epi32(b, SHUFF2);
		a = _mm_xor_si128(a, d);
		b = _mm_srli_epi64(b, SHIFT64);
		c = _mm_and_si128(z, mask);
		c = _mm_xor_si128(c, b);
		c = _mm_xor_si128(c, a);
		return c;
	}

	/* this should work when o == a */
	static inline void
		do_recursion(uint32_t oo[4],
		const uint32_t xx[4],
		const uint32_t yy[4],
		const uint32_t zz[4])
	{
		const __m128i mask GCC_ALIGN = _mm_set_epi32(MSK4, MSK3, MSK2, MSK1);
		__m128i *o = reinterpret_cast<__m128i *>(oo);
		const __m128i *x = reinterpret_cast<const __m128i *>(xx);
		const __m128i *y = reinterpret_cast<const __m128i *>(yy);
		const __m128i z GCC_ALIGN
			= _mm_load_si128(reinterpret_cast<const __m128i *>(zz));
		__m128i a, b, c, d;

		a = _mm_load_si128(x);
		a = _mm_shuffle_epi32(a, RIGHT_ROT);
		b = _mm_load_si128(y);
		d = _mm_shuffle_epi32(b, SHUFF2);
		a = _mm_xor_si128(a, d);
		b = _mm_srli_epi64(b, SHIFT64);
		c = _mm_and_si128(z, mask);
		c = _mm_xor_si128(c, b);
		c = _mm_xor_si128(c, a);
		_mm_store_si128(o, c);
	}
#else // defined(HAVE_SSE2)
	/* this should work when o == a */
	static inline void
		do_recursion(uint32_t o[4],
		const uint32_t a[4],
		const uint32_t b[4],
		const uint32_t c[4])
	{
		uint64_t t;
		uint32_t bb[4];
		uint32_t tmp;

		t = ((uint64_t)b[1] << 32) | ((uint64_t)b[0]);
		t = t >> SHIFT64;
		bb[0] = (uint32_t)t;
		bb[1] = (uint32_t)(t >> 32);
		t = ((uint64_t)b[3] << 32) | ((uint64_t)b[2]);
		t = t >> SHIFT64;
		bb[2] = (uint32_t)t;
		bb[3] = (uint32_t)(t >> 32);
		tmp = a[0];
		o[0] = a[1] ^ b[1] ^ bb[0] ^ (c[0] & MSK1);
		o[1] = a[2] ^ b[3] ^ bb[1] ^ (c[1] & MSK2);
		o[2] = a[3] ^ b[0] ^ bb[2] ^ (c[2] & MSK3);
		o[3] = tmp  ^ b[2] ^ bb[3] ^ (c[3] & MSK4);
	}
#endif // defined(HAVE_SSE2)
#if defined(HAVE_SSE2)
	static void
		sfmt_genrand_block(simd_t *psfmt)
	{
		int i;
		__m128i *sfmt;
		__m128i c;
		const __m128i mask GCC_ALIGN = _mm_set_epi32(MSK4, MSK3, MSK2, MSK1);

		sfmt = &psfmt->si;
		c = _mm_load_si128(&sfmt[ARRAY_SIZE - 1]);
		c = simd_recursion(&sfmt[0],
			&sfmt[POS1],
			c,
			mask);
		_mm_store_si128(&sfmt[0], c);
		for (i = 1; i < ARRAY_SIZE - POS1; i++) {
			c = simd_recursion(&sfmt[i],
				&sfmt[i + POS1],
				c,
				mask);
			_mm_store_si128(&sfmt[i], c);
		}
		for (; i < ARRAY_SIZE; i++) {
			c = simd_recursion(&sfmt[i],
				&sfmt[i + POS1 - ARRAY_SIZE],
				c,
				mask);
			_mm_store_si128(&sfmt[i], c);
		}
	}
#else // defined(HAVE_SSE2)
	static void
		sfmt_genrand_block(simd_t *psfmt)
	{
		int i;
		sfmt_t *ps;
		int offset = 0;

		ps = reinterpret_cast<sfmt_t *>(psfmt);
		do_recursion(ps->sfmt[0].ar,
			ps->sfmt[0].ar,
			ps->sfmt[POS1].ar,
			ps->sfmt[ARRAY_SIZE - 1].ar);
		for (i = 1; i < ARRAY_SIZE - POS1; i++) {
			do_recursion(ps->sfmt[i].ar,
				ps->sfmt[i].ar,
				ps->sfmt[i + POS1].ar,
				ps->sfmt[i - 1].ar);
		}
		for (; i < ARRAY_SIZE; i++) {
			do_recursion(ps->sfmt[i].ar,
				ps->sfmt[i].ar,
				ps->sfmt[i + POS1 - ARRAY_SIZE].ar,
				ps->sfmt[i - 1].ar);
		}
	}

#endif // defined(HAVE_SSE2)
	/* =======================================
	* BOOTER
	* ======================================= */
#if defined(HAVE_SSE2)
	static void
		booter_am(simd_t *p_acc,
		simd_t *p1,
		simd_t *p2,
		int count)
	{
		__m128i *pos1 = &p1->si;
		__m128i *pos2 = &p2->si;
		__m128i a, b, acc, x, y, z;
		int i;
		__m128i mask32 GCC_ALIGN;

		mask32 = _mm_set_epi32(0, UINT32_C(0xffffffff),
			0, UINT32_C(0xffffffff));
		acc = _mm_load_si128(&p_acc->si);
		y = _mm_load_si128(&pos2[0]);
		for (i = 0; i < count; i++) {
			x = _mm_load_si128(&pos1[i]);
			z = _mm_load_si128(&pos2[i + 1]);
			a = _mm_add_epi32(x, y);
			_mm_store_si128(&pos1[i], a);
			a = _mm_xor_si128(_mm_shuffle_epi32(a, BOOT_ROT1),
				_mm_srli_epi32(a, BOOT_SL1));
			b = _mm_xor_si128(_mm_shuffle_epi32(z, BOOT_ROT2),
				_mm_srli_epi32(z, BOOT_SL2));
			y = z;
			/****** _mm_multiply start ******/
			x = acc;
			x = _mm_mul_epu32(x, b);
			x = _mm_and_si128(x, mask32);
			x = _mm_slli_epi32(x, 1);
			x = _mm_add_epi32(x, acc);
			x = _mm_add_epi32(x, b);
			acc = _mm_shuffle_epi32(acc, RIGHT_ROT);
			b = _mm_shuffle_epi32(b, RIGHT_ROT);
			acc = _mm_mul_epu32(acc, b);
			acc = _mm_slli_epi64(acc, 33);
			acc = _mm_add_epi32(acc, x);
			/****** _mm_multiply end ******/
			a = _mm_sub_epi32(a, acc);
			_mm_store_si128(&pos2[i + 2], a);
#if defined(DEBUGX) && DEBUGX >= 2
			cerr << "booter am:" << hex << &pos2[i + 2] << endl;
#endif
		}
		_mm_store_si128(&p_acc->si, acc);
	}
#else // defined(HAVE_SSE2)
	static void
		booter_am(simd_t *acc,
		simd_t pos1[],
		simd_t pos2[],
		int count)
	{
		uint32_t a[4];
		uint32_t b[4];
		uint32_t tmp;

		for (int i = 0; i < count; i++) {
			for (int j = 0; j < 4; j++) {
				pos1[i].ar[j] = a[j] = pos1[i].ar[j] + pos2[i].ar[j];
			}
			tmp = a[0];
			a[0] = a[3] ^ (a[0] >> 13);
			a[3] = a[2] ^ (a[3] >> 13);
			a[2] = a[1] ^ (a[2] >> 13);
			a[1] = tmp ^ (a[1] >> 13);
			b[0] = pos2[i + 1].ar[3] ^ (pos2[i + 1].ar[0] >> 11);
			b[1] = pos2[i + 1].ar[2] ^ (pos2[i + 1].ar[1] >> 11);
			b[2] = pos2[i + 1].ar[0] ^ (pos2[i + 1].ar[2] >> 11);
			b[3] = pos2[i + 1].ar[1] ^ (pos2[i + 1].ar[3] >> 11);
			for (int j = 0; j < 4; j++) {
				acc->ar[j] = (2 * b[j] + 1) * acc->ar[j] + b[j];
				pos2[i + 2].ar[j] = a[j] - acc->ar[j];
			}
		}
	}
#endif // defined(HAVE_SSE2)
	/* =======================================
	* FILTER
	* ======================================= */
#if defined(HAVE_SSE2)
	static void
		filter_16bytes(simd_t *p_sfmt,
		simd_t *p_acc,
		uint8_t cipher[],
		const uint8_t plain[],
		int count)
	{
#if defined(DEBUG)
		cout << "filter 16bytes start" << endl;
		cout << "p_sfmt:" << hex << p_sfmt << endl;
		cout << "p_acc:" << hex << p_acc << endl;
		cout << "cipher:" << hex << (int *)cipher << endl;
		cout << "plain:" << hex << (int *)plain << endl;
		cout << "count:" << dec << count << endl;
#endif // defined(DEBUG)
		__m128i *sfmt = (__m128i *)p_sfmt;
		__m128i *p_accum = (__m128i *)p_acc;
		int i;
		__m128i acc, x, y, out;
		__m128i mask16, mask32 GCC_ALIGN;

		mask16 = _mm_set_epi32(UINT32_C(0x0000ffff), UINT32_C(0x0000ffff),
			UINT32_C(0x0000ffff), UINT32_C(0x0000ffff));
		mask32 = _mm_set_epi32(0, UINT32_C(0xffffffff),
			0, UINT32_C(0xffffffff));
		acc = _mm_load_si128(p_accum);
		for (i = 0; i < count; i++) {
			y = _mm_load_si128(&sfmt[i * 2]);
			x = acc;
			x = _mm_shuffle_epi32(x, RIGHT_ROT);
			x = _mm_srli_epi32(x, 1);
			acc = _mm_xor_si128(acc, x);
			/****** _mm_multiply start ******/
			x = acc;
			x = _mm_mul_epu32(x, y);
			x = _mm_and_si128(x, mask32);
			x = _mm_slli_epi32(x, 1);
			x = _mm_add_epi32(x, acc);
			x = _mm_add_epi32(x, y);
			acc = _mm_shuffle_epi32(acc, RIGHT_ROT);
			y = _mm_shuffle_epi32(y, RIGHT_ROT);
			acc = _mm_mul_epu32(acc, y);
			acc = _mm_slli_epi64(acc, 33);
			acc = _mm_add_epi32(acc, x);
			/****** _mm_multiply end ******/
			y = acc;
			out = _mm_srli_epi32(acc, 16);
			out = _mm_xor_si128(out, y);
			out = _mm_and_si128(out, mask16);
			y = _mm_load_si128(&sfmt[i * 2 + 1]);
			x = acc;
			x = _mm_shuffle_epi32(x, RIGHT_ROT);
			x = _mm_srli_epi32(x, 1);
			acc = _mm_xor_si128(acc, x);
			/****** _mm_multiply start ******/
			x = acc;
			x = _mm_mul_epu32(x, y);
			x = _mm_and_si128(x, mask32);
			x = _mm_slli_epi32(x, 1);
			x = _mm_add_epi32(x, acc);
			x = _mm_add_epi32(x, y);
			acc = _mm_shuffle_epi32(acc, RIGHT_ROT);
			y = _mm_shuffle_epi32(y, RIGHT_ROT);
			acc = _mm_mul_epu32(acc, y);
			acc = _mm_slli_epi64(acc, 33);
			acc = _mm_add_epi32(acc, x);
			/****** _mm_multiply end ******/

			x = _mm_slli_epi32(acc, 16);
			x = _mm_xor_si128(x, acc);
			x = _mm_andnot_si128(mask16, x);
			out = _mm_or_si128(out, x);
			/* output */
#if defined(SEPARATE_XOR)
			_mm_store_si128((__m128i *)&cipher[i * 16], out);
#else
#if defined(USER_DATA_ALIGNED)
			x = _mm_load_si128((__m128i *)&plain[i * 16]);
			x = _mm_xor_si128(x, out);
			_mm_store_si128((__m128i *)&cipher[i * 16], x);
#else
#if defined(HAVE_SSE3)
			x = _mm_lddqu_si128((__m128i *)&plain[i * 16]);
#else
			x = _mm_loadu_si128((__m128i *)&plain[i * 16]);
#endif
			x = _mm_xor_si128(x, out);
			_mm_storeu_si128((__m128i *)&cipher[i * 16], x);
#endif
#endif
		}
		_mm_store_si128(p_accum, acc);
#if defined(DEBUG)
		cout << "filter 16bytes end" << endl;
#endif
	}
#else // defined(HAVE_SSE2)

	static void
		filter_16bytes(simd_t *sfmt,
		simd_t *accum,
		uint8_t cipher[],
		const uint8_t plain[],
		int count)
	{
#if defined(DEBUG)
		cout << "filter 16bytes start" << endl;
#endif
		uint32_t t1, t2, t3, t4;
		uint32_t ac1, ac2, ac3, ac4;
		int i;

		ac1 = accum->ar[0];
		ac2 = accum->ar[1];
		ac3 = accum->ar[2];
		ac4 = accum->ar[3];

		for (i = 0; i < count; i++) {
			t1 = ac1;
			ac1 = ac1 ^ (ac2 >> 1);
			ac2 = ac2 ^ (ac3 >> 1);
			ac3 = ac3 ^ (ac4 >> 1);
			ac4 = ac4 ^ (t1 >> 1);
			ac1 = (2 * ac1 + 1) * sfmt[0].ar[0] + ac1;
			ac2 = (2 * ac2 + 1) * sfmt[0].ar[1] + ac2;
			ac3 = (2 * ac3 + 1) * sfmt[0].ar[2] + ac3;
			ac4 = (2 * ac4 + 1) * sfmt[0].ar[3] + ac4;
			t1 = (ac1 >> 16) ^ ac1;
			t2 = (ac2 >> 16) ^ ac2;
			t3 = (ac3 >> 16) ^ ac3;
			t4 = (ac4 >> 16) ^ ac4;
#if defined(SEPARATE_XOR)
			cipher[0] = (uint8_t)(t1);
			cipher[1] = (uint8_t)(t1 >> 8);
			cipher[4] = (uint8_t)(t2);
			cipher[5] = (uint8_t)(t2 >> 8);
			cipher[8] = (uint8_t)(t3);
			cipher[9] = (uint8_t)(t3 >> 8);
			cipher[12] = (uint8_t)(t4);
			cipher[13] = (uint8_t)(t4 >> 8);
#else
			cipher[0] = (uint8_t)(plain[0] ^ (uint8_t)(t1));
			cipher[1] = (uint8_t)(plain[1] ^ (uint8_t)(t1 >> 8));
			cipher[4] = (uint8_t)(plain[4] ^ (uint8_t)(t2));
			cipher[5] = (uint8_t)(plain[5] ^ (uint8_t)(t2 >> 8));
			cipher[8] = (uint8_t)(plain[8] ^ (uint8_t)(t3));
			cipher[9] = (uint8_t)(plain[9] ^ (uint8_t)(t3 >> 8));
			cipher[12] = (uint8_t)(plain[12] ^ (uint8_t)(t4));
			cipher[13] = (uint8_t)(plain[13] ^ (uint8_t)(t4 >> 8));
#endif
			t1 = ac1;
			ac1 = ac1 ^ (ac2 >> 1);
			ac2 = ac2 ^ (ac3 >> 1);
			ac3 = ac3 ^ (ac4 >> 1);
			ac4 = ac4 ^ (t1 >> 1);
			ac1 = (2 * ac1 + 1) * sfmt[1].ar[0] + ac1;
			ac2 = (2 * ac2 + 1) * sfmt[1].ar[1] + ac2;
			ac3 = (2 * ac3 + 1) * sfmt[1].ar[2] + ac3;
			ac4 = (2 * ac4 + 1) * sfmt[1].ar[3] + ac4;
			t1 = (ac1 >> 16) ^ ac1;
			t2 = (ac2 >> 16) ^ ac2;
			t3 = (ac3 >> 16) ^ ac3;
			t4 = (ac4 >> 16) ^ ac4;
#if defined(SEPARATE_XOR)
			cipher[2] = (uint8_t)(t1);
			cipher[3] = (uint8_t)(t1 >> 8);
			cipher[6] = (uint8_t)(t2);
			cipher[7] = (uint8_t)(t2 >> 8);
			cipher[10] = (uint8_t)(t3);
			cipher[11] = (uint8_t)(t3 >> 8);
			cipher[14] = (uint8_t)(t4);
			cipher[15] = (uint8_t)(t4 >> 8);
#else
			cipher[2] = (uint8_t)(plain[2] ^ (uint8_t)(t1));
			cipher[3] = (uint8_t)(plain[3] ^ (uint8_t)(t1 >> 8));
			cipher[6] = (uint8_t)(plain[6] ^ (uint8_t)(t2));
			cipher[7] = (uint8_t)(plain[7] ^ (uint8_t)(t2 >> 8));
			cipher[10] = (uint8_t)(plain[10] ^ (uint8_t)(t3));
			cipher[11] = (uint8_t)(plain[11] ^ (uint8_t)(t3 >> 8));
			cipher[14] = (uint8_t)(plain[14] ^ (uint8_t)(t4));
			cipher[15] = (uint8_t)(plain[15] ^ (uint8_t)(t4 >> 8));
#endif
			sfmt += 2;
			cipher += 16;
			plain += 16;
		}
		accum->ar[0] = ac1;
		accum->ar[1] = ac2;
		accum->ar[2] = ac3;
		accum->ar[3] = ac4;
#if defined(DEBUG)
		cout << "filter 16bytes end" << endl;
#endif
	}
#endif // defined(HAVE_SSE2)
	static void
		filter_bytes(simd_t *sfmt,
		simd_t *accum,
		uint8_t cipher[],
		const uint8_t plain[],
		int len)
	{
#if defined(DEBUG)
		cout << "filter bytes start" << endl;
#endif
		uint32_t t1, t2, t3, t4, t5, t6, t7, t8;

		t1 = accum->ar[0];
		accum->ar[0] = accum->ar[0] ^ (accum->ar[1] >> 1);
		accum->ar[1] = accum->ar[1] ^ (accum->ar[2] >> 1);
		accum->ar[2] = accum->ar[2] ^ (accum->ar[3] >> 1);
		accum->ar[3] = accum->ar[3] ^ (t1 >> 1);
		accum->ar[0] = (2 * accum->ar[0] + 1) * sfmt[0].ar[0] + accum->ar[0];
		accum->ar[1] = (2 * accum->ar[1] + 1) * sfmt[0].ar[1] + accum->ar[1];
		accum->ar[2] = (2 * accum->ar[2] + 1) * sfmt[0].ar[2] + accum->ar[2];
		accum->ar[3] = (2 * accum->ar[3] + 1) * sfmt[0].ar[3] + accum->ar[3];
		t1 = (accum->ar[0] >> 16) ^ accum->ar[0];
		t2 = (accum->ar[1] >> 16) ^ accum->ar[1];
		t3 = (accum->ar[2] >> 16) ^ accum->ar[2];
		t4 = (accum->ar[3] >> 16) ^ accum->ar[3];

		t5 = accum->ar[0];
		accum->ar[0] = accum->ar[0] ^ (accum->ar[1] >> 1);
		accum->ar[1] = accum->ar[1] ^ (accum->ar[2] >> 1);
		accum->ar[2] = accum->ar[2] ^ (accum->ar[3] >> 1);
		accum->ar[3] = accum->ar[3] ^ (t5 >> 1);
		accum->ar[0] = (2 * accum->ar[0] + 1) * sfmt[1].ar[0] + accum->ar[0];
		accum->ar[1] = (2 * accum->ar[1] + 1) * sfmt[1].ar[1] + accum->ar[1];
		accum->ar[2] = (2 * accum->ar[2] + 1) * sfmt[1].ar[2] + accum->ar[2];
		accum->ar[3] = (2 * accum->ar[3] + 1) * sfmt[1].ar[3] + accum->ar[3];
		t5 = (accum->ar[0] >> 16) ^ accum->ar[0];
		t6 = (accum->ar[1] >> 16) ^ accum->ar[1];
		t7 = (accum->ar[2] >> 16) ^ accum->ar[2];
		t8 = (accum->ar[3] >> 16) ^ accum->ar[3];

		cipher[0] = (uint8_t)(plain[0] ^ (uint8_t)(t1));
		if (--len == 0) return;
		cipher[1] = (uint8_t)(plain[1] ^ (uint8_t)(t1 >> 8));
		if (--len == 0) return;
		cipher[2] = (uint8_t)(plain[2] ^ (uint8_t)(t5));
		if (--len == 0) return;
		cipher[3] = (uint8_t)(plain[3] ^ (uint8_t)(t5 >> 8));
		if (--len == 0) return;
		cipher[4] = (uint8_t)(plain[4] ^ (uint8_t)(t2));
		if (--len == 0) return;
		cipher[5] = (uint8_t)(plain[5] ^ (uint8_t)(t2 >> 8));
		if (--len == 0) return;
		cipher[6] = (uint8_t)(plain[6] ^ (uint8_t)(t6));
		if (--len == 0) return;
		cipher[7] = (uint8_t)(plain[7] ^ (uint8_t)(t6 >> 8));
		if (--len == 0) return;
		cipher[8] = (uint8_t)(plain[8] ^ (uint8_t)(t3));
		if (--len == 0) return;
		cipher[9] = (uint8_t)(plain[9] ^ (uint8_t)(t3 >> 8));
		if (--len == 0) return;
		cipher[10] = (uint8_t)(plain[10] ^ (uint8_t)(t7));
		if (--len == 0) return;
		cipher[11] = (uint8_t)(plain[11] ^ (uint8_t)(t7 >> 8));
		if (--len == 0) return;
		cipher[12] = (uint8_t)(plain[12] ^ (uint8_t)(t4));
		if (--len == 0) return;
		cipher[13] = (uint8_t)(plain[13] ^ (uint8_t)(t4 >> 8));
		if (--len == 0) return;
		cipher[14] = (uint8_t)(plain[14] ^ (uint8_t)(t8));
		if (--len == 0) return;
		cipher[15] = (uint8_t)(plain[15] ^ (uint8_t)(t8 >> 8));
#if defined(DEBUG)
		cout << "filter bytes end" << endl;
#endif
	}
#if defined(SEPARATE_XOR)

	static void
		xor_16bytes(uint8_t cipher[],
		const uint8_t stream[],
		const uint8_t plain[],
		int len)
	{
		for (int i = 0; i < len; i++) {
#if defined(HAVE_SSE2)
			__m128i x, y;
			x = _mm_load_si128((__m128i *)stream);
#if defined(USER_DATA_ALIGNED)
			y = _mm_load_si128((__m128i *)plain);
			x = _mm_xor_si128(x, y);
			_mm_store_si128((__m128i *)cipher, x);
#else // defined(USER_DATA_ALIGNED)
#if defined(HAVE_SSE3)
			x = _mm_lddqu_si128((__m128i *)&plain[i * 16]);
#else // defined(HAVE_SSE3)
			x = _mm_loadu_si128((__m128i *)&plain[i * 16]);
#endif // defined(HAVE_SSE3)
			x = _mm_xor_si128(x, y);
			_mm_storeu_si128((__m128i *)cipher, x);
#endif // defined(USER_DATA_ALIGNED)
#else // defined(HAVE_SSE2)
			for (int j = 0; j < 16; j++) {
				cipher[i] = stream[i] ^ plain[i];
			}
#endif // defined(HAVE_SSE2)
			cipher += 16;
			stream += 16;
			plain += 16;
		}

	}
	static void
		xor_bytes(uint8_t cipher[],
		const uint8_t stream[],
		const uint8_t plain[],
		int len)
	{
		for (int i = 0; i < len; i++) {
			cipher[i] = stream[i] ^ plain[i];
		}
	}
#endif // defined(SEPARATE_XOR)

	/* =======================================
	* CRYPT MT
	* ======================================= */
	static inline uint32_t u8to32(const uint8_t v[4])
	{
		return (uint32_t)v[0]
			| ((uint32_t)v[1] << 8)
			| ((uint32_t)v[2] << 16)
			| ((uint32_t)v[3] << 24);
	}

#if defined(DEBUG)
#include <inttypes.h>
	static void print_simd(simd_t *simd)
	{
		for (int i = 0; i < 4; i++) {
			printf("%08" PRIx32 " ", simd->ar[i]);
		}
		printf("\n");
	}
#endif // defined(DEBUG)

	/**
	* This function alocate and returns aligned memory.
	* Users shoud use aligned_free() to release the memory alocated
	* by this function.
	*
	*@param[in] size size in bytes.
	*@return aligned memory suitable for SIMD.
	*/
	void * aligned_alloc(size_t size)
	{
		void * array = NULL;
#if defined(AVX_ALIGN)
		array = _mm_malloc(size, 32);
#elif defined(MALLOC_OK)
		array = malloc(size);
#elif defined(POSIX_MEMALIGN)
		/* free(3) */
		if (posix_memalign((void **)&array, 16, size) != 0) {
			array = NULL;
		}
#elif defined(memalign)
		array = memalign(16, size);
#elif defined(WIN_ALIGNED_MALLOC)
		array = _aligned_malloc(size, 16);
		if (errno) {
			array = NULL;
		}
#endif
#if defined(DEBUGX) && DEBUGX >= 2
		cerr << "aligned alloc address:" << hex << array << endl;
		cerr << "aligned alloc size:" << dec << size << endl;
#endif
		return array;
	}

	/**
	* This function releases aligned memory allocated by
	* by aligned_alloc().
	*
	*@param[in] size size in bytes.
	*/
	void aligned_free(void * ptr)
	{
#if defined(AVX_ALIGN)
		_mm_free(ptr);
#elif defined(WIN_ALIGNED_MALLOC)
		_aligned_free(ptr);
#else
		free(ptr);
#endif
	}

	uint32_t
		maxKeySize()
	{
		return MAXKEYSIZE;
	}

	uint32_t
		keySizeUnit()
	{
		return 128;
	}

	class CryptMT::Impl {
	public:
		enum { INITIAL, IVSETUP, RUNNING, DONE } stage;
		Impl(const uint8_t *key,
			int keysize,
			int ivsize);
		~Impl();
		void boot_up(int length);
		void IVSetUp(const uint8_t *iv);
		void genrand_bytes_first(uint8_t *cipher,
			const uint8_t *plain,
			uint64_t len);
		void genrand_block_first(uint8_t cipher[],
			const uint8_t plain[]);
		void genrand_block(uint8_t cipher[],
			const uint8_t plain[],
			int blocks);
		void genrand_bytes(uint8_t cipher[],
			const uint8_t plain[],
			uint64_t len);
		bool isFirst() { return stage == IVSETUP; }
		uint32_t blockLength() { return BLOCKLENGTH; }
#if defined(DEBUG)
		void debug_print();
#endif
		int key_area_length;            /* length of first block */
		int keysize;           /* size in 16bit words (bits / 32) */
		int ivsize;            /* size in 16bit words (bits / 32) */
		simd_t *lung;           /* booter */
		uint32_t key[MAXKEYSIZE / 32];
		simd_t *psfmt;            /* pointer to sfmt internal state */
		simd_t *sfmt;             /* the array for the state vector  */
		simd_t *accum;            /* filter */
#if defined(SEPARATE_XOR)
		uint8_t *stream;
#endif
	};

	CryptMT::Impl::Impl(const uint8_t *key,
		int keysize,
		int ivsize)
	{
		for (int i = 0; i < keysize / 32; i++) {
			this->key[i] = u8to32(key);
			key += 4;
		}
		int size = sizeof(simd_t)
			* (ARRAY_SIZE + ((keysize + ivsize) * 4) / 128 + 2);
		sfmt = reinterpret_cast<simd_t *>(aligned_alloc((size_t)size));
		if (sfmt == NULL) {
			throw bad_alloc();
		}
		memset(sfmt, 0, size);
		accum
			= reinterpret_cast<simd_t *>(aligned_alloc((size_t)sizeof(simd_t)));
		if (accum == NULL) {
			aligned_free(sfmt);
			throw bad_alloc();
		}
		memset(accum, 0, sizeof(simd_t));
		lung
			= reinterpret_cast<simd_t *>(aligned_alloc((size_t)sizeof(simd_t)));
		if (lung == NULL) {
			aligned_free(sfmt);
			aligned_free(accum);
			throw bad_alloc();
		}
		memset(lung, 0, sizeof(simd_t));
#if defined(SEPARATE_XOR)
		stream
			= reinterpret_cast<uint8_t *>(aligned_alloc((size_t)sizeof(uint8_t)
			* ARRAY_SIZE));
		if (stream == NULL) {
			aligned_free(sfmt);
			aligned_free(accum);
			aligned_free(lung);
			throw bad_alloc();
		}
		memset(stream, 0, sizeof(uint8_t) * ARRAY_SIZE);
#endif
		this->keysize = keysize / 128;
		this->ivsize = ivsize / 128;
		stage = INITIAL;
		psfmt = sfmt;
	}

	CryptMT::Impl::~Impl()
	{
		aligned_free(accum);
		aligned_free(lung);
		aligned_free(sfmt);
#if defined(SEPARATE_XOR)
		aligned_free(stream);
#endif
	}

	CryptMT::CryptMT(const uint8_t *key,
		int keysize,
		int ivsize)
		throw(std::bad_alloc, std::invalid_argument)
	{
		if (keysize > MAXKEYSIZE) {
			throw invalid_argument("too large keysize");
		}
		if (keysize <= 0) {
			throw invalid_argument("too small keysize");
		}
		if (keysize % 128 != 0) {
			throw invalid_argument("keysize must be multiple of 128");
		}
		if (ivsize > MAXIVSIZE) {
			throw invalid_argument("too large ivsize");
		}
		if (keysize <= 0) {
			throw invalid_argument("too small ivsize");
		}
		if (ivsize % 128 != 0) {
			throw invalid_argument("ivsize must be multiple of 128");
		}
		impl = new Impl(key, keysize, ivsize);
	}

	CryptMT::~CryptMT()
	{
		delete impl;
	}

	void
		CryptMT::Impl::IVSetUp(const uint8_t *iv)
	{
		const int simd_bytes = 16;
		for (int i = 0; i < ivsize; i++) {
			for (int j = 0; j < 4; j++) {
				sfmt[i].ar[j] = u8to32(iv);
				iv += 4;
			}
		}
		int block_size = ivsize + keysize;
		memcpy(&sfmt[ivsize], key, keysize * simd_bytes);
		memcpy(&sfmt[block_size], sfmt, block_size * simd_bytes);
#if defined(DEBUG)
		printf("in ivsetup after memcpy\n");
		printf("sfmt[0]:");
		print_simd(&sfmt[0]);
		printf("sfmt[1]:");
		print_simd(&sfmt[1]);
		printf("sfmt[2]:");
		print_simd(&sfmt[2]);
		printf("sfmt[3]:");
		print_simd(&sfmt[3]);
		printf("sfmt[4]:");
		print_simd(&sfmt[4]);
#endif
		int p = 2 * block_size - 1;
		sfmt[p].ar[0] += 314159UL;
		sfmt[p].ar[1] += 265358UL;
		sfmt[p].ar[2] += 979323UL;
		sfmt[p].ar[3] += 846264UL;
		key_area_length = block_size * 2;
		boot_up(key_area_length);
		stage = IVSETUP;
	}


	void
		CryptMT::IVSetUp(const uint8_t *iv)
	{
		impl->IVSetUp(iv);
	}

	void
		CryptMT::Impl::boot_up(int length)
	{
		int i, p;

		psfmt = &sfmt[length + 2];
		p = ivsize / 4;
		for (i = 0; i < 4; i++) {
			lung->ar[i] = sfmt[p * 4].ar[i] | 1;
		}
		p = length - 2;
#if defined(DEBUG)
		printf("before booter_am in boot_up\n");
		printf("lung:");
		print_simd(lung);
		printf("sfmt[0]:");
		print_simd(&sfmt[0]);
		printf("sfmt[p]:");
		print_simd(&sfmt[p]);
		printf("p:%d\n", p);
		printf("length + 2:%d\n", length + 2);
#endif
		booter_am(lung, &sfmt[0], &sfmt[p], length + 2);
		for (i = 0; i < 4; i++) {
			accum->ar[i] = sfmt[2 * length + 1].ar[i];
		}
	}

	/**
	* For the purpose of speeding up encrypting short messages
	*
	*
	*/
	void
		CryptMT::Impl::genrand_bytes_first(uint8_t *cipher,
		const uint8_t *plain,
		uint64_t msglen)
	{
		if (stage != IVSETUP) {
			throw stage_exception("encryption called before IVSetUp");
		}
		int i;
		int p;
		int count;
		uint64_t len = msglen;

		if (len > BLOCKLENGTH) {
			len = BLOCKLENGTH;
		}
		count = (len + 7) / 8;
		p = key_area_length - 2;
		booter_am(lung,
			&psfmt[0],
			&psfmt[p],
			count);
#if defined(SEPARATE_XOR)
		filter_16bytes(psfmt, accum, stream, plain, len / 16);
		xor_16bytes(cipher, stream, plain, len / 16);
#else
		filter_16bytes(psfmt, accum, cipher, plain, len / 16);
#endif
		i = (len / 16) * 2;
		len = len % 16;
		if (len != 0) {
#if defined(SEPARATE_XOR)
			filter_16bytes(psfmt, accum, stream, plain, 1);
			xor_bytes(cipher, stream, plain, len);
#else
			filter_bytes(&psfmt[i], accum, &cipher[i * 8], &plain[i * 8], len);
#endif
		}
		stage = DONE;
		if (msglen <= BLOCKLENGTH) {
			return;
		}
		len = msglen - BLOCKLENGTH;
		cipher += BLOCKLENGTH;
		plain += BLOCKLENGTH;

		sfmt_t *ps = reinterpret_cast<sfmt_t *>(psfmt);
		ps->sfmt[0].ar[3] = INIL;
		count = (len + 7) / 8;
		do_recursion((ps + 1)->sfmt[0].ar,
			ps->sfmt[0].ar,
			psfmt[POS1].ar,
			psfmt[ARRAY_SIZE - 1].ar);
		psfmt = &ps->sfmt[1];

		for (i = ARRAY_SIZE;
			(count > 0) && (i < ARRAY_SIZE);
			i++, count--) {
			do_recursion(ps->sfmt[i].ar,
				ps->sfmt[i - ARRAY_SIZE].ar,
				ps->sfmt[i + POS1 - ARRAY_SIZE].ar,
				ps->sfmt[i - 1].ar);
		}
		filter_16bytes(ps->sfmt + ARRAY_SIZE, accum, cipher, plain, len / 16);
		i = (len / 16) * 2;
		len = len % 16;
		if (len != 0) {
			filter_bytes(&(ps->sfmt[i + ARRAY_SIZE]),
				accum,
				&cipher[i * 8],
				&plain[i * 8],
				len);
		}
	}

	/**
	* For the purpose of speeding up encrypting short messages
	*
	*
	*/
	void
		CryptMT::Impl::genrand_block_first(uint8_t cipher[],
		const uint8_t plain[])
	{
#if defined(DEBUG)
		cout << "start genrand_block_first" << endl;
#endif
		if (stage != IVSETUP) {
			throw stage_exception("encryption called before IVSetUp");
		}
		int p;
		sfmt_t *ps;

		ps = reinterpret_cast<sfmt_t *>(psfmt);
		p = key_area_length - 2;
		booter_am(lung, &psfmt[0],
			&psfmt[p], ARRAY_SIZE);
#if defined(SEPARATE_XOR)
		filter_16bytes(psfmt, accum, stream, plain, ARRAY_SIZE / 2);
		xor_16bytes(cipher, stream, plain, ARRAY_SIZE / 2);
#else
		filter_16bytes(psfmt, accum, cipher, plain, ARRAY_SIZE / 2);
#endif
		ps->sfmt[0].ar[3] = INIL;
#if 0
		for (i = 0; i < 4; i++) {
			(ps + 1)->sfmt[0].ar[i] = ps->sfmt[0].ar[i];
		}
#endif
		do_recursion((ps + 1)->sfmt[0].ar,
			ps->sfmt[0].ar,
			psfmt[POS1].ar,
			psfmt[ARRAY_SIZE - 1].ar);
		psfmt = &ps->sfmt[1];
		stage = RUNNING;
#if defined(DEBUG)
		cout << "end genrand_block_first" << endl;
#endif
	}

	void
		CryptMT::Impl::genrand_block(uint8_t cipher[],
		const uint8_t plain[],
		int blocks)
	{
#if defined(DEBUG)
		cout << "start genrand_block blocks:" << dec << blocks << endl;
#endif
		if (stage != RUNNING) {
			throw stage_exception("internal error");
		}
		for (int i = 0; i < blocks; i++) {
			sfmt_genrand_block(psfmt);
#if defined(SEPARATE_XOR)
			filter_16bytes(psfmt, accum, stream, plain, ARRAY_SIZE / 2);
			xor_16bytes(cipher, stream, plain, ARRAY_SIZE / 2);
#else
			filter_16bytes(psfmt, accum, cipher, plain, ARRAY_SIZE / 2);
#endif
			cipher += BLOCKLENGTH;
			plain += BLOCKLENGTH;
		}
#if defined(DEBUG)
		cout << "end genrand_block" << endl;
#endif
	}


	void
		CryptMT::Impl::genrand_bytes(uint8_t cipher[],
		const uint8_t plain[],
		uint64_t len)
	{
		if (stage != RUNNING) {
			throw stage_exception("internal error");
		}
#if defined(SEPARATE_XOR)
		sfmt_genrand_block(psfmt);
		filter_16bytes(psfmt, accum, stream, plain, ARRAY_SIZE / 2);
		xor_bytes(cipher, stream, plain, len);
#else
		sfmt_t *ps;
		int i;
		int count;
		ps = reinterpret_cast<sfmt_t *>(psfmt);
		count = (len + 7) / 8;

		do_recursion(ps->sfmt[0].ar,
			ps->sfmt[0].ar,
			ps->sfmt[POS1].ar,
			ps->sfmt[ARRAY_SIZE - 1].ar);
		count--;
		for (i = 1; (count > 0) && (i < ARRAY_SIZE - POS1); i++, count--) {
			do_recursion(ps->sfmt[i].ar,
				ps->sfmt[i].ar,
				ps->sfmt[i + POS1].ar,
				ps->sfmt[i - 1].ar);
		}
		for (; (count > 0) && (i < ARRAY_SIZE); i++, count--) {
			do_recursion(ps->sfmt[i].ar,
				ps->sfmt[i].ar,
				ps->sfmt[i + POS1 - ARRAY_SIZE].ar,
				ps->sfmt[i - 1].ar);
		}
		for (; (count > 0) && (i < ARRAY_SIZE); i++, count--) {
			do_recursion(ps->sfmt[i].ar,
				ps->sfmt[i - ARRAY_SIZE].ar,
				ps->sfmt[i + POS1 - ARRAY_SIZE].ar,
				ps->sfmt[i - 1].ar);
		}
		filter_16bytes(ps->sfmt, accum, cipher, plain, len / 16);
		i = (len / 16) * 2;
		len = len % 16;
		if (len != 0) {
			filter_bytes(&(ps->sfmt[i]), accum, &cipher[i * 8],
				&plain[i * 8], len);
		}
		stage = DONE;
#endif
	}

	uint32_t
		CryptMT::blockLength()
	{
		return impl->blockLength();
	}

	/* Message length in bytes. */
	void
		CryptMT::encrypt(const uint8_t * plaintext,
		uint8_t * ciphertext,
		uint64_t msglen)
		throw(stage_exception)
	{
#if defined(DEBUG)
		cout << "start encrypt: msglen = " << dec << msglen << endl;
#endif
		uint64_t blockLen = blockLength();
		if (impl->isFirst() && (msglen > 0)) {
			if (msglen >= blockLen) {
				impl->genrand_block_first(ciphertext, plaintext);
				ciphertext += blockLen;
				plaintext += blockLen;
				msglen -= blockLen;
			}
			else {
				impl->genrand_bytes_first(ciphertext, plaintext, msglen);
#if defined(DEBUG)
				cout << "end encrypt" << endl;
#endif
				return;
			}
		}
		if (msglen >= blockLen) {
			int blocks = msglen / blockLen;
			impl->genrand_block(ciphertext, plaintext, blocks);
			ciphertext += blockLen * blocks;
			plaintext += blockLen * blocks;
			msglen -= blockLen * blocks;
		}
		if (msglen != 0) {
			impl->genrand_bytes(ciphertext, plaintext, msglen);
		}
#if defined(DEBUG)
		cout << "end encrypt" << endl;
#endif
	}

	/* Message length in blocks. */
	void
		CryptMT::encryptBlocks(const uint8_t * plaintext,
		uint8_t * ciphertext,
		uint32_t blocks)
		throw(stage_exception)
	{
#if defined(DEBUG)
		cout << "start encryptBlocks blocks:" << dec << blocks << endl;
#endif
		int blockLen = blockLength();
		if (impl->isFirst() && (blocks > 0)) {
			impl->genrand_block_first(ciphertext, plaintext);
			ciphertext += blockLen;
			plaintext += blockLen;
			blocks--;
		}
		if (blocks > 0) {
			impl->genrand_block(ciphertext, plaintext, blocks);
		}
#if defined(DEBUG)
		cout << "end encryptBlocks" << endl;
#endif
	}

#if defined(DEBUG)
	void
		CryptMT::Impl::debug_print()
	{
		printf("=== cryptmt ===\n");
		printf("key_area_length:%d\n", key_area_length);
		printf("keysize:%d\n", keysize);
		printf("ivsize:%d\n", ivsize);
		printf("stage:%d\n", stage);
		printf("lung:");
		print_simd(lung);
		printf("key:");
		for (int i = 0; i < keysize * 128 / 32; i++) {
			printf("%08x", key[i]);
			if (i % 4 == 3) {
				printf("\n    ");
			}
		}
		printf("\n");
		printf("psfmt[%016" PRIxPTR "]:\n", (uintptr_t)psfmt);
		print_simd(psfmt);
		printf("sfmt[%016" PRIxPTR "]:\n", (uintptr_t)sfmt);
		print_simd(sfmt);
		printf("accum[%016" PRIxPTR "]:\n", (uintptr_t)accum);
		print_simd(accum);
		printf("=== cryptmt ===\n");
	}

	void
		CryptMT::debug_print()
	{
		impl->debug_print();
	}
#endif

}