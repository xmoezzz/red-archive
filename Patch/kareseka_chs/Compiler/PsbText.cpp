#include "PsbText.h"
#include <ntstatus.h>
#include <vector>
#include <string>

using std::string;
using std::wstring;
using std::vector;
using std::fstream;

vector<string> StringPool;

HINSTANCE g_hInstance;

void WINAPI InitInfo(LPMEL_INFO2 lpMelInfo)
{
	lpMelInfo->dwInterfaceVersion = INTERFACE_VERSION;
	lpMelInfo->dwCharacteristic = MIC_NOPREREAD;
}

void WINAPI PreProc(LPPRE_DATA lpPreData)
{
	lpPreData->hGlobalHeap = GetProcessHeap();
}


DWORD M2GetInt(BYTE* &p, char t)
{
	switch (t)
	{
	case 0xd:
		return *p++;
	case 0xe:
	{
		WORD val = *(WORD*)p;
		p += 2;
		return val;
	}
	case 0xf:
	{
		DWORD val = *(WORD*)p;
		p += 2;
		val += (*p++) * 0x10000;
		return val;
	}
	case 0x10:
	{
		DWORD val = *(DWORD*)p;
		p += 4;
		return val;
	}
	default:
		return 0;
	}
	return 0;
}

DWORD M2GetInt(BYTE* &p)
{
	return M2GetInt(p, *p++);
}

char* M2GetStr(BYTE* p, char t, PsbInfo* pInfo)
{
	int idx = M2GetInt(p, t - (0x15 - 0xd));
	return pInfo->lpStrRes + pInfo->lpStrOffList[idx];
}
char* M2GetStr(BYTE* p, PsbInfo* pInfo)
{
	char t = *p++;
	return M2GetStr(p, t, pInfo);
}

DWORD* M2CopyArray(BYTE* pOri, DWORD count, int type)
{
	DWORD* pArray = new DWORD[count];
	switch (type)
	{
	case 0xd:
	{
		for (int i = 0; i<count; i++)
			pArray[i] = pOri[i];
	}
	break;
	case 0xe:
	{
		WORD* p = (WORD*)pOri;
		for (int i = 0; i<count; i++)
			pArray[i] = p[i];
	}
	break;
	case 0xf:
	{
		DWORD temp;
		for (int i = 0; i<count; i++)
		{
			temp = *(WORD*)pOri;
			pOri += 2;
			temp += *pOri++ * 0x10000;
			pArray[i] = temp;
		}
	}
	break;
	case 0x10:
		memcpy(pArray, pOri, count * 4);
		break;
	default:
		return nullptr;
	}
	return pArray;
}

DWORD* M2CopyArray(BYTE* p)
{
	DWORD count = M2GetInt(p);
	char type = *p++;
	return M2CopyArray(p, count, type);
}

TreeNode* M2MakeBranchTable(BYTE* pOri, DWORD count, int type)
{
	TreeNode* pTable = new TreeNode[count];
	for (int i = 0; i<count; i++)
	{
		pTable[i].nBranch = 0;
	}

	DWORD val;
	int size = type - 12;
	BYTE* p = pOri;
	for (int i = 0; i<count; i++)
	{
		val = M2GetInt(p, type);
		pTable[val].pSub.push_back(i);
		pTable[val].nBranch++;
	}
	return pTable;
}

char** tNames;
char* tName;
DWORD* tTree;
TreeNode* tBranches;
int tMaxId;
void M2TraverseTree(int depth, int node)
{
	if (node == 0 && depth != 0)
		return;
	for (int i = 0; i<tBranches[node].nBranch; i++)
	{
		tName[depth] = tBranches[node].pSub[i] - tTree[node];
		if (tName[depth] != 0)
			M2TraverseTree(depth + 1, tBranches[node].pSub[i]);
		else if (tTree[node] <= tMaxId)
		{
			tNames[tTree[node]] = new char[depth + 1];
			strncpy(tNames[tTree[node]], tName, depth);
		}
	}
}

int M2GetIDFromTree(DWORD* tree, DWORD* vtree, int count, char* name)
{
	int node = 0;
	int len = lstrlenA(name) + 1;
	for (int i = 0; i<len; i++)
	{
		int p = node;
		node = tree[node] + name[i];
		if (vtree[node] != p)
			return -1;
	}
	return tree[node];
}

