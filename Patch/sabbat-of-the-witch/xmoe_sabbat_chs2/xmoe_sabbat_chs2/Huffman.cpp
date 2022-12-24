#include "Huffman.h"
#include <string.h>

#define MAX_LENGTH		9		// bits in symbol it can`t be coded
#define ESC_END			4		// bits in symbol to start code from it
#define ESC_WEIDTH		8		// weight of ESC symbol is source_length/ESC_WEIDTH
#define ESC_LENGTH		4		// bits for count not coded symbols
#define MAX_NOT_CODED	16		// 2 pow ESC_LENGTH
#define MAX_TABLE		256		// maximum number of symbols in table
#define ESC				256		// ESC symbol always 256


VOID WINAPI FatalError()
{
	MessageBoxW(NULL, L"Bad Stream", L"X'moe CoreLib", MB_OK);
	ExitProcess(-1);
}

CHuffman::CHuffman()
{
	memset(&nill, 0, sizeof(nill));
	nill.next = nill.prev = &nill;
	memset(table, 0, (ESC + 1)*sizeof(tagTable));
}
CHuffman::~CHuffman()
{
	FreeTable();
}
void CHuffman::ListInit(DWORD *w)		// init by list of ESC+1 weights
{
	int i;
	tagNode *node;
	for (i = 0; i < ESC + 1; i++)
	{
		node = new tagNode;
		memset(node, 0, sizeof(tagNode));
		node->key = i;
		ListAddSorted(w[i], node);
	}
}
void CHuffman::ListAddSorted(int weight, CHuffman::tagNode *node)
{
	tagList *cur, *cur1;
	cur1 = new tagList;
	memset(cur1, 0, sizeof(tagList));
	cur1->weight = weight;
	cur1->node = node;
	for (cur = nill.next; cur != &nill; cur = cur->next)
	if (cur->weight >= weight) break;
	cur1->next = cur;
	cur1->prev = cur->prev;
	cur1->prev->next = cur1;
	cur1->next->prev = cur1;
}
void CHuffman::ListDelete(CHuffman::tagList *cur)
{
	if (cur == &nill) return;
	cur->next->prev = cur->prev;
	cur->prev->next = cur->next;
	delete cur;
}
CHuffman::tagList *CHuffman::ListExtractMin()
{
	tagList *cur = nill.next;
	if (nill.next == &nill) return NULL;
	cur->next->prev = cur->prev;
	cur->prev->next = cur->next;
	return cur;
}
int CHuffman::ListIsLast()
{
	return (nill.next->next == &nill);
}
int CHuffman::ListIsEmpty()
{
	return (nill.next == &nill);
}

int CHuffman::RecTableFillBits(tagNode *node, int level)
{
	int left, right;
	int p, o;
	tagNode *cur;
	if (level > MAX_LENGTH) return 1;
	if (node == NULL) return 0;
	left = RecTableFillBits(node->left, level + 1);
	right = RecTableFillBits(node->right, level + 1);
	if (!left && !right)
	{
		table[node->key].count = (BYTE)level;
		table[node->key].buf = new BYTE[level / 8 + 1];
		memset(table[node->key].buf, 0, level / 8 + 1);
		p = level / 8;
		o = level % 8;
		// bits are inserted in reverce order
		for (cur = node; cur != NULL; cur = cur->up)
		if (cur->up != NULL)
		{
			if ((--o) < 0)
			{
				o = 7;
				p--;
			}
			if (p < 0) return 1;

			if (cur->up->right == cur)
				table[node->key].buf[p] |= 1 << o;
		}
	}
	return 1;
}

void CHuffman::BitToStream(int bit, BYTE *strm, int &p, int &o, bool clean)
{
	bit = bit ? 1 : 0;
	if (o >= 8)
	{
		p++;
		o = 0;
	}
	if (clean && !o) strm[p] = 0;
	strm[p] |= bit << (o++);
}
void CHuffman::StreamToBit(int &bit, BYTE *strm, int &p, int &o)
{
	if (o >= 8)
	{
		p++;
		o = 0;
	}
	bit = (strm[p] & 1 << (o++)) ? 1 : 0;
}
void CHuffman::StreamToStream(BYTE *target, BYTE *source, int bit_len, int &sp, int &so, int &tp, int &to, bool clean)
{
	int i, bit;
	for (i = 0; i < bit_len; i++)
	{
		StreamToBit(bit, source, sp, so);
		BitToStream(bit, target, tp, to, clean);
	}
}

void CHuffman::FreeList()
{
	while (!ListIsEmpty())
	{
		FreeNode(nill.next->node);
		ListDelete(nill.next);
	}
}
void CHuffman::FreeNode(tagNode *node)
{
	if (node == NULL) return;
	FreeNode(node->left);
	FreeNode(node->right);
	delete node;
}
void CHuffman::FreeTable()
{
	int i;
	for (i = 0; i < ESC + 1; i++)
	if (table[i].buf != NULL)
		delete table[i].buf;
	memset(table, 0, (ESC + 1)*sizeof(table[0]));
}

