#include "MyStream.h"
#include "IStreamEx.h"
#include <string>
#include <WinFile.h>
#include "StreamHolder.h"
#include "ShinkuHook.h"

using std::wstring;


wstring GetFileName(const ttstr& name)
{
	wstring FileName(name.c_str());
	auto    iPos = FileName.find_last_of(L"/");

	if (iPos != wstring::npos)
	{
		return FileName.substr(iPos + 1, wstring::npos);
	}
	else
	{
		return FileName;
	}
	
}

IStream* NTAPI CreateMyStream(const ttstr& name, ULONG Flag)
{
	UNREFERENCED_PARAMETER(Flag);

	IStream*      Result;
	StreamHolder* Holder;
	ULONG         FileSize;
	PBYTE         FileBuffer;
	WinFile       File;


	wstring FileName = GetFileName(name);
	wstring FullFileName;
	FullFileName += L"ProjectDir\\";
	FullFileName += FileName;

	LOOP_ONCE
	{
		Result = NULL;

		if (NT_SUCCESS(ShinkuHook::GetHook()->QueryFile((LPWSTR)FileName.c_str(), FileBuffer, FileSize)))
		{
		}
		else
		{

#if 0
			if (File.Open(FullFileName.c_str(), WinFile::FileRead) != S_OK)
				break;

			FileSize = File.GetSize32();
			FileBuffer = (PBYTE)AllocateMemoryP(FileSize);
			if (!FileBuffer)
				break; //insufficient memory

			File.Read(FileBuffer, FileSize);
			File.Release();
#else
			return NULL;
#endif
		}

		Holder = new StreamHolder  (FileBuffer, FileSize);
		Result = new IStreamAdapter(Holder);
	}
	return Result;
}
