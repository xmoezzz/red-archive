// HarunoPacker.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <cstdio>
#include <string>
#include <vector>
#include <windows.h>

struct FileEntry
{
	std::wstring name_;
	unsigned offset_;
	unsigned size_;
	unsigned checksum_;
};

std::wstring basePath_;
std::vector<FileEntry> entries_;
unsigned checksum_ = 0;

static std::vector<std::wstring> arguments;

int main(int argc, char** argv);
void Run(const std::vector<std::wstring>& arguments);
void ProcessFile(const std::wstring& fileName, const std::wstring& rootDir);
void WritePackageFile(const std::wstring& fileName, const std::wstring& rootDir);
void WriteHeader(FILE* dest);
DWORD GetSize(FILE* file);
void ScanDir(std::vector<std::wstring>& result, const std::wstring& path, std::wstring& file);
const std::vector<std::wstring>& ParseArguments(const std::wstring& cmdLine, bool skipFirstArgument = true);
inline unsigned SDBMHash(unsigned hash, unsigned char c) { return c + (hash << 6) + (hash << 16) - hash; }

int main(int argc, char** argv)
{
	std::vector<std::wstring> arguments;

	arguments = ParseArguments(std::wstring(GetCommandLineW()));

	Run(arguments);
	return 0;
}

void Run(const std::vector<std::wstring>& arguments)
{
	if (arguments.size() < 2)
	{
		printf(
			"Usage: Packager(For Haruno only) <directory to process> <package name>\n"
			);
		ExitProcess(-1);
	}

	const std::wstring& dirName = arguments[0];
	const std::wstring& packageName = arguments[1];

	wprintf(std::wstring(L"Scanning directory " + dirName + L" for files\n").c_str());

	// Get the file list recursively
	std::vector<std::wstring> fileNames;
	ScanDir(fileNames, dirName, std::wstring(L"*.*"));
	if (!fileNames.size())
	{
		wprintf(L"No files found\n");
		ExitProcess(-1);
	}

	for (unsigned i = 0; i < fileNames.size(); ++i)
		ProcessFile(fileNames[i], dirName);

	WritePackageFile(packageName, dirName);
}

void ProcessFile(const std::wstring& fileName, const std::wstring& rootDir)
{
	std::wstring fullPath = rootDir + L"\\" + fileName;
	FILE* file = NULL;
	file = _wfopen(fullPath.c_str(), L"rb");
	if (!file)
	{
		wprintf(L"Could not open file ", fileName.c_str());
		ExitProcess(-1);
	}
	if (!GetSize(file))
	{
		fclose(file);
		return;
	}

	FileEntry newEntry;
	newEntry.name_ = fileName;
	newEntry.offset_ = 0; // Offset not yet known
	newEntry.size_ = GetSize(file);
	newEntry.checksum_ = 0; // Will be calculated later
	entries_.push_back(newEntry);
	fclose(file);
}

#pragma pack (1)
union FileNameKeyInfo
{
	unsigned char c_key[8];
	WORD          w_key[4];
	DWORD         d_key[2];
};
#pragma pack ()

int DecodeFileName(wchar_t* in_file, size_t len)
{
	FileNameKeyInfo Info;
	memcpy(Info.c_key, "kdus*30.d(@~xpag", 16);
	Info.d_key[0] ^= 0x36F51A97;
	Info.d_key[1] ^= 0x65DE7A20;

	Info.w_key[0] ^= 0xFCFC;
	Info.w_key[1] ^= 0x561F;
	Info.w_key[2] ^= 0xCA1A;
	Info.w_key[3] ^= 0x61F7;

	int i;
	for (i = 0; i < len; i++)
	{
		in_file[i] ^= Info.w_key[i % 4];
	}
	return i;
}

union RecordInfo
{
	DWORD size;
	char  c_key[4];
};


DWORD EncodeSize(DWORD in)
{
	RecordInfo info;
	memcpy(info.c_key, "ST,.", 4);

	for (int i = 0; i < 3; i++)
	{
		info.c_key[i] ^= 0x69;
	}
	return in ^ info.size;
}

DWORD EncodeOffset(DWORD in)
{
	RecordInfo info;
	memcpy(info.c_key, "ZXDF", 4);

	for (int i = 0; i < 3; i++)
	{
		info.c_key[i] ^= 0x5F;
	}
	return in ^ info.size;
}


