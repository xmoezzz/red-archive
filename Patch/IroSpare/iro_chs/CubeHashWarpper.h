#pragma once

#include "sph_cubehash.h"
#include "my.h"


ForceInline Void Cubehash224(PBYTE Buffer, ULONG_PTR Length, PBYTE Hash)
{
	sph_cubehash_context ctx_cubehash;
	BYTE                 GrayHash[28];

	sph_cubehash224_init(&ctx_cubehash);
	sph_cubehash224(&ctx_cubehash, Hash, 28);
	sph_cubehash224_close(&ctx_cubehash, GrayHash);
}


ForceInline Void Cubehash256(PBYTE Buffer, ULONG_PTR Length, PBYTE Hash)
{
	sph_cubehash_context ctx_cubehash;

	sph_cubehash256_init(&ctx_cubehash);
	sph_cubehash256(&ctx_cubehash, Buffer, Length);
	sph_cubehash256_close(&ctx_cubehash, Hash);
}


ForceInline Void Cubehash384(PBYTE Buffer, ULONG_PTR Length, PBYTE Hash)
{
	sph_cubehash_context ctx_cubehash;

	sph_cubehash384_init(&ctx_cubehash);
	sph_cubehash384(&ctx_cubehash, Buffer, Length);
	sph_cubehash384_close(&ctx_cubehash, Hash);
}

ForceInline Void Cubehash512(PBYTE Buffer, ULONG_PTR Length, PBYTE Hash)
{
	sph_cubehash_context ctx_cubehash;

	sph_cubehash512_init(&ctx_cubehash);
	sph_cubehash512(&ctx_cubehash, Buffer, Length);
	sph_cubehash512_close(&ctx_cubehash, Hash);
}
