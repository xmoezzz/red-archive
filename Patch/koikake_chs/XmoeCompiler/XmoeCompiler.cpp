#include "my.h"
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include "blake2.h"
#include "MyCxdec.h"
//#include "ThemidaSDK.h"
#include "VMProtectSDK.h"

using std::fstream;
using std::wfstream;
using std::string;
using std::wstring;
using std::vector;
using std::map;



ForceInline Void blake2bp_buffer(LPCBYTE buffer, size_t length, PBYTE resstream)
{
	blake2bp_state S[1];

	blake2bp_init(S, BLAKE2B_OUTBYTES);
	blake2bp_update(S, buffer, length);
	blake2bp_final(S, resstream, BLAKE2B_OUTBYTES);
}


static CCxdec* g_cxdec = NULL;

ForceInline Void EncodeString(LPSTR Buffer, ULONG Size, ULONG Hash, ULONG Index)
{
	//VMStart();
	//VMProtectBeginVirtualization("000");
	LARGE_INTEGER Offset;

	auto DecryptCxdecInternal = [](ULONG Hash, LARGE_INTEGER Offset, PVOID lpBuffer, ULONG BufferSize, ULONG Index)->BOOL
	{
		PBYTE           pbBuffer;
		ULONG           Mask, Mask2;
		LARGE_INTEGER   CurrentPos;
		CCxdec         *pCxdec;
		Byte            HashTable[64];

		pCxdec = g_cxdec;
		if (pCxdec == NULL)
		{
			pCxdec = g_cxdec = new CCxdec;
		}

		pbBuffer = (PBYTE)lpBuffer;
		Mask = pCxdec->GetMask(Hash);

		Mask2 = LOWORD(Mask);
		CurrentPos.QuadPart = Offset.QuadPart + BufferSize;

		if (Mask2 >= Offset.QuadPart && Mask2 < CurrentPos.QuadPart)
		{
			*(pbBuffer + Mask2 - Offset.LowPart) ^= Hash >> 16;
		}

		Mask2 = HIWORD(Mask);
		if (Mask2 >= Offset.QuadPart && Mask2 < CurrentPos.QuadPart)
		{
			*(pbBuffer + Mask2 - Offset.LowPart) ^= Hash >> 8;
		}

		XorMemory(pbBuffer, Hash, BufferSize);

		union
		{
			ULONG InfoHash[2];
			BYTE  ByteInfo[8];
		}PreHash;

		PreHash.InfoHash[0] = Hash;
		PreHash.InfoHash[1] = Index;

		blake2bp_buffer(PreHash.ByteInfo, 8, HashTable);

		PreHash.InfoHash[0] = PreHash.InfoHash[1] = 0;

		for (ULONG i = 0; i < BufferSize; i++)
			((PBYTE)lpBuffer)[i] ^= HashTable[i % countof(HashTable)];

		RtlZeroMemory(HashTable, sizeof(HashTable));
		return TRUE;
	};

	Offset.QuadPart = 0;
	DecryptCxdecInternal(Hash, Offset, Buffer, Size, Index);
	//VMEnd();
	//VMProtectEnd();
}


