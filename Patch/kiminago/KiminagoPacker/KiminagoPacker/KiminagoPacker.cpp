#include <Windows.h>
#include "CMem.h"
#include "WinFile.h"
#include <string>
#include <vector>

using std::string;
using std::vector;

#define TAG4(ulong) ((ULONG) ulong)

#pragma pack(1)
typedef struct FileHeader
{
	ULONG Sign; //FHFD
	ULONG Count;
}FileHeader;


typedef struct FileChunk
{
	ULONG Sign; //File
	ULONG Offset;
	ULONG Size;
	ULONG Hash; //Sort key
	ULONG StringLen;
	CHAR FileName[260]; //Zero Ending
}FileChunk;

#pragma pack()

/*
ULONG Sign
ULONG Count
*/

ULONG HashProc(const byte *key, ULONG length)
{
	register ULONG nr = 1, nr2 = 4;

	while (length--)
	{
		nr ^= (((nr & 63) + nr2)*((ULONG)(UCHAR)*key++)) + (nr << 8);
		nr2 += 3;
	}
	return((ULONG)nr);
}


int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		return 0;
	}

	vector<string> FilePool;
	vector<FileChunk> ChunkPool;

	string FilePath(__argv[1]);
	FilePath += "\\*.*";

	WCHAR WidePath[260] = { 0 };
	MultiByteToWideChar(CP_ACP, 0, FilePath.c_str(), FilePath.length(), WidePath, 260);

	WIN32_FIND_DATAW FindData;
	HANDLE hFind = FindFirstFileW(WidePath, &FindData);
	if (hFind == INVALID_HANDLE_VALUE)
	{
		FindClose(hFind);
		return -1;
	}

	do
	{
		if (!(FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
			!(FindData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN))
		{
			wprintf(L"Found : %s\n", FindData.cFileName);

			if (FindData.cFileName[0] == L'.' ||
				(FindData.cFileName[0] == L'.' && FindData.cFileName[1] == L'.'))
			{
				continue;
			}
			else
			{
				CHAR NarrowName[260] = { 0 };
				WideCharToMultiByte(CP_ACP, 0, FindData.cFileName, lstrlenW(FindData.cFileName), NarrowName, 260, nullptr, nullptr);
				FilePool.push_back(NarrowName);
			}
		}
	}
	while (FindNextFileW(hFind, &FindData));
	FindClose(hFind);

	FileHeader Header = { 0 };
	Header.Sign = TAG4('FHFD');
	Header.Count = FilePool.size();
	WinFile File;

	if (File.Open(L"Kiminago.Pack", WinFile::FileWrite) != S_OK)
	{
		MessageBoxW(NULL, L"写入失败", NULL, MB_OK);
		ExitProcess(-1);
	}
	File.Write((PBYTE)&Header, sizeof(FileHeader));

	for (auto it : FilePool)
	{
		FileChunk ChunkInfo = {0};
		ChunkInfo.Sign = TAG4('File');
		lstrcpyA(ChunkInfo.FileName, it.c_str());
		ChunkInfo.StringLen = it.length();

		ChunkPool.push_back(ChunkInfo);
	}

	ULONG ChunkSize = sizeof(FileChunk)* ChunkPool.size();

	/*
	ULONG Sign; //File
	ULONG Offset;
	ULONG Size;
	ULONG Hash; //Sort key
	ULONG StringLen;
	wstring FileName; //Zero Ending
	*/
	for (auto it : ChunkPool)
	{
		File.Write((PBYTE)&(it.Sign), 4);
		File.Write((PBYTE)&(it.Offset), 4);
		File.Write((PBYTE)&(it.Size), 4);
		File.Write((PBYTE)&(it.Hash), 4);
		File.Write((PBYTE)&(it.StringLen), 4);
		File.Write((PBYTE)it.FileName, 260);
	}

	ULONG Offset = sizeof(FileHeader)+ChunkSize;
	ULONG BytesTranfered = 0;
	
	int iPos = 0;
	for (auto it : FilePool)
	{
		string FullFileName(__argv[1]);
		FullFileName += "\\";
		FullFileName += it;

		WCHAR WideFullFileName[260] = { 0 };
		MultiByteToWideChar(CP_ACP, 0, FullFileName.c_str(), FullFileName.length(), WideFullFileName, 260);

		WinFile OpenFile;
		if (OpenFile.Open(WideFullFileName, WinFile::FileRead) != S_OK)
		{
			MessageBoxW(NULL, WideFullFileName, L"无法打开", MB_OK);
			ExitProcess(-1);
		}
		PBYTE pBuffer = (PBYTE)CMem::Alloc(OpenFile.GetSize32());
		OpenFile.Read(pBuffer, OpenFile.GetSize32());

#if 0
		wstring DumpName = ChunkPool[iPos].FileName;
		DumpName += L".dump";
		FILE* fin = _wfopen(DumpName.c_str(), L"wb");
		fwrite(pBuffer, 1, OpenFile.GetSize32(), fin);
		fclose(fin);
#endif

		ChunkPool[iPos].Offset = Offset;
		Offset += OpenFile.GetSize32();
		ChunkPool[iPos].Size = OpenFile.GetSize32();
		ChunkPool[iPos].Hash = HashProc((byte*)ChunkPool[iPos].FileName, lstrlenA(ChunkPool[iPos].FileName));
		File.Write(pBuffer, OpenFile.GetSize32());
		CMem::Free(pBuffer);
		OpenFile.Release();
		iPos++;
	}

	File.Seek(sizeof(FileHeader), FILE_BEGIN);
	for (auto it : ChunkPool)
	{
		File.Write((PBYTE)&(it.Sign), 4);
		File.Write((PBYTE)&(it.Offset), 4);
		File.Write((PBYTE)&(it.Size), 4);
		File.Write((PBYTE)&(it.Hash), 4);
		File.Write((PBYTE)&(it.StringLen), 4);
		File.Write((PBYTE)it.FileName, 260);
	}

	File.Release();
	return 0;
}

