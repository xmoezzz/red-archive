#include <cstdio>
#include <windows.h>
#include <cstring>

using namespace std;

typedef unsigned __int8 u8;

int main(int argc, char **argv)
{


	FILE *fin = fopen(argv[1], "rb");
	FILE *stackinfo = 0;
	fseek(fin, 0, SEEK_END);
	unsigned __int32 FileSize = ftell(fin);
	rewind(fin);
	char *oFile = new char[FileSize];
	memset(oFile, 0, sizeof(oFile));
	fread(oFile, FileSize, 1, fin);
	fclose(fin);


	//FunctionTable _pTable;
	printf("[FVPdev] X'moe Project 2013,2014\n");
	printf("[FVPdev] FileSize = [%08x]\n", FileSize);
	char stackname[256];

	DWORD iPos = 0; //EIP

	//Get StackEntry Address : DWORD
	DWORD RealStackADD;
	memcpy(&RealStackADD, oFile, 4);

	/************************************/
	iPos = 4;
	printf("StackEntryADD = %08x\n", RealStackADD);
	while (iPos < RealStackADD){
		if (*(oFile + iPos) == 0x0E){
			if ((unsigned __int8)*(oFile + iPos + 1) <= 0 || *(oFile + iPos + 1) == 1){
				if (*(oFile + iPos + 1) <= 0){
					iPos++;
					iPos++;
					continue;
				}
				else{
					fwprintf(stackinfo, L"[0x%08x]\r\n", iPos);
					fwprintf(stackinfo, L";[0x%08x]\r\n", iPos);
					iPos += 3;
					continue;
				}
			}
			else{
				unsigned __int8 PopStrLen = *(oFile + iPos + 1);
				fwprintf(stackinfo, L"[0x%08x]", iPos);
				DWORD t = iPos;
				iPos += 2;
				int i = 0;

				WCHAR Info[2000] = { 0 };
				MultiByteToWideChar(932, 0, (oFile + iPos), lstrlenA(oFile + iPos), Info, 2000);

				fwprintf(stackinfo, L"%s\r\n", Info);
				fwprintf(stackinfo, L";[0x%08x]%s\r\n", t, Info);
				iPos += PopStrLen;
			}
		}
		else if (*(oFile + iPos) == 0x06){
			iPos++;
			iPos += 4;
		}
		else if (*(oFile + iPos) == 0x02){
			iPos++;
			iPos += 4;
		}
		else if (*(oFile + iPos) == 0x03){
			iPos++;
			iPos += 2;
		}

		else if (*(oFile + iPos) == 0x01){
			//stackinfo

			if (stackinfo) fclose(stackinfo);
			stackinfo = 0;
			memset(stackname, 0, sizeof(stackname));
			sprintf(stackname, "%08x.txt", iPos);
			printf("dumping function [%08x]\n", iPos);
			stackinfo = fopen(stackname, "wb");
			BYTE Bom[] = { 0xFF, 0xFE };
			fwrite(Bom, 1, 2, stackinfo);
			iPos++;
			iPos += 2;
		}
		else if (*(oFile + iPos) == 0x04){
			iPos++;

		}
		else if (*(oFile + iPos) == 0x05){
			iPos++;
			//fclose(stackinfo);
		}
		else if (*(oFile + iPos) == 0x07){
			iPos++;
			iPos += 4;
		}
		else if (*(oFile + iPos) == 0x08){
			iPos++;
		}
		else if (*(oFile + iPos) == 0x09){
			iPos++;
		}
		else if (*(oFile + iPos) == 0x0A){
			iPos++;
			iPos += 4;
		}
		else if (*(oFile + iPos) == 0x0B){
			iPos++;
			iPos += 2;
		}
		else if (*(oFile + iPos) == 0x0C){
			iPos++;
			iPos++;
		}
		else if (*(oFile + iPos) == 0x0D){
			iPos++;
			iPos += 4;
		}
		else if (*(oFile + iPos) == 0x0F){
			iPos++;
			iPos += 2;
		}
		else if (*(oFile + iPos) == 0x10){
			iPos++;
			iPos++;
		}
		else if (*(oFile + iPos) == 0x11){
			iPos++;
			iPos += 2;
		}
		else if (*(oFile + iPos) == 0x12){
			iPos += 2;
		}
		else if (*(oFile + iPos) == 0x13){
			iPos++;
		}
		else if (*(oFile + iPos) == 0x14){
			iPos++;
		}
		else if (*(oFile + iPos) == 0x15){
			iPos++;
			iPos += 2;
		}
		else if (*(oFile + iPos) == 0x16){
			iPos += 2;
		}
		else if (*(oFile + iPos) == 0x17){
			iPos++;
			iPos += 2;
		}
		else if (*(oFile + iPos) == 0x18){
			iPos++;
			iPos++;
		}
		else if (*(oFile + iPos) == 0x19){
			iPos++;
		}
		else if (*(oFile + iPos) == 0x1A){
			iPos++;
		}
		else if (*(oFile + iPos) == 0x1B){
			iPos++;
		}
		else if (*(oFile + iPos) == 0x1C){
			iPos++;
		}
		else if (*(oFile + iPos) == 0x1D){
			iPos++;
		}
		else if (*(oFile + iPos) == 0x1E){
			iPos++;
		}
		else if (*(oFile + iPos) == 0x1F){
			iPos++;
		}
		else if (*(oFile + iPos) == 0x20){
			iPos++;
		}
		else if (*(oFile + iPos) == 0x21){
			iPos++;
		}
		else if (*(oFile + iPos) == 0x22){
			iPos++;
		}
		else if (*(oFile + iPos) == 0x23){

			iPos++;
		}
		else if (*(oFile + iPos) == 0x24){
			iPos++;
		}
		else if (*(oFile + iPos) == 0x25){
			iPos++;
		}
		else if (*(oFile + iPos) == 0x26){
			iPos++;
		}
		else if (*(oFile + iPos) == 0x27){
			iPos++;
		}
		else{ //unknown
			printf("[%08x]Parser Error!\n", iPos);
			iPos++;
		}
	}// Loop end.


	/*******************************************/

	printf("ALL DONE!\n");

	return 0;
}

