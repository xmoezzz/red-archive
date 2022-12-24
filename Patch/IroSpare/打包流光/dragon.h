/* ecrypt-sync.h */

/*
* Header file for synchronous stream ciphers without authentication
* mechanism.
*
* *** Please only edit parts marked with "[edit]". ***
*/

#ifndef drECRYPT_SYNC
#define drECRYPT_SYNC

#include "ecrypt-portable.h"

/* ------------------------------------------------------------------------- */

/* Cipher parameters */

/*
* The name of your cipher.
*/

/*
* Specify which key and IV sizes are supported by your cipher. A user
* should be able to enumerate the supported sizes by running the
* following code:
*
* for (i = 0; drECRYPT_KEYSIZE(i) <= drECRYPT_MAXKEYSIZE; ++i)
*   {
*     keysize = drECRYPT_KEYSIZE(i);
*
*     ...
*   }
*
* All sizes are in bits.
*/

#define drECRYPT_MAXKEYSIZE 256                 /* [edit] */
#define drECRYPT_KEYSIZE(i) (128 + (i)*128)      /* [edit] */

#define drECRYPT_MAXIVSIZE 256                  /* [edit] */
#define drECRYPT_IVSIZE(i) (128 + (i)*128)        /* [edit] */

/* ------------------------------------------------------------------------- */

/* Data structures */

/*
* drECRYPT_ctx is the structure containing the representation of the
* internal state of your cipher.
*/

#define DRAGON_NLFSR_SIZE      32 /* size of NLFSR in 32-bit multiples */
#define DRAGON_KEYSTREAM_SIZE  2 /* size of output in 32-bit multiples */

#ifdef _DRAGON_OPT
#define DRAGON_BUFFER_SIZE     16 /* number of keystream blocks to buffer */
#else
#define DRAGON_BUFFER_SIZE      1
#endif

#define DRAGON_BUFFER_BYTES    (DRAGON_BUFFER_SIZE * DRAGON_KEYSTREAM_SIZE * 4)

typedef struct
{
	/* The NLFSR and counter comprise the state of Dragon */
	u32  nlfsr_word[DRAGON_NLFSR_SIZE];

#ifdef _DRAGON_OPT_
	u32  state_counter[2];
#else
	u64  state_counter;
#endif
	/* NLFSR shifting is modelled by the decrement of the nlfsr_offset
	* pointer, which indicates the 0th element of the NLFSR
	*/
	u32  nlfsr_offset;

	/* Although key and IV injection are not seperated processes in Dragon
	* the drECRYPT API requires that they are added to the state separately.
	* Thus, to maintain consistency, the state at the end of the key
	* injection needs to be recalled for IV injection.
	*/
	u32  init_state[DRAGON_NLFSR_SIZE];
	u8   full_rekeying;
	u32  key_size;


	/* Dragon is a block-cipher but the drECRYPT API mandates that partial
	* blocks must be able to be encrypted. Usually this will involve
	* caller-managed buffering, but the drECRYPT API makes no provision
	* for this, so buffering unforutnately needs to be managed internally
	* to the primitive.
	*/
	u8   keystream_buffer[DRAGON_BUFFER_BYTES];
	u32  buffer_index;
} drECRYPT_ctx;

/* ------------------------------------------------------------------------- */

/* Mandatory functions */

/*
* Key and message independent initialization. This function will be
* called once when the program starts (e.g., to build expanded S-box
* tables).
*/
void drECRYPT_init(void);

/*
* Key setup. It is the user's responsibility to select the values of
* keysize and ivsize from the set of supported values specified
* above.
*/
void drECRYPT_keysetup(
	drECRYPT_ctx* ctx,
	const u8* key,
	u32 keysize,                /* Key size in bits. */
	u32 ivsize);                /* IV size in bits. */