BYTE TempTable[512] =
{
	0x7d, 0x54, 0x11, 0xd1, 0xfa, 0xad, 0xb0,
	0xc2, 0x77, 0x0f, 0xe2, 0x5a, 0x95, 0x19, 0x4e,
	0x93, 0x27, 0x43, 0x15, 0x7d, 0xb5, 0x54, 0x48,
	0xba, 0xfb, 0xe9, 0x6d, 0x02, 0x6b, 0xf4, 0xfe,
	0xa0, 0xa7, 0x3b, 0xe9, 0xf3, 0x08, 0xd5, 0x11,
	0xee, 0x1a, 0xb0, 0xcb, 0x98, 0x1d, 0x0e, 0x62,
	0x8d, 0x86, 0x03, 0x94, 0x7b, 0x7b, 0xf9, 0x13,
	0xa5, 0x5b, 0x2c, 0x05, 0x64, 0x34, 0x2f, 0x84,
	0xa1, 0x4b, 0x65, 0x1e, 0x5c, 0x97, 0x88, 0x56,
	0x29, 0x46, 0x25, 0x21, 0xad, 0x37, 0x1e, 0x6b,
	0x25, 0x7e, 0x27, 0x90, 0xe0, 0xe3, 0x4a, 0xe3,
	0xc0, 0x63, 0x63, 0x2a, 0xbd, 0xae, 0xa4, 0x1f,
	0x61, 0xa7, 0x12, 0xf1, 0x4d, 0xe8, 0x06, 0xc1,
	0xb3, 0x3b, 0xad, 0x25, 0xda, 0x21, 0x89, 0xa9,
	0x9d, 0x4f, 0xee, 0x49, 0xec, 0x2c, 0x86, 0xf8,
	0x4a, 0x55, 0xcd, 0x1c, 0x4d, 0x19, 0x95, 0x10,
	0x21, 0xfd, 0x83, 0xa0, 0x05, 0x39, 0x90, 0x91,
	0xcc, 0x39, 0x89, 0x16, 0x5e, 0x1e, 0x8f, 0x5c,
	0x34, 0x3a, 0x98, 0xff, 0xe0, 0x97, 0xed, 0x93,
	0x83, 0x70, 0xaa, 0x1c, 0x54, 0xb6, 0x41, 0x95,
	0x20, 0x8d, 0xf7, 0x6d, 0xc4, 0xcc, 0x65, 0x06,
	0xb5, 0x81, 0xf8, 0x35, 0x79, 0x6b, 0x71, 0xc4,
	0x2b, 0x7e, 0x66, 0xf3, 0xfb, 0x62, 0xbf, 0xf2,
	0xab, 0xf4, 0x3a, 0x69, 0x13, 0xc4, 0xe8, 0xf1,
	0x9e, 0x95, 0xae, 0x98, 0xcb, 0xe1, 0xc5, 0x60,
	0xad, 0x52, 0x3a, 0xc0, 0x6b, 0x49, 0x6e, 0x22,
	0xc1, 0x5b, 0x97, 0x64, 0x7d, 0xcf, 0x3d, 0x57,
	0x02, 0x22, 0xbe, 0x43, 0xc9, 0x83, 0xcb, 0x61,
	0xdb, 0x57, 0xe8, 0x5f, 0x58, 0xb6, 0xf0, 0xe0,
	0xf4, 0xec, 0x8f, 0xf9, 0x74, 0xf9, 0xc6, 0xb5,
	0x36, 0x12, 0x6b, 0x92, 0xa6, 0x1d, 0xa6, 0x02,
	0xc9, 0x39, 0x75, 0xeb, 0xb6, 0x33, 0x28, 0x27,
	0x18, 0x12, 0xe6, 0x04, 0xad, 0x8d, 0x26, 0xc5,
	0xca, 0x8f, 0x37, 0x1f, 0xd5, 0xba, 0xb9, 0xbd,
	0xca, 0xe1, 0x22, 0xbd, 0xb7, 0x8d, 0x3a, 0x31,
	0x3f, 0x79, 0x9f, 0x9f, 0x1a, 0x15, 0x41, 0x81,
	0x94, 0x07, 0xe7, 0xc6, 0x0a, 0xa5, 0xa8, 0x4f,
	0x70, 0x7c, 0x73, 0x73, 0xcd, 0xcc, 0x88, 0x7b,
	0xbd, 0x0a, 0xfc, 0x26, 0xee, 0x5d, 0x39, 0x26,
	0xa4, 0x22, 0x7c, 0xa1, 0x36, 0x68, 0x55, 0xb2,
	0x8f, 0x74, 0x2b, 0xe5, 0xad, 0x3e, 0xb5, 0xbe,
	0x25, 0xf2, 0x82, 0x33, 0x9d, 0x70, 0x72, 0x2e,
	0x50, 0xcc, 0x3a, 0x0c, 0x8e, 0xcf, 0xe4, 0x20,
	0x39, 0x74, 0x4d, 0x30, 0x49, 0x6c, 0xa5, 0xf7,
	0x49, 0x9b, 0xf2, 0xa2, 0xd8, 0x98, 0x8e, 0x53,
	0x29, 0x31, 0xa4, 0xa1, 0x83, 0xe5, 0xb7, 0x16,
	0xc2, 0x68, 0x1b, 0xaf, 0xd4, 0x22, 0x7a, 0x5f,
	0x3d, 0xb0, 0x50, 0x8d, 0x93, 0x62, 0x70, 0x92,
	0x02, 0xbb, 0x7d, 0x3c, 0xca, 0xf4, 0x71, 0x4d,
	0xbc, 0x79, 0x1a, 0xfc, 0xc1, 0x6b, 0x97, 0x73,
	0x53, 0x1c, 0xdf, 0x4f, 0x01, 0x97, 0x3b, 0x24,
	0xf0, 0x15, 0xc7, 0xf7, 0x55, 0x88, 0xf6, 0xc1,
	0xfb, 0x14, 0x0b, 0xf3, 0xc3, 0x91, 0xa0, 0xec,
	0x1f, 0x0b, 0x22, 0x84, 0x96, 0x42, 0x53, 0x85,
	0x43, 0x2a, 0xc7, 0x2d, 0x56, 0x6c, 0x68, 0xae,
	0x92, 0xe3, 0xf2, 0xae, 0xcd, 0x20, 0x77, 0xc7,
	0x73, 0xe7, 0xdc, 0x07, 0x03, 0xaf, 0x5a, 0x71,
	0x91, 0x26, 0xfe, 0x7a, 0x42, 0xab, 0x2a, 0x8d,
	0xd3, 0xd2, 0x12, 0x88, 0x12, 0xe3, 0x3f, 0x3d,
	0x63, 0x5b, 0x0f, 0xf2, 0x3d, 0x69, 0x33, 0xe1,
	0xab, 0x73, 0x30, 0xb8, 0xcb, 0x8f, 0xdf, 0x1a,
	0x52, 0x0a, 0xed, 0x1d, 0x06, 0xe4, 0x5c, 0xca,
	0x42, 0x52, 0x00, 0xa0, 0x76, 0x3b, 0x02, 0x11,
	0xa4, 0xbc, 0x60, 0x03, 0xe4, 0xa4, 0x6b, 0x51,
	0xe1
};

