#include "Charset.h"

static tjs_int inline AnzWideCharToUtf8(tjs_char in, char * out)
{
	// convert a wide character 'in' to utf-8 character 'out'
	if (in < (1 << 7))
	{
		if (out)
		{
			out[0] = (char)in;
		}
		return 1;
	}
	else if (in < (1 << 11))
	{
		if (out)
		{
			out[0] = (char)(0xc0 | (in >> 6));
			out[1] = (char)(0x80 | (in & 0x3f));
		}
		return 2;
	}
	else if (in < (1 << 16))
	{
		if (out)
		{
			out[0] = (char)(0xe0 | (in >> 12));
			out[1] = (char)(0x80 | ((in >> 6) & 0x3f));
			out[2] = (char)(0x80 | (in & 0x3f));
		}
		return 3;
	}
#if 1
	else
	{
		TVPThrowExceptionMessage(TJS_W("Cannot covert from UTF-16 to UTF-8"));
	}
#else
	else if (in < (1 << 21))
	{
		if (out)
		{
			out[0] = (char)(0xf0 | (in >> 18));
			out[1] = (char)(0x80 | ((in >> 12) & 0x3f));
			out[2] = (char)(0x80 | ((in >> 6) & 0x3f));
			out[3] = (char)(0x80 | (in & 0x3f));
		}
		return 4;
	}
	else if (in < (1 << 26))
	{
		if (out)
		{
			out[0] = (char)(0xf8 | (in >> 24));
			out[1] = (char)(0x80 | ((in >> 16) & 0x3f));
			out[2] = (char)(0x80 | ((in >> 12) & 0x3f));
			out[3] = (char)(0x80 | ((in >> 6) & 0x3f));
			out[4] = (char)(0x80 | (in & 0x3f));
		}
		return 5;
	}
	else if (in < (1 << 31))
	{
		if (out)
		{
			out[0] = (char)(0xfc | (in >> 30));
			out[1] = (char)(0x80 | ((in >> 24) & 0x3f));
			out[2] = (char)(0x80 | ((in >> 18) & 0x3f));
			out[3] = (char)(0x80 | ((in >> 12) & 0x3f));
			out[4] = (char)(0x80 | ((in >> 6) & 0x3f));
			out[5] = (char)(0x80 | (in & 0x3f));
		}
		return 6;
	}
#endif
	return -1;
}


tjs_int AnzWideCharToUtf8String(const tjs_char *in, char * out)
{
	// convert input wide string to output utf-8 string
	int count = 0;
	while (*in)
	{
		tjs_int n;
		if (out)
		{
			n = AnzWideCharToUtf8(*in, out);
			out += n;
		}
		else
		{
			n = AnzWideCharToUtf8(*in, NULL);
		}
		if (n == -1) return -1; // invalid character found
		count += n;
		in++;
	}
	return count;
}

static bool inline AnzUtf8ToWideChar(const char * & in, tjs_char *out)
{
	const unsigned char * & p = (const unsigned char * &)in;
	if (p[0] < 0x80)
	{
		if (out) *out = (tjs_char)in[0];
		in++;
		return true;
	}
	else if (p[0] < 0xc2)
	{
		// invalid character
		return false;
	}
	else if (p[0] < 0xe0)
	{
		// two bytes (11bits)
		if ((p[1] & 0xc0) != 0x80) return false;
		if (out) *out = ((p[0] & 0x1f) << 6) + (p[1] & 0x3f);
		in += 2;
		return true;
	}
	else if (p[0] < 0xf0)
	{
		// three bytes (16bits)
		if ((p[1] & 0xc0) != 0x80) return false;
		if ((p[2] & 0xc0) != 0x80) return false;
		if (out) *out = ((p[0] & 0x1f) << 12) + ((p[1] & 0x3f) << 6) + (p[2] & 0x3f);
		in += 3;
		return true;
	}
	else if (p[0] < 0xf8)
	{
		// four bytes (21bits)
		if ((p[1] & 0xc0) != 0x80) return false;
		if ((p[2] & 0xc0) != 0x80) return false;
		if ((p[3] & 0xc0) != 0x80) return false;
		if (out) *out = ((p[0] & 0x07) << 18) + ((p[1] & 0x3f) << 12) +
			((p[2] & 0x3f) << 6) + (p[3] & 0x3f);
		in += 4;
		return true;
	}
	else if (p[0] < 0xfc)
	{
		// five bytes (26bits)
		if ((p[1] & 0xc0) != 0x80) return false;
		if ((p[2] & 0xc0) != 0x80) return false;
		if ((p[3] & 0xc0) != 0x80) return false;
		if ((p[4] & 0xc0) != 0x80) return false;
		if (out) *out = ((p[0] & 0x03) << 24) + ((p[1] & 0x3f) << 18) +
			((p[2] & 0x3f) << 12) + ((p[3] & 0x3f) << 6) + (p[4] & 0x3f);
		in += 5;
		return true;
	}
	else if (p[0] < 0xfe)
	{
		// six bytes (31bits)
		if ((p[1] & 0xc0) != 0x80) return false;
		if ((p[2] & 0xc0) != 0x80) return false;
		if ((p[3] & 0xc0) != 0x80) return false;
		if ((p[4] & 0xc0) != 0x80) return false;
		if ((p[5] & 0xc0) != 0x80) return false;
		if (out) *out = ((p[0] & 0x01) << 30) + ((p[1] & 0x3f) << 24) +
			((p[2] & 0x3f) << 18) + ((p[3] & 0x3f) << 12) +
			((p[4] & 0x3f) << 6) + (p[5] & 0x3f);
		in += 6;
		return true;
	}
	return false;
}

tjs_int AnzUtf8ToWideCharString(const char * in, tjs_char *out)
{
	// convert input utf-8 string to output wide string
	int count = 0;
	while (*in)
	{
		tjs_char c;
		if (out)
		{
			if (!AnzUtf8ToWideChar(in, &c))
				return -1; // invalid character found
			*out++ = c;
		}
		else
		{
			if (!AnzUtf8ToWideChar(in, NULL))
				return -1; // invalid character found
		}
		count++;
	}
	return count;
}