BYTE* M2FindInDict(char* name, BYTE* dict, PsbInfo* pInfo)
{
	int id = M2GetIDFromTree(pInfo->lpTree, pInfo->lpVerifyTree, pInfo->nTreeSize, name);
	if (id == -1)
		return NULL;
	BYTE* p = dict;
	DWORD count = M2GetInt(p);
	char type = *p++;
	DWORD* pCases = M2CopyArray(p, count, type);

	if (pCases == nullptr)
		return nullptr;

	DWORD* rslt = (DWORD*)bsearch(&id, pCases, count, 4, (int(*)(const void*, const void*))compare4);
	if (rslt == NULL)
	{
		delete[] pCases;
		return NULL;
	}
	id = rslt - pCases;
	delete[] pCases;

	p += count*(type - 12);
	count = M2GetInt(p);
	type = *p++;

	BYTE* p2 = p + id*(type - 12);
	return p + count*(type - 12) + M2GetInt(p2, type);
}

int M2AddStr(BYTE* pC, LPFILE_INFO lpFileInfo, int nLine)
{
	char type = *pC++;
	if (type != 1)
	{
		PsbInfo* pInfo = (PsbInfo*)lpFileInfo->lpCustom;
		DWORD idx = M2GetInt(pC, type - 0x15 + 0xd);
		if (idx<pInfo->nStrs)
		{
			char* pFinalStr = pInfo->lpStrRes + pInfo->lpStrOffList[idx];
			if (*pFinalStr != 0)
			{
				lpFileInfo->lpStreamIndex[nLine].StringStart = pFinalStr;
				lpFileInfo->lpStreamIndex[nLine].nStringLen = lstrlenA(pFinalStr);
				lpFileInfo->lpStreamIndex[nLine].lpStart = (LPVOID)idx;
				lpFileInfo->lpStreamIndex[nLine].lpInformation = (LPVOID)lstrlenA(pFinalStr);
				lpFileInfo->lpStreamIndex[nLine].Offset = (DWORD)(pFinalStr)-(DWORD)(pInfo->lpStrRes);
				return 1;
			}
		}
	}
	return 0;
}

int M2GetScenes(BYTE* pStart, PsbInfo* pInfo, LPFILE_INFO lpFileInfo)
{
	BYTE* p = pStart;
	DWORD type, count;

	if (*p++ != PSBVALTYPE_DICT)
		return 0;

	p = M2FindInDict("scenes", p, pInfo);
	if (p == NULL)
		return 0;

	if (*p++ != PSBVALTYPE_ARRAY)
		return 0;

	DWORD scCount = M2GetInt(p);
	type = *p++;
	DWORD* pScOff = M2CopyArray(p, scCount, type);

	if (pScOff == nullptr)
		return 0;

	BYTE* pScStart = p + scCount*(type - 12);

	int nLine = lpFileInfo->nLine;

	for (int k = 0; k<scCount; k++)
	{
		p = pScStart + pScOff[k];

		if (*p++ != PSBVALTYPE_DICT)
			return 0;

		p = M2FindInDict("texts", p, pInfo);
		if (p == NULL)
			continue;
		if (*p == PSBVALTYPE_ARRAY)
		{
			p++;
			count = M2GetInt(p);
			type = *p++;
			DWORD* textsOffTable = M2CopyArray(p, count, type);

			if (textsOffTable == nullptr)
				return 0;

			BYTE* pStart = p + count*(type - 12);
			for (int i = 0; i<count; i++)
			{
				p = pStart + textsOffTable[i];
				if (*p++ == PSBVALTYPE_ARRAY)
				{
					DWORD cnt = M2GetInt(p);
					/*
					if (cnt<3)
					continue;
					*/
					type = *p++;
					DWORD* offsets = M2CopyArray(p, cnt, type);

					if (offsets == nullptr)
						return 0;

					p += cnt*(type - 12);

					int nToAdd = M2AddStr(p + offsets[1], lpFileInfo, nLine);
					nLine += nToAdd;
					if (!nToAdd)
						nLine += M2AddStr(p + offsets[0], lpFileInfo, nLine);
					nLine += M2AddStr(p + offsets[2], lpFileInfo, nLine);
					delete[] offsets;
				}
				else
				{
					return 0;
				}
			}
			delete[] textsOffTable;
		}
		else
		{
			//nLine+=M2AddStr(p,lpFileInfo,nLine);
		}
	}
	delete[] pScOff;

	count = lpFileInfo->nLine;
	lpFileInfo->nLine = nLine;
	return nLine - count;
}