ForceInline ULONG MyHashMask(ULONG Hash)
{
	union
	{
		BYTE  ByteInfo[4];
		ULONG Info;
	};

	ByteInfo[0] = TempTable[Hash & 0xFF];
	ByteInfo[1] = TempTable[((Hash >> 3) * 4) % countof(TempTable)];
	ByteInfo[2] = TempTable[((Hash * 6) ^ 0x1542) % countof(TempTable)];
	ByteInfo[3] = TempTable[Hash % countof(TempTable)];

	return Info;
}



class ITextCompiler
{
public:
	virtual BOOL NTAPI LoadTextFromBuffer(PBYTE Buffer, ULONG Size) = 0;
	virtual BOOL NTAPI LoadText(LPCWSTR lpPath) = 0;
	virtual BOOL NTAPI ReCompile() = 0;
	virtual BOOL NTAPI QueryText(ULONG Index, PBYTE Data, PULONG Size, PULONG DecIndex) = 0;
	virtual BOOL NTAPI DoPostDec(ULONG DecIndex, PBYTE Data, ULONG Size, LPSTR String, ULONG CurIndex) = 0;
	virtual BOOL NTAPI Release() = 0;
};


static BYTE Prefix[] = { 0xE2, 0x97, 0x8F };
static BYTE UTFBOM[] = { 0xEF, 0xBB, 0xBF };

BOOL ProcessLineXmoe(ULONG& Line, string& Info, string& Input);

VOID OutputInfo(LPCWSTR Info)
{
	DWORD nRet;
	HANDLE hStd = GetStdHandle(STD_OUTPUT_HANDLE);
	WriteConsoleW(hStd, Info, lstrlenW(Info), &nRet, NULL);
	WriteConsoleW(hStd, L"\n", 1, &nRet, NULL);
}


VOID OutputInfo(LPCSTR Info, ULONG CodePage)
{
	DWORD nRet;
	WCHAR wInfo[2000] = { 0 };
	MultiByteToWideChar(CodePage, 0, Info, lstrlenA(Info), wInfo, 2000);
	HANDLE hStd = GetStdHandle(STD_OUTPUT_HANDLE);
	WriteConsoleW(hStd, wInfo, lstrlenW(wInfo), &nRet, NULL);
	WriteConsoleW(hStd, L"\n", 1, &nRet, NULL);
}

