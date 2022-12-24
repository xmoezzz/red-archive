#include <stdlib.h>
#include <my.h>
#include <string>

extern "C"{
#include "ecrypt-sync.h"
}

#pragma comment(lib, "MyLibrary_x86_static.lib")

int wmain(int argc, WCHAR* argv[])
{
	NtFileDisk File;
	NTSTATUS   Status;
	ULONG      Size;
	PBYTE      Buffer;
	PBYTE      Buffer2;

	BYTE       Iv[32];
	BYTE       Key[32];

	if (argc < 2)
		return 0;

	srand(time(NULL));
	ml::MlInitialize();
	Status = File.Open(argv[1]);
	if (NT_FAILED(Status))
		return 0;

	Size = File.GetSize32();
	Buffer = (BYTE*)AllocateMemoryP(Size - (0x10 + 0x20 * 2), HEAP_ZERO_MEMORY);
	Buffer2 = (BYTE*)AllocateMemoryP(Size - (0x10 + 0x20 * 2), HEAP_ZERO_MEMORY);

	DWORD Sign = TAG4('NANA');
	BYTE  Padding[12];
	File.Read(&Sign, 4);
	File.Read(Padding, 12);
	File.Read(Key, 32);
	File.Read(Iv, 32);
	File.Read(Buffer, Size - (0x10 + 0x20 * 2));
	File.Close();

	ECRYPT_ctx ctx = { 0 };

	ECRYPT_keysetup(&ctx, Key, 256, 256);
	ECRYPT_ivsetup(&ctx, Iv);
	ECRYPT_decrypt_bytes(&ctx, Buffer, Buffer2, Size - (0x10 + 0x20 * 2));

	NtFileDisk Writer;
	Writer.Create((std::wstring(argv[1]) + L".in").c_str());
	Writer.Write(Buffer2, Size - (0x10 + 0x20 * 2));
	Writer.Close();
	return 0;
}


