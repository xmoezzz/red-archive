#pragma once

#include "ecrypt-portable.h"

/* ------------------------------------------------------------------------- */

/* Cipher parameters */

/*
* Specify which key and IV sizes are supported by your cipher. A user
* should be able to enumerate the supported sizes by running the
* following code:
*
* for (i = 0; Edon80ECRYPT_KEYSIZE(i) <= Edon80ECRYPT_MAXKEYSIZE; ++i)
*   {
*     keysize = Edon80ECRYPT_KEYSIZE(i);
*
*     ...
*   }
*
* All sizes are in bits.
*/

#define Edon80ECRYPT_MAXKEYSIZE 80                 /* [edit] */
#define Edon80ECRYPT_KEYSIZE(i) (80 + (i)*8)       /* [edit] */
/*
The design of Edon80 in principle is not restricted by the key size.
However, for Edon80ECRYPT call for Stream Ciphers - PROFILE 2, we restrict
Edon80 key size only on requested size of 80 bits.
*/

#define Edon80ECRYPT_MAXIVSIZE 64                 /* [edit] */
#define Edon80ECRYPT_IVSIZE(i) (64 + (i)*8)       /* [edit] */
/*
We repeat the same comment from above for IVSIZE. For Edon80ECRYPT call for
Stream Ciphers, we restrict Edon80 key size only on requested size of 64 bits.
*/

/* ------------------------------------------------------------------------- */

/* Data structures */

/*
* Edon80ECRYPT_ctx is the structure containing the representation of the
* internal state of your cipher.
*/

typedef struct
{
	/* Edon80 is a Stream Cipher based on Quasigropup String Transformations.  */
	/* For the definition of Edon80 we need quasigroups of order 4     */
	u8  Q[Edon80ECRYPT_MAXKEYSIZE][4][4];
	/* Counter is internal variable that has values in the range 0 to 3 */
	u8  Counter;
	/* The working size of the key (in pairs of bits). */
	u32 keysize;
	/* The values of the Initial Vector are kept in this array. */
	u8  key[Edon80ECRYPT_MAXKEYSIZE / 2];
	/* The working size of the Initial Vector (in pairs of bits). */
	u32 ivsize;
	/* The values of the Initial Vector are kept in this array. */
	u8  iv[Edon80ECRYPT_MAXKEYSIZE / 2];
	/* The actual number of internal states. */
	u32 NumberOfInternalStates;
	/* All internal states are kept in this array. */
	u8  InternalState[Edon80ECRYPT_MAXKEYSIZE];
	/*
	* [edit]
	*
	* Put here all state variable needed during the encryption process.
	*/
} Edon80ECRYPT_ctx;

/* ------------------------------------------------------------------------- */

/* Mandatory functions */

/*
* Key and message independent initialization. This function will be
* called once when the program starts (e.g., to build expanded S-box
* tables).
*/
void Edon80ECRYPT_init();

/*
* Key setup. It is the user's responsibility to select the values of
* keysize and ivsize from the set of supported values specified
* above.
*/
void Edon80ECRYPT_keysetup(
	Edon80ECRYPT_ctx* ctx,
	const u8* key,
	u32 keysize,                 /* Key size in bits. */
	u32 ivsize);                 /* IV size in bits. */


/*
* IV setup. After having called Edon80ECRYPT_keysetup(), the user is
* allowed to call Edon80ECRYPT_ivsetup() different times in order to
* encrypt/decrypt different messages with the same key but different
* IV's.
*/
void Edon80ECRYPT_ivsetup(
	Edon80ECRYPT_ctx* ctx,
	const u8* iv);


