//---------------------------------------------------------------------------
/*
	Risa [�肳]      alias �g���g��3 [kirikiri-3]
	 stands for "Risa Is a Stagecraft Architecture"
	Copyright (C) 2000 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
//! @file
//! @brief FreeType �� Face ���N���X�̒�`
//---------------------------------------------------------------------------

#ifndef FREETYPEFACE_H
#define FREETYPEFACE_H

#include <ft2build.h>
#include FT_FREETYPE_H
#include <vector>
#include <string>

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
/**
 * FreeType Face �̊��N���X
 */
class tBaseFreeTypeFace
{
public:
	virtual FT_Face GetFTFace() const = 0; //!< FreeType �� Face �I�u�W�F�N�g��Ԃ�
	virtual void GetFaceNameList(std::vector<std::wstring> & dest) const = 0; //!< ���̃t�H���g�t�@�C���������Ă���t�H���g��z��Ƃ��ĕԂ�
	virtual ~tBaseFreeTypeFace() {;}
	virtual tjs_char GetDefaultChar() const = 0; //!< �`��ł��Ȃ����ɕ`�悷�镶���R�[�h��Ԃ�
};
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------



#endif