void DecodeFile2(byte* debuf, size_t desize)
{
	size_t key_idx = 0;
	size_t xor_idx = 0;

	static byte key[] =
	{
		0xFE, 0xF8, 0xE7, 0xB0, 0xE5, 0xE5, 0x28, 0x4F, 0xB5, 0x2F, 0x48, 0xFE, 0xE5, 0xE9, 0x4B, 0xDE,
		0xB7, 0x4F, 0x72, 0x95, 0x8B, 0xE0, 0x03, 0x80, 0xE7, 0xCF, 0x0F, 0x7B, 0x92, 0x05, 0xEB, 0xF8,
		0xE2, 0x88, 0xCE, 0x73, 0x04, 0x38, 0xD2, 0x7D, 0x8C, 0xD2, 0x88, 0x77, 0xE7, 0x92, 0x75, 0x8F,
		0x4E, 0xB7, 0x8D, 0x05, 0x79, 0x88, 0x83, 0x0E, 0xF9, 0xE9, 0x2C, 0xDB, 0x77, 0xDB, 0x95, 0x54,
		0xD5, 0x9E, 0x4E, 0x79, 0x57, 0x23, 0x08, 0x97, 0x0E, 0x5D, 0x55, 0xF9, 0xE5, 0xE0, 0x7F, 0x58,
		0x57, 0xC8, 0xE9, 0x47, 0xDE, 0x22, 0xFF, 0xFD, 0x87, 0x52, 0x42, 0xFB, 0xE9, 0xB8, 0x77, 0x7C,
		0x95, 0x77, 0x74, 0xF9, 0xD5, 0x5E, 0xE4, 0x50, 0x74, 0x7F, 0xF2, 0x0B, 0xDE, 0x40, 0xE7, 0x47,
		0xF5, 0x03, 0xCC, 0x2E, 0xED, 0x7F, 0x34, 0x25, 0xE0, 0x74, 0x27, 0x98, 0x7C, 0xED, 0x79, 0xF4,
		0xB5, 0x23, 0x08, 0x7E, 0x7D, 0x92, 0xF7, 0xEB, 0x93, 0xF0, 0x7E, 0x89, 0x5E, 0xF9, 0xF8, 0x7E,
		0xEF, 0xE8, 0xE9, 0x48, 0xC2, 0xEC, 0x55, 0x7B, 0x2B, 0x33, 0xE7, 0x40, 0x0D, 0xDC, 0x7D, 0xE7,
		0x5B, 0xCF, 0xC8, 0x35, 0xD5, 0x77, 0x52, 0x8D, 0x82, 0xEC, 0x45, 0xB8, 0x73, 0xE5, 0x4F, 0x27,
		0x7C, 0x0F, 0x39, 0xDE, 0x5B, 0x37, 0x4E, 0xDE, 0xE4, 0x49, 0x0B, 0x7C, 0x57, 0xE3, 0x43, 0xEE,
		0x77, 0x07, 0x74, 0x73, 0xC0, 0x43, 0xE3, 0x58, 0x5E, 0x0F, 0x9F, 0x02, 0x4C, 0x7E, 0x8B, 0x05,
		0x9F, 0x2D, 0xEE, 0x72, 0x54, 0x53, 0xFF, 0x97, 0xEE, 0x0B, 0x34, 0x58, 0xCF, 0xE3, 0x00, 0x78,
		0xBE, 0xE3, 0xF5, 0x75, 0xE4, 0x87, 0x7C, 0xFC, 0x80, 0xEF, 0xC4, 0x8D, 0x47, 0x3E, 0x5D, 0xD0,
		0x37, 0xBC, 0xE5, 0x70, 0x77, 0x78, 0x08, 0x4F, 0xBB, 0xEB, 0xE2, 0x78, 0x07, 0xE8, 0x73, 0xBF,
		0xD8, 0x29, 0xB9, 0x57, 0x3D, 0x5E, 0x77, 0xD0, 0x87, 0x9B, 0x2D, 0x0C, 0x7B, 0xD5, 0xE9, 0x59,
		0x22, 0x9F, 0x95, 0x73, 0x7E, 0x35, 0xB5, 0x7E, 0xD5, 0xB5, 0xE7, 0xE7, 0xD5, 0xF5, 0x07, 0xD7,
		0xBE, 0xBF, 0xF3, 0x45, 0x3F, 0xF5, 0x75, 0xDD, 0x4C, 0x77, 0x7E, 0x7F, 0x74, 0xEC, 0x7E, 0x7F,
		0x27, 0x74, 0x0E, 0xDB, 0x27, 0x4C, 0xE5, 0xF5, 0x0E, 0x2D, 0x70, 0xC4, 0x40, 0x5D, 0x4F, 0xDE
	};
	for (xor_idx = 0; xor_idx < desize; xor_idx++, key_idx++, key_idx = key_idx & 0x800000FF)
	{
		debuf[xor_idx] ^= key[key_idx];
	}

}