int M2GetLists(BYTE* pStart, PsbInfo* pInfo, LPFILE_INFO lpFileInfo)
{
	BYTE* p = pStart;
	DWORD type, count;

	if (*p++ != PSBVALTYPE_DICT)
		return 0;

	p = M2FindInDict("list", p, pInfo);
	if (p == NULL)
		return 0;

	if (*p++ != PSBVALTYPE_ARRAY)
		return 0;

	DWORD scCount = M2GetInt(p);
	type = *p++;
	DWORD* pScOff = M2CopyArray(p, scCount, type);

	if (pScOff == nullptr)
		return 0;

	BYTE* pScStart = p + scCount*(type - 12);

	int nLine = lpFileInfo->nLine;

	for (int k = 0; k<scCount; k++)
	{
		p = pScStart + pScOff[k];

		if (*p++ != PSBVALTYPE_DICT)
			return 0;

		p = M2FindInDict("title", p, pInfo);
		if (p == NULL)
			continue;

		int nToAdd = M2AddStr(p, lpFileInfo, nLine);
		nLine += nToAdd;

#if 0
		p = M2FindInDict("selects", p, pInfo);
		if (p == NULL)
			continue;
		if (*p++ != PSBVALTYPE_ARRAY)
			continue;
		DWORD scSelectCount = M2GetInt(p);
		DWORD type2 = *p++;

		DWORD* pScOff2 = M2CopyArray(p, scSelectCount, type2);
		BYTE* pScStart2 = p + scSelectCount*(type2 - 12);

		for (int j = 0; j < scSelectCount; j++)
		{
			p = pScStart2 + pScOff2[j];
			if (*p++ != PSBVALTYPE_DICT)
				return 0;

			p = M2FindInDict("text", p, pInfo);
			if (p == NULL)
				continue;

			int nToAdd2 = M2AddStr(p, lpFileInfo, nLine);
			nLine += nToAdd2;
		}
		delete[] pScOff2;
#endif

	}
	delete[] pScOff;


	count = lpFileInfo->nLine;
	lpFileInfo->nLine = nLine;
	return nLine - count;
}


int M2GetSelects(BYTE* pStart, PsbInfo* pInfo, LPFILE_INFO lpFileInfo)
{
	BYTE* p = pStart;
	DWORD type, count;

	if (*p++ != PSBVALTYPE_DICT)
		return 0;

	p = M2FindInDict("scenes", p, pInfo);
	if (p == NULL)
		return 0;

	if (*p++ != PSBVALTYPE_ARRAY)
		return 0;

	DWORD scCount = M2GetInt(p);
	type = *p++;
	DWORD* pScOff = M2CopyArray(p, scCount, type);

	if (pScOff == nullptr)
		return 0;

	BYTE* pScStart = p + scCount*(type - 12);

	int nLine = lpFileInfo->nLine;

	for (int k = 0; k<scCount; k++)
	{
		p = pScStart + pScOff[k];

		if (*p++ != PSBVALTYPE_DICT)
			return 0;

		p = M2FindInDict("selects", p, pInfo);
		if (p == NULL)
			continue;
		if (*p++ != PSBVALTYPE_ARRAY)
			continue;
		DWORD scSelectCount = M2GetInt(p);
		DWORD type2 = *p++;

		DWORD* pScOff2 = M2CopyArray(p, scSelectCount, type2);
		BYTE* pScStart2 = p + scSelectCount*(type2 - 12);

		for (int j = 0; j < scSelectCount; j++)
		{
			p = pScStart2 + pScOff2[j];
			if (*p++ != PSBVALTYPE_DICT)
				return 0;

			p = M2FindInDict("text", p, pInfo);
			if (p == NULL)
				continue;

			int nToAdd2 = 0;

			if (p)
			{
				nToAdd2 = M2AddStr(p, lpFileInfo, nLine);
			}

			nLine += nToAdd2;
		}
		delete[] pScOff2;

	}
	delete[] pScOff;


	count = lpFileInfo->nLine;
	lpFileInfo->nLine = nLine;
	return nLine - count;
}


