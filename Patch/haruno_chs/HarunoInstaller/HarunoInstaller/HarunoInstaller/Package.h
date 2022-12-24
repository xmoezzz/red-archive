#ifndef _Package_
#define _Package_

#include <afx.h>
#include <string>
#include <vector>

struct PackageEntry
{
    /// Offset from the beginning.
    unsigned offset_;
    /// File size.
    unsigned size_;
    /// File checksum.
    unsigned checksum_;
};

struct Header
{
	char     sign_[4];
	unsigned count_;
	unsigned offset_;
};

struct FileBuf
{
	std::wstring name_;
	/// Offset from the beginning.
    unsigned offset_;
    /// File size.
    unsigned size_;
    /// File checksum.
    unsigned checksum_;
};


DWORD WINAPI ThreadProc(LPVOID lpParam);

class Package
{
public:
	Package();
	~Package();

	void Extract();
	double GetProcess();
	void SetPath(std::wstring& P){Path = P;};
	bool isDone();
	bool TestPackage();

	static void RunProc(Package* p);

	std::vector<FileBuf> FileList;
private:
	
	HANDLE hFile;
	bool isStarted;
	bool isFinal;
	double ProcessBar;
	std::wstring Path;
};

#endif
