#include "cryptmt.h"
#include "StreamDecoder.h"


Void NTAPI CryptMTDecoder(PBYTE Buffer, ULONG_PTR Length, DWORD Key)
{
	cryptmt::CryptMT* pCrypt;
	PBYTE pKeyTable = (PBYTE)AllocStack(256);

	auto GenerateTable = [](PBYTE lpKey, DWORD iV)->PBYTE
	{
		ULONG Vector  = iV ^ 0x21457896;

		for (ULONG_PTR i = 0; i < 256; i++)
		{
			lpKey[i] = (BYTE)_lrotr(iV, Vector & 0x1F) * 0xAD128436;
			Vector *= lpKey[i];
		}
		return lpKey;
	};

	try
	{
		pCrypt = new cryptmt::CryptMT(GenerateTable(pKeyTable, Key), 2048, 2048);
	}
	catch (...)
	{
		Ps::ExitProcess(0);
	}

	pCrypt->decrypt(Buffer, Buffer, Length);
	RtlZeroMemory(pKeyTable, 256);

	delete pCrypt;
}