int M2GetSelectsRunLine(BYTE* pStart, PsbInfo* pInfo, LPFILE_INFO lpFileInfo)
{
	BYTE* p = pStart;
	DWORD type, count;

	if (*p++ != PSBVALTYPE_DICT)
		return 0;

	p = M2FindInDict("scenes", p, pInfo);
	if (p == NULL)
		return 0;

	if (*p++ != PSBVALTYPE_ARRAY)
		return 0;

	DWORD scCount = M2GetInt(p);
	type = *p++;
	DWORD* pScOff = M2CopyArray(p, scCount, type);

	if (pScOff == nullptr)
		return 0;

	BYTE* pScStart = p + scCount*(type - 12);

	int nLine = lpFileInfo->nLine;

	for (int k = 0; k<scCount; k++)
	{
		p = pScStart + pScOff[k];

		if (*p++ != PSBVALTYPE_DICT)
			return 0;

		p = M2FindInDict("selects", p, pInfo);
		if (p == NULL)
			continue;
		if (*p++ != PSBVALTYPE_ARRAY)
			continue;
		DWORD scSelectCount = M2GetInt(p);
		DWORD type2 = *p++;

		DWORD* pScOff2 = M2CopyArray(p, scSelectCount, type2);
		BYTE* pScStart2 = p + scSelectCount*(type2 - 12);

		for (int j = 0; j < scSelectCount; j++)
		{
			p = pScStart2 + pScOff2[j];
			if (*p++ != PSBVALTYPE_DICT)
				return 0;

			p = M2FindInDict("runLineStr", p, pInfo);
			if (p == NULL)
				continue;

			int nToAdd2 = 0;

			if (p)
			{
				nToAdd2 = M2AddStr(p, lpFileInfo, nLine);
			}

			nLine += nToAdd2;
		}
		delete[] pScOff2;

	}
	delete[] pScOff;


	count = lpFileInfo->nLine;
	lpFileInfo->nLine = nLine;
	return nLine - count;
}

//root.scene[selectInfo.select[text]]
int M2GetSelectsInfo(BYTE* pStart, PsbInfo* pInfo, LPFILE_INFO lpFileInfo)
{
	BYTE* p = pStart;
	DWORD type, count;

	if (*p++ != PSBVALTYPE_DICT)
		return 0;

	p = M2FindInDict("scenes", p, pInfo);
	if (p == NULL)
		return 0;

	if (*p++ != PSBVALTYPE_ARRAY)
		return 0;

	DWORD scCount = M2GetInt(p);
	type = *p++;
	DWORD* pScOff = M2CopyArray(p, scCount, type);

	if (pScOff == nullptr)
		return 0;

	BYTE* pScStart = p + scCount*(type - 12);

	int nLine = lpFileInfo->nLine;

	for (int k = 0; k<scCount; k++)
	{
		p = pScStart + pScOff[k];

		if (*p++ != PSBVALTYPE_DICT)
			return 0;

		p = M2FindInDict("selectInfo", p, pInfo);
		if (p == NULL)
			continue;

		if (*p++ != PSBVALTYPE_DICT)
			return 0;

		p = M2FindInDict("select", p, pInfo);
		if (p == NULL)
			return 0;

		if (*p++ != PSBVALTYPE_ARRAY)
			continue;

		DWORD scSelectCount = M2GetInt(p);
		DWORD type2 = *p++;

		DWORD* pScOff2 = M2CopyArray(p, scSelectCount, type2);
		BYTE* pScStart2 = p + scSelectCount*(type2 - 12);

		for (int j = 0; j < scSelectCount; j++)
		{
			p = pScStart2 + pScOff2[j];
			if (*p++ != PSBVALTYPE_DICT)
				return 0;

			p = M2FindInDict("text", p, pInfo);
			if (p == NULL)
				continue;

			int nToAdd2 = 0;

			if (p)
			{
				nToAdd2 = M2AddStr(p, lpFileInfo, nLine);
			}

			nLine += nToAdd2;
		}
		delete[] pScOff2;

	}
	delete[] pScOff;


	count = lpFileInfo->nLine;
	lpFileInfo->nLine = nLine;
	return nLine - count;
}


int M2GetSelectsInfoRunLine(BYTE* pStart, PsbInfo* pInfo, LPFILE_INFO lpFileInfo)
{
	BYTE* p = pStart;
	DWORD type, count;

	if (*p++ != PSBVALTYPE_DICT)
		return 0;

	p = M2FindInDict("scenes", p, pInfo);
	if (p == NULL)
		return 0;

	if (*p++ != PSBVALTYPE_ARRAY)
		return 0;

	DWORD scCount = M2GetInt(p);
	type = *p++;
	DWORD* pScOff = M2CopyArray(p, scCount, type);

	if (pScOff == nullptr)
		return 0;

	BYTE* pScStart = p + scCount*(type - 12);

	int nLine = lpFileInfo->nLine;

	for (int k = 0; k<scCount; k++)
	{
		p = pScStart + pScOff[k];

		if (*p++ != PSBVALTYPE_DICT)
			return 0;

		p = M2FindInDict("selectInfo", p, pInfo);
		if (p == NULL)
			continue;

		if (*p++ != PSBVALTYPE_DICT)
			return 0;

		p = M2FindInDict("select", p, pInfo);
		if (p == NULL)
			return 0;

		if (*p++ != PSBVALTYPE_ARRAY)
			continue;

		DWORD scSelectCount = M2GetInt(p);
		DWORD type2 = *p++;

		DWORD* pScOff2 = M2CopyArray(p, scSelectCount, type2);
		BYTE* pScStart2 = p + scSelectCount*(type2 - 12);

		for (int j = 0; j < scSelectCount; j++)
		{
			p = pScStart2 + pScOff2[j];
			if (*p++ != PSBVALTYPE_DICT)
				return 0;

			p = M2FindInDict("runLineStr", p, pInfo);
			if (p == NULL)
				continue;

			int nToAdd2 = 0;

			if (p)
			{
				nToAdd2 = M2AddStr(p, lpFileInfo, nLine);
			}

			nLine += nToAdd2;
		}
		delete[] pScOff2;

	}
	delete[] pScOff;


	count = lpFileInfo->nLine;
	lpFileInfo->nLine = nLine;
	return nLine - count;
}