void CHuffman::InitTable(BYTE *buf, long size)
{
	DWORD w[ESC + 1];
	int i;
	tagList *l1, *l2;
	tagNode *node = NULL;

	FreeTable();

	memset(w, 0, (ESC + 1)*sizeof(DWORD));
	for (i = 0; i < size; i++)
		w[buf[i]]++;
	w[ESC] = size / ESC_WEIDTH;
	if (!w[ESC])
		w[ESC]++;

	ListInit(w);

	while (!ListIsLast())
	{
		l1 = ListExtractMin();
		l2 = ListExtractMin();
		node = new tagNode;
		memset(node, 0, sizeof(tagNode));
		node->left = l1->node;
		node->right = l2->node;
		l1->node->up = node;
		l2->node->up = node;
		ListAddSorted(l1->weight + l2->weight, node);
		delete l1;
		delete l2;
	}

	if (ListIsEmpty())
		FatalError();

	RecTableFillBits(node);
	FreeList();

	if (!table[ESC].count)
	{
		FatalError();
	}
}
int CHuffman::GetTableLength()
{
	int i, bit_len = 0;
	bit_len += 8;		// place for sizeof table
	bit_len += sizeof(BYTE)* 8 + table[ESC].count;
	for (i = 0; i < ESC; i++)
	if (table[i].count)
		bit_len += sizeof(BYTE)* 8 * 2 + table[i].count;
	return bit_len / 8 + 1;
}
int CHuffman::GetTable(BYTE *buf)
{
	int sp, tp, so, to;
	int i, bit_len = 0;
	BYTE len_tab = 0;
	tp = to = 0;
	bit_len += sizeof(BYTE)* 8;
	tp++;			// one byte for table length

	sp = so = 0;
	StreamToStream(buf, &table[ESC].count, sizeof(BYTE)* 8, sp, so, tp, to);
	sp = so = 0;
	StreamToStream(buf, table[ESC].buf, table[ESC].count, sp, so, tp, to);
	bit_len += sizeof(BYTE)* 8 + table[ESC].count;

	for (i = 0; i < ESC; i++)
	if (table[i].count)
	{
		sp = so = 0;
		StreamToStream(buf, (BYTE*)&i, sizeof(BYTE)* 8, sp, so, tp, to);
		sp = so = 0;
		StreamToStream(buf, &table[i].count, sizeof(BYTE)* 8, sp, so, tp, to);
		sp = so = 0;
		StreamToStream(buf, table[i].buf, table[i].count, sp, so, tp, to);
		bit_len += sizeof(BYTE)* 8 * 2 + table[i].count;
		len_tab++;
	}
	sp = so = 0;
	tp = to = 0;
	StreamToStream(buf, &len_tab, sizeof(BYTE)* 8, sp, so, tp, to);

	return bit_len / 8 + 1;
}
int CHuffman::SetTable(BYTE *buf)
{
	int sp, tp, so, to;
	int i;
	BYTE tab_len, key;

	FreeTable();

	sp = so = 0;
	tp = to = 0;
	StreamToStream(&tab_len, buf, sizeof(BYTE)* 8, sp, so, tp, to);

	tp = to = 0;
	StreamToStream(&table[ESC].count, buf, sizeof(BYTE)* 8, sp, so, tp, to);
	table[ESC].buf = new BYTE[table[ESC].count / 8 + 1];
	memset(table[ESC].buf, 0, table[ESC].count / 8 + 1);
	tp = to = 0;
	StreamToStream(table[ESC].buf, buf, table[ESC].count, sp, so, tp, to);

	for (i = 0; i < tab_len; i++)
	{
		tp = to = 0;
		StreamToStream(&key, buf, sizeof(BYTE)* 8, sp, so, tp, to);
		tp = to = 0;
		StreamToStream(&table[key].count, buf, sizeof(BYTE)* 8, sp, so, tp, to);
		table[key].buf = new BYTE[table[key].count / 8 + 1];
		memset(table[key].buf, 0, table[key].count / 8 + 1);
		tp = to = 0;
		StreamToStream(table[key].buf, buf, table[key].count, sp, so, tp, to);
	}
	// size must be equal to sp+1
	return sp + 1 + ((so >= 8) ? 1 : 0);
}
int CHuffman::FindKey(BYTE *buf)
{
	int i, j, k;
	BYTE last;
	int ok;
	for (i = 0; i < ESC + 1; i++)
	if (table[i].count)
	{
		ok = 1;
		for (j = 0; j < table[i].count / 8 + 1; j++)
		if (j == table[i].count / 8)
		{
			if (!(table[i].count % 8)) return i;
			last = 0;
			for (k = 0; k < table[i].count % 8; k++)
				last |= buf[j] & (1 << k);
			if (last == table[i].buf[j])
				return i;
			else
			{
				ok = 0;
				break;
			}
		}
		else	if (table[i].buf[j] != buf[j])
		{
			ok = 0;
			break;
		}
	}
	FatalError();
	return -1;
}
void CHuffman::EncodeHuffman(BYTE *target, long &tlen, BYTE *source, long slen)
{
	int sp, so, tp, to;
	int ep, eo, tsp, tso;
	int i, count = 0;
	DWORD tmp;
	int status = 1;		// 1 - encode, 0 - escape
	tp = to = 0;
	ep = eo = 0;
	target += sizeof(DWORD);
	for (i = 0; i < slen; i++)
	{
		if (status)
		{
			if (table[source[i]].count)
			{
				sp = so = 0;
				StreamToStream(target, table[source[i]].buf, table[source[i]].count, sp, so, tp, to);
			}
			else
			{
				status = 0;
				sp = so = 0;
				StreamToStream(target, table[ESC].buf, table[ESC].count, sp, so, tp, to);
				ep = tp; eo = to;		// save address to save value later
				tsp = tso = 0;
				count = 0;
				tmp = 0;				// value will be computed later
				StreamToStream(target, (BYTE*)&tmp, ESC_LENGTH, tsp, tso, tp, to);
			}
		}
		if (!status)
		{
			if ((!table[source[i]].count || (table[source[i]].count > ESC_END)) && (count < MAX_NOT_CODED - 1))
			{
				sp = so = 0;
				StreamToStream(target, &source[i], sizeof(BYTE)* 8, sp, so, tp, to);
				count++;
			}
			else
			{
				status = 1;
				tsp = tso = 0;
				sp = so = 0;
				StreamToStream(target, (BYTE*)&count, ESC_LENGTH, tsp, tso, ep, eo, false);
				if (count < MAX_NOT_CODED - 1)
					StreamToStream(target, table[source[i]].buf, table[source[i]].count, sp, so, tp, to);
				else i--;
			}
		}
		if (tp > slen)
		{
			tlen = 0;
			return;
		}
		//OnStep();
	}
	if (!status)
	{
		tsp = tso = 0;
		StreamToStream(target, (BYTE*)&count, ESC_LENGTH, tsp, tso, ep, eo, false);
	}
	tlen = tp + 1 + sizeof(DWORD);
	target -= sizeof(DWORD);
	((DWORD*)target)[0] = slen;
}
long CHuffman::DecodeHuffman(BYTE *target, long &tlen, BYTE *source, long slen)
{
	int sp, so, tp, to, tsp, tso, ttp, tto;
	int i, len_buf;
	int status = 1, count = 0;
	int key;
	BYTE buf[MAX_LENGTH / 8 + 1];
	sp = so = 0;
	len_buf = MAX_LENGTH;
	tlen = ((DWORD*)source)[0];
	source += sizeof(DWORD);
	for (i = 0; i < tlen; i++)
	{
		if (slen - sp < len_buf / 8 + 1)
			len_buf = (slen - sp) * 8;
		if (status)
		{
			tsp = sp; tso = so;
			ttp = tto = 0;
			StreamToStream(buf, source, len_buf, tsp, tso, ttp, tto);
			key = FindKey(buf);
			if (key != ESC)
			{
				tp = to = 0;
				StreamToStream(buf, source, table[key].count, sp, so, tp, to);
				target[i] = (BYTE)key;
			}
			else
			{
				count = 0;
				tp = to = 0;
				StreamToStream(buf, source, table[ESC].count, sp, so, tp, to);
				tp = to = 0;
				StreamToStream((BYTE*)&count, source, ESC_LENGTH, sp, so, tp, to);
				status = 0;
				if (!count)
				{
					FatalError();
				}
			}
		}
		if (!status)
		{
			tp = to = 0;
			StreamToStream(&target[i], source, sizeof(BYTE)* 8, sp, so, tp, to);
			if (!(--count))
				status = 1;
		}
		//OnStep();
	}
	return sp + 1 + sizeof(DWORD);
}
void CHuffman::Encode(BYTE *target, long &tlen, BYTE *source, long slen)
{
	long hlen;
	InitTable(source, slen);
	hlen = GetTable(target);
	EncodeHuffman(target + hlen, tlen, source, slen);
	if (tlen) tlen += hlen;
}
long CHuffman::Decode(BYTE *target, long &tlen, BYTE *source, long slen)
{
	long hlen;
	hlen = SetTable(source);
	return hlen + DecodeHuffman(target, tlen, source + hlen, slen - hlen);
}
long CHuffman::GetMaxEncoded(long len)
{
	return 257 * (2 + 32) + len + 256;
}
long CHuffman::GetMaxDecoded(BYTE *source)
{
	long tab_len = SetTable(source);
	return ((DWORD*)(source + tab_len))[0];
}