void WritePackageFile(const std::wstring& fileName, const std::wstring& rootDir)
{
	printf("Writing package\n");

	FILE *dest = NULL;
	dest = _wfopen(fileName.c_str(), L"wb");
	if (!dest)
	{
		wprintf(L"Could not open output file %s\n", fileName.c_str());
		ExitProcess(-1);
	}

	unsigned dataOffset = 0xc;
	// Write ID, number of files & placeholder for checksum
	WriteHeader(dest);

	for (unsigned i = 0; i < entries_.size(); ++i)
	{
		// Write entry (correct offset is still unknown, will be filled in later)
		fwrite((const char*)entries_[i].name_.c_str(), 1, (entries_[i].name_.length() + 1) * 2, dest);
		fwrite(&(entries_[i].offset_), 1, 4, dest);
		fwrite(&(entries_[i].size_), 1, 4, dest);
		fwrite(&(entries_[i].checksum_), 1, 4, dest);
		dataOffset += (entries_[i].name_.length() + 1) * 2;
		dataOffset += 0xc;
	}

	unsigned totalDataSize = 0;

	// Write file data, calculate checksums & correct offsets
	unsigned lastSize = 0;
	for (unsigned i = 0; i < entries_.size(); ++i)
	{
		entries_[i].offset_ = dataOffset + lastSize;
		std::wstring fileFullPath = rootDir + L"\\" + entries_[i].name_;

		FILE* srcFile = NULL;
		srcFile = _wfopen(fileFullPath.c_str(), L"rb");
		if (!srcFile)
		{
			wprintf(std::wstring(L"Could not open file " + fileFullPath + L"\n").c_str());
			ExitProcess(-1);
		}

		entries_[i].size_ = GetSize(srcFile);
		unsigned dataSize = entries_[i].size_;
		lastSize += dataSize;
		totalDataSize += dataSize;
		unsigned char* buffer = new unsigned char[dataSize];

		if (fread(&buffer[0], 1, dataSize, srcFile) != dataSize)
		{
			wprintf(std::wstring(L"Could not read file " + fileFullPath + L"\n").c_str());
			fclose(srcFile);
			ExitProcess(-1);
		}
		fclose(srcFile);

		for (unsigned j = 0; j < dataSize; ++j)
		{
			checksum_ = SDBMHash(checksum_, buffer[j]);
			entries_[i].checksum_ = SDBMHash(entries_[i].checksum_, buffer[j]);
		}

		fwrite(&buffer[0], 1, entries_[i].size_, dest);
		delete[] buffer;
	}

	// Write package size to the end of file to allow finding it linked to an executable file
	unsigned currentSize = GetSize(dest);
	unsigned writeSize = currentSize + sizeof(unsigned);
	fwrite(&writeSize, 1, 4, dest);

	// Write header again with correct offsets & checksums
	rewind(dest);

	checksum_ = dataOffset;
	WriteHeader(dest);

	byte* pBuf = new byte[dataOffset - 12];
	DWORD iPos = 0;
	for (unsigned i = 0; i < entries_.size(); ++i)
	{
		/*
		wchar_t* pBuf = new wchar_t[entries_[i].name_.length()];
		wmemcpy(pBuf, entries_[i].name_.c_str(), entries_[i].name_.length());
		DecodeFileName(pBuf, entries_[i].name_.length());
		*/

		fwrite((const char*)entries_[i].name_.c_str(), 1, (entries_[i].name_.length() + 1) * 2, dest);
		memcpy((pBuf + iPos), (const char*)entries_[i].name_.c_str(), (entries_[i].name_.length() + 1) * 2);
		iPos += (entries_[i].name_.length() + 1) * 2;

		DWORD newOffset = entries_[i].offset_;
		fwrite(&newOffset, 1, 4, dest);
		memcpy((pBuf + iPos), &newOffset, 4);
		iPos += 4;

		DWORD newSize = entries_[i].size_;
		fwrite(&newSize, 1, 4, dest);
		memcpy((pBuf + iPos), &newSize, 4);
		iPos += 4;

		fwrite(&(entries_[i].checksum_), 1, 4, dest);
		memcpy((pBuf + iPos), &(entries_[i].checksum_), 4);
		iPos += 4;
	}

	FILE* index = fopen("index.data", "wb");
	fwrite(pBuf, 1, dataOffset - 12, index);
	fclose(index);
	DecodeFile2(pBuf, dataOffset - 12);

	fseek(dest, 12, SEEK_SET);
	fwrite(pBuf, 1, dataOffset - 12, dest);
	fclose(dest);
}


