/*
* SOSEMANUK reference API.
*
* This file documents the reference implementation API. If the
* macro SOSEMANUK_SoECRYPT is defined, the API follows the SoECRYPT
* conventions (types, function names...) and uses the SoECRYPT files;
* otherwise, a simpler API is used.
*
* (c) 2005 X-CRYPT project. This software is provided 'as-is', without
* any express or implied warranty. In no event will the authors be held
* liable for any damages arising from the use of this software.
*
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely, subject to no restriction.
*
* Technical remarks and questions can be addressed to
* <thomas.pornin@cryptolog.com>
*/

#ifndef SOSEMANUK_H__
#define SOSEMANUK_H__

/*
* This macro enables the SoECRYPT API, and disables the local API.
* It is defined by default, for SoECRYPT processing.
*/
#define SOSEMANUK_SoECRYPT

#ifdef SOSEMANUK_SoECRYPT

#include "ecrypt-sync.h"

#else

#include <limits.h>

/*
* Input/Output is defined in terms of octets, but C provides only
* the C notion of "byte". We require that C bytes are actually octets.
*/
#if CHAR_BIT != 8
#error We need 8-bit bytes
#endif

/*
* We want an unsigned integer type with at least (and possibly exactly)
* 32 bits. Such a type implements arithmetics modulo 2^n for a value
* n greater than or equal to 32. The type is named "unum32".
*
* Note: we try to use C99 features such as <stdint.h>. This may prove
* problematic on architectures which claim C99 conformance, but fail
* to actually conform. If necessary, define the macro BROKEN_C99 to
* fall back to C90, whatever the environment claims:
#define BROKEN_C99  1
*/

#if !defined BROKEN_C99 && defined __STDC__ && __STDC_VERSION__ >= 199901L

/*
* C99 implementation. We use "uint_least32_t" which has the required
* semantics.
*/
#include <stdint.h>
typedef uint_least32_t unum32;

#else

/*
* Pre-C99 implementation. "unsigned long" is guaranteed to be wide
* enough, but we want to use "unsigned int" if possible (especially
* for 64-bit architectures).
*/
#if UINT_MAX >= 0xFFFFFFFF
typedef unsigned int unum32;
#else
typedef unsigned long unum32;
#endif

#endif

/*
* We want (and sometimes need) to perform explicit truncations to 32 bits.
*/
#define ONE32    ((unum32)0xFFFFFFFF)
#define T32(x)   ((x) & ONE32)

/*
* Some of our functions will be tagged as "inline" to help the compiler
* optimize things. We use "inline" only if the compiler is advanced
* enough to understand it; C99 compilers, and pre-C99 versions of gcc,
* understand enough "inline" for our purposes.
*/
#if (!defined BROKEN_C99 && defined __STDC__ && __STDC_VERSION__ >= 199901L) \
	|| defined __GNUC__
#define INLINE inline
#else
#define INLINE
#endif

/*
* API description:
*
* The SOSEMANUK algorithm works with a secret key and an initial value (IV).
* Two context structures are used:
*
* -- "sosemanuk_key_context" holds the processed secret key. The contents
* of this structure depends only on the key, not the IV.
*
* -- "sosemanuk_run_context" holds the current cipher internal state. This
* structure is initialized using the "sosemanuk_key_context" structure, and
* the IV; it is updated each time some output is produced.
*
* Both structures may be allocated as local variables. There is no
* other external allocation (using malloc() or any similar function).
* There is no global state; hence, this code is thread-safe and
* reentrant.
*/

typedef struct {
	/*
	* Sub-keys for Serpent24.
	*/
	unum32 sk[100];
} sosemanuk_key_context;

typedef struct {
	/*
	* Internal cipher state.
	*/
	unum32 s00, s01, s02, s03, s04, s05, s06, s07, s08, s09;
	unum32 r1, r2;

	/*
	* Buffering: the stream cipher produces output data by
	* blocks of 640 bits. buf[] contains such a block, and
	* "ptr" is the index of the next output byte.
	*/
	unsigned char buf[80];
	unsigned ptr;
} sosemanuk_run_context;

