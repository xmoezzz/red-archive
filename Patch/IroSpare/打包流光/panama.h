#pragma once

#include <Windows.h>
/* panama.h */

/**************************************************************************+
*
*  PANAMA high-performance reference C-code, based on the description in 
*  the paper 'Fast Hashing and Stream Encryption with PANAMA', presented 
*  at the Fast Software Encryption Workshop, Paris, 1998, see "Fast 
*  Software Encryption - 5th International Workshop, FSE'98", edited by 
*  Serge Vaudenay, LNCS-1372, Springer-Verlag, 1998, pp 60-74, also 
*  available on-line at http://standard.pictel.com/ftp/research/security
*
*  Algorithm design by Joan Daemen and Craig Clapp
*
*  panama.h  -  Header file for Panama C-code implementation.
*
*
*  History:
*
*  29-Oct-98  Craig Clapp  Implemention for Dr. Dobbs, Dec. 1998 issue, 
*                          based on earlier performance-benchmark code.
*
*
*  Notes:  This code is supplied for the purposes of evaluating the 
*          performance of the Panama stream/hash module and as a 
*          reference implementation for generating test vectors for 
*          compatibility / interoperability verification.
*
*
+**************************************************************************/

#ifndef NULL
#define NULL 0
#endif

#define WORDLENGTH   32
#define ONES         0xffffffffL


/* standard C idioms for Microsoft and TriMedia compiler features */
#define restrict		/* 'restrict' keyword is not part of ANSI C, null it out */
#define ROTL32(a,shift)  (((a) << (shift)) | ((a) >> (WORDLENGTH - (shift))))



/****** structure definitions ******/

#define PAN_STAGE_SIZE   8
#define PAN_STAGES       32
#define PAN_STATE_SIZE   17


typedef struct {
	DWORD word[PAN_STAGE_SIZE];
} PAN_STAGE;

typedef struct {
	PAN_STAGE stage[PAN_STAGES];
	int tap_0;
} PAN_BUFFER;

typedef struct {
	DWORD word[PAN_STATE_SIZE];
} PAN_STATE;

typedef struct {
	PAN_BUFFER buffer;
	PAN_STATE state;
	DWORD wkeymat[8];
	byte *keymat;
	int keymat_pointer;
} PANAMA_KEY;


/****** function prototypes ******/

static void pan_pull(DWORD * restrict In,	/* input array                    */
	      DWORD * restrict Out,	/* output array                   */
	      DWORD pan_blocks,	/* number of blocks to be Pulled  */
	      PAN_BUFFER * restrict buffer,	/* LFSR buffer                    */
	      PAN_STATE * restrict state);	/* 17-word finite-state machine   */

static void pan_push(DWORD * restrict In,	/* input array                    */
	      DWORD pan_blocks,	/* number of blocks to be Pushed  */
	      PAN_BUFFER * restrict buffer,	/* LFSR buffer                    */
	      PAN_STATE * restrict state);	/* 17-word finite-state machine   */

static void pan_reset(PAN_BUFFER * buffer, PAN_STATE * state);

#ifndef USE_MODULES
int _mcrypt_set_key(PANAMA_KEY * pan_key, char *in_key, int keysize,
	char *init_vec, int vecsize);

void _mcrypt_encrypt(PANAMA_KEY * pan_key,	/* the key from pan_init */
	byte * buf,	/* input array                         */
	int length);

void _mcrypt_decrypt(PANAMA_KEY * pan_key,	/* the key from pan_init */
	byte * buf,	/* input array                         */
	int length);

#define _mcrypt_panama_set_key _mcrypt_set_key
#define _mcrypt_panama_decrypt _mcrypt_decrypt
#define _mcrypt_panama_encrypt _mcrypt_encrypt

#endif
