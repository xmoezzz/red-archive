#pragma once

#include "ecrypt-portable.h"

/* ------------------------------------------------------------------------- */

/* Cipher parameters */

/*
* Specify which key and IV sizes are supported by your cipher. A user
* should be able to enumerate the supported sizes by running the
* following code:
*
* for (i = 0; ChaECRYPT_KEYSIZE(i) <= ChaECRYPT_MAXKEYSIZE; ++i)
*   {
*     keysize = ChaECRYPT_KEYSIZE(i);
*
*     ...
*   }
*
* All sizes are in bits.
*/

#define ChaECRYPT_MAXKEYSIZE 256                 /* [edit] */
#define ChaECRYPT_KEYSIZE(i) (128 + (i)*128)     /* [edit] */

#define ChaECRYPT_MAXIVSIZE 64                   /* [edit] */
#define ChaECRYPT_IVSIZE(i) (64 + (i)*64)        /* [edit] */

/* ------------------------------------------------------------------------- */

/* Data structures */

/*
* ChaECRYPT_ctx is the structure containing the representation of the
* internal state of your cipher.
*/

typedef struct
{
	u32 input[16]; /* could be compressed */
	/*
	* [edit]
	*
	* Put here all state variable needed during the encryption process.
	*/
} ChaECRYPT_ctx;

/* ------------------------------------------------------------------------- */

/* Mandatory functions */

/*
* Key and message independent initialization. This function will be
* called once when the program starts (e.g., to build expanded S-box
* tables).
*/
void ChaECRYPT_init();

/*
* Key setup. It is the user's responsibility to select the values of
* keysize and ivsize from the set of supported values specified
* above.
*/
void ChaECRYPT_keysetup(
	ChaECRYPT_ctx* ctx,
	const u8* key,
	u32 keysize,                /* Key size in bits. */
	u32 ivsize);                /* IV size in bits. */

/*
* IV setup. After having called ChaECRYPT_keysetup(), the user is
* allowed to call ChaECRYPT_ivsetup() different times in order to
* encrypt/decrypt different messages with the same key but different
* IV's.
*/
void ChaECRYPT_ivsetup(
	ChaECRYPT_ctx* ctx,
	const u8* iv);

/*
* Encryption/decryption of arbitrary length messages.
*
* For efficiency reasons, the API provides two types of
* encrypt/decrypt functions. The ChaECRYPT_encrypt_bytes() function
* (declared here) encrypts byte strings of arbitrary length, while
* the ChaECRYPT_encrypt_blocks() function (defined later) only accepts
* lengths which are multiples of ChaECRYPT_BLOCKLENGTH.
*
* The user is allowed to make multiple calls to
* ChaECRYPT_encrypt_blocks() to incrementally encrypt a long message,
* but he is NOT allowed to make additional encryption calls once he
* has called ChaECRYPT_encrypt_bytes() (unless he starts a new message
* of course). For example, this sequence of calls is acceptable:
*
* ChaECRYPT_keysetup();
*
* ChaECRYPT_ivsetup();
* ChaECRYPT_encrypt_blocks();
* ChaECRYPT_encrypt_blocks();
* ChaECRYPT_encrypt_bytes();
*
* ChaECRYPT_ivsetup();
* ChaECRYPT_encrypt_blocks();
* ChaECRYPT_encrypt_blocks();
*
* ChaECRYPT_ivsetup();
* ChaECRYPT_encrypt_bytes();
*
* The following sequence is not:
*
* ChaECRYPT_keysetup();
* ChaECRYPT_ivsetup();
* ChaECRYPT_encrypt_blocks();
* ChaECRYPT_encrypt_bytes();
* ChaECRYPT_encrypt_blocks();
*/

void ChaECRYPT_encrypt_bytes(
	ChaECRYPT_ctx* ctx,
	const u8* plaintext,
	u8* ciphertext,
	u32 msglen);                /* Message length in bytes. */

void ChaECRYPT_decrypt_bytes(
	ChaECRYPT_ctx* ctx,
	const u8* ciphertext,
	u8* plaintext,
	u32 msglen);                /* Message length in bytes. */

/* ------------------------------------------------------------------------- */

/* Optional features */

/*
* For testing purposes it can sometimes be useful to have a function
* which immediately generates keystream without having to provide it
* with a zero plaintext. If your cipher cannot provide this function
* (e.g., because it is not strictly a synchronous cipher), please
* reset the ChaECRYPT_GENERATES_KEYSTREAM flag.
*/

