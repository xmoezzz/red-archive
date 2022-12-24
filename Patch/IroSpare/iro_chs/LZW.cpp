// Implementation of Lempel-Ziv & Welch compression in its pure form.
// Appear to be one of most popular looseless compression algorithms.
//
// Implemented by Arkadi Kagan
//
#include "LZW.h"
#include "my.h"

CLZW::CLZW()
{
	memset(&nill, 0, sizeof(nill));
	nill.nill_key.next = nill.nill_key.prev = &nill.nill_key;
	nill.nill_key.pnode = &nill;
	LastKey = 0;
	border = 2;
	bits_len = 1;

	InitTable();
}
CLZW::~CLZW()
{
	CleanAll();
}

void CLZW::CleanAll()
{
	while (tree.root != tree.NIL)
		tree.deleteNode(tree.root);

	while (nill.nill_key.next != &nill.nill_key)
		DeleteKey(nill.nill_key.next);
	LastKey = 0;
	border = 2;
	bits_len = 1;

	InitTable();
}
void CLZW::InitTable()
{
	int i;
	for (i = 0; i < 256; i++)
		AddKey(&nill, GenerateKey(), (BYTE)i);
}

void CLZW::bitscpy(BYTE *target, long &toffset, BYTE *source, long &soffset, long bitslen)
{
	long i;
	for (i = 0; i < bitslen; i++)
		target[(toffset + i) / 8] |= ((source[(soffset + i) / 8] >> ((soffset + i) % 8)) & 1) << ((toffset + i) % 8);
	toffset += bitslen;
	soffset += bitslen;
}

CLZW::tagNode *CLZW::FindKeyWord(DWORD key_word)
{
	tagNode tmpNode;
	tmpNode.key_word = key_word;
	RedBlackNode::Node *tnode = tree.findNode(&tmpNode);
	if (tnode == NULL)
		return NULL;
	return tnode->data;
}
CLZW::tagNode *CLZW::FindKey(CLZW::tagNode *node, BYTE key)
{
	tagKey *cur;
	if (node == NULL) node = &nill;
	for (cur = node->nill_key.next; cur != &node->nill_key; cur = cur->next)
		if (cur->key == key)
			return cur->node;
	return NULL;
}
CLZW::tagNode *CLZW::AddKey(CLZW::tagNode *node, DWORD key_word, BYTE key)
{
	if (node == NULL) node = &nill;

	tagKey *cur_key = new tagKey;
	tagNode *cur_node = new tagNode;
	memset(cur_key, 0, sizeof(tagKey));
	memset(cur_node, 0, sizeof(tagNode));

	cur_key->key = key;
	cur_key->node = cur_node;
	cur_key->pnode = node;

	cur_key->next = node->nill_key.next;
	cur_key->prev = &node->nill_key;
	cur_key->next->prev = cur_key;
	cur_key->prev->next = cur_key;

	cur_node->nill_key.next = cur_node->nill_key.prev = &cur_node->nill_key;
	cur_node->nill_key.pnode = cur_node;

	cur_node->key_word = key_word;
	cur_node->parent = cur_key;
	cur_node->level = node->level + 1;

	tree.insertNode(cur_node);

	return cur_node;
}
DWORD CLZW::GenerateKey()
{
	if (LastKey >= border - 1)
	{
		border = border << 1;
		bits_len++;
	}
	//if (bits_len >= 8 * sizeof(DWORD))
	//	FatalError("Dictionary is full. Source is too big or demaged");
	return ++LastKey;
}

void CLZW::DeleteNode(CLZW::tagNode *node)
{
	while (node->nill_key.next != &node->nill_key)
		DeleteKey(node->nill_key.next);
}
void CLZW::DeleteKey(CLZW::tagKey *key)
{
	DeleteNode(key->node);

	key->next->prev = key->prev;
	key->prev->next = key->next;

	delete key->node;
	delete key;
}

// slength must be length of source sequence in bytes
long CLZW::EncodeOnce(BYTE *target, long &toffset, BYTE *source, long slength, CLZW::tagNode **last_node)
{
	long i, offset;
	tagNode *node = &nill, *tnode;
	for (i = 0; i < slength; i++)
	{
		tnode = FindKey(node, source[i]);
		if (tnode == NULL)
		{
			offset = 0;
			bitscpy(target, toffset, (BYTE*)&node->key_word, offset, bits_len);
			if (*last_node != NULL)
				if (FindKey(*last_node, source[0]) == NULL)
					AddKey(*last_node, GenerateKey(), source[0]);
			*last_node = node;
			return i;
		}
		node = tnode;
	}
	if (node != &nill)
	{
		offset = 0;
		bitscpy(target, toffset, (BYTE*)&node->key_word, offset, bits_len);
	}
	return slength;
}
long CLZW::DecodeOnce(BYTE *target, BYTE *source, long &soffset, CLZW::tagNode **last_node)
{
	tagNode *node, *fnode = NULL;
	long len = 0, offset = 0;
	DWORD key_word = 0;
	bitscpy((BYTE*)&key_word, offset, source, soffset, bits_len);
	node = FindKeyWord(key_word);
	if (node != NULL)
	{
		fnode = node;
		len = node->level;
		while (node->parent != NULL)
		{
			target[node->level - 1] = node->parent->key;
			node = node->parent->pnode;
		}
		if (*last_node != NULL)
			if (FindKey(*last_node, target[0]) == NULL)
				AddKey(*last_node, GenerateKey(), target[0]);
		*last_node = fnode;
	}
	else
	{
		//FatalError("Source is damaged");
	}
	return len;
}

long CLZW::GetMaxEncoded(long len)
{
	return len + sizeof(DWORD);
}
long CLZW::GetMaxDecoded(BYTE *source)
{
	return ((DWORD*)source)[0];
}
void CLZW::Encode(BYTE *target, long &tlen, BYTE *source, long slen)
{
	long toffset = 0;
	long coded = 0;
	long tmp;
	tagNode *last_node = NULL;
	DWORD *size = (DWORD*)target;
	memset(target, 0, tlen);
	*size = slen;
	target += sizeof(DWORD);
	tlen = sizeof(DWORD);
	while ((tmp = EncodeOnce(target, toffset, source + coded, slen - coded, &last_node)) != 0)
	{
		coded += tmp;
		if (toffset / 8 >= slen)
		{
			tlen = 0;
			return;
		}
		OnStep();
	}
	tlen += toffset / 8 + 1;

	CleanAll();
}
long CLZW::Decode(BYTE *target, long &tlen, BYTE *source, long)
{
	long coded = 0;
	long offset = 0;
	tagNode *last_node = NULL;
	tlen = ((DWORD*)source)[0];
	source += sizeof(DWORD);
	memset(target, 0, tlen);

	while (coded < tlen)
	{
		coded += DecodeOnce(target + coded, source, offset, &last_node);
		OnStep();
	}
	CleanAll();

	return offset / 8 + 1 + sizeof(DWORD);
}
