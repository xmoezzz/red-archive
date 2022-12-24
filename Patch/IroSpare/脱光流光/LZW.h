#ifndef __LZW_H
#define __LZW_H

#include <Windows.h>

#define	compLT(a, b)	(a->key_word < b->key_word)
#define	compEQ(a, b)	(a->key_word == b->key_word)
#include "redblack.h"

class CLZW
{
private:
	struct tagNode;
	struct tagKey;
	struct tagKey
	{
		BYTE	key;
		tagNode *node;
		tagNode *pnode;
		tagKey	*prev;
		tagKey	*next;
	};
	struct tagNode
	{
		DWORD	key_word;
		long	level;
		tagKey	nill_key;
		tagKey *parent;
	} nill;
	DWORD LastKey;
	DWORD border;
	DWORD bits_len;
	typedef TRedBlack<tagNode*>	RedBlackNode;
	RedBlackNode tree;
private:
	// source can`t be more then 2^28 bytes (2^28 = 268435456)
	// key-word 0 is predefined for single character
	void bitscpy(BYTE *target, long &toffset, BYTE *source, long &soffset, long bitslen);

	tagNode *FindKeyWord(DWORD key_word);
	tagNode *FindKey(tagNode *node, BYTE key);
	tagNode *AddKey(tagNode *node, DWORD key_word, BYTE key);
	DWORD GenerateKey();

	void DeleteNode(tagNode *node);
	void DeleteKey(tagKey *key);
	void CleanAll();
	void InitTable();

	long EncodeOnce(BYTE *target, long &toffset, BYTE *source, long slength, tagNode **last_node);
	long DecodeOnce(BYTE *target, BYTE *source, long &soffset, tagNode **last_node);
public:
	CLZW();
	virtual ~CLZW();

	void Encode(BYTE *target, long &tlen, BYTE *source, long slen);
	long Decode(BYTE *target, long &tlen, BYTE *source, long slen);
	long GetMaxEncoded(long len);
	long GetMaxDecoded(BYTE *source);

	virtual void OnStep() = 0;
};

#endif