/*
* Encryption/decryption of arbitrary length messages.
*
* For efficiency reasons, the API provides two types of
* encrypt/decrypt functions. The Edon80ECRYPT_encrypt_bytes() function
* (declared here) encrypts byte strings of arbitrary length, while
* the Edon80ECRYPT_encrypt_blocks() function (defined later) only accepts
* lengths which are multiples of Edon80ECRYPT_BLOCKLENGTH.
*
* The user is allowed to make multiple calls to
* Edon80ECRYPT_encrypt_blocks() to incrementally encrypt a long message,
* but he is NOT allowed to make additional encryption calls once he
* has called Edon80ECRYPT_encrypt_bytes() (unless he starts a new message
* of course). For example, this sequence of calls is acceptable:
*
* Edon80ECRYPT_keysetup();
*
* Edon80ECRYPT_ivsetup();
* Edon80ECRYPT_encrypt_blocks();
* Edon80ECRYPT_encrypt_blocks();
* Edon80ECRYPT_encrypt_bytes();
*
* Edon80ECRYPT_ivsetup();
* Edon80ECRYPT_encrypt_blocks();
* Edon80ECRYPT_encrypt_blocks();
*
* Edon80ECRYPT_ivsetup();
* Edon80ECRYPT_encrypt_bytes();
*
* The following sequence is not:
*
* Edon80ECRYPT_keysetup();
* Edon80ECRYPT_ivsetup();
* Edon80ECRYPT_encrypt_blocks();
* Edon80ECRYPT_encrypt_bytes();
* Edon80ECRYPT_encrypt_blocks();
*/

void Edon80ECRYPT_encrypt_bytes(
	Edon80ECRYPT_ctx* ctx,
	const u8* plaintext,
	u8* ciphertext,
	u32 msglen);                /* Message length in bytes. */


void Edon80ECRYPT_decrypt_bytes(
	Edon80ECRYPT_ctx* ctx,
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
* reset the Edon80ECRYPT_GENERATES_KEYSTREAM flag.
*/

#define Edon80ECRYPT_GENERATES_KEYSTREAM
#ifdef Edon80ECRYPT_GENERATES_KEYSTREAM

void Edon80ECRYPT_keystream_bytes(
	Edon80ECRYPT_ctx* ctx,
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
* undef the Edon80ECRYPT_USES_DEFAULT_ALL_IN_ONE flag.
*/
#define Edon80ECRYPT_USES_DEFAULT_ALL_IN_ONE        /* [edit] */

void Edon80ECRYPT_encrypt_packet(
	Edon80ECRYPT_ctx* ctx,
	const u8* iv,
	const u8* plaintext,
	u8* ciphertext,
	u32 msglen);

void Edon80ECRYPT_decrypt_packet(
	Edon80ECRYPT_ctx* ctx,
	const u8* iv,
	const u8* ciphertext,
	u8* plaintext,
	u32 msglen);

/*
* Encryption/decryption of blocks.
*
* By default, these functions are defined as macros. If you want to
* provide a different implementation, please undef the
* Edon80ECRYPT_USES_DEFAULT_BLOCK_MACROS flag and implement the functions
* declared below.
*/

#define Edon80ECRYPT_BLOCKLENGTH 4                  /* [edit] */

#define Edon80ECRYPT_USES_DEFAULT_BLOCK_MACROS      /* [edit] */
#ifdef Edon80ECRYPT_USES_DEFAULT_BLOCK_MACROS

#define Edon80ECRYPT_encrypt_blocks(ctx, plaintext, ciphertext, blocks)  \
  Edon80ECRYPT_encrypt_bytes(ctx, plaintext, ciphertext,                 \
    (blocks) * Edon80ECRYPT_BLOCKLENGTH)

#define Edon80ECRYPT_decrypt_blocks(ctx, ciphertext, plaintext, blocks)  \
  Edon80ECRYPT_decrypt_bytes(ctx, ciphertext, plaintext,                 \
    (blocks) * Edon80ECRYPT_BLOCKLENGTH)

#ifdef Edon80ECRYPT_GENERATES_KEYSTREAM

#define Edon80ECRYPT_keystream_blocks(ctx, keystream, blocks)            \
  Edon80ECRYPT_AE_keystream_bytes(ctx, keystream,                        \
    (blocks) * Edon80ECRYPT_BLOCKLENGTH)

#endif

#else

void Edon80ECRYPT_encrypt_blocks(
	Edon80ECRYPT_ctx* ctx,
	const u8* plaintext,
	u8* ciphertext,
	u32 blocks);                /* Message length in blocks. */

void Edon80ECRYPT_decrypt_blocks(
	Edon80ECRYPT_ctx* ctx,
	const u8* ciphertext,
	u8* plaintext,
	u32 blocks);                /* Message length in blocks. */

#ifdef Edon80ECRYPT_GENERATES_KEYSTREAM

void Edon80ECRYPT_keystream_blocks(
	Edon80ECRYPT_AE_ctx* ctx,
	const u8* keystream,
	u32 blocks);                /* Keystream length in blocks. */

#endif

#endif

/* ------------------------------------------------------------------------- */

