#include "FileLoader.h"
#include "NtCore.h"
#include "Log.h"

FileLoader* FileLoader::m_Inst = NULL;

DWORD HeaderKey = 0x45DC1F8AUL;
DWORD CountXor  = 0x54EE31ACUL;

FileLoader::FileLoader() :
	InitFileSystem(FALSE)
{

}

FileLoader* FileLoader::GetFileSystem()
{
	if (!m_Inst)
		m_Inst = new FileLoader();

	return m_Inst;
}


unsigned char XorTable[1024] =
{
	0xb4, 0x63, 0x32, 0xcc, 0xc8, 0x08, 0x44,
	0xe4, 0x87, 0xee, 0x2b, 0x38, 0x69, 0x58, 0x39,
	0xb3, 0xba, 0xfc, 0xd5, 0xf7, 0x1d, 0x83, 0xfa,
	0xd7, 0x07, 0x0c, 0xb8, 0xdc, 0x40, 0xa9, 0xb2,
	0xc0, 0x66, 0xe0, 0x9e, 0xf6, 0x69, 0x2b, 0xc8,
	0x20, 0x10, 0x78, 0x8f, 0x98, 0x72, 0xaa, 0xe6,
	0xe8, 0x7e, 0x17, 0xd5, 0x51, 0x74, 0x08, 0xf6,
	0x48, 0x6a, 0x3c, 0xf8, 0xf4, 0xc8, 0x65, 0x1f,
	0xb2, 0xcb, 0xaa, 0xc2, 0x91, 0x06, 0x23, 0xcb,
	0xd6, 0xdc, 0x60, 0x3b, 0x79, 0x07, 0xe2, 0xa3,
	0xa6, 0x15, 0xa0, 0xac, 0x3d, 0xe6, 0x83, 0x90,
	0x52, 0x2f, 0xea, 0x9f, 0xae, 0xfa, 0x28, 0xbc,
	0x4c, 0x24, 0x01, 0xdd, 0xdd, 0xdd, 0x30, 0x8e,
	0x45, 0x2b, 0xe4, 0x6d, 0x1b, 0x68, 0x3f, 0xaf,
	0x2d, 0xbf, 0xd5, 0x9b, 0xfa, 0xb3, 0x33, 0x0a,
	0x36, 0x98, 0x55, 0xee, 0x49, 0x18, 0x2f, 0xc7,
	0xd0, 0xb0, 0x25, 0x2f, 0x1b, 0x30, 0x93, 0x4f,
	0xad, 0x3e, 0x46, 0x67, 0xbf, 0xd3, 0x00, 0x4b,
	0xbd, 0xbd, 0xf8, 0xe1, 0xc8, 0x1b, 0x58, 0xa4,
	0x32, 0xe5, 0xbe, 0x23, 0x05, 0x61, 0xbb, 0x82,
	0x7d, 0xaf, 0x57, 0xf8, 0x89, 0x3e, 0x8b, 0x50,
	0x4e, 0x54, 0xc5, 0x69, 0xa4, 0x8a, 0x68, 0xb5,
	0x97, 0x4d, 0x49, 0xbe, 0xe7, 0x5f, 0x33, 0x9c,
	0x88, 0x54, 0x63, 0x80, 0x23, 0x16, 0x0d, 0x2d,
	0x93, 0x61, 0xd6, 0x79, 0x69, 0x47, 0x58, 0xd0,
	0x69, 0xad, 0xa1, 0xb1, 0x0a, 0xcc, 0xb4, 0x2f,
	0xfa, 0xb1, 0x06, 0x72, 0x98, 0xbe, 0x02, 0x34,
	0x78, 0x27, 0x86, 0x44, 0xe2, 0x75, 0x64, 0x06,
	0x53, 0x07, 0xe2, 0xf1, 0xfa, 0x8c, 0x3a, 0x0f,
	0x3d, 0x89, 0x1b, 0x81, 0x32, 0xd9, 0x26, 0xf9,
	0x26, 0x29, 0x71, 0x3d, 0x19, 0x78, 0x07, 0xab,
	0x40, 0x9d, 0x67, 0xaf, 0x82, 0xc0, 0x01, 0x4f,
	0xfc, 0xdf, 0xbc, 0xa0, 0x7d, 0x4b, 0x72, 0x4e,
	0x0a, 0x29, 0x72, 0x18, 0x5b, 0xf2, 0xfd, 0x51,
	0x5b, 0xf3, 0xc9, 0x60, 0xac, 0xcd, 0x82, 0x41,
	0x22, 0xf6, 0x44, 0x02, 0x43, 0x36, 0x22, 0x47,
	0xce, 0x2b, 0xa2, 0xc6, 0x30, 0xc6, 0x3f, 0xcc,
	0x10, 0xcc, 0xe6, 0xb6, 0xc4, 0x56, 0x79, 0x79,
	0xda, 0x51, 0x4f, 0x1a, 0x91, 0xfe, 0xb1, 0x36,
	0x5d, 0x73, 0x5f, 0x7c, 0x66, 0x18, 0x09, 0x2e,
	0x09, 0x2b, 0xd6, 0xa4, 0x55, 0x3d, 0xe1, 0xc9,
	0x90, 0xb2, 0xb7, 0x9b, 0xb0, 0x45, 0xda, 0xaf,
	0xe2, 0x82, 0x41, 0xab, 0x06, 0x4a, 0xd6, 0xcb,
	0x31, 0x52, 0xf6, 0x5d, 0x2a, 0xa5, 0xf5, 0x45,
	0xed, 0x1d, 0x98, 0x79, 0x2b, 0xee, 0x98, 0x85,
	0xc8, 0x1c, 0x25, 0x08, 0x5c, 0xff, 0x61, 0x36,
	0xb3, 0xc6, 0xe1, 0x54, 0x4d, 0xf1, 0x30, 0x3f,
	0xde, 0xd5, 0x4c, 0xe5, 0xcf, 0x1c, 0x26, 0xca,
	0xbb, 0x43, 0x26, 0x85, 0xf3, 0x1b, 0xa5, 0x41,
	0xfa, 0x48, 0x71, 0x3c, 0x0a, 0xc5, 0x4d, 0x4b,
	0x8d, 0x5d, 0x6e, 0x54, 0xa5, 0x33, 0xff, 0xd2,
	0xa5, 0x3c, 0x9e, 0x55, 0x95, 0xc0, 0xdd, 0xff,
	0xb2, 0xdc, 0xc2, 0x08, 0xec, 0x03, 0x47, 0x3b,
	0x66, 0x78, 0xda, 0x77, 0xf9, 0xd5, 0xde, 0x30,
	0xb1, 0x88, 0x29, 0xeb, 0x4e, 0x51, 0x84, 0xc5,
	0xc5, 0xc5, 0x2e, 0xec, 0xbd, 0xce, 0x59, 0x24,
	0x12, 0x29, 0xab, 0x43, 0x55, 0xe6, 0xbe, 0xb5,
	0x4a, 0xeb, 0xa0, 0xfa, 0x69, 0x72, 0x54, 0x23,
	0x5e, 0x86, 0x50, 0x59, 0x89, 0x8a, 0xfd, 0x56,
	0x7e, 0xb2, 0x3b, 0xea, 0x86, 0x88, 0xda, 0x77,
	0x1c, 0x68, 0x21, 0x75, 0x71, 0x04, 0x4a, 0xef,
	0xe8, 0xe2, 0x04, 0x04, 0x9b, 0xd9, 0xf0, 0x67,
	0xd4, 0x97, 0x25, 0xdf, 0x95, 0x1e, 0xac, 0xc7,
	0x11, 0x42, 0x05, 0x8f, 0x30, 0x2d, 0xa0, 0x3a,
	0x0f, 0xdb, 0x64, 0xde, 0x7d, 0x9e, 0x2c, 0x27,
	0x7f, 0x9b, 0x45, 0xd5, 0xcd, 0x4b, 0xf1, 0x39,
	0x53, 0xfc, 0xe7, 0xbb, 0xb2, 0x4d, 0xd1, 0x57,
	0xbc, 0xb5, 0xcc, 0x1c, 0xfb, 0xfd, 0xec, 0xac,
	0x2b, 0xc1, 0xb5, 0xbf, 0xbb, 0xf3, 0xa3, 0x9f,
	0x50, 0x58, 0xa3, 0xad, 0x41, 0x09, 0x97, 0xdb,
	0x1c, 0xf3, 0xd6, 0x2f, 0x20, 0x58, 0xaa, 0x47,
	0xc1, 0x4c, 0xd1, 0xd0, 0x28, 0x38, 0xfc, 0x0d,
	0xb0, 0x5a, 0x53, 0x56, 0x6a, 0x43, 0xef, 0x96,
	0x99, 0x58, 0x5e, 0xcc, 0x37, 0x52, 0x23, 0x8b,
	0x6e, 0xbe, 0x33, 0x7b, 0x20, 0x7e, 0x79, 0xd5,
	0x5f, 0x46, 0x53, 0xeb, 0xf6, 0x1f, 0x12, 0x9e,
	0xde, 0xe7, 0x7e, 0xe5, 0xca, 0xcf, 0x50, 0x4d,
	0x9c, 0xdc, 0xb7, 0x73, 0xed, 0x67, 0xd3, 0x8c,
	0x89, 0x9d, 0x3d, 0xdd, 0xf0, 0xff, 0x7d, 0x43,
	0xd7, 0xe3, 0x92, 0xad, 0xa5, 0xf1, 0x6e, 0x9d,
	0xf6, 0xa7, 0x77, 0xab, 0x1b, 0xd6, 0x07, 0x02,
	0x98, 0x22, 0xec, 0xe1, 0xa5, 0x86, 0xe9, 0x1b,
	0xad, 0xce, 0x34, 0x97, 0xd2, 0x1b, 0xf6, 0xd1,
	0x68, 0x63, 0xce, 0x57, 0x75, 0xee, 0x4e, 0x4c,
	0x37, 0xda, 0x7c, 0xe9, 0x9e, 0x98, 0x53, 0xf7,
	0xcd, 0x6c, 0x3f, 0x56, 0x9e, 0xf1, 0xa5, 0x7a,
	0x1b, 0x93, 0x58, 0xe8, 0x06, 0x13, 0x25, 0xbd,
	0x51, 0x06, 0x48, 0x27, 0xa7, 0x56, 0xf4, 0xeb,
	0xe1, 0xc0, 0xcf, 0xdd, 0x92, 0x55, 0x74, 0x6b,
	0x7c, 0xf9, 0xf0, 0x13, 0x18, 0xe7, 0x45, 0xe7,
	0x12, 0x2b, 0xea, 0x10, 0xca, 0x26, 0x48, 0x49,
	0xd5, 0x0d, 0x3f, 0x60, 0x7a, 0x6a, 0x9f, 0xb8,
	0x35, 0x9a, 0xb0, 0xc9, 0x37, 0x4d, 0xaa, 0x9e,
	0xe4, 0x0a, 0x3d, 0x56, 0x53, 0xa8, 0x0b, 0xa5,
	0xd2, 0xd6, 0x29, 0x50, 0x60, 0x94, 0xa1, 0xb4,
	0x31, 0xb7, 0xf3, 0x3f, 0x2e, 0x69, 0x8f, 0xf5,
	0x72, 0xa7, 0x5d, 0xec, 0xcd, 0xc1, 0x36, 0xd1,
	0x45, 0xdd, 0x68, 0x61, 0x90, 0x75, 0x36, 0xf1,
	0x9c, 0xd4, 0x55, 0xe7, 0x07, 0x9d, 0x70, 0x3e,
	0xa7, 0x44, 0xa4, 0x06, 0x03, 0x93, 0x05, 0xe1,
	0xd8, 0x27, 0x18, 0x87, 0x95, 0xf0, 0x57, 0x43,
	0xdf, 0xb4, 0xb0, 0x74, 0x0e, 0x8d, 0x06, 0x0d,
	0xae, 0x66, 0xae, 0x15, 0xff, 0x82, 0xf3, 0x27,
	0x76, 0xf5, 0x93, 0xf3, 0x3a, 0x29, 0x40, 0xbc,
	0xa7, 0x5a, 0x20, 0xd8, 0xce, 0x1a, 0x4d, 0x34,
	0xf3, 0xce, 0x55, 0xcd, 0x0d, 0x30, 0xbb, 0x37,
	0x4a, 0xcb, 0x75, 0x1a, 0x89, 0x82, 0x6c, 0xb0,
	0xde, 0x09, 0xff, 0x48, 0x11, 0x69, 0x80, 0xc7,
	0x1f, 0x81, 0xb5, 0x21, 0xb8, 0x80, 0x58, 0xe4,
	0xc0, 0x6c, 0x98, 0xae, 0xce, 0x9e, 0x96, 0xb2,
	0xaf, 0x43, 0xe8, 0x37, 0xe4, 0xdd, 0x1a, 0x18,
	0x1f, 0xc0, 0x28, 0x45, 0xcb, 0x95, 0x05, 0x40,
	0x81, 0xda, 0x17, 0xa2, 0x94, 0x60, 0xb9, 0x94,
	0x86, 0xcc, 0xb8, 0x56, 0x90, 0x17, 0xd6, 0xbb,
	0x1e, 0x0f, 0x4a, 0xaa, 0x50, 0xd3, 0x3d, 0x9f,
	0x7a, 0x5a, 0x4e, 0x28, 0xa5, 0xec, 0x10, 0x69,
	0x0c, 0xa8, 0x87, 0x99, 0xa0, 0xfc, 0xaf, 0x82,
	0x85, 0x30, 0xf5, 0x05, 0x93, 0xdc, 0xbb, 0x94,
	0xd5, 0x6d, 0xd8, 0xb5, 0x0d, 0xa5, 0x16, 0x86,
	0x2e, 0x17, 0xb2, 0x33, 0xe1, 0xaf, 0xe0, 0x82,
	0x00, 0x28, 0x44, 0x48, 0x1e, 0x94, 0x7a, 0xf0,
	0xfd, 0xd7, 0x8f, 0xfb, 0x17, 0x2c, 0x85, 0x7b,
	0x16, 0x9f, 0xd3, 0x98, 0x5c, 0x92, 0xe3, 0x0b,
	0x7b, 0x38, 0x93, 0xa5, 0xbd, 0x1d, 0xb5, 0xc9,
	0x9e, 0x9b, 0x8e, 0xee, 0x4d, 0x66, 0x5a, 0x1e,
	0x2f, 0x02, 0xc6, 0x79, 0x5c, 0x48, 0x75, 0xb3,
	0x20, 0xe4, 0x7c, 0x91, 0x7b, 0xda, 0xe6, 0x70,
	0xa2, 0xfc, 0x31, 0xbf, 0x7b, 0x75, 0xcf, 0x80,
	0x25
};

