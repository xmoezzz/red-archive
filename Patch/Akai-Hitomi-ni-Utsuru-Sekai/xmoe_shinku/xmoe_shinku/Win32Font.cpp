#include "Win32Font.h"

VOID ConvertUnicodeLogfontToAnsi(PLOGFONTA LogFontA, PLOGFONTW LogFontW)
{
	//RtlCopyMemory(LogFontA, LogFontW, PtrOffset(&LogFontW->lfFaceName, LogFontW));
	WideCharToMultiByte(CP_ACP, 0, LogFontW->lfFaceName, (lstrlenW(LogFontW->lfFaceName) + 1) * sizeof(WCHAR), LogFontA->lfFaceName, sizeof(LogFontA->lfFaceName), nullptr, nullptr);
}

VOID ConvertAnsiLogfontToUnicode(PLOGFONTW LogFontW, PLOGFONTA LogFontA)
{
	//CopyMemory(LogFontW, LogFontA, PtrOffset(&LogFontA->lfFaceName, LogFontA));
	MultiByteToWideChar(CP_ACP, 0, LogFontA->lfFaceName, lstrlenA(LogFontA->lfFaceName) + 1, LogFontW->lfFaceName, sizeof(LogFontW->lfFaceName));
}

VOID ConvertUnicodeTextMetricToAnsi(PTEXTMETRICA TextMetricA, CONST TEXTMETRICW *TextMetricW)
{
	TextMetricA->tmHeight = TextMetricW->tmHeight;
	TextMetricA->tmAscent = TextMetricW->tmAscent;
	TextMetricA->tmDescent = TextMetricW->tmDescent;
	TextMetricA->tmInternalLeading = TextMetricW->tmInternalLeading;
	TextMetricA->tmExternalLeading = TextMetricW->tmExternalLeading;
	TextMetricA->tmAveCharWidth = TextMetricW->tmAveCharWidth;
	TextMetricA->tmMaxCharWidth = TextMetricW->tmMaxCharWidth;
	TextMetricA->tmWeight = TextMetricW->tmWeight;
	TextMetricA->tmOverhang = TextMetricW->tmOverhang;
	TextMetricA->tmDigitizedAspectX = TextMetricW->tmDigitizedAspectX;
	TextMetricA->tmDigitizedAspectY = TextMetricW->tmDigitizedAspectY;

	TextMetricA->tmFirstChar = TextMetricW->tmStruckOut;
	TextMetricA->tmLastChar = min(0xFF, TextMetricW->tmLastChar);
	TextMetricA->tmDefaultChar = TextMetricW->tmDefaultChar;
	TextMetricA->tmBreakChar = TextMetricW->tmBreakChar;

	TextMetricA->tmItalic = TextMetricW->tmItalic;
	TextMetricA->tmUnderlined = TextMetricW->tmUnderlined;
	TextMetricA->tmStruckOut = TextMetricW->tmStruckOut;
	TextMetricA->tmPitchAndFamily = TextMetricW->tmPitchAndFamily;
	TextMetricA->tmCharSet = TextMetricW->tmCharSet;
}

#if 0