#define XmoeStringStart 0xFFFFFFFEUL
#define XmoeStringEnd   0xFFFFFFFFUL

typedef union DwordPack
{
	unsigned int  DwordPart;
	unsigned char BytePart[4];
}DwordPack;


template<class T>
T CPP_ROL(T n, const int bitN)
{
	const int BITLEN = sizeof(T) * 8;
	n = (n >> (BITLEN - bitN)) | (n << bitN);
	return n;
}

template<class T>
T CPP_ROR(T n, const int bitN)
{
	const int BITLEN = sizeof(T) * 8;
	n = (n << (BITLEN - bitN)) | (n >> bitN);
	return n;
}


unsigned char CPP_ROL_8(unsigned char n, const int bitN)
{
	const int BITLEN = sizeof(unsigned char) * 8;
	n = (n >> (BITLEN - bitN)) | (n << bitN);
	return n;
}


unsigned char CPP_ROR_8(unsigned char n, const int bitN)
{
	const int BITLEN = sizeof(unsigned char) * 8;
	n = (n << (BITLEN - bitN)) | (n >> bitN);
	return n;
}


void Decode(unsigned char* code, unsigned int size)
{
	for (unsigned int i = 0; i < size; i++)
	{
		code[i] = CPP_ROR_8(code[i], 2);
	}
}

string DecodeString(const char* code)
{
	if (strlen(code) <= 4)
		return string(code);

	string PackStr;

	static unsigned char DecodeArea[2000];

	unsigned int iPos = 0;
	while (iPos < strlen(code))
	{
		if (*(unsigned int*)(code + iPos) == XmoeStringStart)
		{
			iPos += 4;
			unsigned int Ptr = 0;

			memset(DecodeArea, 0, sizeof(DecodeArea));
			while (iPos < strlen(code))
			{
				if (*(unsigned int*)(code + iPos) == XmoeStringEnd)
				{
					iPos += 4;
					break;
				}
				else
				{
					DecodeArea[Ptr] = code[iPos];
					iPos++;
					Ptr++;
				}
			}
			Decode(DecodeArea, strlen((char*)DecodeArea));
			PackStr += (char*)DecodeArea;
		}
		else
		{
			PackStr += code[iPos];
			iPos++;
		}
	}
	return PackStr;
}


void Encode(unsigned char* code, unsigned int size)
{
	for (unsigned int i = 0; i < size; i++)
	{
		code[i] = CPP_ROL_8(code[i], 2);
	}
}


string EncodeString(const char *code)
{
	string Result;
	DwordPack d;
	d.DwordPart = XmoeStringStart;
	Result += d.BytePart[0];
	Result += d.BytePart[1];
	Result += d.BytePart[2];
	Result += d.BytePart[3];

	string Enc = code;
	Encode((unsigned char*)Enc.c_str(), Enc.length());
	Result += Enc;

	d.DwordPart = XmoeStringEnd;
	Result += d.BytePart[0];
	Result += d.BytePart[1];
	Result += d.BytePart[2];
	Result += d.BytePart[3];

	return Result;
}