void EncodeOrDecode(PBYTE InBuffer, ULONG InSize)
{
	for (ULONG i = 0; i < InSize; i++)
	{
		InBuffer[i] ^= XorTable[i % 1024];
	}
}

NTSTATUS NTAPI FileLoader::Init()
{
	NTSTATUS   Status;
	ChunkInfo* IndexBuffer;
	HeaderInfo Header;
	ULONG      IndexSize;

	LOOP_ONCE
	{
		Status      = STATUS_UNSUCCESSFUL;
		IndexBuffer = NULL;

		if (SystemFile.Open(L"ama_trial.osu", WinFile::FileRead) != S_OK)
		{
			MessageBoxW(NULL, L"无法读取汉化文件", L"错误", MB_OK);
			break;
		}

		SystemFile.Read((PBYTE)&Header, sizeof(HeaderInfo));
		Header.Count ^= CountXor;

		if (Header.Magic != TAG4('PACK'))
		{
			MessageBoxW(NULL, L"破损的文件", L"错误", MB_OK);
			break;
		}
		
		IndexSize = sizeof(ChunkInfo) * Header.Count;

		IndexBuffer = (ChunkInfo*)HeapAlloc(GetProcessHeap(), 0, IndexSize);
		if (!IndexBuffer)
		{
			MessageBoxW(NULL, L"内存不足（初始化错误）", L"错误", MB_OK);
			break;
		}

		SystemFile.Read((PBYTE)IndexBuffer, IndexSize);
		EncodeOrDecode((PBYTE)IndexBuffer, IndexSize);

		for (ULONG i = 0; i < Header.Count; i++)
		{
			ChunkAtom Item;
			Item.Offset = IndexBuffer[i].Offset;
			Item.Size = IndexBuffer[i].Size;
			RtlCopyMemory(Item.lpFileName, IndexBuffer[i].lpFileName, MAX_PATH);

			ChunkList.insert(std::make_pair(IndexBuffer[i].HashName, Item));
		}
		InitFileSystem = TRUE;
		Status         = STATUS_SUCCESS;
	}
	return Status;
}


