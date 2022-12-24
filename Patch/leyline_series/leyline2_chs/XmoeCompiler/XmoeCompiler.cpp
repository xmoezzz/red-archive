//X'moe Compiler V3.0
//With Text-Based Decoder
//With Mmeory Buffer Mode
//With VMP SDK

#include <Windows.h>
#include <fstream>
#include <istream>
#include <string>
#include <vector>
#include <map>

//#include "VMProtectSDK.h"


using std::fstream;
using std::wfstream;
using std::istream;
using std::string;
using std::wstring;
using std::vector;
using std::map;

#define LOOP_ONCE    for(BOOL Condition_ = 0; Condition_ < 1; Condition_ ++)
#define LOOP_FOREVER while(1)


//#define MakeVM

class ITextCompiler
{
public:
	virtual BOOL NTAPI LoadTextFromBuffer(PBYTE Buffer, ULONG Size) = 0;
	virtual BOOL NTAPI LoadText(LPCWSTR lpPath) = 0;
	virtual BOOL NTAPI ReCompile() = 0;
	virtual BOOL NTAPI QueryText(ULONG Index, LPSTR lpStr) = 0;
	virtual BOOL NTAPI Release() = 0;
};


static BYTE Prefix[] = { 0xE2, 0x97, 0x8F };
static BYTE UTFBOM[] = { 0xEF, 0xBB, 0xBF };

BOOL ProcessLineXmoe(ULONG& Line, string& Info, string& Input);
BOOL ProcessLineOther(ULONG& Line, string& Info, string& Input);

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

unsigned char CPP_ROL_8(unsigned char n, const int bitN)
{
	const int BITLEN = sizeof(unsigned char) * 8;
	n = (n >> (BITLEN - bitN)) | (n << bitN);
	return n;
}

unsigned char CPP_ROR_8(unsigned char n, const int bitN)
{
	const int BITLEN = sizeof(unsigned char) * 8;
	n = (n << (BITLEN - bitN)) | (n >> bitN);
	return n;
}


unsigned char CPP_ROL(unsigned char n, const int bitN)
{
	n = (n >> (bitN)) | (n << bitN);
	return n;
}


unsigned char CPP_ROR(unsigned char n, const int bitN)
{
	n = (n << (bitN)) | (n >> bitN);
	return n;
}



const char * base64char = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";


char *base64_decode(char const *base64Str, char *debase64Str, int encodeStrLen)
{
#ifdef MakeVM
	VMProtectBeginVirtualization("001");
#endif

	int i = 0;
	int j = 0;
	int k = 0;
	char temp[4] = "";

	for (i = 0; i < encodeStrLen; i += 4) {
		for (j = 0; j < 64; j++) {
			if (*(base64Str + i) == base64char[j]) {
				temp[0] = j;
			}
		}

		for (j = 0; j < 64; j++) {
			if (*(base64Str + i + 1) == base64char[j]) {
				temp[1] = j;
			}
		}


		for (j = 0; j < 64; j++) {
			if (*(base64Str + i + 2) == base64char[j]) {
				temp[2] = j;
			}
		}


		for (j = 0; j < 64; j++) {
			if (*(base64Str + i + 3) == base64char[j]) {
				temp[3] = j;
			}
		}

		*(debase64Str + k++) = ((temp[0] << 2) & 0xFC) | ((temp[1] >> 4) & 0x03);
		if (*(base64Str + i + 2) == '=')
			break;

		*(debase64Str + k++) = ((temp[1] << 4) & 0xF0) | ((temp[2] >> 2) & 0x0F);
		if (*(base64Str + i + 3) == '=')
			break;

		*(debase64Str + k++) = ((temp[2] << 6) & 0xF0) | (temp[3] & 0x3F);
	}

#ifdef MakeVM
	VMProtectEnd();
#endif
	return debase64Str;
}

void DecodeString(string& str)
{
#ifdef MakeVM
	VMProtectBeginVirtualization("002");
#endif
	LPSTR Str = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, str.length() * 2);
	base64_decode((char*)str.c_str(), Str, str.length());
	str = Str;
	HeapFree(GetProcessHeap(), 0, Str);

#ifdef MakeVM
	VMProtectEnd();
#endif
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


class TextCompilerV1 : public ITextCompiler
{
public:
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
				LoadText(PathToFile);

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

	BOOL NTAPI QueryText(ULONG Index, LPSTR lpStr)
	{
		BOOL Result;

		LOOP_ONCE
		{
			Result = FALSE;
			auto it = TextPool.find(Index);

			if (it == TextPool.end())
				break;

			if (lpStr)
				lstrcpyA(lpStr, it->second.c_str());

			Result = TRUE;
		}
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
		return Result;
	}

private:

	BOOL NTAPI LoadText(LPWSTR lpFileName)
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

			DecodeString(ReadLine);

			if (!memcmp(ReadLine.c_str(), UTFBOM, 3))
				ReadLine = &ReadLine[3];

			if (ReadLine[0] == ';')
			{
				ResultOfParse = ProcessLineXmoe(Index, CNLine, ReadLine);

				if (ResultOfParse)
					TextPool.insert(std::make_pair(Index, CNLine));
			}
			else if (!memcmp(ReadLine.c_str(), Prefix, 3))
			{
				ResultOfParse = ProcessLineOther(Index, CNLine, ReadLine);

				if (ResultOfParse)
					TextPool.insert(std::make_pair(Index, CNLine));
			}
		}
		return TRUE;
	}


	BOOL NTAPI LoadTextBuffer(PBYTE Buffer, ULONG Size)
	{
		BOOL           ResultOfParse;
		ULONG          Index;
		string         CNLine;
		vector<string> StringPool;
		MemoryBuffer   MemBuffer;

		MemBuffer.Init(Buffer, Size);
		MemBuffer.GetLines(StringPool);

		for (auto& ReadLine : StringPool)
		{
			if (ReadLine.length() == 0)
				continue;

			DecodeString(ReadLine);

			if (!memcmp(ReadLine.c_str(), UTFBOM, 3))
				ReadLine = &ReadLine[3];

			if (ReadLine[0] == ';')
			{
				ResultOfParse = ProcessLineXmoe(Index, CNLine, ReadLine);

				if (ResultOfParse)
					TextPool.insert(std::make_pair(Index, CNLine));
			}
			else if (!memcmp(ReadLine.c_str(), Prefix, 3))
			{
				ResultOfParse = ProcessLineOther(Index, CNLine, ReadLine);

				if (ResultOfParse)
					TextPool.insert(std::make_pair(Index, CNLine));
			}
		}
		return TRUE;

	}

	map<ULONG, string> TextPool;
};


BOOL ProcessLineXmoe(ULONG& Line, string& Info, string& Input)
{
#ifdef MakeVM
	VMProtectBeginVirtualization("003");
#endif

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
#ifdef MakeVM
	VMProtectEnd();
#endif
	return Result;
}


BOOL ProcessLineOther(ULONG& Line, string& Info, string& Input)
{
	BOOL   Result;
	CHAR NumStr[12];
	WCHAR WideStr[2000] = { 0 };
	CHAR  GBKStr[2000] = { 0 };

	LOOP_ONCE
	{
		Result = FALSE;

		if (Input.length() <= 14)
			break;

		if (memcmp(&Input[10], Prefix, 3))
			break;

		RtlZeroMemory(NumStr, sizeof(NumStr));
		RtlCopyMemory(NumStr, &Input[3], 8);
		sscanf(NumStr, "%08d", &Line);
		//Line = atoi(NumStr);

		Info = Input.substr(14, Input.length());
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

