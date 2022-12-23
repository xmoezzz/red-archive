#pragma once

#include "my.h"
#include <string>
#include "json/json.h"

using std::string;

NTSTATUS DecompilePSBFile(PBYTE Buffer, ULONG Size, LPCWSTR FileName, Json::Value& DecompiledResult, Json::Value& ResourceResult);
NTSTATUS CompilePSBFile(PBYTE& Buffer, ULONG& Size, Json::Value& DecompiledResult, Json::Value& ResourceResult);
int WINAPI ExtractPsbText(IStream* Stream, PBYTE& OutBuffer, ULONG& OutSize, LPCWSTR FileName);
