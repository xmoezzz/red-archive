#ifndef _UCIGraph_
#define _UCIGraph_

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


typedef unsigned char	U8;
typedef unsigned short	U16;
typedef unsigned int	U32;

#ifdef __cplusplus
extern "C" {
#endif

	typedef struct
	{
		U8 red[256];
		U8 green[256];
		U8 blue[256];
	}SColorMap;

#pragma pack(push)
#pragma pack(1)
	typedef struct PictureInfo
	{
		int left, top;
		int width, height;
		unsigned short x_density, y_density;
		short bit;
		void* hInfo;
	}PictureInfo;
#pragma pack(pop)

	int __stdcall UCIDecode(
		const void* src,	// ����UCI����ָ��(���ܴ���null,����ָ��������Դ���null��ʾ����Ҫ���)
		int 		srclen, // ����UCI���ݳ���
		void**		dst,	// ���RAW���ݵ�ָ��(BGR��BGRA��ʽ)
		int*		stride, // ���RAW���ݵ��м��ֽڿ��(dst��Ϊnullʱ,stride���ܴ���null)
		int*		width,	// ���ͼ��Ŀ��ֵ
		int*		height, // ���ͼ��ĸ߶�ֵ
		int*		bit);	// ���ͼ���bppֵ(ÿ����λ��)

	void __stdcall UCIFree(void* p);

	void __stdcall UCIDebug(int level);

	BOOL __stdcall UCIInitOrUninit(DWORD reason);

	BOOL __stdcall gfpGetPluginInfo(DWORD version, char* label, int label_max_size, char* extension, int extension_max_size, int* support);

	void* __stdcall gfpLoadPictureInit(const WCHAR* filename);

	BOOL __stdcall gfpLoadPictureGetInfo(void* p, int* pictype, int* width, int* height, int* dpi, int* bits_per_pixel, int* bytes_per_line, BOOL* has_colormap, char* label, int label_max_size);

	BOOL __stdcall gfpLoadPictureGetLine(void* p, int line, U8* buf);

	BOOL __stdcall gfpLoadPictureGetColormap(void* p, SColorMap* cmap);

	void __stdcall gfpLoadPictureExit(void* p);

	int __stdcall IsSupported(const WCHAR* filename, int dw);

	int __stdcall GetPictureInfo(const WCHAR* filename, int len, unsigned flag, PictureInfo* pi);

	int __stdcall GetPicture(const WCHAR* filename, int len, unsigned flag, HLOCAL* hbi, HLOCAL* hbm, void* lpPrgressCallback, int lData);

	int __stdcall GetPreview(const WCHAR* filename, int len, unsigned flag, HLOCAL* hbi, HLOCAL* hbm, void* lpPrgressCallback, int lData);

#ifdef __cplusplus
}
#endif

#endif

