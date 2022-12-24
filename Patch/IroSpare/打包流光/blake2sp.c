/*
BLAKE2 reference source code package - optimized C implementations

Copyright 2012, Samuel Neves <sneves@dei.uc.pt>.  You may use this under the
terms of the CC0, the OpenSSL Licence, or the Apache Public License 2.0, at
your option.  The terms of these licenses can be found at:

- CC0 1.0 Universal : http://creativecommons.org/publicdomain/zero/1.0
- OpenSSL license   : https://www.openssl.org/source/license.html
- Apache 2.0        : http://www.apache.org/licenses/LICENSE-2.0

More information about the BLAKE2 hash function can be found at
https://blake2.net.
*/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#if defined(_OPENMP)
#include <omp.h>
#endif

#include "blake2.h"
#include "blake2-impl.h"

#define PARALLELISM_DEGREE 8

BLAKE2_LOCAL_INLINE(int) blake2sp_init_leaf(blake2s_state *S, uint8_t outlen, uint8_t keylen, uint64_t offset)
{
	blake2s_param P[1];
	P->digest_length = outlen;
	P->key_length = keylen;
	P->fanout = PARALLELISM_DEGREE;
	P->depth = 2;
	P->leaf_length = 0;
	store48(P->node_offset, offset);
	P->node_depth = 0;
	P->inner_length = BLAKE2S_OUTBYTES;
	memset(P->salt, 0, sizeof(P->salt));
	memset(P->personal, 0, sizeof(P->personal));
	return blake2s_init_param(S, P);
}

BLAKE2_LOCAL_INLINE(int) blake2sp_init_root(blake2s_state *S, uint8_t outlen, uint8_t keylen)
{
	blake2s_param P[1];
	P->digest_length = outlen;
	P->key_length = keylen;
	P->fanout = PARALLELISM_DEGREE;
	P->depth = 2;
	P->leaf_length = 0;
	store48(P->node_offset, 0ULL);
	P->node_depth = 1;
	P->inner_length = BLAKE2S_OUTBYTES;
	memset(P->salt, 0, sizeof(P->salt));
	memset(P->personal, 0, sizeof(P->personal));
	return blake2s_init_param(S, P);
}


int blake2sp_init(blake2sp_state *S, const uint8_t outlen)
{
	if (!outlen || outlen > BLAKE2S_OUTBYTES) return -1;

	memset(S->buf, 0, sizeof(S->buf));
	S->buflen = 0;

	if (blake2sp_init_root(S->R, outlen, 0) < 0)
		return -1;

	for (size_t i = 0; i < PARALLELISM_DEGREE; ++i)
		if (blake2sp_init_leaf(S->S[i], outlen, 0, i) < 0) return -1;

	S->R->last_node = 1;
	S->S[PARALLELISM_DEGREE - 1]->last_node = 1;
	return 0;
}

int blake2sp_init_key(blake2sp_state *S, const uint8_t outlen, const void *key, const uint8_t keylen)
{
	if (!outlen || outlen > BLAKE2S_OUTBYTES) return -1;

	if (!key || !keylen || keylen > BLAKE2S_KEYBYTES) return -1;

	memset(S->buf, 0, sizeof(S->buf));
	S->buflen = 0;

	if (blake2sp_init_root(S->R, outlen, keylen) < 0)
		return -1;

	for (size_t i = 0; i < PARALLELISM_DEGREE; ++i)
		if (blake2sp_init_leaf(S->S[i], outlen, keylen, i) < 0) return -1;

	S->R->last_node = 1;
	S->S[PARALLELISM_DEGREE - 1]->last_node = 1;
	{
		uint8_t block[BLAKE2S_BLOCKBYTES];
		memset(block, 0, BLAKE2S_BLOCKBYTES);
		memcpy(block, key, keylen);

		for (size_t i = 0; i < PARALLELISM_DEGREE; ++i)
			blake2s_update(S->S[i], block, BLAKE2S_BLOCKBYTES);

		secure_zero_memory(block, BLAKE2S_BLOCKBYTES); /* Burn the key from stack */
	}
	return 0;
}


int blake2sp_update(blake2sp_state *S, const uint8_t *in, uint64_t inlen)
{
	size_t left = S->buflen;
	size_t fill = sizeof(S->buf) - left;

	if (left && inlen >= fill)
	{
		memcpy(S->buf + left, in, fill);

		for (size_t i = 0; i < PARALLELISM_DEGREE; ++i)
			blake2s_update(S->S[i], S->buf + i * BLAKE2S_BLOCKBYTES, BLAKE2S_BLOCKBYTES);

		in += fill;
		inlen -= fill;
		left = 0;
	}

#if defined(_OPENMP)
#pragma omp parallel shared(S), num_threads(PARALLELISM_DEGREE)
#else

	for (size_t id__ = 0; id__ < PARALLELISM_DEGREE; ++id__)
#endif
	{
#if defined(_OPENMP)
		size_t      id__ = omp_get_thread_num();
#endif
		uint64_t inlen__ = inlen;
		const uint8_t *in__ = (const uint8_t *)in;
		in__ += id__ * BLAKE2S_BLOCKBYTES;

		while (inlen__ >= PARALLELISM_DEGREE * BLAKE2S_BLOCKBYTES)
		{
			blake2s_update(S->S[id__], in__, BLAKE2S_BLOCKBYTES);
			in__ += PARALLELISM_DEGREE * BLAKE2S_BLOCKBYTES;
			inlen__ -= PARALLELISM_DEGREE * BLAKE2S_BLOCKBYTES;
		}
	}

	in += inlen - inlen % (PARALLELISM_DEGREE * BLAKE2S_BLOCKBYTES);
	inlen %= PARALLELISM_DEGREE * BLAKE2S_BLOCKBYTES;

	if (inlen > 0)
		memcpy(S->buf + left, in, inlen);

	S->buflen = left + inlen;
	return 0;
}


