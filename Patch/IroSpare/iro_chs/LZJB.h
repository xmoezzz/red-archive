#pragma once

#ifndef LZJB_RESTRICT
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
#define LZJB_RESTRICT restrict
#else
#define LZJB_RESTRICT
#endif
#endif

#include <stddef.h>
#include <stdint.h>

/* Determined based on testing with random data as ((s_len * 1.125) +
* (2 * NBBY)).  If you are aware of a situation where this is
* insufficient please let me know. */
#define LZJB_MAX_COMPRESSED_SIZE(s_len) \
	(s_len +															\
	 (s_len / 8) +												\
	 (((s_len % 8) != 0) ? 1 : 0) +				\
	 (2 * 8))

typedef enum {
	LZJB_OK,
	LZJB_BAD_DATA,
	LZJB_WOULD_OVERFLOW
} LZJBResult;

size_t lzjb_compress(const uint8_t* LZJB_RESTRICT src, uint8_t* LZJB_RESTRICT dst, size_t s_len, size_t d_len);
LZJBResult lzjb_decompress(const uint8_t* LZJB_RESTRICT src, uint8_t* LZJB_RESTRICT dst, size_t s_len, size_t* d_len);
