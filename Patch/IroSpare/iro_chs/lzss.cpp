#include "lzss.h"

void ps2_uncompress(BYTE *uncompr, DWORD uncomprlen, BYTE *compr, DWORD comprlen)
{
	DWORD curbyte = 0;
	DWORD flag = 0;
	DWORD cur_winpos = 0x7df;
	BYTE window[2048];

	memset(window, 0, cur_winpos);
	while (1) {
		flag >>= 1;
		if (!(flag & 0x0100)) {
			if (curbyte >= comprlen)
				break;
			flag = compr[curbyte++] | 0xff00;
		}

		if (flag & 1) {
			BYTE data;

			if (curbyte >= comprlen)
				break;

			data = compr[curbyte++];
			window[cur_winpos++] = data;
			*uncompr++ = data;
			cur_winpos &= 0x7ff;
		}
		else {
			DWORD offset, count;

			if (curbyte >= comprlen)
				break;
			offset = compr[curbyte++];

			if (curbyte >= comprlen)
				break;
			count = compr[curbyte++];

			offset |= (count & 0xe0) << 3;
			count = (count & 0x1f) + 2;

			for (DWORD i = 0; i < count; ++i) {
				BYTE data;

				data = window[(offset + i) & 0x7ff];
				*uncompr++ = data;
				window[cur_winpos++] = data;
				cur_winpos &= 0x7ff;
			}
		}
	}
}
