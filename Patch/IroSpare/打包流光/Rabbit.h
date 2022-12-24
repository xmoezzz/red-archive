#pragma once

#include "ecrypt-portable.h"

/* ------------------------------------------------------------------------- */

/* Cipher parameters */
/*
* Specify which key and IV sizes are supported by your cipher. A user
* should be able to enumerate the supported sizes by running the
* following code:
*
* for (i = 0; RaECRYPT_KEYSIZE(i) <= RaECRYPT_MAXKEYSIZE; ++i)
*   {
*     keysize = RaECRYPT_KEYSIZE(i);
*
*     ...
*   }
*
* All sizes are in bits.
*/

#define RaECRYPT_MAXKEYSIZE 128
#define RaECRYPT_KEYSIZE(i) (128 + (i)*32)

#define RaECRYPT_MAXIVSIZE 64
#define RaECRYPT_IVSIZE(i) (64 + (i)*64)

/* ------------------------------------------------------------------------- */

/* Data structures */

/*
* RaECRYPT_ctx is the structure containing the representation of the
* internal state of your cipher.
*/

typedef struct
{
	u32 x[8];
	u32 c[8];
	u32 carry;
} RABBIT_ctx;

typedef struct
{
	/*
	* Put here all state variable needed during the encryption process.
	*/
	RABBIT_ctx master_ctx;
	RABBIT_ctx work_ctx;
} RaECRYPT_ctx;

/* ------------------------------------------------------------------------- */

/* Mandatory functions */

/*
* Key and message independent initialization. This function will be
* called once when the program starts (e.g., to build expanded S-box
* tables).
*/
void RaECRYPT_init(void);

/*
* Key setup. It is the user's responsibility to select the values of
* keysize and ivsize from the set of supported values specified
* above.
*/
void RaECRYPT_keysetup(
	RaECRYPT_ctx* ctx,
	const u8* key,
	u32 keysize,                /* Key size in bits. */
	u32 ivsize);                /* IV size in bits. */

/*
* IV setup. After having called RaECRYPT_keysetup(), the user is
* allowed to call RaECRYPT_ivsetup() different times in order to
* encrypt/decrypt different messages with the same key but different
* IV's.
*/
void RaECRYPT_ivsetup(
	RaECRYPT_ctx* ctx,
	const u8* iv);

/*
* Encryption/decryption of arbitrary length messages.
*
* For efficiency reasons, the API provides two types of
* encrypt/decrypt functions. The RaECRYPT_encrypt_bytes() function
* (declared here) encrypts byte strings of arbitrary length, while
* the RaECRYPT_encrypt_blocks() function (defined later) only accepts
* lengths which are multiples of RaECRYPT_BLOCKLENGTH.
*
* The user is allowed to make multiple calls to
* RaECRYPT_encrypt_blocks() to incrementally encrypt a long message,
* but he is NOT allowed to make additional encryption calls once he
* has called RaECRYPT_encrypt_bytes() (unless he starts a new message
* of course). For example, this sequence of calls is acceptable:
*
* RaECRYPT_keysetup();
*
* RaECRYPT_ivsetup();
* RaECRYPT_encrypt_blocks();
* RaECRYPT_encrypt_blocks();
* RaECRYPT_encrypt_bytes();
*
* RaECRYPT_ivsetup();
* RaECRYPT_encrypt_blocks();
* RaECRYPT_encrypt_blocks();
*
* RaECRYPT_ivsetup();
* RaECRYPT_encrypt_bytes();
*
* The following sequence is not:
*
* RaECRYPT_keysetup();
* RaECRYPT_ivsetup();
* RaECRYPT_encrypt_blocks();
* RaECRYPT_encrypt_bytes();
* RaECRYPT_encrypt_blocks();
*/

/*
* By default RaECRYPT_encrypt_bytes() and RaECRYPT_decrypt_bytes() are
* defined as macros which redirect the call to a single function
* RaECRYPT_process_bytes(). If you want to provide separate encryption
* and decryption functions, please undef
* RaECRYPT_HAS_SINGLE_BYTE_FUNCTION.
*/
#define RaECRYPT_HAS_SINGLE_BYTE_FUNCTION
#ifdef RaECRYPT_HAS_SINGLE_BYTE_FUNCTION

#define RaECRYPT_encrypt_bytes(ctx, plaintext, ciphertext, msglen)   \
  RaECRYPT_process_bytes(0, ctx, plaintext, ciphertext, msglen)

#define RaECRYPT_decrypt_bytes(ctx, ciphertext, plaintext, msglen)   \
  RaECRYPT_process_bytes(1, ctx, ciphertext, plaintext, msglen)

void RaECRYPT_process_bytes(
	int action,                 /* 0 = encrypt; 1 = decrypt; */
	RaECRYPT_ctx* ctx,
	const u8* input,
	u8* output,
	u32 msglen);                /* Message length in bytes. */

#else

void RaECRYPT_encrypt_bytes(
	RaECRYPT_ctx* ctx,
	const u8* plaintext,
	u8* ciphertext,
	u32 msglen);                /* Message length in bytes. */

void RaECRYPT_decrypt_bytes(
	RaECRYPT_ctx* ctx,
	const u8* ciphertext,
	u8* plaintext,
	u32 msglen);                /* Message length in bytes. */

#endif

/* ------------------------------------------------------------------------- */

/* Optional features */

/*
* For testing purposes it can sometimes be useful to have a function
* which immediately generates keystream without having to provide it
* with a zero plaintext. If your cipher cannot provide this function
* (e.g., because it is not strictly a synchronous cipher), please
* reset the RaECRYPT_GENERATES_KEYSTREAM flag.
*/

