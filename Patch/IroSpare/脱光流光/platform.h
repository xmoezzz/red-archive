/* platform-specific definitions for Phelix */

#ifndef _PLATFORM_H_
#define _PLATFORM_H_

#include <limits.h>	 /* get definitions: UINT_MAX, ULONG_MAX, USHORT_MAX */

typedef	unsigned char	u08b;

#if   (UINT_MAX   == 0xFFFFFFFFu)		/* find correct typedef for u32b */
typedef	unsigned int	u32b;
#elif (ULONG_MAX  == 0xFFFFFFFFu)
typedef	unsigned long	u32b;
#elif (USHORT_MAX == 0xFFFFFFFFu)
typedef	unsigned short	u32b;
#else
#error Need typedef for u32b!!
#endif

/* now figure out endianness */
#if   defined(_MSC_VER)					/* x86 (MSC) */
#define _LITTLE_ENDIAN_
#pragma intrinsic(_lrotl,_lrotr)		/* MSC: compile rotations "inline"   */
#define ROTL32(x,n) _lrotl(x,n)
#define ROTR32(x,n) _lrotr(x,n)
#elif defined(i386)						/* x86 (gcc) */
#define _LITTLE_ENDIAN_
#elif defined(__i386)					/* x86 (gcc) */
#define _LITTLE_ENDIAN_
#elif defined(_M_IX86)			        /* x86  */
#define _LITTLE_ENDIAN_
#elif defined(__INTEL_COMPILER)			/* x86  */
#define _LITTLE_ENDIAN_
#elif defined(__ultrix)					/* Older MIPS? */
#define ECRYPT__LITTLE_ENDIAN_
#elif defined(__alpha)					/* Alpha */
#define _LITTLE_ENDIAN_

/* BIG endian machines:  */
#elif defined(sun) || defined(sparc)	/* Sun */
#define _BIG_ENDIAN_
#elif defined(__ppc__)					/* PowerPC */
#define _BIG_ENDIAN_
#endif

#ifndef ROTL32
#define ROTL32(x,n) ((u32b)(((x) << (n)) ^ ((x) >> (32-(n)))))
#endif

#ifndef ROTR32
#define ROTR32(x,n) ((u32b)(((x) >> (n)) ^ ((x) << (32-(n)))))
#endif

/* handle Phelix byte swapping -- only needed on big-endian CPUs */
#if	  defined(_LITTLE_ENDIAN_)
#define	BSWAP(x) (x)
#elif defined(_BIG_ENDIAN_)
#define	BSWAP(x) ((ROTL32(x,8) & 0x00FF00FF) ^ (ROTR32(x,8) & 0xFF00FF00))
#endif

#endif /* _PLATFORM_H_ */
