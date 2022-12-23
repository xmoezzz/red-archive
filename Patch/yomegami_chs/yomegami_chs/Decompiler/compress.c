#include "compress.h"

void psb_pixel_uncompress(const unsigned char* pInput, unsigned char* pOutput, uint32_t actualSize, uint32_t align)
{
	int i;
	int count;
	uint32_t totalBytes = 0;
	int cmdByte = 0;

	while (actualSize != totalBytes)
	{
		cmdByte = *pInput++; totalBytes++;

		if (cmdByte & PSB_LZSS_LOOKAHEAD)
		{
			count = (cmdByte ^ PSB_LZSS_LOOKAHEAD) + 3;

			for (i = 0; i < count; i++)
			{
				memcpy(pOutput, pInput, align);
				pOutput += align;
			}

			pInput += align; totalBytes += align;
		}
		else {
			count = (cmdByte + 1) * align;

			for (i = 0; i < count; i++) {
				*pOutput++ = *pInput++;
			}

			totalBytes += count;
		}
	}
}

int psb_pixel_compress_bound(unsigned char *data, const unsigned char *end, uint32_t align, unsigned char *result)
{
	uint32_t count = 0;
	unsigned char *p;
	unsigned char temp[4];

	int i;


	memcpy(temp, data, align);

	for (i = 0; i < (PSB_LZSS_LOOKAHEAD + 2); i++)
	{
		p = &data[i * align];		

		if (p >= end) {		
			break;
		}

		if (memcmp(temp, &data[i * align], align) == 0) {	
			count++;
		}
		else {
			break;
		}
	}

	if (count >= 3) {
		*result = (count - 3) | PSB_LZSS_LOOKAHEAD;	
		return count;
	}

	return 0;
}

int psb_pixel_compress_bound_np(unsigned char *data, const unsigned char *end, uint32_t align, unsigned char *result)
{
	uint32_t count = 1;
	unsigned char *p;
	unsigned char temp[4];

	int i;

	memcpy(temp, data, align);

	for (i = 1; i < PSB_LZSS_LOOKAHEAD; i++)
	{
		p = &data[i * align];
		if(p >= end) break;
		if (psb_pixel_compress_bound(p, end, align, result) == 0) {
			count++;
		}
		else {
			break;
		}
	}
	*result = (count - 1);	
	return count;
}


unsigned char* psb_pixel_compress(unsigned char *data, uint32_t length, uint32_t align, uint32_t *actualSize)
{
	unsigned char *result, *p, *end;
	unsigned char cmdByte;
	uint32_t pos, blockSize;
	int count;

	p = data;
	end = data + length;
	pos = 0;

	result = malloc(0);

	while (p != end)
	{
		count = psb_pixel_compress_bound(p, end, align, &cmdByte);

		if (count > 0) {
			
			blockSize = align + sizeof(cmdByte);
			result = realloc(result, pos + blockSize);
			result[pos] = cmdByte;
			memcpy(&result[pos + sizeof(cmdByte)], p, align);
		}
		else {
			count = psb_pixel_compress_bound_np(p, end, align, &cmdByte);
			
			blockSize = count * align + sizeof(cmdByte);
			result = realloc(result, pos + blockSize);
			result[pos] = cmdByte;
			memcpy(&result[pos + sizeof(cmdByte)], p, count * align);
		}

		pos += blockSize;
		p += count * align;
	}

	*actualSize = pos;
	return result;
}