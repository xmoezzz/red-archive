#include <stdio.h>
#include <string>
#include <Windows.h>

using std::string;

struct DATHDR {
	unsigned char signature[8];
	unsigned long unknown1; // hash for parity check?
	unsigned long unknown2;
};

struct DATDATAHDR {
	unsigned long seed;
	unsigned long length;
};

struct DATENTRYDIR {
	unsigned long name_hash;
	unsigned long entry_count;
};

struct DATENTRYFILE {
	unsigned long name_hash;
	unsigned long length;
};

void unobfuscate(unsigned char* buff,
	unsigned long  len,
	unsigned long  seed)
{
	unsigned long t1 = seed ^ (seed + 0x5D588B65);
	unsigned long t2 = t1 ^ (seed - 0x359D3E2A);

	unsigned long t3 = t2 ^ (t1 - 0x70E44324);
	unsigned long t4 = t3 ^ (t2 + 0x6C078965);

	unsigned long long key = ((unsigned long long)t4 << 32) | t3;;

	unsigned long long* p = (unsigned long long*) buff;
	unsigned long long* end = p + (len / 8);

	while (p < end) {
		*p ^= key;

		// paddw
		unsigned short* ka = (unsigned short*)&key;
		unsigned short* pa = (unsigned short*)p;
		ka[0] += pa[0];
		ka[1] += pa[1];
		ka[2] += pa[2];
		ka[3] += pa[3];

		p++;
	}
}

void read_unobfuscate(FILE* fd, unsigned char*& buff, unsigned long& len) {
	DATDATAHDR hdr;
	fread(&hdr, 1, sizeof(hdr), fd);

	len = hdr.length;
	unsigned long padded_len = (len + 7) & ~7;
	buff = new unsigned char[padded_len];

	fread(buff, 1, len, fd);

	if (hdr.seed) {
		unobfuscate(buff, padded_len, hdr.seed);
	}
}


BYTE OggsSign[4] = { 0x4f, 0x67, 0x67, 0x53 };
BYTE PngSign[4] = { 0x89, 0x50, 0x4e, 0x47 };
BYTE MngSign[4] = { 0x8a, 0x4d, 0x4e, 0x47 };
BYTE PsdSign[4] = { 0x38, 0x42, 0x50, 0x53 };


bool IsTextFile(PBYTE Buffer, ULONG Length)
{
	for (ULONG i = 0; i < Length; i++)
	{
		if (Buffer[i] == 0)
			return false;
	}
	return true;
}

int main(int argc, char** argv) {
	if (argc != 2) {
		return -1;
	}

	string in_filename(argv[1]);

	FILE* fd = fopen(in_filename.c_str(), "rb");
	if (!fd)
		return -1;

	FILE* txt = fopen("Rename.txt", "wb");
	if (!txt)
		return -1;

	DATHDR hdr;
	fread(&hdr, 1, sizeof(hdr), fd);

	unsigned long  toc_len = 0;
	unsigned char* toc_buff = NULL;
	read_unobfuscate(fd, toc_buff, toc_len);

	unsigned char* toc_p = toc_buff;
	unsigned char* toc_end = toc_buff + toc_len;

	int index = 0;
	for (unsigned long i = 0; toc_p < toc_end; i++) {
		DATENTRYDIR* dir = (DATENTRYDIR*)toc_p;
		toc_p += sizeof(*dir);

		for (unsigned long j = 0; j < dir->entry_count; j++) {
			DATENTRYFILE* file = (DATENTRYFILE*)toc_p;
			toc_p += sizeof(*file);

			unsigned long  len = 0;
			unsigned char* buff = NULL;
			read_unobfuscate(fd, buff, len);


			if (!memcmp(OggsSign, buff, 4))
				fprintf(txt, "{ \"%05d+%05d.ogg\", \"%d_%08x.ogg\" },\r\n", i, j, index, file->name_hash);
			else if (!memcmp(PngSign, buff, 4))
				fprintf(txt, "{ \"%05d+%05d.png\", \"%d_%08x.png\" },\r\n", i, j, index, file->name_hash);
			else if (!memcmp(PsdSign, buff, 4))
				fprintf(txt, "{ \"%05d+%05d.psd\", \"%d_%08x.pad\" },\r\n", i, j, index, file->name_hash);
			else if (!memcmp(MngSign, buff, 4))
				fprintf(txt, "{ \"%05d+%05d.mng\", \"%d_%08x.mng\" },\r\n", i, j, index, file->name_hash);
			else if (IsTextFile(buff, len))
				fprintf(txt, "{ \"%05d+%05d.txt\", \"%d_%08x.txt\" },\r\n", i, j, index, file->name_hash);
			else
				fprintf(txt, "{ \"%05d+%05d\", \"%d_%08x\" },\r\n", i, j, index, file->name_hash);

			delete[] buff;
		}
		index++;
	}

	delete[] toc_buff;
	fclose(fd);
	fclose(txt);
	return 0;
}