MRESULT WINAPI ModifyLine(LPFILE_INFO lpFileInfo, DWORD nLine, string& LineStr)
{
	PsbInfo* pInfo = (PsbInfo*)lpFileInfo->lpCustom;

	int nNewLen = LineStr.length() + 1;
	const char* pNewStr = LineStr.c_str();

	DWORD nStrIdx = (DWORD)lpFileInfo->lpStreamIndex[nLine].lpStart;
	DWORD nStrOff = pInfo->lpStrOffList[nStrIdx];
	char* pOldStr = pInfo->lpStrRes + nStrOff;
	int nOldLen = lstrlenA(pOldStr) + 1;

	if (nNewLen <= (DWORD)lpFileInfo->lpStreamIndex[nLine].lpInformation)
	{
		memcpy(pOldStr, pNewStr, nNewLen);
	}
	else
	{
		memcpy(pInfo->lpStrRes + pInfo->nTotalStrLen, pNewStr, nNewLen);
		pInfo->lpStrOffList[nStrIdx] = pInfo->nTotalStrLen;
		pInfo->nTotalStrLen += nNewLen;
	}

	return E_SUCCESS;
}

MRESULT WINAPI GetText(LPFILE_INFO lpFileInfo, LPDWORD lpdwRInfo)
{
	DWORD magic[2], nBytesRead;

	DWORD nFileSize;

	rewind(lpFileInfo->hStream);
	fseek(lpFileInfo->hStream, 0, SEEK_END);
	nFileSize = ftell(lpFileInfo->hStream);
	rewind(lpFileInfo->hStream);

	fread(magic, 1, 8, lpFileInfo->hStream);

	rewind(lpFileInfo->hStream);

	BOOL bIsmdf;
	if (magic[0] == '\0BSP')
	{
		bIsmdf = 0;
		rewind(lpFileInfo->hStream);
		lpFileInfo->lpStream = HeapAlloc(GetProcessHeap(), 0, nFileSize);
		fread(lpFileInfo->lpStream, 1, nFileSize, lpFileInfo->hStream);

		lpFileInfo->nStreamSize = nFileSize;
	}
	else if (magic[0] == '\0fdm')
	{
		lpFileInfo->lpStream = HeapAlloc(GetProcessHeap(), 0, magic[1]);
		bIsmdf = 1;
		BYTE* tmp = (BYTE*)new BYTE[nFileSize - 8];
		DWORD ret;
		fread(tmp, 1, nFileSize - 8,lpFileInfo->hStream);

		ret = uncompress((Bytef*)lpFileInfo->lpStream, &magic[1], tmp, nFileSize - 8);
		delete[] tmp;
		if (ret != Z_OK)
			return E_WRONGFORMAT;
		lpFileInfo->nStreamSize = magic[1];
		if (*(DWORD*)lpFileInfo->lpStream != '\0BSP')
		{
			return E_WRONGFORMAT;
		}
	}

	PsbInfo* pInfo = new PsbInfo;
	memset(pInfo, 0, sizeof(PsbInfo));
	lpFileInfo->lpCustom = pInfo;

	pInfo->bIsCompressed = bIsmdf;

	PsbHeader* pHeader = (PsbHeader*)(lpFileInfo->lpStream);

	if (pHeader->dwMagic != '\0BSP')
		return E_WRONGFORMAT;


	BYTE* p = (BYTE*)lpFileInfo->lpStream + pHeader->nNameTree;
	DWORD count = M2GetInt(p);
	char type = *p++;
	tTree = M2CopyArray(p, count, type);

	if (tTree == nullptr)
		return E_WRONGFORMAT;

	pInfo->lpTree = tTree;
	pInfo->nTreeSize = count;
	p += count*(type - 12);
	count = M2GetInt(p);
	type = *p++;
	tBranches = M2MakeBranchTable(p, count, type);
	pInfo->lpVerifyTree = M2CopyArray(p, count, type);

	if (pInfo->lpVerifyTree == nullptr)
		return E_WRONGFORMAT;

	p += count*(type - 12);
	count = M2GetInt(p);
	type = *p++;
	DWORD* pIDTable = M2CopyArray(p, count, type);

	if (pIDTable == nullptr)
		return E_WRONGFORMAT;

	for (int i = 0; i<count; i++)
		if (pIDTable[i]>tMaxId)
			tMaxId = pIDTable[i];

	tNames = new char*[tMaxId + 1];
	memset(tNames, 0, (tMaxId + 1) * 4);
	//pInfo->lpNamesTable=tNames;
	pInfo->nNames = tMaxId + 1;
	tName = new char[512];

	M2TraverseTree(0, 0);

	delete[] tName;
	delete[] tBranches;
	for (int i = 0; i<pInfo->nNames; i++)
		if (tNames[i])
			delete[] tNames[i];
	delete[] tNames;

	p = (BYTE*)lpFileInfo->lpStream + pHeader->nStrOffList;
	count = M2GetInt(p);
	type = *p++;
	DWORD* pOff = M2CopyArray(p, count, type);

	if (pOff == nullptr)
		return E_WRONGFORMAT;

	pInfo->lpStrOffList = pOff;
	pInfo->nStrs = count;

	int max = 0;
	for (int i = 0; i<count; i++)
		if (pOff[i]>max)
			max = pOff[i];


	p = (BYTE*)lpFileInfo->lpStream + pHeader->nStrRes;
	int size = lstrlenA((char*)&p[max]) + 1 + max;
	pInfo->lpStrRes = new char[size * 3];
	pInfo->nTotalStrLen = size;
	pInfo->nOriTotalStrLen = size;
	memcpy(pInfo->lpStrRes, p, size);

	lpFileInfo->lpStreamIndex = (STREAM_ENTRY*)VirtualAlloc(0,
		pInfo->nStrs * 2 * sizeof(STREAM_ENTRY), MEM_COMMIT, PAGE_READWRITE);

	p = (BYTE*)lpFileInfo->lpStream + pHeader->nResIndexTree;
	M2GetScenes(p, pInfo, lpFileInfo);
	M2GetLists(p, pInfo, lpFileInfo);
	M2GetSelects(p, pInfo, lpFileInfo);
	M2GetSelectsRunLine(p, pInfo, lpFileInfo);
	M2GetSelectsInfo(p, pInfo, lpFileInfo);
	M2GetSelectsInfoRunLine(p, pInfo, lpFileInfo);

	lpFileInfo->dwMemoryType = MT_POINTERONLY;
	lpFileInfo->dwStringType = ST_ENDWITHZERO;
	*lpdwRInfo = RI_SUC_LINEONLY;

	return E_SUCCESS;
}


