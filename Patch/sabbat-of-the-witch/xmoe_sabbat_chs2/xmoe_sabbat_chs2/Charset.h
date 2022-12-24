#pragma once

#include "tp_stub.h"

tjs_int AnzWideCharToUtf8String(const tjs_char *in, char * out);
tjs_int AnzUtf8ToWideCharString(const char * in, tjs_char *out);

