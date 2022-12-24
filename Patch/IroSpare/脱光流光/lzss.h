#pragma once

#pragma once

#include "my.h"

void ps2_uncompress(BYTE *uncompr, DWORD uncomprlen, BYTE *compr, DWORD comprlen);
size_t lzss_encode(unsigned char* input, size_t inlen, unsigned char* output, size_t outlen);
