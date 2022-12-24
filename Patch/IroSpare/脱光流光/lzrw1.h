#pragma once

#include "my.h"

void lzrw1_compress(PBYTE p_src_first, ULONG src_len, PBYTE p_dst_first, PULONG p_dst_len);
void lzrw1_decompress(PBYTE p_src_first, ULONG src_len, PBYTE p_dst_first, PULONG p_dst_len);
