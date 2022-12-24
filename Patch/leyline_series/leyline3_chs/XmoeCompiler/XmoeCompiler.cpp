//X'moe Compiler V2.0
//With Text-Based Decoder

#include <Windows.h>
#include <fstream>
#include <string>
#include <vector>
#include <map>


using std::fstream;
using std::wfstream;
using std::string;
using std::wstring;
using std::vector;
using std::map;

#define LOOP_ONCE for(BOOL Condition_ = 0; Condition_ < 1; Condition_ ++)
#define LOOP_FOREVER while(1)

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

char *base64_encode(char *binData, char *base64, int binLength)
{
	int i = 0;
	int j = 0;
	int current = 0;
	for (i = 0; i < binLength; i += 3) {

		//获取第一个6位
		current = (*(binData + i) >> 2) & 0x3F;
		*(base64 + j++) = base64char[current];

		//获取第二个6位的前两位
		current = (*(binData + i) << 4) & 0x30;

		//如果只有一个字符，那么需要做特殊处理
		if (binLength <= (i + 1)) {
			*(base64 + j++) = base64char[current];
			*(base64 + j++) = '=';
			*(base64 + j++) = '=';
			break;
		}

		//获取第二个6位的后四位
		current |= (*(binData + i + 1) >> 4) & 0xf;
		*(base64 + j++) = base64char[current];
		//获取第三个6位的前四位
		current = (*(binData + i + 1) << 2) & 0x3c;
		if (binLength <= (i + 2)) {
			*(base64 + j++) = base64char[current];
			*(base64 + j++) = '=';
			break;
		}

		//获取第三个6位的后两位
		current |= (*(binData + i + 2) >> 6) & 0x03;
		*(base64 + j++) = base64char[current];

		//获取第四个6位
		current = *(binData + i + 2) & 0x3F;
		*(base64 + j++) = base64char[current];
	}
	*(base64 + j) = '\0';

	return base64;
}




char *base64_decode(char const *base64Str, char *debase64Str, int encodeStrLen)
{
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
	return debase64Str;
}

void DecodeString(string& str)
{
	LPSTR Str = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, str.length() * 2);
	base64_decode((char*)str.c_str(), Str, str.length());
	str = Str;
	HeapFree(GetProcessHeap(), 0, Str);
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


//leyline 3
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

			//OutputInfo(it->second.c_str(), 936);

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

			//OutputInfo(ReadLine.c_str());
			if (ReadLine[0] == '[')
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

			if (ReadLine[0] == '[')
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
	BOOL Result;
	CHAR NumStr[12];
	WCHAR WideStr[2000] = { 0 };
	CHAR  GBKStr[2000] = { 0 };

	LOOP_ONCE
	{
		Result = FALSE;

		if (Input.length() <= 10)
			break;

		if (Input[9] != ']')
			break;

		RtlZeroMemory(NumStr, sizeof(NumStr));
		RtlCopyMemory(NumStr, &Input[1], 8);
		sscanf(NumStr, "%08d", &Line);
		//Line = atoi(NumStr);
		Info = Input.substr(10, Input.length());

		//OutputInfo(NumStr);
		//OutputInfo(Info.c_str());

		MultiByteToWideChar(CP_UTF8, 0, Info.c_str(), Info.length(), WideStr, 2000);
		WideCharToMultiByte(936, 0, WideStr, lstrlenW(WideStr), GBKStr, 2000, 0, 0);
		Info = GBKStr;
		Result = TRUE;
	}
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

