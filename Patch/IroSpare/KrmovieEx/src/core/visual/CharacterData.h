

#ifndef __CHARACTER_DATA_H__
#define __CHARACTER_DATA_H__

#include "tjsCommHead.h"
#include "tvpfontstruc.h"

/**
 * �P�O���t�̃��g���b�N��\���\����
 */
struct tGlyphMetrics
{
	tjs_int CellIncX;		//!< �ꕶ���i�߂�̕K�v��X�����̃s�N�Z����
	tjs_int CellIncY;		//!< �ꕶ���i�߂�̕K�v��Y�����̃s�N�Z����
};

//---------------------------------------------------------------------------
/**
 * �P�O���t��\���N���X
 */
class tTVPCharacterData
{
	// character data holder for caching
private:
	tjs_uint8 * Data;
	tjs_int RefCount;

public:
	tjs_int OriginX; //!< ����Bitmap��`�悷��ascent�ʒu�Ƃ̉��I�t�Z�b�g
	tjs_int OriginY; //!< ����Bitmap��`�悷��ascent�ʒu�Ƃ̏c�I�t�Z�b�g
	tGlyphMetrics	Metrics; //!< ���g���b�N�A���蕝�ƍ�����ێ�
	tjs_int Pitch; //!< �ێ����Ă���摜�s�b�`
	tjs_uint BlackBoxX; //!< �ێ����Ă���摜��
	tjs_uint BlackBoxY; //!< �ێ����Ă���摜����
	tjs_int BlurLevel;
	tjs_int BlurWidth;
	tjs_uint Gray; // �K��

	bool Antialiased;
	bool Blured;
	bool FullColored;

public:
	tTVPCharacterData() : Gray(65), FullColored(false) { RefCount = 1; Data = NULL; }
	tTVPCharacterData( const tjs_uint8 * indata,
		tjs_int inpitch,
		tjs_int originx, tjs_int originy,
		tjs_uint blackboxw, tjs_uint blackboxh,
		const tGlyphMetrics & metrics,
		bool fullcolor = false );
	tTVPCharacterData(const tTVPCharacterData & ref);
	~tTVPCharacterData() { if(Data) delete [] Data; }

	void Alloc(tjs_int size) {
		if(Data) delete [] Data, Data = NULL;
		Data = new tjs_uint8[size];
	}

	tjs_uint8 * GetData() const { return Data; }

	void AddRef() { RefCount ++; }
	void Release() {
		if(RefCount == 1) {
			delete this;
		} else {
			RefCount--;
		}
	}

	void Expand();

	void Blur(tjs_int blurlevel, tjs_int blurwidth);
	void Blur();

	void Bold(tjs_int size);
	void Bold2(tjs_int size);

	void Resample4();
	void Resample8();

	/**
	 * ��������ǉ�����(���������A�A���_�[���C���p)
	 * @param liney : ���C�����S�ʒu
	 * @param thickness : ���C������
	 * @param val : ���C���l
	 */
	void AddHorizontalLine( tjs_int liney, tjs_int thickness, tjs_uint8 val );
};

//---------------------------------------------------------------------------
// Character Cache management
//---------------------------------------------------------------------------
struct tTVPFontAndCharacterData
{
	tTVPFont Font;
	tjs_uint32 FontHash;
	tjs_char Character;
	tjs_int BlurLevel;
	tjs_int BlurWidth;
	bool Antialiased;
	bool Blured;
	bool Hinting;
	bool operator == (const tTVPFontAndCharacterData &rhs) const {
		return Character == rhs.Character && Font == rhs.Font &&
			Antialiased == rhs.Antialiased && BlurLevel == rhs.BlurLevel &&
			BlurWidth == rhs.BlurWidth && Blured == rhs.Blured &&
			Hinting == rhs.Hinting;
	}
};
//---------------------------------------------------------------------------
class tTVPFontHashFunc
{
public:
	static tjs_uint32 Make(const tTVPFontAndCharacterData &val)
	{
		tjs_uint32 v = val.FontHash;

		v ^= val.Antialiased?1:0;
		v ^= val.Character;
		v ^= val.Blured?1:0;
		v ^= val.BlurLevel ^ val.BlurWidth;
		return v;
	}
};





#endif // __CHARACTER_DATA_H__
