#ifndef _Win32Font_
#define _Win32Font_

#include <Windows.h>
#include "MinNtdll.h"
#include <malloc.h>
#include <memory>

#define TAG4(ulong) ((ULONG) ulong)
#define LOOP_ONCE for (int xxasdafnuinoinasconi = 1; xxasdafnuinoinasconi; xxasdafnuinoinasconi--)

typedef struct TEXT_METRIC_INTERNAL
{
	ULONG       Magic;
	BOOL        Filled;
	TEXTMETRICA TextMetricA;
	TEXTMETRICW TextMetricW;

	TEXT_METRIC_INTERNAL()
	{
		this->Magic = TAG4('TMIN');
		this->Filled = FALSE;
	}

	BOOL VerifyMagic()
	{
		return this->Magic == TAG4('TMIN');
	}

} TEXT_METRIC_INTERNAL, *PTEXT_METRIC_INTERNAL;

typedef struct GDI_ENUM_FONT_PARAM
{
	LPARAM                  lParam;
	PVOID                   Callback;
	ULONG                   Charset;
	HDC                     DC;

	GDI_ENUM_FONT_PARAM()
	{
		this->DC = nullptr;
	}

	HRESULT Prepare()
	{
		this->DC = CreateCompatibleDC(nullptr);
		return this->DC == nullptr ? S_FALSE : S_OK;
	}

	~GDI_ENUM_FONT_PARAM()
	{
		if (this->DC != nullptr)
		{
			DeleteDC(this->DC);
		}
	}

} GDI_ENUM_FONT_PARAM, *PGDI_ENUM_FONT_PARAM;


typedef struct ADJUST_FONT_DATA
{
	HDC                 DC;
	HFONT               Font;
	HFONT               OldFont;
	ULONG_PTR           FontType;
	LPENUMLOGFONTEXW    EnumLogFontEx;

} ADJUST_FONT_DATA, *PADJUST_FONT_DATA;


#endif