ULONG WINAPI AnsiToUnicode(
	LPCSTR lpAnsi,
	ULONG  Length,
	LPWSTR lpUnicodeBuffer,
	ULONG  BufferCount,
	ULONG  CodePage = CP_ACP)
{
	return MultiByteToWideChar(CodePage, 0, lpAnsi, Length, lpUnicodeBuffer, BufferCount);
}


wstring WINAPI GetPackageName(wstring& fileName)
{
	wstring temp(fileName);
	wstring::size_type pos = temp.find_last_of(L"\\");

	if (pos != wstring::npos)
	{
		temp = temp.substr(pos + 1, temp.length());
	}

	wstring temp2(temp);
	wstring::size_type pos2 = temp2.find_last_of(L"/");
	if (pos2 != wstring::npos)
	{
		temp2 = temp2.substr(pos + 1, temp2.length());
	}
	return temp2;
}


ULONG64 NTAPI Hash64(const void * key, LONG len, ULONG seed = 0xEE6B27EB)
{
	const unsigned int m = 0x5bd1e995;
	const int r = 24;

	unsigned int h1 = seed ^ len;
	unsigned int h2 = 0;

	const unsigned int * data = (const unsigned int *)key;

	while (len >= 8)
	{
		unsigned int k1 = *data++;
		k1 *= m; k1 ^= k1 >> r; k1 *= m;
		h1 *= m; h1 ^= k1;
		len -= 4;

		unsigned int k2 = *data++;
		k2 *= m; k2 ^= k2 >> r; k2 *= m;
		h2 *= m; h2 ^= k2;
		len -= 4;
	}

	if (len >= 4)
	{
		unsigned int k1 = *data++;
		k1 *= m; k1 ^= k1 >> r; k1 *= m;
		h1 *= m; h1 ^= k1;
		len -= 4;
	}

	switch (len)
	{
	case 3: h2 ^= ((unsigned char*)data)[2] << 16;
	case 2: h2 ^= ((unsigned char*)data)[1] << 8;
	case 1: h2 ^= ((unsigned char*)data)[0];
		h2 *= m;
	};

	h1 ^= h2 >> 18; h1 *= m;
	h2 ^= h1 >> 22; h2 *= m;
	h1 ^= h2 >> 17; h1 *= m;
	h2 ^= h1 >> 19; h2 *= m;

	unsigned long long h = h1;

	h = (h << 32) | h2;

	return h;
}