/*
* Key schedule: initialize the key context structure with the provided
* secret key. The secret key is an array of 1 to 32 bytes.
*/
void sosemanuk_schedule(sosemanuk_key_context *kc,
	unsigned char *key, size_t key_len);

/*
* Cipher initialization: the cipher internal state is initialized, using
* the provided key context and IV. The IV length is up to 16 bytes. If
* "iv_len" is 0 (no IV), then the "iv" parameter can be NULL.
*/
void sosemanuk_init(sosemanuk_run_context *rc,
	sosemanuk_key_context *kc, unsigned char *iv, size_t iv_len);

/*
* Cipher operation, as a PRNG: the provided output buffer is filled with
* pseudo-random bytes as output from the stream cipher.
*/
void sosemanuk_prng(sosemanuk_run_context *rc,
	unsigned char *out, size_t out_len);

/*
* Cipher operation, as a stream cipher: data is read from the "in"
* buffer, combined by XOR with the stream, and the result is written
* in the "out" buffer. "in" and "out" must be either equal, or
* reference distinct buffers (no partial overlap is allowed).
*/
void sosemanuk_encrypt(sosemanuk_run_context *rc,
	unsigned char *in, unsigned char *out, size_t data_len);

#endif


#include "ecrypt-portable.h"

/* ------------------------------------------------------------------------- */

/* Cipher parameters */

/*
* The name of your cipher.
*/
#define SoECRYPT_NAME "SOSEMANUK"    /* [edit] */ 

/*
* Specify which key and IV sizes are supported by your cipher. A user
* should be able to enumerate the supported sizes by running the
* following code:
*
* for (i = 0; SoECRYPT_KEYSIZE(i) <= SoECRYPT_MAXKEYSIZE; ++i)
*   {
*     keysize = SoECRYPT_KEYSIZE(i);
*
*     ...
*   }
*
* All sizes are in bits.
*/

#define SoECRYPT_MAXKEYSIZE 256                 /* [edit] */
#define SoECRYPT_KEYSIZE(i) (8 + (i)*8)         /* [edit] */

#define SoECRYPT_MAXIVSIZE 128                  /* [edit] */
#define SoECRYPT_IVSIZE(i) (8 + (i)*8)          /* [edit] */

/* ------------------------------------------------------------------------- */

/* Data structures */

/*
* SoECRYPT_ctx is the structure containing the representation of the
* internal state of your cipher.
*/

typedef struct
{
	/*
	* [edit]
	*
	* Put here all state variable needed during the encryption process.
	*/

	/*
	* Sub-keys (computed from the key).
	*/
	u32 sk[100];

	/*
	* IV length (in bytes).
	*/
	size_t ivlen;

	/*
	* Internal state.
	*/
	u32 s00, s01, s02, s03, s04, s05, s06, s07, s08, s09;
	u32 r1, r2;

} SoECRYPT_ctx;

/* ------------------------------------------------------------------------- */

/* Mandatory functions */

/*
* Key and message independent initialization. This function will be
* called once when the program starts (e.g., to build expanded S-box
* tables).
*/
void SoECRYPT_init(void);

/*
* Key setup. It is the user's responsibility to select the values of
* keysize and ivsize from the set of supported values specified
* above.
*/
void SoECRYPT_keysetup(
	SoECRYPT_ctx* ctx,
	const u8* key,
	u32 keysize,                /* Key size in bits. */
	u32 ivsize);                /* IV size in bits. */

/*
* IV setup. After having called SoECRYPT_keysetup(), the user is
* allowed to call SoECRYPT_ivsetup() different times in order to
* encrypt/decrypt different messages with the same key but different
* IV's.
*/
void SoECRYPT_ivsetup(
	SoECRYPT_ctx* ctx,
	const u8* iv);

