#pragma once

#include "blake2.h"
#include "my.h"

ForceInline Void blake2s_buffer(LPCBYTE buffer, size_t length, PBYTE resstream)
{
	blake2s_state S[1];

	blake2s_init(S, BLAKE2S_OUTBYTES);
	blake2s_update(S, buffer, length);
	blake2s_final(S, resstream, BLAKE2S_OUTBYTES);
}

ForceInline Void blake2b_buffer(LPCBYTE buffer, size_t length, PBYTE resstream)
{
	blake2b_state S[1];

	blake2b_init(S, BLAKE2B_OUTBYTES);
	blake2b_update(S, buffer, length);
	blake2b_final(S, resstream, BLAKE2B_OUTBYTES);
}

ForceInline Void blake2sp_buffer(LPCBYTE buffer, size_t length, PBYTE resstream)
{
	blake2sp_state S[1];

	blake2sp_init(S, BLAKE2S_OUTBYTES);
	blake2sp_update(S, buffer, length);
	blake2sp_final(S, resstream, BLAKE2S_OUTBYTES);
}

ForceInline Void blake2bp_buffer(LPCBYTE buffer, size_t length, PBYTE resstream)
{
	blake2bp_state S[1];

	blake2bp_init(S, BLAKE2B_OUTBYTES);
	blake2bp_update(S, buffer, length);
	blake2bp_final(S, resstream, BLAKE2B_OUTBYTES);
}