#define ChaECRYPT_GENERATES_KEYSTREAM
#ifdef ChaECRYPT_GENERATES_KEYSTREAM

void ChaECRYPT_keystream_bytes(
	ChaECRYPT_ctx* ctx,
	u8* keystream,
	u32 length);                /* Length of keystream in bytes. */

#endif

/* ------------------------------------------------------------------------- */

/* Optional optimizations */

/*
* By default, the functions in this section are implemented using
* calls to functions declared above. However, you might want to
* implement them differently for performance reasons.
*/

/*
* All-in-one encryption/decryption of (short) packets.
*
* The default definitions of these functions can be found in
* "ecrypt-sync.c". If you want to implement them differently, please
* undef the ChaECRYPT_USES_DEFAULT_ALL_IN_ONE flag.
*/
#define ChaECRYPT_USES_DEFAULT_ALL_IN_ONE        /* [edit] */

void ChaECRYPT_encrypt_packet(
	ChaECRYPT_ctx* ctx,
	const u8* iv,
	const u8* plaintext,
	u8* ciphertext,
	u32 msglen);

void ChaECRYPT_decrypt_packet(
	ChaECRYPT_ctx* ctx,
	const u8* iv,
	const u8* ciphertext,
	u8* plaintext,
	u32 msglen);

/*
* Encryption/decryption of blocks.
*
* By default, these functions are defined as macros. If you want to
* provide a different implementation, please undef the
* ChaECRYPT_USES_DEFAULT_BLOCK_MACROS flag and implement the functions
* declared below.
*/

#define ChaECRYPT_BLOCKLENGTH 64                  /* [edit] */

#define ChaECRYPT_USES_DEFAULT_BLOCK_MACROS      /* [edit] */
#ifdef ChaECRYPT_USES_DEFAULT_BLOCK_MACROS

#define ChaECRYPT_encrypt_blocks(ctx, plaintext, ciphertext, blocks)  \
  ChaECRYPT_encrypt_bytes(ctx, plaintext, ciphertext,                 \
    (blocks) * ChaECRYPT_BLOCKLENGTH)

#define ChaECRYPT_decrypt_blocks(ctx, ciphertext, plaintext, blocks)  \
  ChaECRYPT_decrypt_bytes(ctx, ciphertext, plaintext,                 \
    (blocks) * ChaECRYPT_BLOCKLENGTH)

#ifdef ChaECRYPT_GENERATES_KEYSTREAM

#define ChaECRYPT_keystream_blocks(ctx, keystream, blocks)            \
  ChaECRYPT_keystream_bytes(ctx, keystream,                        \
    (blocks) * ChaECRYPT_BLOCKLENGTH)

#endif

#else

void ChaECRYPT_encrypt_blocks(
	ChaECRYPT_ctx* ctx,
	const u8* plaintext,
	u8* ciphertext,
	u32 blocks);                /* Message length in blocks. */

void ChaECRYPT_decrypt_blocks(
	ChaECRYPT_ctx* ctx,
	const u8* ciphertext,
	u8* plaintext,
	u32 blocks);                /* Message length in blocks. */

#ifdef ChaECRYPT_GENERATES_KEYSTREAM

void ChaECRYPT_keystream_blocks(
	ChaECRYPT_ctx* ctx,
	const u8* keystream,
	u32 blocks);                /* Keystream length in blocks. */

#endif

#endif

/*
* If your cipher can be implemented in different ways, you can use
* the ChaECRYPT_VARIANT parameter to allow the user to choose between
* them at compile time (e.g., gcc -DChaECRYPT_VARIANT=3 ...). Please
* only use this possibility if you really think it could make a
* significant difference and keep the number of variants
* (ChaECRYPT_MAXVARIANT) as small as possible (definitely not more than
* 10). Note also that all variants should have exactly the same
* external interface (i.e., the same ChaECRYPT_BLOCKLENGTH, etc.).
*/
#define ChaECRYPT_MAXVARIANT 1                   /* [edit] */

#ifndef ChaECRYPT_VARIANT
#define ChaECRYPT_VARIANT 1
#endif

#if (ChaECRYPT_VARIANT > ChaECRYPT_MAXVARIANT)
#error this variant does not exist
#endif

/* ------------------------------------------------------------------------- */