NTSTATUS
GetNameRecordFromNameTable(
PVOID           TableBuffer,
ULONG_PTR       TableSize,
ULONG_PTR       NameID,
ULONG_PTR       LanguageID,
PUNICODE_STRING Name
)
{
	using namespace Gdi;

	ULONG_PTR               StorageOffset, NameRecordCount;
	PTT_NAME_TABLE_HEADER   NameHeader;
	PTT_NAME_RECORD         NameRecord, NameRecordUser, NameRecordEn;

	NameHeader = (PTT_NAME_TABLE_HEADER)TableBuffer;

	NameRecordCount = Bswap(NameHeader->NameRecordCount);
	StorageOffset = Bswap(NameHeader->StorageOffset);

	if (StorageOffset >= TableSize)
		return STATUS_NOT_SUPPORTED;

	if (StorageOffset < NameRecordCount * sizeof(*NameRecord))
		return STATUS_NOT_SUPPORTED;

	LanguageID = Bswap((USHORT)LanguageID);
	NameRecordUser = nullptr;
	NameRecordEn = nullptr;
	NameRecord = (PTT_NAME_RECORD)(NameHeader + 1);

	FOR_EACH(NameRecord, NameRecord, NameRecordCount)
	{
		if (NameRecord->PlatformID != TT_PLATFORM_ID_WINDOWS)
			continue;

		if (NameRecord->EncodingID != TT_ENCODEING_ID_UTF16_BE)
			continue;

		if (NameRecord->NameID != NameID)
			continue;

		if (NameRecord->LanguageID == Bswap((USHORT)0x0409))
			NameRecordEn = NameRecord;

		if (NameRecord->LanguageID != LanguageID)
			continue;

		NameRecordUser = NameRecord;
		break;
	}

	NameRecordUser = NameRecordUser == nullptr ? NameRecordEn : NameRecordUser;

	if (NameRecordUser != nullptr)
	{
		PWSTR     FaceName, Buffer;
		ULONG_PTR Offset, Length;

		Offset = StorageOffset + Bswap(NameRecordUser->StringOffset);
		Length = Bswap(NameRecordUser->StringLength);
		FaceName = (PWSTR)PtrAdd(TableBuffer, Offset);

		Buffer = Name->Buffer;
		Length = (USHORT)ML_MIN(Length, Name->MaximumLength);
		Name->Length = Length;

		for (ULONG_PTR Index = 0; Index != Length / sizeof(WCHAR); ++Index)
		{
			Buffer[Index] = Bswap(FaceName[Index]);
		}

		if (Length < Name->MaximumLength)
			*PtrAdd(Buffer, Length) = 0;

		return STATUS_SUCCESS;
	}

	return STATUS_NOT_FOUND;
}

NTSTATUS AdjustFontDataInternal(PADJUST_FONT_DATA AdjustData)
{
	NTSTATUS        Status;
	PVOID           Table;
	ULONG_PTR       TableSize, TableName;
	WCHAR           FaceNameBuffer[LF_FACESIZE];
	WCHAR           FullNameBuffer[LF_FULLFACESIZE];
	UNICODE_STRING  FaceName, FullName;

	if (AdjustData->FontType & RASTER_FONTTYPE)
	{
		return STATUS_NOT_SUPPORTED;
	}

	TableName = Gdi::TT_TABLE_TAG_NAME;
	TableSize = GetFontData(AdjustData->DC, TableName, 0, 0, 0);
	if (TableSize == GDI_ERROR)
		return STATUS_OBJECT_NAME_NOT_FOUND;

	Table = alloca(TableSize);
	TableSize = GetFontData(AdjustData->DC, TableName, 0, Table, TableSize);
	if (TableSize == GDI_ERROR)
		return STATUS_OBJECT_NAME_NOT_FOUND;

	RtlInitEmptyUnicodeString(&FaceName, FaceNameBuffer, sizeof(FaceNameBuffer) / 2);
	RtlInitEmptyUnicodeString(&FullName, FullNameBuffer, sizeof(FullNameBuffer) / 2);

	Status = this->GetNameRecordFromNameTable(
		Table,
		TableSize,
		Gdi::TT_NAME_ID_FACENAME,
		this->GetLePeb()->OriginalLocaleID,
		&FaceName
		);

	if (NT_FAILED(Status) || wcsicmp(FaceName.Buffer, AdjustData->EnumLogFontEx->elfLogFont.lfFaceName) != 0)
		return STATUS_CONTEXT_MISMATCH;

	Status = this->GetNameRecordFromNameTable(
		Table,
		TableSize,
		Gdi::TT_NAME_ID_FACENAME,
		this->GetLeb()->LocaleID,
		&FaceName
		);

	Status = NT_SUCCESS(Status) ?
		this->GetNameRecordFromNameTable(
		Table,
		TableSize,
		Gdi::TT_NAME_ID_FULLNAME,
		this->GetLeb()->LocaleID,
		&FullName)
		: Status;

	if (NT_SUCCESS(Status))
	{
		BOOL        Vertical;
		PWSTR       Buffer;
		ULONG_PTR   Length;

		Vertical = AdjustData->EnumLogFontEx->elfLogFont.lfFaceName[0] == '@';

		Buffer = AdjustData->EnumLogFontEx->elfLogFont.lfFaceName + Vertical;
		Length = min(sizeof(AdjustData->EnumLogFontEx->elfLogFont.lfFaceName) - Vertical, FaceName.Length);
		CopyMemory(Buffer, FaceName.Buffer, Length);
		Buffer[Length] = NULL;

		Buffer = AdjustData->EnumLogFontEx->elfFullName + Vertical;
		Length = min(sizeof(AdjustData->EnumLogFontEx->elfFullName) - Vertical, FullName.Length);
		CopyMemory(Buffer, FullName.Buffer, Length);
		Buffer[Length] = NULL;
	}

	return Status;
}

