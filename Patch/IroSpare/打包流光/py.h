/* ecrypt-sync.h */

/*
* Header file for synchronous stream ciphers without authentication
* mechanism.
*
* *** Please only edit parts marked with "[edit]". ***
*/

#ifndef PyECRYPT_SYNC
#define PyECRYPT_SYNC

#include "ecrypt-portable.h"
#include "my.h"
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
* for (i = 0; PyECRYPT_KEYSIZE(i) <= PyECRYPT_MAXKEYSIZE; ++i)
*   {
*     keysize = PyECRYPT_KEYSIZE(i);
*
*     ...
*   }
*
* All sizes are in bits.
*/

#define PyECRYPT_MAXKEYSIZE (256-1)             /* [edit] */
#define PyECRYPT_KEYSIZE(i) ((i+1)*8)	      /* [edit] */

#define PyECRYPT_MAXIVSIZE (64-1)                 /* [edit] */
#define PyECRYPT_IVSIZE(i) ((i+1)*8)              /* [edit] */

/* ------------------------------------------------------------------------- */

/* Data structures */

/*
* PyECRYPT_ctx is the structure containing the representation of the
* internal state of your cipher.
*/

#define PYSIZE 260
#define YMININD (-3)
#define YMAXIND (256)

typedef struct
{
	/*
	* [edit]
	*
	* Put here all state variable needed during the encryption process.
	*/
	u32 KPY[PYSIZE][2];
	/* KPY and PY must be consecutive, as we use them as one large array in IVsetup */
	u32 PY[PYSIZE][2];
	/* there are accesses after PY, dummy should take these values and ignore them */
	u32 dummy[3][2];
	u32 s;
	int keysize;
	int ivsize;
} PyECRYPT_ctx;

/* ------------------------------------------------------------------------- */

/* Mandatory functions */

/*
* Key and message independent initialization. This function will be
* called once when the program starts (e.g., to build expanded S-box
* tables).
*/
void PyECRYPT_init(void);

/*
* Key setup. It is the user's responsibility to select the values of
* keysize and ivsize from the set of supported values specified
* above.
*/
void PyECRYPT_keysetup(
	PyECRYPT_ctx* ctx,
	const u8* key,
	u32 keysize,                /* Key size in bits. */
	u32 ivsize);                /* IV size in bits. */

/*
* IV setup. After having called PyECRYPT_keysetup(), the user is
* allowed to call PyECRYPT_ivsetup() different times in order to
* encrypt/decrypt different messages with the same key but different
* IV's.
*/
void PyECRYPT_ivsetup(
	PyECRYPT_ctx* ctx,
	const u8* iv);

/*
* Encryption/decryption of arbitrary length messages.
*
* For efficiency reasons, the API provides two types of
* encrypt/decrypt functions. The PyECRYPT_encrypt_bytes() function
* (declared here) encrypts byte strings of arbitrary length, while
* the PyECRYPT_encrypt_blocks() function (defined later) only accepts
* lengths which are multiples of PyECRYPT_BLOCKLENGTH.
*
* The user is allowed to make multiple calls to
* PyECRYPT_encrypt_blocks() to incrementally encrypt a long message,
* but he is NOT allowed to make additional encryption calls once he
* has called PyECRYPT_encrypt_bytes() (unless he starts a new message
* of course). For example, this sequence of calls is acceptable:
*
* PyECRYPT_keysetup();
*
* PyECRYPT_ivsetup();
* PyECRYPT_encrypt_blocks();
* PyECRYPT_encrypt_blocks();
* PyECRYPT_encrypt_bytes();
*
* PyECRYPT_ivsetup();
* PyECRYPT_encrypt_blocks();
* PyECRYPT_encrypt_blocks();
*
* PyECRYPT_ivsetup();
* PyECRYPT_encrypt_bytes();
*
* The following sequence is not:
*
* PyECRYPT_keysetup();
* PyECRYPT_ivsetup();
* PyECRYPT_encrypt_blocks();
* PyECRYPT_encrypt_bytes();
* PyECRYPT_encrypt_blocks();
*/

/*
* By default PyECRYPT_encrypt_bytes() and PyECRYPT_decrypt_bytes() are
* defined as macros which redirect the call to a single function
* PyECRYPT_process_bytes(). If you want to provide separate encryption
* and decryption functions, please undef
* PyECRYPT_HAS_SINGLE_BYTE_FUNCTION.
*/
#define PyECRYPT_HAS_SINGLE_BYTE_FUNCTION       /* [edit] */
#ifdef PyECRYPT_HAS_SINGLE_BYTE_FUNCTION

#define PyECRYPT_encrypt_bytes(ctx, plaintext, ciphertext, msglen)   \
  PyECRYPT_process_bytes(0, ctx, plaintext, ciphertext, msglen)

#define PyECRYPT_decrypt_bytes(ctx, ciphertext, plaintext, msglen)   \
  PyECRYPT_process_bytes(1, ctx, ciphertext, plaintext, msglen)

void PyECRYPT_process_bytes(
	int action,                 /* 0 = encrypt; 1 = decrypt; */
	PyECRYPT_ctx* ctx,
	const u8* input,
	u8* output,
	u32 msglen);                /* Message length in bytes. */