#define RaECRYPT_GENERATES_KEYSTREAM
#ifdef RaECRYPT_GENERATES_KEYSTREAM

void RaECRYPT_keystream_bytes(
	RaECRYPT_ctx* ctx,
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
* undef the RaECRYPT_USES_DEFAULT_ALL_IN_ONE flag.
*/
#define RaECRYPT_USES_DEFAULT_ALL_IN_ONE

/*
* Undef RaECRYPT_HAS_SINGLE_PACKET_FUNCTION if you want to provide
* separate packet encryption and decryption functions.
*/
#define RaECRYPT_HAS_SINGLE_PACKET_FUNCTION
#ifdef RaECRYPT_HAS_SINGLE_PACKET_FUNCTION

#define RaECRYPT_encrypt_packet(                                        \
    ctx, iv, plaintext, ciphertext, mglen)                            \
  RaECRYPT_process_packet(0,                                            \
    ctx, iv, plaintext, ciphertext, mglen)

#define RaECRYPT_decrypt_packet(                                        \
    ctx, iv, ciphertext, plaintext, mglen)                            \
  RaECRYPT_process_packet(1,                                            \
    ctx, iv, ciphertext, plaintext, mglen)

void RaECRYPT_process_packet(
	int action,                 /* 0 = encrypt; 1 = decrypt; */
	RaECRYPT_ctx* ctx,
	const u8* iv,
	const u8* input,
	u8* output,
	u32 msglen);

#else

void RaECRYPT_encrypt_packet(
	RaECRYPT_ctx* ctx,
	const u8* iv,
	const u8* plaintext,
	u8* ciphertext,
	u32 msglen);

void RaECRYPT_decrypt_packet(
	RaECRYPT_ctx* ctx,
	const u8* iv,
	const u8* ciphertext,
	u8* plaintext,
	u32 msglen);

#endif

/*
* Encryption/decryption of blocks.
*
* By default, these functions are defined as macros. If you want to
* provide a different implementation, please undef the
* RaECRYPT_USES_DEFAULT_BLOCK_MACROS flag and implement the functions
* declared below.
*/

#define RaECRYPT_BLOCKLENGTH 16

#undef RaECRYPT_USES_DEFAULT_BLOCK_MACROS
#ifdef RaECRYPT_USES_DEFAULT_BLOCK_MACROS

#define RaECRYPT_encrypt_blocks(ctx, plaintext, ciphertext, blocks)  \
  RaECRYPT_encrypt_bytes(ctx, plaintext, ciphertext,                 \
    (blocks) * RaECRYPT_BLOCKLENGTH)

#define RaECRYPT_decrypt_blocks(ctx, ciphertext, plaintext, blocks)  \
  RaECRYPT_decrypt_bytes(ctx, ciphertext, plaintext,                 \
    (blocks) * RaECRYPT_BLOCKLENGTH)

#ifdef RaECRYPT_GENERATES_KEYSTREAM

#define RaECRYPT_keystream_blocks(ctx, keystream, blocks)            \
  RaECRYPT_keystream_bytes(ctx, keystream,                           \
    (blocks) * RaECRYPT_BLOCKLENGTH)

#endif

#else

/*
* Undef RaECRYPT_HAS_SINGLE_BLOCK_FUNCTION if you want to provide
* separate block encryption and decryption functions.
*/
#define RaECRYPT_HAS_SINGLE_BLOCK_FUNCTION
#ifdef RaECRYPT_HAS_SINGLE_BLOCK_FUNCTION

#define RaECRYPT_encrypt_blocks(ctx, plaintext, ciphertext, blocks)     \
  RaECRYPT_process_blocks(0, ctx, plaintext, ciphertext, blocks)

#define RaECRYPT_decrypt_blocks(ctx, ciphertext, plaintext, blocks)     \
  RaECRYPT_process_blocks(1, ctx, ciphertext, plaintext, blocks)

void RaECRYPT_process_blocks(
	int action,                 /* 0 = encrypt; 1 = decrypt; */
	RaECRYPT_ctx* ctx,
	const u8* input,
	u8* output,
	u32 blocks);                /* Message length in blocks. */

#else

void RaECRYPT_encrypt_blocks(
	RaECRYPT_ctx* ctx,
	const u8* plaintext,
	u8* ciphertext,
	u32 blocks);                /* Message length in blocks. */

void RaECRYPT_decrypt_blocks(
	RaECRYPT_ctx* ctx,
	const u8* ciphertext,
	u8* plaintext,
	u32 blocks);                /* Message length in blocks. */

#endif

#ifdef RaECRYPT_GENERATES_KEYSTREAM

void RaECRYPT_keystream_blocks(
	RaECRYPT_ctx* ctx,
	u8* keystream,
	u32 blocks);                /* Keystream length in blocks. */

#endif

#endif

/*
* If your cipher can be implemented in different ways, you can use
* the RaECRYPT_VARIANT parameter to allow the user to choose between
* them at compile time (e.g., gcc -DRaECRYPT_VARIANT=3 ...). Please
* only use this possibility if you really think it could make a
* significant difference and keep the number of variants
* (RaECRYPT_MAXVARIANT) as small as possible (definitely not more than
* 10). Note also that all variants should have exactly the same
* external interface (i.e., the same RaECRYPT_BLOCKLENGTH, etc.).
*/
#define RaECRYPT_MAXVARIANT 1

#ifndef RaECRYPT_VARIANT
#define RaECRYPT_VARIANT 1
#endif

#if (RaECRYPT_VARIANT > RaECRYPT_MAXVARIANT)
#error this variant does not exist
#endif

/* ------------------------------------------------------------------------- */
