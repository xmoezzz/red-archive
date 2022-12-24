#include "TextStream.h"
#include "MyStream.h"
#include "MyUtf8.h"

class tTVPTextReadStream : public iXmoeTJSTextReadStream
{
	IStream * Stream;
	bool      DirectLoad;
	tjs_char *Buffer;
	size_t    BufferLen;
	tjs_int   CryptMode;

public:
	tTVPTextReadStream(const ttstr  & name)
	{
		Stream = NULL;
		Buffer = NULL;

		Stream = CreateMyStream(name, TJS_BS_READ);

		//PrintConsoleW(L"Loading.... %s \n", name.c_str());

		try
		{
			STATSTG Stat;
			ULONG   Size;

			Stream->Stat(&Stat, STATFLAG_DEFAULT);
			Size = Stat.cbSize.LowPart;
			PBYTE PreBuffer = (PBYTE)AllocateMemoryP(Size + 1, HEAP_ZERO_MEMORY);
			Stream->Read(PreBuffer, Size, NULL);
			PreBuffer[Size] = 0;
			BufferLen = AnzUtf8ToWideCharString((LPCSTR)PreBuffer, NULL);
			Buffer = (LPWSTR)AllocateMemoryP((BufferLen + 1) * 2);
			AnzUtf8ToWideCharString((LPCSTR)PreBuffer, Buffer);

			FreeMemoryP(PreBuffer);
			Buffer[BufferLen] = NULL;

		}
		catch (...)
		{
			delete Stream;
			Stream = NULL;
			throw;
		}
	}


	~tTVPTextReadStream()
	{
		if (Stream) delete Stream;
		if (Buffer) FreeMemoryP(Buffer);
		Buffer = NULL;
	}

	tjs_uint TJS_INTF_METHOD Read(LPWSTR& StrBuffer, tjs_uint& size)
	{
		StrBuffer = (LPWSTR)AllocateMemoryP(BufferLen * 2, HEAP_ZERO_MEMORY);
		RtlCopyMemory(StrBuffer, Buffer, sizeof(tjs_char) * BufferLen);
		size = BufferLen;
		//PrintConsoleW(L"on read\n");
		return 0;
	}

	void TJS_INTF_METHOD Destruct() { delete this; }

};


//---------------------------------------------------------------------------
iXmoeTJSTextReadStream * XmoeCreateTextStreamForRead(const ttstr & name)
{
	return new tTVPTextReadStream(name);
}