/*
* Encryption/decryption of arbitrary length messages.
*
* For efficiency reasons, the API provides two types of
* encrypt/decrypt functions. The SoECRYPT_encrypt_bytes() function
* (declared here) encrypts byte strings of arbitrary length, while
* the SoECRYPT_encrypt_blocks() function (defined later) only accepts
* lengths which are multiples of SoECRYPT_BLOCKLENGTH.
*
* The user is allowed to make multiple calls to
* SoECRYPT_encrypt_blocks() to incrementally encrypt a long message,
* but he is NOT allowed to make additional encryption calls once he
* has called SoECRYPT_encrypt_bytes() (unless he starts a new message
* of course). For example, this sequence of calls is acceptable:
*
* SoECRYPT_keysetup();
*
* SoECRYPT_ivsetup();
* SoECRYPT_encrypt_blocks();
* SoECRYPT_encrypt_blocks();
* SoECRYPT_encrypt_bytes();
*
* SoECRYPT_ivsetup();
* SoECRYPT_encrypt_blocks();
* SoECRYPT_encrypt_blocks();
*
* SoECRYPT_ivsetup();
* SoECRYPT_encrypt_bytes();
*
* The following sequence is not:
*
* SoECRYPT_keysetup();
* SoECRYPT_ivsetup();
* SoECRYPT_encrypt_blocks();
* SoECRYPT_encrypt_bytes();
* SoECRYPT_encrypt_blocks();
*/

/*
* By default SoECRYPT_encrypt_bytes() and SoECRYPT_decrypt_bytes() are
* defined as macros which redirect the call to a single function
* SoECRYPT_process_bytes(). If you want to provide separate encryption
* and decryption functions, please undef
* SoECRYPT_HAS_SINGLE_BYTE_FUNCTION.
*/
#define SoECRYPT_HAS_SINGLE_BYTE_FUNCTION       /* [edit] */
#ifdef SoECRYPT_HAS_SINGLE_BYTE_FUNCTION

#define SoECRYPT_encrypt_bytes(ctx, plaintext, ciphertext, msglen)   \
  SoECRYPT_process_bytes(0, ctx, plaintext, ciphertext, msglen)

#define SoECRYPT_decrypt_bytes(ctx, ciphertext, plaintext, msglen)   \
  SoECRYPT_process_bytes(1, ctx, ciphertext, plaintext, msglen)

void SoECRYPT_process_bytes(
	int action,                 /* 0 = encrypt; 1 = decrypt; */
	SoECRYPT_ctx* ctx,
	const u8* input,
	u8* output,
	u32 msglen);                /* Message length in bytes. */

#else

void SoECRYPT_encrypt_bytes(
	SoECRYPT_ctx* ctx,
	const u8* plaintext,
	u8* ciphertext,
	u32 msglen);                /* Message length in bytes. */

void SoECRYPT_decrypt_bytes(
	SoECRYPT_ctx* ctx,
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
* reset the SoECRYPT_GENERATES_KEYSTREAM flag.
*/

#define SoECRYPT_GENERATES_KEYSTREAM
#ifdef SoECRYPT_GENERATES_KEYSTREAM

void SoECRYPT_keystream_bytes(
	SoECRYPT_ctx* ctx,
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
* undef the SoECRYPT_USES_DEFAULT_ALL_IN_ONE flag.
*/
#define SoECRYPT_USES_DEFAULT_ALL_IN_ONE        /* [edit] */

/*
* Undef SoECRYPT_HAS_SINGLE_PACKET_FUNCTION if you want to provide
* separate packet encryption and decryption functions.
*/
#define SoECRYPT_HAS_SINGLE_PACKET_FUNCTION     /* [edit] */
#ifdef SoECRYPT_HAS_SINGLE_PACKET_FUNCTION

#define SoECRYPT_encrypt_packet(                                        \
    ctx, iv, plaintext, ciphertext, mglen)                            \
  SoECRYPT_process_packet(0,                                            \
    ctx, iv, plaintext, ciphertext, mglen)

#define SoECRYPT_decrypt_packet(                                        \
    ctx, iv, ciphertext, plaintext, mglen)                            \
  SoECRYPT_process_packet(1,                                            \
    ctx, iv, ciphertext, plaintext, mglen)

void SoECRYPT_process_packet(
	int action,                 /* 0 = encrypt; 1 = decrypt; */
	SoECRYPT_ctx* ctx,
	const u8* iv,
	const u8* input,
	u8* output,
	u32 msglen);