/*
* IV setup. After having called drECRYPT_keysetup(), the user is
* allowed to call drECRYPT_ivsetup() different times in order to
* encrypt/decrypt different messages with the same key but different
* IV's.
*/
void drECRYPT_ivsetup(
	drECRYPT_ctx* ctx,
	const u8* iv);

/*
* Encryption/decryption of arbitrary length messages.
*
* For efficiency reasons, the API provides two types of
* encrypt/decrypt functions. The drECRYPT_encrypt_bytes() function
* (declared here) encrypts byte strings of arbitrary length, while
* the drECRYPT_encrypt_blocks() function (defined later) only accepts
* lengths which are multiples of drECRYPT_BLOCKLENGTH.
*
* The user is allowed to make multiple calls to
* drECRYPT_encrypt_blocks() to incrementally encrypt a long message,
* but he is NOT allowed to make additional encryption calls once he
* has called drECRYPT_encrypt_bytes() (unless he starts a new message
* of course). For example, this sequence of calls is acceptable:
*
* drECRYPT_keysetup();
*
* drECRYPT_ivsetup();
* drECRYPT_encrypt_blocks();
* drECRYPT_encrypt_blocks();
* drECRYPT_encrypt_bytes();
*
* drECRYPT_ivsetup();
* drECRYPT_encrypt_blocks();
* drECRYPT_encrypt_blocks();
*
* drECRYPT_ivsetup();
* drECRYPT_encrypt_bytes();
*
* The following sequence is not:
*
* drECRYPT_keysetup();
* drECRYPT_ivsetup();
* drECRYPT_encrypt_blocks();
* drECRYPT_encrypt_bytes();
* drECRYPT_encrypt_blocks();
*/

/*
* By default drECRYPT_encrypt_bytes() and drECRYPT_decrypt_bytes() are
* defined as macros which redirect the call to a single function
* drECRYPT_process_bytes(). If you want to provide separate encryption
* and decryption functions, please undef
* drECRYPT_HAS_SINGLE_BYTE_FUNCTION.
*/
#define drECRYPT_HAS_SINGLE_BYTE_FUNCTION       /* [edit] */
#ifdef drECRYPT_HAS_SINGLE_BYTE_FUNCTION

#define drECRYPT_encrypt_bytes(ctx, plaintext, ciphertext, msglen)   \
  drECRYPT_process_bytes(0, ctx, plaintext, ciphertext, msglen)

#define drECRYPT_decrypt_bytes(ctx, ciphertext, plaintext, msglen)   \
  drECRYPT_process_bytes(1, ctx, ciphertext, plaintext, msglen)

void drECRYPT_process_bytes(
	int action,                 /* 0 = encrypt; 1 = decrypt; */
	drECRYPT_ctx* ctx,
	const u8* input,
	u8* output,
	u32 msglen);                /* Message length in bytes. */

#else

void drECRYPT_encrypt_bytes(
	drECRYPT_ctx* ctx,
	const u8* plaintext,
	u8* ciphertext,
	u32 msglen);                /* Message length in bytes. */

void drECRYPT_decrypt_bytes(
	drECRYPT_ctx* ctx,
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
* reset the drECRYPT_GENERATES_KEYSTREAM flag.
*/

#define drECRYPT_GENERATES_KEYSTREAM
#ifdef drECRYPT_GENERATES_KEYSTREAM

void drECRYPT_keystream_bytes(
	drECRYPT_ctx* ctx,
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
* undef the drECRYPT_USES_DEFAULT_ALL_IN_ONE flag.
*/
#define drECRYPT_USES_DEFAULT_ALL_IN_ONE        /* [edit] */

/*
* Undef drECRYPT_HAS_SINGLE_PACKET_FUNCTION if you want to provide
* separate packet encryption and decryption functions.
*/
#define drECRYPT_HAS_SINGLE_PACKET_FUNCTION     /* [edit] */
#ifdef drECRYPT_HAS_SINGLE_PACKET_FUNCTION

#define drECRYPT_encrypt_packet(                                        \
    ctx, iv, plaintext, ciphertext, mglen)                            \
  drECRYPT_process_packet(0,                                            \
    ctx, iv, plaintext, ciphertext, mglen)

#define drECRYPT_decrypt_packet(                                        \
    ctx, iv, ciphertext, plaintext, mglen)                            \
  drECRYPT_process_packet(1,                                            \
    ctx, iv, ciphertext, plaintext, mglen)

