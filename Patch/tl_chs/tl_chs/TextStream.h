#pragma once

#include <my.h>
#include "tp_stub.h"
#include "VMProtectSDK.h"

extern "C" {
#include "ecrypt-sync.h"
}


class AnzTextReadStream : public iTJSTextReadStream
{
	tjs_char *Buffer;
	tjs_char *BufferPtr;
	size_t    BufferLen;

public:
	AnzTextReadStream(const ttstr& name, IStream* Stream)
	{
		STATSTG    StatInfo;
		BYTE       Iv[32];
		BYTE       Key[32];
		DWORD      Sign;
		BYTE       Padding[12];
		PBYTE      OriBuffer = NULL;
		ULONG      cbRead;

		VMProtectBeginVirtualization("TextSec");

		Buffer    = NULL;
		BufferLen = 0;

		Stream->Stat(&StatInfo, STATFLAG_DEFAULT);
		Stream->Read(&Sign,  4,  &cbRead);
		Stream->Read(Padding,12, &cbRead);
		Stream->Read(Key,    32, &cbRead);
		Stream->Read(Iv,     32, &cbRead);

		OriBuffer = (BYTE*)AllocateMemoryP(StatInfo.cbSize.LowPart - (0x10 + 0x20 * 2), HEAP_ZERO_MEMORY);
		Stream->Read(OriBuffer, StatInfo.cbSize.LowPart - (0x10 + 0x20 * 2), &cbRead);
		
		Buffer = new tjs_char[StatInfo.cbSize.LowPart / 2];
		RtlZeroMemory(Buffer, StatInfo.cbSize.LowPart / 2);

		ECRYPT_ctx ctx = { 0 };

		ECRYPT_keysetup(&ctx, Key, 256, 256);
		ECRYPT_ivsetup(&ctx, Iv);
		ECRYPT_decrypt_bytes(&ctx, OriBuffer, (uint8_t*)Buffer, StatInfo.cbSize.LowPart - (0x10 + 0x20 * 2));

		BufferPtr = Buffer;

		RtlZeroMemory(OriBuffer, StatInfo.cbSize.LowPart - (0x10 + 0x20 * 2));
		RtlZeroMemory(&ctx,    sizeof(ctx));
		RtlZeroMemory(Iv,      sizeof(Iv));
		RtlZeroMemory(Key,     sizeof(Key));
		RtlZeroMemory(Padding, sizeof(Padding));
		RtlZeroMemory(&Sign,   4);
		BufferLen = (StatInfo.cbSize.LowPart - (0x10 + 0x20 * 2)) / 2;

		if (BufferLen >= 1)
		{
			BufferPtr++;
			BufferLen--;
		}
		
		delete Stream;
		
		VMProtectEnd();
	}


	~AnzTextReadStream()
	{
		//VMProtectBeginVirtualization("TextDel");

		if (Buffer)
		{
			delete[] Buffer;
			Buffer = NULL;
		}

		BufferPtr = NULL;

		//VMProtectEnd();
	}

	tjs_uint TJS_INTF_METHOD Read(tTJSString & targ, tjs_uint size)
	{
		//VMProtectBeginVirtualization("TextRead");

		if (size == 0) 
			size = BufferLen;

		if (size)
		{
			tjs_char *buf = targ.AllocBuffer(size);
			wcsncpy(buf, BufferPtr, size);
			buf[size] = 0;
			BufferPtr += size;
			BufferLen -= size;
			targ.FixLen();
		}

		//VMProtectEnd();
		return size;
	}

	void TJS_INTF_METHOD Destruct() { delete this; }

};