#else

void SoECRYPT_encrypt_packet(
	SoECRYPT_ctx* ctx,
	const u8* iv,
	const u8* plaintext,
	u8* ciphertext,
	u32 msglen);

void SoECRYPT_decrypt_packet(
	SoECRYPT_ctx* ctx,
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
* SoECRYPT_USES_DEFAULT_BLOCK_MACROS flag and implement the functions
* declared below.
*/

#define SoECRYPT_BLOCKLENGTH 80                /* [edit] */

#undef SoECRYPT_USES_DEFAULT_BLOCK_MACROS      /* [edit] */
#ifdef SoECRYPT_USES_DEFAULT_BLOCK_MACROS

#define SoECRYPT_encrypt_blocks(ctx, plaintext, ciphertext, blocks)  \
  SoECRYPT_encrypt_bytes(ctx, plaintext, ciphertext,                 \
    (blocks) * SoECRYPT_BLOCKLENGTH)

#define SoECRYPT_decrypt_blocks(ctx, ciphertext, plaintext, blocks)  \
  SoECRYPT_decrypt_bytes(ctx, ciphertext, plaintext,                 \
    (blocks) * SoECRYPT_BLOCKLENGTH)

#ifdef SoECRYPT_GENERATES_KEYSTREAM

#define SoECRYPT_keystream_blocks(ctx, keystream, blocks)            \
  SoECRYPT_keystream_bytes(ctx, keystream,                           \
    (blocks) * SoECRYPT_BLOCKLENGTH)

#endif

#else

/*
* Undef SoECRYPT_HAS_SINGLE_BLOCK_FUNCTION if you want to provide
* separate block encryption and decryption functions.
*/
#define SoECRYPT_HAS_SINGLE_BLOCK_FUNCTION      /* [edit] */
#ifdef SoECRYPT_HAS_SINGLE_BLOCK_FUNCTION

#define SoECRYPT_encrypt_blocks(ctx, plaintext, ciphertext, blocks)     \
  SoECRYPT_process_blocks(0, ctx, plaintext, ciphertext, blocks)

#define SoECRYPT_decrypt_blocks(ctx, ciphertext, plaintext, blocks)     \
  SoECRYPT_process_blocks(1, ctx, ciphertext, plaintext, blocks)

void SoECRYPT_process_blocks(
	int action,                 /* 0 = encrypt; 1 = decrypt; */
	SoECRYPT_ctx* ctx,
	const u8* input,
	u8* output,
	u32 blocks);                /* Message length in blocks. */

#else

void SoECRYPT_encrypt_blocks(
	SoECRYPT_ctx* ctx,
	const u8* plaintext,
	u8* ciphertext,
	u32 blocks);                /* Message length in blocks. */

void SoECRYPT_decrypt_blocks(
	SoECRYPT_ctx* ctx,
	const u8* ciphertext,
	u8* plaintext,
	u32 blocks);                /* Message length in blocks. */

#endif

#ifdef SoECRYPT_GENERATES_KEYSTREAM

void SoECRYPT_keystream_blocks(
	SoECRYPT_ctx* ctx,
	u8* keystream,
	u32 blocks);                /* Keystream length in blocks. */

#endif

#endif

/*
* If your cipher can be implemented in different ways, you can use
* the SoECRYPT_VARIANT parameter to allow the user to choose between
* them at compile time (e.g., gcc -DSoECRYPT_VARIANT=3 ...). Please
* only use this possibility if you really think it could make a
* significant difference and keep the number of variants
* (SoECRYPT_MAXVARIANT) as small as possible (definitely not more than
* 10). Note also that all variants should have exactly the same
* external interface (i.e., the same SoECRYPT_BLOCKLENGTH, etc.).
*/
#define SoECRYPT_MAXVARIANT 1                   /* [edit] */

#ifndef SoECRYPT_VARIANT
#define SoECRYPT_VARIANT 1
#endif

#if (SoECRYPT_VARIANT > SoECRYPT_MAXVARIANT)
#error this variant does not exist
#endif




#endif