NTSTATUS AdjustFontData(HDC DC, LPENUMLOGFONTEXW EnumLogFontEx, PTEXT_METRIC_INTERNAL TextMetric, ULONG_PTR FontType)
{
	NTSTATUS Status;
	ADJUST_FONT_DATA AdjustData;

	ZeroMemory(&AdjustData, sizeof(AdjustData));
	AdjustData.EnumLogFontEx = EnumLogFontEx;
	AdjustData.DC = DC;

	Status = STATUS_UNSUCCESSFUL;

	LOOP_ONCE
	{
		AdjustData.Font = CreateFontIndirectBypassW(&EnumLogFontEx->elfLogFont);
		if (AdjustData.Font == nullptr)
			break;

		AdjustData.OldFont = (HFONT)SelectObject(DC, AdjustData.Font);
		if (AdjustData.OldFont == nullptr)
			break;

		Status = AdjustFontDataInternal(&AdjustData);
		if (Status == STATUS_CONTEXT_MISMATCH)
			break;

		if (TextMetric != nullptr)
		{
			if (GetTextMetricsA(DC, &TextMetric->TextMetricA) &&
				GetTextMetricsW(DC, &TextMetric->TextMetricW))
			{
				TextMetric->Filled = TRUE;
			}
		}
	}

		if (AdjustData.OldFont != nullptr)
			SelectObject(DC, AdjustData.OldFont);

	if (AdjustData.Font != nullptr)
		this->DeleteObject(AdjustData.Font);

	return Status;
}

