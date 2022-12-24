#pragma once

typedef unsigned char BitSequence;
typedef unsigned long long DataLength;

typedef enum { SUCCESS = 0, FAIL = 1, BAD_HASHLEN = 2 } HashReturn;
/* hash a message,
three inputs: message digest size in bits (hashbitlen); message (data); message length in bits (databitlen)
one output:   message digest (hashval)
*/
HashReturn JHHash(int hashbitlen, const BitSequence *data, DataLength databitlen, BitSequence *hashval);

