#include "lzrw1.h"

#define UBYTE unsigned char /* Unsigned     byte (1 byte )        */

#define FLAG_BYTES    4     /* Number of bytes used by copy flag. */
#define FLAG_COMPRESS 0     /* Signals that compression occurred. */
#define FLAG_COPY     1     /* Signals that a copyover occurred.  */

#define fast_copy(s, d, l) RtlCopyMemory(s, d, l)

/******************************************************************************/

void lzrw1_compress(PBYTE p_src_first, ULONG src_len, PBYTE p_dst_first, PULONG p_dst_len)
/* Input  : Specify input block using p_src_first and src_len.          */
/* Input  : Point p_dst_first to the start of the output zone (OZ).     */
/* Input  : Point p_dst_len to a ULONG to receive the output length.    */
/* Input  : Input block and output zone must not overlap.               */
/* Output : Length of output block written to *p_dst_len.               */
/* Output : Output block in Mem[p_dst_first..p_dst_first+*p_dst_len-1]. */
/* Output : May write in OZ=Mem[p_dst_first..p_dst_first+src_len+256-1].*/
/* Output : Upon completion guaranteed *p_dst_len<=src_len+FLAG_BYTES.  */
#define PS *p++!=*s++  /* Body of inner unrolled matching loop.         */
#define ITEMMAX 16     /* Maximum number of bytes in an expanded item.  */
{
	UBYTE *p_src = p_src_first, *p_dst = p_dst_first;
	UBYTE *p_src_post = p_src_first + src_len, *p_dst_post = p_dst_first + src_len;
	UBYTE *p_src_max1 = p_src_post - ITEMMAX, *p_src_max16 = p_src_post - 16 * ITEMMAX;
	UBYTE *hash[4096], *p_control; UWORD control = 0, control_bits = 0;
	*p_dst = FLAG_COMPRESS; p_dst += FLAG_BYTES; p_control = p_dst; p_dst += 2;
	while (TRUE)
	{
		UBYTE *p, *s; UWORD unroll = 16, len, index; ULONG offset;
		if (p_dst > p_dst_post) goto overrun;
		if (p_src > p_src_max16)
		{
			unroll = 1;
			if (p_src > p_src_max1)
			{
				if (p_src == p_src_post) break; goto literal;
			}
		}
	begin_unrolled_loop:
		index = ((40543 * ((((p_src[0] << 4) ^ p_src[1]) << 4) ^ p_src[2])) >> 4) & 0xFFF;
		p = hash[index]; hash[index] = s = p_src; offset = s - p;
		if (offset > 4095 || p < p_src_first || offset == 0 || PS || PS || PS)
		{
			literal: *p_dst++ = *p_src++; control >>= 1; control_bits++;
		}
		else
		{
			PS || PS || PS || PS || PS || PS || PS ||
				PS || PS || PS || PS || PS || PS || s++; len = s - p_src - 1;
			*p_dst++ = ((offset & 0xF00) >> 4) + (len - 1); *p_dst++ = offset & 0xFF;
			p_src += len; control = (control >> 1) | 0x8000; control_bits++;
		}
	end_unrolled_loop: if (--unroll) goto begin_unrolled_loop;
		if (control_bits == 16)
		{
			*p_control = control & 0xFF; *(p_control + 1) = control >> 8;
			p_control = p_dst; p_dst += 2; control = control_bits = 0;
		}
	}
	control >>= 16 - control_bits;
	*p_control++ = control & 0xFF; *p_control++ = control >> 8;
	if (p_control == p_dst) p_dst -= 2;
	*p_dst_len = p_dst - p_dst_first;
	return;
overrun:
	fast_copy(p_src_first, p_dst_first + FLAG_BYTES, src_len);
	*p_dst_first = FLAG_COPY; *p_dst_len = src_len + FLAG_BYTES;
}

/******************************************************************************/

void lzrw1_decompress(PBYTE p_src_first, ULONG src_len, PBYTE p_dst_first, PULONG p_dst_len)

/* Input  : Specify input block using p_src_first and src_len.          */
/* Input  : Point p_dst_first to the start of the output zone.          */
/* Input  : Point p_dst_len to a ULONG to receive the output length.    */
/* Input  : Input block and output zone must not overlap. User knows    */
/* Input  : upperbound on output block length from earlier compression. */
/* Input  : In any case, maximum expansion possible is eight times.     */
/* Output : Length of output block written to *p_dst_len.               */
/* Output : Output block in Mem[p_dst_first..p_dst_first+*p_dst_len-1]. */
/* Output : Writes only  in Mem[p_dst_first..p_dst_first+*p_dst_len-1]. */
{
	UWORD controlbits = 0, control;
	UBYTE *p_src = p_src_first + FLAG_BYTES, *p_dst = p_dst_first,
		*p_src_post = p_src_first + src_len;
	if (*p_src_first == FLAG_COPY)
	{
		fast_copy(p_src_first + FLAG_BYTES, p_dst_first, src_len - FLAG_BYTES);
		*p_dst_len = src_len - FLAG_BYTES; return;
	}
	while (p_src != p_src_post)
	{
		if (controlbits == 0)
		{
			control = *p_src++; control |= (*p_src++) << 8; controlbits = 16;
		}
		if (control & 1)
		{
			UWORD offset, len; UBYTE *p;
			offset = (*p_src & 0xF0) << 4; len = 1 + (*p_src++ & 0xF);
			offset += *p_src++ & 0xFF; p = p_dst - offset;
			while (len--) *p_dst++ = *p++;
		}
		else
			*p_dst++ = *p_src++;
		control >>= 1; controlbits--;
	}
	*p_dst_len = p_dst - p_dst_first;
}

/******************************************************************************/
/*                          End of LZRW1.C                                    */
/******************************************************************************/