INT NTAPI LeEnumFontCallbackAFromW(CONST LOGFONTW *lf, CONST TEXTMETRICW *TextMetricW, DWORD FontType, LPARAM param)
{
	ENUMLOGFONTEXA          EnumLogFontA;
	LPENUMLOGFONTEXW        EnumLogFontW;
	PGDI_ENUM_FONT_PARAM    EnumParam;
	PTEXT_METRIC_INTERNAL   TextMetric;
	TEXTMETRICA             TextMetricA;
	PTEXTMETRICA            tma;

	TextMetric = FIELD_BASE(TextMetricW, TEXT_METRIC_INTERNAL, TextMetricW);
	EnumParam = (PGDI_ENUM_FONT_PARAM)param;

	EnumLogFontW = (LPENUMLOGFONTEXW)lf;
	ConvertUnicodeLogfontToAnsi(&EnumLogFontA.elfLogFont, &EnumLogFontW->elfLogFont);

	RtlUnicodeToMultiByteN((PSTR)EnumLogFontA.elfFullName, sizeof(EnumLogFontA.elfFullName), nullptr, EnumLogFontW->elfFullName, (lstrlenW(EnumLogFontW->elfFullName) + 1) * sizeof(WCHAR));
	RtlUnicodeToMultiByteN((PSTR)EnumLogFontA.elfScript, sizeof(EnumLogFontA.elfScript), nullptr, EnumLogFontW->elfScript, (lstrlenW(EnumLogFontW->elfScript) + 1) * sizeof(WCHAR));
	RtlUnicodeToMultiByteN((PSTR)EnumLogFontA.elfStyle, sizeof(EnumLogFontA.elfStyle), nullptr, EnumLogFontW->elfStyle, (lstrlenW(EnumLogFontW->elfStyle) + 1) * sizeof(WCHAR));

	if (TextMetric->VerifyMagic() == FALSE)
	{
		ConvertUnicodeTextMetricToAnsi(&TextMetricA, TextMetricW);
		tma = &TextMetricA;
	}
	else
	{
		tma = &TextMetric->TextMetricA;
	}

	return ((FONTENUMPROCA)(EnumParam->Callback))(&EnumLogFontA.elfLogFont, tma, FontType, EnumParam->lParam);
}

INT NTAPI LeEnumFontCallbackW(CONST LOGFONTW *lf, CONST TEXTMETRICW *TextMetricW, DWORD FontType, LPARAM param)
{
	NTSTATUS                Status;
	PGDI_ENUM_FONT_PARAM    EnumParam;
	TEXT_METRIC_INTERNAL    TextMetric;

	EnumParam = (PGDI_ENUM_FONT_PARAM)param;

	LOOP_ONCE
	{
		if (EnumParam->Charset != DEFAULT_CHARSET)
		break;

		if (lf->lfFaceName != ANSI_CHARSET && lf->lfCharSet == 936)
			break;

		((LPLOGFONTW)lf)->lfCharSet = 936;
	}

	Status = AdjustFontData(EnumParam->DC, (LPENUMLOGFONTEXW)lf, &TextMetric, FontType);
	if (Status == STATUS_OBJECT_NAME_NOT_FOUND || Status == STATUS_CONTEXT_MISMATCH)
		return TRUE;

	if (TextMetric.Filled == FALSE)
	{
		TextMetric.TextMetricW = *TextMetricW;
		TextMetric.Magic = 0;
	}

	TextMetricW = &TextMetric.TextMetricW;

	return ((FONTENUMPROCW)(EnumParam->Callback))(lf, TextMetricW, FontType, EnumParam->lParam);
}

int NTAPI HookEnumFontFamiliesExW(HDC hdc, LPLOGFONTW lpLogfont, FONTENUMPROCW lpProc, LPARAM lParam, DWORD dwFlags)
{
	INT                 Result;
	GDI_ENUM_FONT_PARAM Param;
	LOGFONTW            LocalLogFont;

	if ((Param.Prepare()) != S_OK)
		return FALSE;

	Param.Callback = lpProc;
	Param.lParam = lParam;
	Param.Charset = lpLogfont->lfCharSet;
	LocalLogFont = *lpLogfont;

	return EnumFontFamiliesExW(hdc, &LocalLogFont, LeEnumFontCallbackW, (LPARAM)&Param, dwFlags);
}

int NTAPI HookEnumFontFamiliesExA(HDC hdc, LPLOGFONTA lpLogfont, FONTENUMPROCA lpProc, LPARAM lParam, DWORD dwFlags)
{
	LOGFONTW            lf;
	GDI_ENUM_FONT_PARAM Param;

	Param.Callback = lpProc;
	Param.lParam = lParam;
	Param.Charset = lpLogfont->lfCharSet;

	ConvertAnsiLogfontToUnicode(&lf, lpLogfont);

	return HookEnumFontFamiliesExW(hdc, &lf, LeEnumFontCallbackAFromW, (LPARAM)&Param, dwFlags);
}


#endif