int blake2sp_final(blake2sp_state *S, uint8_t *out, const uint8_t outlen)
{
	uint8_t hash[PARALLELISM_DEGREE][BLAKE2S_OUTBYTES];

	for (size_t i = 0; i < PARALLELISM_DEGREE; ++i)
	{
		if (S->buflen > i * BLAKE2S_BLOCKBYTES)
		{
			size_t left = S->buflen - i * BLAKE2S_BLOCKBYTES;

			if (left > BLAKE2S_BLOCKBYTES) left = BLAKE2S_BLOCKBYTES;

			blake2s_update(S->S[i], S->buf + i * BLAKE2S_BLOCKBYTES, left);
		}

		blake2s_final(S->S[i], hash[i], BLAKE2S_OUTBYTES);
	}

	for (size_t i = 0; i < PARALLELISM_DEGREE; ++i)
		blake2s_update(S->R, hash[i], BLAKE2S_OUTBYTES);

	return blake2s_final(S->R, out, outlen);
}


int blake2sp(uint8_t *out, const void *in, const void *key, uint8_t outlen, uint64_t inlen, uint8_t keylen)
{
	uint8_t hash[PARALLELISM_DEGREE][BLAKE2S_OUTBYTES];
	blake2s_state S[PARALLELISM_DEGREE][1];
	blake2s_state FS[1];

	/* Verify parameters */
	if (NULL == in && inlen > 0) return -1;

	if (NULL == out) return -1;

	if (NULL == key && keylen > 0) return -1;

	if (!outlen || outlen > BLAKE2S_OUTBYTES) return -1;

	if (keylen > BLAKE2S_KEYBYTES) return -1;

	for (size_t i = 0; i < PARALLELISM_DEGREE; ++i)
		if (blake2sp_init_leaf(S[i], outlen, keylen, i) < 0) return -1;

	S[PARALLELISM_DEGREE - 1]->last_node = 1; /* mark last node */

	if (keylen > 0)
	{
		uint8_t block[BLAKE2S_BLOCKBYTES];
		memset(block, 0, BLAKE2S_BLOCKBYTES);
		memcpy(block, key, keylen);

		for (size_t i = 0; i < PARALLELISM_DEGREE; ++i)
			blake2s_update(S[i], block, BLAKE2S_BLOCKBYTES);

		secure_zero_memory(block, BLAKE2S_BLOCKBYTES); /* Burn the key from stack */
	}

#if defined(_OPENMP)
#pragma omp parallel shared(S,hash), num_threads(PARALLELISM_DEGREE)
#else

	for (size_t id__ = 0; id__ < PARALLELISM_DEGREE; ++id__)
#endif
	{
#if defined(_OPENMP)
		size_t      id__ = omp_get_thread_num();
#endif
		uint64_t inlen__ = inlen;
		const uint8_t *in__ = (const uint8_t *)in;
		in__ += id__ * BLAKE2S_BLOCKBYTES;

		while (inlen__ >= PARALLELISM_DEGREE * BLAKE2S_BLOCKBYTES)
		{
			blake2s_update(S[id__], in__, BLAKE2S_BLOCKBYTES);
			in__ += PARALLELISM_DEGREE * BLAKE2S_BLOCKBYTES;
			inlen__ -= PARALLELISM_DEGREE * BLAKE2S_BLOCKBYTES;
		}

		if (inlen__ > id__ * BLAKE2S_BLOCKBYTES)
		{
			const size_t left = inlen__ - id__ * BLAKE2S_BLOCKBYTES;
			const size_t len = left <= BLAKE2S_BLOCKBYTES ? left : BLAKE2S_BLOCKBYTES;
			blake2s_update(S[id__], in__, len);
		}

		blake2s_final(S[id__], hash[id__], BLAKE2S_OUTBYTES);
	}

	if (blake2sp_init_root(FS, outlen, keylen) < 0)
		return -1;

	FS->last_node = 1;

	for (size_t i = 0; i < PARALLELISM_DEGREE; ++i)
		blake2s_update(FS, hash[i], BLAKE2S_OUTBYTES);

	return blake2s_final(FS, out, outlen);
}

#if defined(BLAKE2SP_SELFTEST)
#include <string.h>
#include "blake2-kat.h"
int main(int argc, char **argv)
{
	uint8_t key[BLAKE2S_KEYBYTES];
	uint8_t buf[KAT_LENGTH];

	for (size_t i = 0; i < BLAKE2S_KEYBYTES; ++i)
		key[i] = (uint8_t)i;

	for (size_t i = 0; i < KAT_LENGTH; ++i)
		buf[i] = (uint8_t)i;

	for (size_t i = 0; i < KAT_LENGTH; ++i)
	{
		uint8_t hash[BLAKE2S_OUTBYTES];
		blake2sp(hash, buf, key, BLAKE2S_OUTBYTES, i, BLAKE2S_KEYBYTES);

		if (0 != memcmp(hash, blake2sp_keyed_kat[i], BLAKE2S_OUTBYTES))
		{
			return -1;
		}
	}
	return 0;
}
#endif