#else

void PyECRYPT_encrypt_bytes(
	PyECRYPT_ctx* ctx,
	const u8* plaintext,
	u8* ciphertext,
	u32 msglen);                /* Message length in bytes. */

void PyECRYPT_decrypt_bytes(
	PyECRYPT_ctx* ctx,
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
* reset the PyECRYPT_GENERATES_KEYSTREAM flag.
*/

#define PyECRYPT_GENERATES_KEYSTREAM
#ifdef PyECRYPT_GENERATES_KEYSTREAM

void PyECRYPT_keystream_bytes(
	PyECRYPT_ctx* ctx,
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
* undef the PyECRYPT_USES_DEFAULT_ALL_IN_ONE flag.
*/
#define PyECRYPT_USES_DEFAULT_ALL_IN_ONE        /* [edit] */

/*
* Undef PyECRYPT_HAS_SINGLE_PACKET_FUNCTION if you want to provide
* separate packet encryption and decryption functions.
*/
#define PyECRYPT_HAS_SINGLE_PACKET_FUNCTION     /* [edit] */
#ifdef PyECRYPT_HAS_SINGLE_PACKET_FUNCTION

#define PyECRYPT_encrypt_packet(                                        \
    ctx, iv, plaintext, ciphertext, mglen)                            \
  PyECRYPT_process_packet(0,                                            \
    ctx, iv, plaintext, ciphertext, mglen)

#define PyECRYPT_decrypt_packet(                                        \
    ctx, iv, ciphertext, plaintext, mglen)                            \
  PyECRYPT_process_packet(1,                                            \
    ctx, iv, ciphertext, plaintext, mglen)

void PyECRYPT_process_packet(
	int action,                 /* 0 = encrypt; 1 = decrypt; */
	PyECRYPT_ctx* ctx,
	const u8* iv,
	const u8* input,
	u8* output,
	u32 msglen);

#else

void PyECRYPT_encrypt_packet(
	PyECRYPT_ctx* ctx,
	const u8* iv,
	const u8* plaintext,
	u8* ciphertext,
	u32 msglen);

void PyECRYPT_decrypt_packet(
	PyECRYPT_ctx* ctx,
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
* PyECRYPT_USES_DEFAULT_BLOCK_MACROS flag and implement the functions
* declared below.
*/

#define PyECRYPT_BLOCKLENGTH 8                  /* [edit] */

#define PyECRYPT_USES_DEFAULT_BLOCK_MACROS      /* [edit] */
#ifdef PyECRYPT_USES_DEFAULT_BLOCK_MACROS

#define PyECRYPT_encrypt_blocks(ctx, plaintext, ciphertext, blocks)  \
  PyECRYPT_encrypt_bytes(ctx, plaintext, ciphertext,                 \
    (blocks) * PyECRYPT_BLOCKLENGTH)

#define PyECRYPT_decrypt_blocks(ctx, ciphertext, plaintext, blocks)  \
  PyECRYPT_decrypt_bytes(ctx, ciphertext, plaintext,                 \
    (blocks) * PyECRYPT_BLOCKLENGTH)

#ifdef PyECRYPT_GENERATES_KEYSTREAM

#define PyECRYPT_keystream_blocks(ctx, keystream, blocks)            \
  PyECRYPT_keystream_bytes(ctx, keystream,                           \
    (blocks) * PyECRYPT_BLOCKLENGTH)

#endif

#else

/*
* Undef PyECRYPT_HAS_SINGLE_BLOCK_FUNCTION if you want to provide
* separate block encryption and decryption functions.
*/
#define PyECRYPT_HAS_SINGLE_BLOCK_FUNCTION      /* [edit] */
#ifdef PyECRYPT_HAS_SINGLE_BLOCK_FUNCTION

#define PyECRYPT_encrypt_blocks(ctx, plaintext, ciphertext, blocks)     \
  PyECRYPT_process_blocks(0, ctx, plaintext, ciphertext, blocks)

#define PyECRYPT_decrypt_blocks(ctx, ciphertext, plaintext, blocks)     \
  PyECRYPT_process_blocks(1, ctx, ciphertext, plaintext, blocks)

void PyECRYPT_process_blocks(
	int action,                 /* 0 = encrypt; 1 = decrypt; */
	PyECRYPT_ctx* ctx,
	const u8* input,
	u8* output,
	u32 blocks);                /* Message length in blocks. */

#else

void PyECRYPT_encrypt_blocks(
	PyECRYPT_ctx* ctx,
	const u8* plaintext,
	u8* ciphertext,
	u32 blocks);                /* Message length in blocks. */

void PyECRYPT_decrypt_blocks(
	PyECRYPT_ctx* ctx,
	const u8* ciphertext,
	u8* plaintext,
	u32 blocks);                /* Message length in blocks. */

#endif

#ifdef PyECRYPT_GENERATES_KEYSTREAM

void PyECRYPT_keystream_blocks(
	PyECRYPT_ctx* ctx,
	u8* keystream,
	u32 blocks);                /* Keystream length in blocks. */

#endif

#endif

/* ------------------------------------------------------------------------- */

#endif