#define ProjectDir L"ProjectDir\\"

NTSTATUS NTAPI FileLoader::LoadFile(LPCSTR FileName, PBYTE& Buffer, SIZE_T& Size)
{
	NTSTATUS Status;
	WCHAR    FileNameUnicode[MAX_PATH];
	WCHAR    Path[MAX_PATH];
	WinFile  File;
	DWORD    OutSize;

	RtlZeroMemory(FileNameUnicode, countof(FileNameUnicode) * sizeof(FileNameUnicode[0]));
	RtlZeroMemory(Path,            countof(Path) * sizeof(Path[0]));
	AnsiToUnicode(FileName, lstrlenA(FileName), FileNameUnicode, MAX_PATH);

	LOOP_ONCE
	{
		Status = STATUS_UNSUCCESSFUL;
		Buffer = NULL;
		Size   = 0;

		if (!InitFileSystem)
		{
			lstrcatW(Path, ProjectDir);
			lstrcatW(Path, FileNameUnicode);

			//OutputInfo(Path);

			if (File.Open(Path, WinFile::FileRead) != S_OK)
				break;

			OutSize = File.GetSize32();
			Alloc(OutSize, Buffer);

			if (Buffer == nullptr)
				break;

			File.Read(Buffer, OutSize);
			File.Release();
			Size = OutSize;

			Status = STATUS_SUCCESS;
		}
		else
		{
			auto it = ChunkList.find(Hash64(FileNameUnicode, lstrlenW(FileNameUnicode) * 2));
			if (it != ChunkList.end())
			{
				OutSize = it->second.Size;
				Alloc(OutSize, Buffer);

				if (Buffer == nullptr)
					break;

				SystemFile.Seek(it->second.Offset, FILE_BEGIN);
				SystemFile.Read(Buffer, OutSize);
				Size = OutSize;

				Status = STATUS_SUCCESS;
			}
		}
	}
	return Status;
}


NTSTATUS NTAPI FileLoader::Alloc(SIZE_T Size, LPBYTE& lpMem)
{
	lpMem = NULL;

	__try
	{
		lpMem = (LPBYTE)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, Size);
	}
	__except (-1)
	{
	}

	return lpMem ? STATUS_SUCCESS : STATUS_UNSUCCESSFUL;
}

NTSTATUS NTAPI FileLoader::Free(LPBYTE& lpMem)
{
	BOOL Sucess;

	__try
	{
		Sucess = HeapFree(GetProcessHeap(), NULL, lpMem);
	}
	__except (-1)
	{
		Sucess = FALSE;
	}
	lpMem = NULL;

	return Sucess ? STATUS_SUCCESS : STATUS_UNSUCCESSFUL;
}