VOID OutputInfo(LPCSTR Info)
{
	DWORD nRet;
	WCHAR wInfo[2000] = { 0 };
	MultiByteToWideChar(CP_UTF8, 0, Info, lstrlenA(Info), wInfo, 2000);
	HANDLE hStd = GetStdHandle(STD_OUTPUT_HANDLE);
	WriteConsoleW(hStd, wInfo, lstrlenW(wInfo), &nRet, NULL);
	WriteConsoleW(hStd, L"\n", 1, &nRet, NULL);
}




class MemoryBuffer
{
private:
	ULONG    Size;
	PBYTE    Buffer;
	ULONG    iPos;

public:
	MemoryBuffer() :
		iPos(0)
	{
	}

	~MemoryBuffer()
	{
	}

	BOOL Init(PBYTE InBuffer, ULONG InSize)
	{
		Buffer = InBuffer;
		Size = InSize;

		return InBuffer && InSize ? TRUE : FALSE;
	}

	BOOL GetLines(vector<string>& ReadPool)
	{
		string ReadLine;

		ReadLine.clear();
		LOOP_FOREVER
		{
			if (iPos >= Size)
			break;

			if (Buffer[iPos] == '\r')
			{
				ReadPool.push_back(ReadLine);
				ReadLine.clear();
				iPos++;

				if (Buffer[iPos] == '\n')
					iPos++;
			}

			if (Buffer[iPos] == '\n')
			{
				ReadPool.push_back(ReadLine);
				ReadLine.clear();
				iPos++;
			}

			ReadLine += Buffer[iPos];
			iPos++;
		}

			if (ReadLine.length())
				ReadPool.push_back(ReadLine);

		return TRUE;
	}
};


class InfoString
{
public:
	ULONG Hash;
	ULONG Length;
	vector<CHAR> String;

	InfoString& operator = (const InfoString& o)
	{
		Hash = o.Hash;
		Length = o.Length;
		String = o.String;

		return *this;
	}
};

class TextCompilerV1 : public ITextCompiler
{
private:
	BOOL     Encoded;

public:
	TextCompilerV1() : Encoded(FALSE){}

	BOOL NTAPI LoadText(LPCWSTR lpPath)
	{
		WIN32_FIND_DATAW   FindInfo;
		HANDLE             hSearch;
		BOOL               Result;
		WCHAR              PathToFile[MAX_PATH];

		RtlZeroMemory(&FindInfo, sizeof(FindInfo));
		RtlZeroMemory(PathToFile, MAX_PATH);

		LOOP_ONCE
		{
			Result = FALSE;
			hSearch = FindFirstFileW(L"ProjectDir\\*.txt", &FindInfo);

			if (hSearch == INVALID_HANDLE_VALUE)
				break;

			do
			{
				lstrcpyW(PathToFile, L"ProjectDir\\");
				lstrcatW(PathToFile, FindInfo.cFileName);
				LoadTextInternal(PathToFile);

			} while (FindNextFileW(hSearch, &FindInfo));

			FindClose(hSearch);
			Result = TRUE;
		}
		return Result;
	}

	BOOL NTAPI ReCompile()
	{
		return FALSE;
	}

	BOOL NTAPI Release()
	{
		TextPool.clear();
		return TRUE;
	}

	BOOL NTAPI DoPostDec(ULONG DecIndex, PBYTE Data, ULONG Size, LPSTR String, ULONG CurIndex)
	{
		BOOL Result;

		LOOP_ONCE
		{
			Result = FALSE;
			if (!Data || !Size || !String)
				break;

			if (DecIndex == 0xFFFFFFFFUL || !Encoded)
			{
				RtlCopyMemory(String, Data, Size + 1);
				Result = TRUE;
				break;
			}
			else
			{
				RtlZeroMemory(String, Size + 1);
				RtlCopyMemory(String, Data, Size);
				EncodeString(String, Size, DecIndex, CurIndex);
			}
		}
		return Result;
	}