int compare1(BYTE* arg1, BYTE* arg2)
{
	return *arg1 - *arg2;
}
int compare2(WORD* arg1, WORD* arg2)
{
	return *arg1 - *arg2;
}
int compare3(BYTE* arg1, BYTE* arg2)
{
	return M2GetInt(arg1, 0xf) - M2GetInt(arg2, 0xf);
}
int compare4(DWORD* arg1, DWORD* arg2)
{
	return *arg1 - *arg2;
}


void M2CorrectHeader(PsbHeader* pHeader, DWORD off, int nDiff)
{
	DWORD* p = (DWORD*)pHeader + 4;
	for (int i = 0; i<sizeof(PsbHeader) / 4 - 4; i++)
		if (p[i]>off)
			p[i] += nDiff;
}


unsigned int sub_4150E9(unsigned int a1, void *a2, const void *a3)
{
	unsigned int result; // eax@1

	result = a1;
	memcpy(a2, a3, a1);
	return result;
}

MRESULT __stdcall
_ReplaceInMem(LPVOID lpNew, DWORD nNewLen, LPVOID lpOld, DWORD nOldLen, DWORD nLeftLen)
{
	int result; // eax@3
	void *v6 = nullptr; // eax@5
	void *v7 = nullptr; // ST0C_4@7

	if (nNewLen != nOldLen && nLeftLen)
	{
		if (nNewLen > nOldLen || nOldLen - nNewLen < 4)
		{
			v6 = HeapAlloc(GetProcessHeap(), 0, nLeftLen);
			if (v6)
			{
				v7 = v6;
				sub_4150E9(nLeftLen, v6, (char *)lpOld + nOldLen);
				memcpy(lpOld, lpNew, nNewLen);
				sub_4150E9(nLeftLen, (char *)lpOld + nNewLen, v7);
				HeapFree(GetProcessHeap(), 0, v7);
				result = 0;
			}
			else
			{
				result = 1;
			}
		}
		else
		{
			memcpy(lpOld, lpNew, nNewLen);
			sub_4150E9(nLeftLen, (char *)lpOld + nNewLen, (char *)lpOld + nOldLen);
			result = 0;
		}
	}
	else
	{
		memcpy(lpOld, lpNew, nNewLen);
		result = 0;
	}
	return result;
}

