#include <cstdio>

int main()
{
	FILE* fin = fopen("Gen.txt", "wb");
	for(int i = 13 ; i <= 26; i++)
	{
		fprintf(fin, "DetourTransactionBegin();\nDetourUpdateThread(GetCurrentThread());\nDetourAttach((void**)&SaveStub%dStart, SaveStub%d);\nDetourTransactionCommit();\n\n\n", i, i);
	}
	fclose(fin);
	return 0;
} 