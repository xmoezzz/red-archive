#pragma once
#include <emmintrin.h>
#include "HashConst.h"
#include <Windows.h>


void hashcat_md4_64(__m128i digests[4], __m128i W[16]);
void hashcat_md5_64(__m128i digests[4], __m128i W[16]);
void hashcat_sha1_64(__m128i digests[5], __m128i W[16]);
void hashcat_sha256_64(__m128i digests[8], __m128i W[16]);
void hashcat_sha512_64(__m128i digests[8], __m128i W[16]);