MRESULT WINAPI SaveText(LPFILE_INFO lpFileInfo, PBYTE& OutBuffer, ULONG& OutSize)
{
	PsbInfo* pInfo = (PsbInfo*)lpFileInfo->lpCustom;
	BYTE* lpTemp = (BYTE*)VirtualAlloc(0, lpFileInfo->nStreamSize * 2, MEM_COMMIT, PAGE_READWRITE);
	memcpy(lpTemp, lpFileInfo->lpStream, lpFileInfo->nStreamSize);
	PsbHeader* pHeader = (PsbHeader*)lpTemp;

	MRESULT ret = _ReplaceInMem(pInfo->lpStrRes, pInfo->nTotalStrLen,
		lpTemp + pHeader->nStrRes, pInfo->nOriTotalStrLen,
		lpFileInfo->nStreamSize - pHeader->nStrRes - pInfo->nOriTotalStrLen);

	if (ret != E_SUCCESS)
	{
		VirtualFree(lpTemp, 0, MEM_RELEASE);
		return ret;
	}

	int nDiff = pInfo->nTotalStrLen - pInfo->nOriTotalStrLen;
	M2CorrectHeader(pHeader, pHeader->nStrRes, nDiff);
	DWORD nNewStreamSize = lpFileInfo->nStreamSize + nDiff;

	BYTE* p = lpTemp + pHeader->nStrOffList;
	DWORD count = M2GetInt(p);
	DWORD nOriOffSize = (*p - 12)*count;
	*p++ = 4 + 12;


	ret = _ReplaceInMem(pInfo->lpStrOffList, pInfo->nStrs * 4, p, nOriOffSize,
		lpTemp + nNewStreamSize - p - nOriOffSize);


	if (ret != E_SUCCESS)
	{
		VirtualFree(lpTemp, 0, MEM_RELEASE);
		return ret;
	}


	nDiff = pInfo->nStrs * 4 - nOriOffSize;
	M2CorrectHeader(pHeader, pHeader->nStrOffList, nDiff);
	nNewStreamSize += nDiff;

	DWORD nBytesRead;


	if (!pInfo->bIsCompressed)
	{
		OutSize = nNewStreamSize;
		OutBuffer = new BYTE[nNewStreamSize];

		memcpy(OutBuffer, lpTemp, nNewStreamSize);

		VirtualFree(lpTemp, 0, MEM_RELEASE);
	}
	else
	{
		DWORD magic[2];
		magic[0] = '\0fdm';
		magic[1] = nNewStreamSize;
		BYTE* tmp = new BYTE[nNewStreamSize];
		ret = compress(tmp, &nNewStreamSize, lpTemp, nNewStreamSize);
		VirtualFree(lpTemp, 0, MEM_RELEASE);
		if (ret != 0)
		{
			delete[] tmp;
			return E_ERROR;
		}

		OutSize = 8 + nNewStreamSize;
		OutBuffer = new BYTE[OutSize];

		memcpy(OutBuffer, magic, 8);
		memcpy(OutBuffer + 8, tmp, nNewStreamSize);

		delete[] tmp;

		//SetEndOfFile(lpFileInfo->hFile);
	}

	return E_SUCCESS;
}

#include <fstream>

static BYTE UTFBOM[] = { 0xEF, 0xBB, 0xBF };


wstring GetTextName(LPCWSTR FileName)
{
	wstring TextName(FileName);

	TextName = TextName.substr(0, TextName.find_first_of(L'.'));
	return TextName;
}

NTSTATUS MyReplaceText(LPCWSTR FileName)
{
	NTSTATUS            Status;
	wstring             TextFileName;
	PBYTE               Buffer;
	ULONG               Size, LineIndex;

	do
	{
		Status = STATUS_UNSUCCESSFUL;
		LineIndex = 0;

		TextFileName += GetTextName(FileName);
		TextFileName += L".txt";

		std::fstream        File(TextFileName);
		if (!File.is_open())
		{
			printf("Cannot open\n");
			break;
		}

		string ReadLine;

		while (getline(File, ReadLine))
		{
			if (!memcmp(&ReadLine[0], UTFBOM, sizeof(UTFBOM)))
				ReadLine = &ReadLine[3];

			StringPool.push_back(ReadLine);
			LineIndex++;
		}

		File.close();

		Status = STATUS_SUCCESS;
	} while (0);
	return Status;
}

int WINAPI ExtractPsbText(FILE* Stream, PBYTE& OutBuffer, ULONG& OutSize, LPCWSTR FileName)
{
	FILE_INFO FileInfo = { 0 };
	DWORD     ResInfo = 0;
	FileInfo.dwCharSet = CS_UTF8;
	FileInfo.hStream = Stream;

	StringPool.clear();
	GetText(&FileInfo, &ResInfo);

	if (MyReplaceText(FileName) < 0)
	{
		printf("Read failed\n");
		return E_ERROR;
	}

	int iPos = 0;
	for (auto& it : StringPool)
	{
		ModifyLine(&FileInfo, iPos, it);
		iPos++;
	}
	auto Ret = SaveText(&FileInfo, OutBuffer, OutSize);
	StringPool.clear();
	fclose(Stream);
	return Ret;
}