void drECRYPT_process_packet(
	int action,                 /* 0 = encrypt; 1 = decrypt; */
	drECRYPT_ctx* ctx,
	const u8* iv,
	const u8* input,
	u8* output,
	u32 msglen);

#else

void drECRYPT_encrypt_packet(
	drECRYPT_ctx* ctx,
	const u8* iv,
	const u8* plaintext,
	u8* ciphertext,
	u32 msglen);

void drECRYPT_decrypt_packet(
	drECRYPT_ctx* ctx,
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
* drECRYPT_USES_DEFAULT_BLOCK_MACROS flag and implement the functions
* declared below.
*/

#define drECRYPT_BLOCKLENGTH 8                  /* [edit] */

#undef drECRYPT_USES_DEFAULT_BLOCK_MACROS      /* [edit] */
#ifdef drECRYPT_USES_DEFAULT_BLOCK_MACROS

#define drECRYPT_encrypt_blocks(ctx, plaintext, ciphertext, blocks)  \
  drECRYPT_encrypt_bytes(ctx, plaintext, ciphertext,                 \
    (blocks) * drECRYPT_BLOCKLENGTH)

#define drECRYPT_decrypt_blocks(ctx, ciphertext, plaintext, blocks)  \
  drECRYPT_decrypt_bytes(ctx, ciphertext, plaintext,                 \
    (blocks) * drECRYPT_BLOCKLENGTH)

#ifdef drECRYPT_GENERATES_KEYSTREAM

#define drECRYPT_keystream_blocks(ctx, keystream, blocks)            \
  drECRYPT_keystream_bytes(ctx, keystream,                           \
    (blocks) * drECRYPT_BLOCKLENGTH)

#endif

#else

/*
* Undef drECRYPT_HAS_SINGLE_BLOCK_FUNCTION if you want to provide
* separate block encryption and decryption functions.
*/
#define drECRYPT_HAS_SINGLE_BLOCK_FUNCTION      /* [edit] */
#ifdef drECRYPT_HAS_SINGLE_BLOCK_FUNCTION

#define drECRYPT_encrypt_blocks(ctx, plaintext, ciphertext, blocks)     \
  drECRYPT_process_blocks(0, ctx, plaintext, ciphertext, blocks)

#define drECRYPT_decrypt_blocks(ctx, ciphertext, plaintext, blocks)     \
  drECRYPT_process_blocks(1, ctx, ciphertext, plaintext, blocks)

void drECRYPT_process_blocks(
	int action,                 /* 0 = encrypt; 1 = decrypt; */
	drECRYPT_ctx* ctx,
	const u8* input,
	u8* output,
	u32 blocks);                /* Message length in blocks. */

#else

void drECRYPT_encrypt_blocks(
	drECRYPT_ctx* ctx,
	const u8* plaintext,
	u8* ciphertext,
	u32 blocks);                /* Message length in blocks. */

void drECRYPT_decrypt_blocks(
	drECRYPT_ctx* ctx,
	const u8* ciphertext,
	u8* plaintext,
	u32 blocks);                /* Message length in blocks. */

#endif

#ifdef drECRYPT_GENERATES_KEYSTREAM

void drECRYPT_keystream_blocks(
	drECRYPT_ctx* ctx,
	u8* keystream,
	u32 blocks);                /* Keystream length in blocks. */

#endif

#endif

/* ------------------------------------------------------------------------- */

#endif
