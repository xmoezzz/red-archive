#ifndef __HUFFMAN_H
#define __HUFFMAN_H

#include <Windows.h>

class CHuffman
{
private:
	struct tagNode
	{
		int		key;

		tagNode	*up;
		tagNode	*left;
		tagNode	*right;
	};
	struct tagList
	{
		int		weight;
		tagNode	*node;

		tagList	*next;
		tagList	*prev;
	} nill;
	struct tagTable
	{
		BYTE	count;		// count of bits
		BYTE	*buf;		// dynamic allocated array of bits
	} table[257];

	void ListInit(DWORD *w);		// init by list of 256 weights
	void ListAddSorted(int weight, tagNode *node);
	void ListDelete(tagList *cur);
	tagList *ListExtractMin();
	int ListIsLast();
	int ListIsEmpty();

	int RecTableFillBits(tagNode *node, int level = 0);
	void BitToStream(int bit, BYTE *strm, int &p, int &o, bool clean = true);
	void StreamToBit(int &bit, BYTE *strm, int &p, int &o);
	void StreamToStream(BYTE *target, BYTE *source, int bit_len, int &sp, int &so, int &tp, int &to, bool clean = true);
	void FreeList();
	void FreeNode(tagNode *node);
	void FreeTable();
	int FindKey(BYTE *buf);

	void InitTable(BYTE *buf, long size);
	int GetTableLength();
	int GetTable(BYTE *buf);
	int SetTable(BYTE *buf);
	void EncodeHuffman(BYTE *target, long &tlen, BYTE *source, long slen);
	long DecodeHuffman(BYTE *target, long &tlen, BYTE *source, long slen);
public:
	CHuffman();
	virtual ~CHuffman();
	void Encode(BYTE *target, long &tlen, BYTE *source, long slen);
	long Decode(BYTE *target, long &tlen, BYTE *source, long slen);
	long GetMaxEncoded(long len);
	long GetMaxDecoded(BYTE *source);

};

#endif