	BOOL NTAPI QueryText(ULONG Index, PBYTE Data, PULONG Size, PULONG DecIndex)
	{
		//VMStart();
		//VMProtectBeginVirtualization("001");
		BOOL Result;

		LOOP_ONCE
		{
			Result = FALSE;
			if (!Encoded)
			{
				auto it = TextPool.find(Index);

				if (it == TextPool.end())
					break;

				if (DecIndex)
					*DecIndex = 0xFFFFFFFFUL;

				*Size = it->second.length();

				if (Data)
					StrCopyA((PChar)Data, it->second.c_str());
			}
			else
			{
				auto it = TextPoolBinary.find(Index);

				if (it == TextPoolBinary.end())
					break;

				if (DecIndex)
					*DecIndex = it->second.Hash;

				*Size = it->second.String.size();

				if (Data)
					RtlCopyMemory(Data, &(it->second.String[0]), it->second.String.size());

				Result = FALSE;
			}
			Result = TRUE;
		}
		//VMEnd();
		//VMProtectEnd();

		return Result;
	}

	BOOL NTAPI LoadTextFromBuffer(PBYTE Buffer, ULONG Size)
	{
		BOOL               Result;

		LOOP_ONCE
		{
			Result = FALSE;
			if (!Buffer || !Size)
				break;

			Result = LoadTextBuffer(Buffer, Size);
		}
		if (Result)
			Encoded = TRUE;

		return Result;
	}

private:

	BOOL NTAPI LoadTextInternal(LPWSTR lpFileName)
	{
		fstream        File;
		string         ReadLine;
		ULONG          Index;
		string         CNLine;
		BOOL           ResultOfParse;


		File.open(lpFileName, std::ios_base::in | std::ios_base::out);
		while (getline(File, ReadLine))
		{
			if (ReadLine.length() == 0)
				continue;

			if (!memcmp(ReadLine.c_str(), UTFBOM, 3))
				ReadLine = &ReadLine[3];

			if (ReadLine[0] == ';')
			{
				ResultOfParse = ProcessLineXmoe(Index, CNLine, ReadLine);

				if (ResultOfParse)
					TextPool.insert(std::make_pair(Index, CNLine));
			}
		}
		return TRUE;
	}

	BOOL NTAPI LoadTextBuffer(PBYTE Buffer, ULONG Size)
	{
		//VMStart();

		ULONG iPos = 4;
		while (iPos < Size)
		{
			InfoString Info;

			Info.Length = *(PULONG)(Buffer + iPos);
			iPos += 4;
			Info.Hash = *(PULONG)(Buffer + iPos);
			iPos += 4;
			ULONG Index = *(PULONG)(Buffer + iPos);
			iPos += 4;

			Index ^= MyHashMask(~Info.Hash);

			for (ULONG i = 0; i < (Info.Length ^ MyHashMask(Info.Hash)); i++)
			{
				Info.String.push_back(Buffer[iPos]);
				iPos++;
			}
			TextPoolBinary.insert(std::make_pair(Index, Info));
		}
		//VMEnd();
		return TRUE;
	}

	map<ULONG, string>     TextPool;
	map<ULONG, InfoString> TextPoolBinary;
};


BOOL ProcessLineXmoe(ULONG& Line, string& Info, string& Input)
{
	BOOL  Result;
	CHAR  NumStr[12];
	WCHAR WideStr[2000] = { 0 };
	CHAR  GBKStr[2000] = { 0 };

	LOOP_ONCE
	{
		Result = FALSE;

		if (Input.length() <= 13)
			break;

		if (Input[12] != ']')
			break;

		RtlZeroMemory(NumStr, sizeof(NumStr));
		RtlCopyMemory(NumStr, &Input[2], 10);
		sscanf(NumStr, "%x", &Line);
		//Line = atoi(NumStr);
		Info = Input.substr(13, Input.length());

		MultiByteToWideChar(CP_UTF8, 0, Info.c_str(), Info.length(), WideStr, 2000);
		WideCharToMultiByte(936, 0, WideStr, lstrlenW(WideStr), GBKStr, 2000, 0, 0);

		Info = GBKStr;
		Result = TRUE;
	}
	return Result;
}


TextCompilerV1* Compiler = NULL;

extern "C" __declspec(dllexport)
BOOL NTAPI CreateCompiler(ITextCompiler** Server)
{
	//AllocConsole();

	if (Compiler)
		*Server = Compiler;
	else
		*Server = new TextCompilerV1;

	return TRUE;

}

extern "C" __declspec(dllexport)
BOOL NTAPI DeleteCompiler()
{
	if (Compiler)
	{
		delete Compiler;
		Compiler = NULL;
	}
	return TRUE;
}

BOOL NTAPI DllMain(HMODULE hModule, DWORD Reason, LPVOID lpReserved)
{
	return TRUE;
}


