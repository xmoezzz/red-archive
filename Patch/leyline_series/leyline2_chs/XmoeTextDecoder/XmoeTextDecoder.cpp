#include <Windows.h>
#include <stdio.h>
#include <string>
#include <fstream>

using std::string;
using std::wstring;
using std::fstream;

// FF FF F0 F0 -- The beginning of each filter text
// The last byte of header refers to an encoding mothod
// The 3rd byte of header refers to a parameter used in related encoder
// the first 4 bits refers to an encoder
// the next 4 bits refers to the parameter of encoder
// FF FF FF FF -- The end mark of filter text


// FF FF (not in Unicode area)
// FF (mask, as a private section key)
// FF (method + parameter, the parameter mush be encrypted.)
// In order to avoid weak key, the master encoding engine must generate a sub-key.

template<class T>
T CPP_ROL(T n, const int bitN)
{
	const int BITLEN = sizeof(T) * 8;
	n = (n >> (BITLEN - bitN)) | (n << bitN);
	return n;
}

template<class T>
T CPP_ROR(T n, const int bitN)
{
	const int BITLEN = sizeof(T) * 8;
	n = (n << (BITLEN - bitN)) | (n >> bitN);
	return n;
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


unsigned int
reverse(register unsigned int x)
{
	x = (((x & 0xaaaaaaaa) >> 1) | ((x & 0x55555555) << 1));
	x = (((x & 0xcccccccc) >> 2) | ((x & 0x33333333) << 2));
	x = (((x & 0xf0f0f0f0) >> 4) | ((x & 0x0f0f0f0f) << 4));
	x = (((x & 0xff00ff00) >> 8) | ((x & 0x00ff00ff) << 8));
	return((x >> 16) | (x << 16));
}

static const unsigned char BitReverseTable256[] =
{
	0x00, 0x80, 0x40, 0xC0, 0x20, 0xA0, 0x60, 0xE0, 0x10, 0x90, 0x50, 0xD0, 0x30, 0xB0, 0x70, 0xF0,
	0x08, 0x88, 0x48, 0xC8, 0x28, 0xA8, 0x68, 0xE8, 0x18, 0x98, 0x58, 0xD8, 0x38, 0xB8, 0x78, 0xF8,
	0x04, 0x84, 0x44, 0xC4, 0x24, 0xA4, 0x64, 0xE4, 0x14, 0x94, 0x54, 0xD4, 0x34, 0xB4, 0x74, 0xF4,
	0x0C, 0x8C, 0x4C, 0xCC, 0x2C, 0xAC, 0x6C, 0xEC, 0x1C, 0x9C, 0x5C, 0xDC, 0x3C, 0xBC, 0x7C, 0xFC,
	0x02, 0x82, 0x42, 0xC2, 0x22, 0xA2, 0x62, 0xE2, 0x12, 0x92, 0x52, 0xD2, 0x32, 0xB2, 0x72, 0xF2,
	0x0A, 0x8A, 0x4A, 0xCA, 0x2A, 0xAA, 0x6A, 0xEA, 0x1A, 0x9A, 0x5A, 0xDA, 0x3A, 0xBA, 0x7A, 0xFA,
	0x06, 0x86, 0x46, 0xC6, 0x26, 0xA6, 0x66, 0xE6, 0x16, 0x96, 0x56, 0xD6, 0x36, 0xB6, 0x76, 0xF6,
	0x0E, 0x8E, 0x4E, 0xCE, 0x2E, 0xAE, 0x6E, 0xEE, 0x1E, 0x9E, 0x5E, 0xDE, 0x3E, 0xBE, 0x7E, 0xFE,
	0x01, 0x81, 0x41, 0xC1, 0x21, 0xA1, 0x61, 0xE1, 0x11, 0x91, 0x51, 0xD1, 0x31, 0xB1, 0x71, 0xF1,
	0x09, 0x89, 0x49, 0xC9, 0x29, 0xA9, 0x69, 0xE9, 0x19, 0x99, 0x59, 0xD9, 0x39, 0xB9, 0x79, 0xF9,
	0x05, 0x85, 0x45, 0xC5, 0x25, 0xA5, 0x65, 0xE5, 0x15, 0x95, 0x55, 0xD5, 0x35, 0xB5, 0x75, 0xF5,
	0x0D, 0x8D, 0x4D, 0xCD, 0x2D, 0xAD, 0x6D, 0xED, 0x1D, 0x9D, 0x5D, 0xDD, 0x3D, 0xBD, 0x7D, 0xFD,
	0x03, 0x83, 0x43, 0xC3, 0x23, 0xA3, 0x63, 0xE3, 0x13, 0x93, 0x53, 0xD3, 0x33, 0xB3, 0x73, 0xF3,
	0x0B, 0x8B, 0x4B, 0xCB, 0x2B, 0xAB, 0x6B, 0xEB, 0x1B, 0x9B, 0x5B, 0xDB, 0x3B, 0xBB, 0x7B, 0xFB,
	0x07, 0x87, 0x47, 0xC7, 0x27, 0xA7, 0x67, 0xE7, 0x17, 0x97, 0x57, 0xD7, 0x37, 0xB7, 0x77, 0xF7,
	0x0F, 0x8F, 0x4F, 0xCF, 0x2F, 0xAF, 0x6F, 0xEF, 0x1F, 0x9F, 0x5F, 0xDF, 0x3F, 0xBF, 0x7F, 0xFF
};


unsigned int
reverse_fast(register unsigned int v)
{
	unsigned int c; // c will get v reversed

	// Option 1:
	c = (BitReverseTable256[v & 0xff] << 24) |
		(BitReverseTable256[(v >> 8) & 0xff] << 16) |
		(BitReverseTable256[(v >> 16) & 0xff] << 8) |
		(BitReverseTable256[(v >> 24) & 0xff]);

	return c;
}

unsigned char
reverse_char_fast32(register unsigned char x)
{
	unsigned char b = x;
	b = ((b * 0x0802LU & 0x22110LU) | (b * 0x8020LU & 0x88440LU)) * 0x10101LU >> 16;
	return b;
}

unsigned char
reverse_char_fast64(register unsigned char b)
{
	b = (b * 0x0202020202ULL & 0x010884422010ULL) % 1023;
	return b;
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

/*****************************************************/

//00 Shift
string EncodeStringByShift(LPCSTR Code)
{
	return NULL;
}

//01 Xor (do nothing if source code equal to xor key)
string EncodeStringByXor(LPCSTR Code)
{
	return NULL;
}

//02 Reversal
string EncodeStringByReversal(LPCSTR Code)
{
	return NULL;
}

#if 0
//03 Not
string EncodeStringByNot(LPCSTR Code)
{
	return NULL;
}

//04 Base64
string EncodeStringByBase64(LPCSTR Code)
{
	return NULL;
}

//05
string EncodeStringBy(LPCSTR Code)
{
	return NULL;
}

//06
string EncodeStringBy(LPCSTR Code)
{
	return NULL;
}

//07
string EncodeStringBy(LPCSTR Code)
{
	return NULL;
}

//08
string EncodeStringBy(LPCSTR Code)
{
	return NULL;
}

//09
string EncodeStringBy(LPCSTR Code)
{
	return NULL;
}

//0A
string EncodeStringBy(LPCSTR Code)
{
	return NULL;
}

//0B
string EncodeStringBy(LPCSTR Code)
{
	return NULL;
}

//0C
string EncodeStringBy(LPCSTR Code)
{
	return NULL;
}

//0D
string EncodeStringBy(LPCSTR Code)
{
	return NULL;
}

//0E
string EncodeStringBy(LPCSTR Code)
{
	return NULL;
}

//0F
string EncodeStringBy(LPCSTR Code)
{
	return NULL;
}
#endif


void EncodeString(string& str)
{
	LPSTR Base64 = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, str.length() * 4);
	base64_encode((char*)str.c_str(), Base64, str.length());
	str = Base64;
	HeapFree(GetProcessHeap(), 0, Base64);
}

void DecodeString(string& str)
{
	LPSTR Str = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, str.length() * 2);
	base64_decode((char*)str.c_str(), Str, str.length());
	str = Str;
	HeapFree(GetProcessHeap(), 0, Str);
}

int wmain(int argc, WCHAR* argv[])
{
	fstream        File;
	string         ReadLine;
	ULONG          Index;
	wstring        OutName;
	BOOL           ResultOfParse;
	FILE*          FileOut;

	if (argc != 2)
		return 0;

	OutName = argv[1] + wstring(L".txt");

	FileOut = _wfopen(OutName.c_str(), L"wb");

	File.open(argv[1], std::ios_base::in | std::ios_base::out);
	while (getline(File, ReadLine))
	{
		//EncodeString(ReadLine);
		DecodeString(ReadLine);
		fprintf(FileOut, "%s\r\n", ReadLine.c_str());
	}
	fclose(FileOut);
	File.close();
	return 0;
}