DWORD EncodeHeaderSize(DWORD in)
{
	RecordInfo info;
	memcpy(info.c_key, "-2dM", 4);

	for (int i = 0; i < 3; i++)
	{
		info.c_key[i] ^= 0x7B;
	}
	return in ^ info.size;
}

DWORD EncodeHeaderOffset(DWORD in)
{
	RecordInfo info;
	memcpy(info.c_key, "+=dw", 4);

	for (int i = 0; i < 3; i++)
	{
		info.c_key[i] ^= 0x4D;
	}
	return in ^ info.size;
}

//12
void WriteHeader(FILE* dest)
{
	fwrite("XMOE", 1, 4, dest);
	DWORD size = entries_.size();
	fwrite(&size, 1, 4, dest);
	DWORD Offset = checksum_;
	fwrite(&Offset, 1, 4, dest);//FileDataoffset
}


DWORD GetSize(FILE* file)
{
	if (!file)
		return 0;
	else
	{
		fseek(file, 0, SEEK_END);
		DWORD FileSize = ftell(file);
		rewind(file);
		return FileSize;
	}
}


void ScanDir(std::vector<std::wstring>& result, const std::wstring& path, std::wstring& file)
{
	WIN32_FIND_DATAW info;
	HANDLE handle = FindFirstFileW(std::wstring(path + L"\\" + file).c_str(), &info);
	if (handle != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (info.cFileName[0] != _T('.'))
			{
				std::wstring fileName(info.cFileName);
				if (!fileName.empty())
				{
					if (info.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)
						continue;
					if (info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
					{
						continue;
					}
					else
					{
						result.push_back(fileName);
					}
				}
			}
		} while (FindNextFileW(handle, &info));

		FindClose(handle);
	}
}


void ReplaceString(std::wstring& str, const std::wstring& src, const std::wstring& des)
{
	int pos = -1;
	int curPos = 0;
	while (-1 != (pos = str.find(src, curPos)))
	{
		str.replace(pos, src.size(), des);
		curPos = pos + des.size();
	}
}

const std::vector<std::wstring>& ParseArguments(const std::wstring& cmdLine, bool skipFirstArgument)
{
	arguments.clear();

	unsigned cmdStart = 0, cmdEnd = 0;
	bool inCmd = false;
	bool inQuote = false;

	for (unsigned i = 0; i < cmdLine.length(); ++i)
	{
		if (cmdLine[i] == '\"')
			inQuote = !inQuote;
		if (cmdLine[i] == ' ' && !inQuote)
		{
			if (inCmd)
			{
				inCmd = false;
				cmdEnd = i;
				// Do not store the first argument (executable name)
				if (!skipFirstArgument)
					arguments.push_back(cmdLine.substr(cmdStart, cmdEnd - cmdStart));
				skipFirstArgument = false;
			}
		}
		else
		{
			if (!inCmd)
			{
				inCmd = true;
				cmdStart = i;
			}
		}
	}
	if (inCmd)
	{
		cmdEnd = cmdLine.length();
		if (!skipFirstArgument)
			arguments.push_back(cmdLine.substr(cmdStart, cmdEnd - cmdStart));
	}

	// Strip double quotes from the arguments
	for (unsigned i = 0; i < arguments.size(); ++i)
		ReplaceString(arguments[i], L"\"", L"");

	return arguments;
}


