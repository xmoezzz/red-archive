#pragma once
//Check all status, ensure that the caller is valid

#include "my.h"
#include "tp_stub.h"

IStream* NTAPI CreateMyStream(const ttstr& name, ULONG Flag);

