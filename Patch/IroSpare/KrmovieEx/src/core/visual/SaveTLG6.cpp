#include "tjsCommHead.h"

#include "GraphicsLoaderIntf.h"
#include "MsgIntf.h"
#include "tjsUtils.h"
#include "tvpgl.h"
#include "LayerBitmapIntf.h"
#include "StorageIntf.h"
#include "SaveTLG.h"
#include "UtilStreams.h"

#include <stdlib.h>
#include <memory.h>
#include <sstream>

// Table for 'k' (predicted bit length) of golomb encoding
#define TVP_TLG6_GOLOMB_N_COUNT 4

// golomb bit length table is compressed, so we need to
// decompress it.
char TVPTLG6GolombBitLengthTable[TVP_TLG6_GOLOMB_N_COUNT*2*128][TVP_TLG6_GOLOMB_N_COUNT];
static bool TVPTLG6GolombTableInit = false;
static short int TVPTLG6GolombCompressed[TVP_TLG6_GOLOMB_N_COUNT][9] = {
		{3,7,15,27,63,108,223,448,130,},
		{3,5,13,24,51,95,192,384,257,},
		{2,5,12,21,39,86,155,320,384,},
		{2,3,9,18,33,61,129,258,511,},
	// Tuned by W.Dee, 2004/03/25
};

void TVPTLG6InitGolombTable()
{
	if(TVPTLG6GolombTableInit) return;
	for(int n = 0; n < TVP_TLG6_GOLOMB_N_COUNT; n++)
	{
		int a = 0;
		for(int i = 0; i < 9; i++)
		{
			for(int j = 0; j < TVPTLG6GolombCompressed[n][i]; j++)
				TVPTLG6GolombBitLengthTable[a++][n] = (char)i;
		}
		if(a != TVP_TLG6_GOLOMB_N_COUNT*2*128)
			*(char*)0 = 0;   // THIS MUST NOT BE EXECUETED!
				// (this is for compressed table data check)
	}
	TVPTLG6GolombTableInit = true;
}

// TLG6.0 bitstream output implementation


class TLG6BitStream
{
	int BufferBitPos; // bit position of output buffer
	long BufferBytePos; // byte position of output buffer
	tTJSBinaryStream * OutStream; // output stream
	unsigned char *Buffer; // output buffer
	long BufferCapacity; // output buffer capacity

public:
	TLG6BitStream(tTJSBinaryStream * outstream) :
		OutStream(outstream),
		BufferBitPos(0),
		BufferBytePos(0),
		Buffer(NULL),
		BufferCapacity(0)
	{
	}

	~TLG6BitStream()
	{
		Flush();
	}

public:
	int GetBitPos() const { return BufferBitPos; }
	long GetBytePos() const { return BufferBytePos; }

	void Flush()
	{
		if(Buffer && (BufferBitPos || BufferBytePos))
		{
			if(BufferBitPos) BufferBytePos ++;
			OutStream->Write(Buffer, BufferBytePos);
			BufferBytePos = 0;
			BufferBitPos = 0;
		}
		free(Buffer);
		BufferCapacity = 0;
		Buffer = NULL;
	}

	long GetBitLength() const { return BufferBytePos * 8 + BufferBitPos; }

	void Put1Bit(bool b)
	{
		if(BufferBytePos == BufferCapacity)
		{
			// need more bytes
			long org_cap = BufferCapacity;
			BufferCapacity += 0x1000;
			if(Buffer)
				Buffer = (unsigned char *)realloc(Buffer, BufferCapacity);
			else
				Buffer = (unsigned char *)malloc(BufferCapacity);
			if(!Buffer) TVPThrowExceptionMessage( TVPTlgInsufficientMemory );
			memset(Buffer + org_cap, 0, BufferCapacity - org_cap);
		}

		if(b) Buffer[BufferBytePos] |= 1 << BufferBitPos;
		BufferBitPos ++;
		if(BufferBitPos == 8)
		{
			BufferBitPos = 0;
			BufferBytePos ++;
		}
	}

	void PutGamma(int v)
	{
		// Put a gamma code.
		// v must be larger than 0.
		int t = v;
		t >>= 1;
		int cnt = 0;
		while(t)
		{
			Put1Bit(0);
			t >>= 1;
			cnt ++;
		}
		Put1Bit(1);
		while(cnt--)
		{
			Put1Bit(v&1);
			v >>= 1;
		}
	}

	void PutInterleavedGamma(int v)
	{
		// Put a gamma code, interleaved.
		// interleaved gamma codes are:
		//     1 :                   1
		//   <=3 :                 1x0
		//   <=7 :               1x0x0
		//  <=15 :             1x0x0x0
		//  <=31 :           1x0x0x0x0
		// and so on.
		// v must be larger than 0.
		
		v --;
		while(v)
		{
			v >>= 1;
			Put1Bit(0);
			Put1Bit(v&1);
		}
		Put1Bit(1);
	}

	static int GetGammaBitLengthGeneric(int v)
	{
		int needbits = 1;
		v >>= 1;
		while(v)
		{
			needbits += 2;
			v >>= 1;
		}
		return needbits;
	}

	static int GetGammaBitLength(int v)
	{
		// Get bit length where v is to be encoded as a gamma code.
		if(v<=1) return 1;    //                   1
		if(v<=3) return 3;    //                 x10
		if(v<=7) return 5;    //               xx100
		if(v<=15) return 7;   //             xxx1000
		if(v<=31) return 9;   //          x xxx10000
		if(v<=63) return 11;  //        xxx xx100000
		if(v<=127) return 13; //      xxxxx x1000000
		if(v<=255) return 15; //    xxxxxxx 10000000
		if(v<=511) return 17; //   xxxxxxx1 00000000
		return GetGammaBitLengthGeneric(v);
	}

	void PutNonzeroSigned(int v, int len)
	{
		// Put signed value into the bit pool, as length of "len".
		// v must not be zero. abs(v) must be less than 257.
		if(v > 0) v--;
		while(len --)
		{
			Put1Bit(v&1);
			v >>= 1;
		}
	}

	static int GetNonzeroSignedBitLength(int v)
	{
		// Get bit (minimum) length where v is to be encoded as a non-zero signed value.
		// v must not be zero. abs(v) must be less than 257.
		if(v == 0) return 0;
		if(v < 0) v = -v;
		if(v <= 1) return 1;
		if(v <= 2) return 2;
		if(v <= 4) return 3;
		if(v <= 8) return 4;
		if(v <= 16) return 5;
		if(v <= 32) return 6;
		if(v <= 64) return 7;
		if(v <= 128) return 8;
		if(v <= 256) return 9;
		return 10;
	}

	void PutValue(long v, int len)
	{
		// put value "v" as length of "len"
		while(len --)
		{
			Put1Bit(v&1);
			v >>= 1;
		}
	}

};
#define MAX_COLOR_COMPONENTS 4

#define FILTER_TRY_COUNT 16

#define W_BLOCK_SIZE 8
#define H_BLOCK_SIZE 8

//------------------------------ FOR DEBUG
//#define FILTER_TEST
//#define WRITE_ENTROPY_VALUES
//#define WRITE_VSTXT
//------------------------------


/*
	shift-jis

	TLG6 ���k�A���S���Y��(�T�v)

	TLG6 �͋g���g���Q�ŗp������摜���k�t�H�[�}�b�g�ł��B�O���[�X�P�[���A
	24bit�r�b�g�}�b�v�A8bit�A���t�@�`�����l���t��24bit�r�b�g�}�b�v�ɑΉ���
	�Ă��܂��B�f�U�C���̃R���Z�v�g�́u�����k���ł��邱�Ɓv�u�摜�W�J�ɂ�����
	�\���ɍ����Ȏ������ł��邱�Ɓv�ł��B

	TLG6 �͈ȉ��̏����ŉ摜�����k���܂��B

	1.   MED �܂��� ���ϖ@ �ɂ��P�x�\��
	2.   ���I�[�_�����O�ɂ����ʍ팸
	3.   �F���փt�B���^�ɂ����ʍ팸
	4.   �S�����E���C�X�����ɂ��G���g���s�[������



	1.   MED �܂��� ���ϖ@ �ɂ��P�x�\��

		TLG6 �� MED (Median Edge Detector) ���邢�͕��ϖ@�ɂ��P�x�\�����s
		���A�\���l�Ǝ��ۂ̒l�̌덷���L�^���܂��B

		MED �́A�\���ΏۂƂ���s�N�Z���ʒu(���}x�ʒu)�ɑ΂��A���ׂ̃s�N�Z��
		�̋P�x��a�A��ׂ̃s�N�Z���̋P�x��b�A����̃s�N�Z���̋P�x��c�Ƃ��A�\
		���Ώۂ̃s�N�Z���̋P�x x ���ȉ��̂悤�ɗ\�����܂��B

			+-----+-----+
			|  c  |  b  |
			+-----+-----+
			|  a  |  x  |
			+-----+-----+

			x = min(a,b)    (c > max(a,b) �̏ꍇ)
			x = max(a,b)    (c < min(a,b) �̏ꍇ)
			x = a + b - c   (���̑��̏ꍇ)

		MED�́A�P���ȁA���͂̃s�N�Z���̋P�x�̕��ςɂ��\���ȂǂƔ�ׁA�摜
		�̏c�����̃G�b�W�ł͏�̃s�N�Z�����A�������̃G�b�W�ł͍��̃s�N�Z����
		�Q�Ƃ��ė\�����s���悤�ɂȂ邽�߁A�����̂悢�\�����\�ł��B

		���ϖ@�ł́Aa �� b �̋P�x�̕��ς���\���Ώۂ̃s�N�Z���̋P�x x ��\
		�����܂��B�ȉ��̎���p���܂��B

			x = (a + b + 1) >> 1

		MED �� ���ϖ@�́A8x8 �ɋ�؂�ꂽ�摜�u���b�N���ŗ����ɂ�鈳�k����
		��r���A���k���̍����������̕��@���I������܂��B

		R G B �摜�� A R G B �摜�ɑ΂��Ă͂����F�R���|�[�l���g���Ƃɍs��
		�܂��B

	2.   ���I�[�_�����O�ɂ����ʍ팸

		�덷�݂̂ƂȂ����f�[�^�́A���I�[�_�����O(���ёւ�)���s���܂��B
		TLG6�ł́A�܂��摜��8�~8�̏����ȃu���b�N�ɕ������܂��B�u���b�N�͉�
		�����ɁA���̃u���b�N���珇�ɉE�̃u���b�N�Ɍ������A��񕪂̃u���b�N
		�̏�������������΁A���̈�񉺂̃u���b�N�A�Ƃ��������ŏ������s����
		���B
		���̌X�̃u���b�N���ł́A�s�N�Z�����ȉ��̏��Ԃɕ��ёւ��܂��B

		���ʒu�������̃u���b�N      ���ʒu����̃u���b�N
		 1  2  3  4  5  6  7  8     57 58 59 60 61 62 63 64
		16 15 14 13 12 11 10  9     56 55 54 53 52 51 50 49
		17 18 19 20 21 22 23 24     41 42 43 44 45 46 47 48
		32 31 30 29 28 27 26 25     40 39 38 37 36 35 34 33
		33 34 35 36 37 38 39 40     25 26 27 28 29 30 31 32
		48 47 46 45 44 43 42 41     24 23 22 21 20 19 18 17
		49 50 51 52 53 54 55 56      9 10 11 12 13 14 15 16
		64 63 62 61 60 59 58 57      8  7  6  5  4  3  2  1

		���̂悤�ȁu�W�O�U�O�v��̕��ёւ����s�����Ƃɂ��A����(�P�x��F���A
		�G�b�W�Ȃ�)�̎����s�N�Z�����A������\���������Ȃ�A��i�̃G���g��
		�s�[�������i�ł̃[���E�����E�����O�X���k�̌��ʂ����߂邱�Ƃ��ł��܂��B

	3.   �F���փt�B���^�ɂ����ʍ팸

		R G B �摜 ( A R G B �摜�������� ) �ɂ����ẮA�F�R���|�[�l���g��
		�̑��֐��𗘗p���ď��ʂ����点��\��������܂��BTLG6�ł́A�ȉ���
		16��ނ̃t�B���^���u���b�N���ƂɓK�p���A��i�̃S�����E���C�X�����ɂ�
		�鈳�k��T�C�Y�����Z���A�����Ƃ��T�C�Y�̏������Ȃ�t�B���^��K�p����
		���B�t�B���^�ԍ��� MED/���ϖ@�̕ʂ� LZSS �@�ɂ�舳�k����A�o�͂���
		�܂��B

		0          B        G        R              (�t�B���^����)
		1          B-G      G        R-G
		2          B        G-B      R-B-G
		3          B-R-G    G-R      R
		4          B-R      G-B-R    R-B-R-G
		5          B-R      G-B-R    R
		6          B-G      G        R
		7          B        G-B      R
		8          B        G        R-G
		9          B-G-R-B  G-R-B    R-B
		10         B-R      G-R      R
		11         B        G-B      R-B
		12         B        G-R-B    R-B
		13         B-G      G-R-B-G  R-B-G
		14         B-G-R    G-R      R-B-G-R
		15         B        G-(B<<1) R-(B<<1)

		�����̃t�B���^�́A�����̉摜�Ńe�X�g�������A�t�B���^�̎g�p�p�x��
		�ׁA�����Ƃ��K�p�̑�������16�̃t�B���^��I�o���܂����B

	4.   �S�����E���C�X�����ɂ��G���g���s�[������

		�ŏI�I�Ȓl�̓S�����E���C�X�����ɂ��r�b�g�z��Ɋi�[����܂��B������
		�ŏI�I�Ȓl�ɂ͘A������[�������ɑ������߁A�[���Ɣ�[���̘A������
		�K���}������p���ĕ��������܂�(�[���E�����E�����O�X)�B���̂��߁A�r�b
		�g�z�񒆂ɂ͊�{�I��

		1 ��[���̘A����(�����E�����O�X)�̃K���}����
		2 ��[���̒l�̃S�����E���C�X���� (�A������ �J��Ԃ����)
		3 �[���̘A����(�����E�����O�X)�̃K���}����

		�������񌻂�邱�ƂɂȂ�܂��B

		�K���}�����͈ȉ��̌`���ŁA0��ۑ�����K�v���Ȃ�����(����0�͂��蓾��
		���̂�)�A�\���ł���Œ�̒l��1�ƂȂ��Ă��܂��B

		                  MSB ��    �� LSB
		(v=1)                            1
		(2<=v<=3)                      x10          x = v-2
		(4<=v<=7)                    xx100         xx = v-4
		(8<=v<=15)                 xxx1000        xxx = v-8
		(16<=v<=31)             x xxx10000       xxxx = v-16
		(32<=v<=63)           xxx xx100000      xxxxx = v-32
		(64<=v<=127)        xxxxx x1000000     xxxxxx = v-64
		(128<=v<=255)     xxxxxxx 10000000    xxxxxxx = v-128
		(256<=v<=511)    xxxxxxx1 00000000   xxxxxxxx = v-256
		      :                    :                  :
		      :                    :                  :

		�S�����E���C�X�����͈ȉ��̕��@�ŕ��������܂��B�܂��A���������悤�Ƃ�
		��l��e�Ƃ��܂��Be�ɂ�1�ȏ�̐��̐�����-1�ȉ��̕��̐��������݂��܂��B
		�����0�ȏ�̐����ɕϊ����܂��B��̓I�ɂ͈ȉ��̎��ŕϊ����s���܂��B
		(�ϊ���̒l��m�Ƃ���)

		m = ((e >= 0) ? 2*e : -2*e-1) - 1

		���̕ϊ��ɂ��A���̕\�̂悤�ɁA����e��������m�ɁA����e�����m�ɕ�
		������܂��B

		 e     m
		-1     0
		 1     1
		-2     2
		 2     3
		-3     4
		 3     5
		 :     :
		 :     :

		���̒l m ���ǂꂮ�炢�̃r�b�g���ŕ\���ł��邩��\�����܂��B���̃r�b
		�g���� k �Ƃ��܂��Bk �� 0 �ȏ�̐����ł��B�������\�����͂���Am �� k
		�r�b�g�ŕ\���ł��Ȃ��ꍇ������܂��B���̏ꍇ�́Ak �r�b�g�ŕ\���ł�
		�Ȃ��������� m�A�܂� k>>m ���̃[�����o�͂��A�Ō�ɂ��̏I�[�L����
		���� 1 ���o�͂��܂��B
		m �� k�r�b�g�ŕ\���ł����ꍇ�́A�����P�� 1 ���o�͂��܂��B���̂��ƁA
		m �����ʃr�b�g���� k �r�b�g���o�͂��܂��B

		���Ƃ��� e �� 66 �� k �� 5 �̏ꍇ�͈ȉ��̂悤�ɂȂ�܂��B

		m �� m = ((e >= 0) ? 2*e : -2*e-1) - 1 �̎��ɂ�� 131 �ƂȂ�܂��B
		�\�������r�b�g�� 5 �ŕ\���ł���l�̍ő�l�� 31 �ł��̂ŁA131 �ł���
		m �� k �r�b�g�ŕ\���ł��Ȃ��Ƃ������ƂɂȂ�܂��B�ł��̂ŁAm >> k
		�ł��� 4 �� 0 ���o�͂��܂��B

		0000

		���ɁA����4�� 0 �̏I�[�L���Ƃ��� 1 ���o�͂��܂��B

		10000

		�Ō�� m �����ʃr�b�g���� k �r�b�g���o�͂��܂��B131 ���Q�i���ŕ\��
		����� 10000011 �ƂȂ�܂����A����̉��ʂ��� 5 �r�b�g�����o�͂��܂��B


        MSB��  ��LSB
		00011 1 0000      (�v10�r�b�g)
		~~~~~ ~ ~~~~
		  c   b   a

		a   (m >> k) �� 0
		b   a �� �I�[�L��
		c   m �̉��� k �r�b�g��

		���ꂪ e=66 k=5 �𕄍��������r�b�g��ƂȂ�܂��B

		�����I�ɂ� k �̗\���́A���܂łɏo�͂��� e �̐�Βl�̍��v�Ƃ���܂ł�
		�o�͂����������ɁA���炩���ߎ��ۂ̉摜�̓��v����쐬���ꂽ�e�[�u��
		���Q�Ƃ��邱�ƂŎZ�o���܂��B����� C++ ���ꕗ�ɋL�q����ƈȉ��̂悤
		�ɂȂ�܂��B

		n = 3; // �J�E���^
		a = 0; // e �̐�Βl�̍��v

		while(e ��ǂݍ���)
		{
			k = table[a][n]; // �e�[�u������ k ���Q��
			m = ((e >= 0) ? 2*e : -2*e-1) - 1;
			e �� m �� k �ŕ�����;
			a += m >> 1; // e �̐�Βl�̍��v (������ m >>1 �ő�p)
			if(--n < 0) a >>= 1, n = 3;
		}

		4 �񂲂Ƃ� a ���������� n �����Z�b�g����邽�߁A�O�̒l�̉e���͕�����
		���i�ނɂ�Ĕ����Ȃ�A���߂̒l��������I�� k ��\�����邱�Ƃ��ł�
		�܂��B


	�Q�l :
		Golomb codes
		http://oku.edu.mie-u.ac.jp/~okumura/compression/golomb/
		�O�d��w���� �������F���ɂ��S���������̊ȒP�ȉ��
*/

//---------------------------------------------------------------------------
#ifdef WRITE_VSTXT
FILE *vstxt = fopen("vs.txt", "wt");
#endif
#define GOLOMB_GIVE_UP_BYTES 4
void CompressValuesGolomb(TLG6BitStream &bs, char *buf, int size)
{
	// golomb encoding, -- http://oku.edu.mie-u.ac.jp/~okumura/compression/golomb/

	// run-length golomb method
	bs.PutValue(buf[0]?1:0, 1); // initial value state

	int count;

	int n = TVP_TLG6_GOLOMB_N_COUNT - 1; // ���̃J�E���^
	int a = 0; // �\���덷�̐�Βl�̘a

	count = 0;
	for(int i = 0; i < size; i++)
	{
		if(buf[i])
		{
			// write zero count
			if(count) bs.PutGamma(count);

			// count non-zero values
			count = 0;
			int ii;
			for(ii = i; ii < size; ii++)
			{
				if(buf[ii]) count++; else break;
			}

			// write non-zero count
			bs.PutGamma(count);

			// write non-zero values
			for(; i < ii; i++)
			{
				int e = buf[i];
#ifdef WRITE_VSTXT
				fprintf(vstxt, "%d ", e);
#endif
				int k = TVPTLG6GolombBitLengthTable[a][n];
				int m = ((e >= 0) ? 2*e : -2*e-1) - 1;
				int store_limit = bs.GetBytePos() + GOLOMB_GIVE_UP_BYTES;
				bool put1 = true;
				for(int c = (m >> k); c > 0; c--)
				{
					if(store_limit == bs.GetBytePos())
					{
						bs.PutValue(m >> k, 8);
#ifdef WRITE_VSTXT
						fprintf(vstxt, "[ %d ] ", m>>k);
#endif
						put1 = false;
						break;
					}
					bs.Put1Bit(0);
				}
				if(store_limit == bs.GetBytePos())
				{
					bs.PutValue(m >> k, 8);
#ifdef WRITE_VSTXT
					fprintf(vstxt, "[ %d ] ", m>>k);
#endif
					put1 = false;
				}
				if(put1) bs.Put1Bit(1);
				bs.PutValue(m, k);
				a += (m>>1);
				if (--n < 0) {
					a >>= 1; n = TVP_TLG6_GOLOMB_N_COUNT - 1;
				}
			}

#ifdef WRITE_VSTXT
			fprintf(vstxt, "\n");
#endif

			i = ii - 1;
			count = 0;
		}
		else
		{
			// zero
			count ++;
		}
	}

	if(count) bs.PutGamma(count);
}
//---------------------------------------------------------------------------
class TryCompressGolomb
{
	int TotalBits; // total bit count
	int Count; // running count
	int N;
	int A;
	bool LastNonZero;

public:
	TryCompressGolomb()
	{
		Reset();
	}

	TryCompressGolomb(const TryCompressGolomb &ref)
	{
		Copy(ref);
	}

	~TryCompressGolomb() { ; }

	void Copy(const TryCompressGolomb &ref)
	{
		TotalBits = ref.TotalBits;
		Count = ref.Count;
		N = ref.N;
		A = ref.A;
		LastNonZero = ref.LastNonZero;
	}

	void Reset()
	{
		TotalBits = 1;
		Count = 0;
		N = TVP_TLG6_GOLOMB_N_COUNT - 1;
		A = 0;
		LastNonZero = false;
	}

	int Try(char *buf, int size)
	{
		for(int i = 0; i < size; i++)
		{
			if(buf[i])
			{
				// write zero count
				if(!LastNonZero)
				{
					if(Count)
						TotalBits +=
							TLG6BitStream::GetGammaBitLength(Count);

					// count non-zero values
					Count = 0;
				}

				// write non-zero values
				for(; i < size; i++)
				{
					int e = buf[i];
					if(!e) break;
					Count ++;
					int k = TVPTLG6GolombBitLengthTable[A][N];
					int m = ((e >= 0) ? 2*e : -2*e-1) - 1;
					int unexp_bits = (m>>k);
					if(unexp_bits >= (GOLOMB_GIVE_UP_BYTES*8-8/2))
						unexp_bits = (GOLOMB_GIVE_UP_BYTES*8-8/2)+8;
					TotalBits += unexp_bits + 1 + k;
					A += (m>>1);
					if (--N < 0) {
						A >>= 1; N = TVP_TLG6_GOLOMB_N_COUNT - 1;
					}
				}

				// write non-zero count

				i--;
				LastNonZero = true;
			}
			else
			{
				// zero
				if(LastNonZero)
				{
					if(Count)
					{
						TotalBits += TLG6BitStream::GetGammaBitLength(Count);
						Count = 0;
					}
				}

				Count ++;
				LastNonZero = false;
			}
		}
		return TotalBits;
	}

	int Flush()
	{
		if(Count)
		{
			TotalBits += TLG6BitStream::GetGammaBitLength(Count);
			Count = 0;
		}
		return TotalBits;
	}
};
//---------------------------------------------------------------------------
#define DO_FILTER \
	len -= 4; \
	for(d = 0; d < len; d+=4) \
	{ FILTER_FUNC(0); FILTER_FUNC(1); FILTER_FUNC(2); FILTER_FUNC(3); } \
	len += 4; \
	for(; d < len; d++) \
	{ FILTER_FUNC(0); }

void ApplyColorFilter(char * bufb, char * bufg, char * bufr, int len, int code)
{
	int d;
	unsigned char t;
	switch(code)
	{
	case 0:
		break;
	case 1:
		#define FILTER_FUNC(n) \
			bufr[d+n] -= bufg[d+n], \
			bufb[d+n] -= bufg[d+n];
		DO_FILTER
		break;
	case 2:
		for( d = 0; d < len; d++)
			bufr[d] -= bufg[d],
			bufg[d] -= bufb[d];
		break;
	case 3:
		for( d = 0; d < len; d++)
			bufb[d] -= bufg[d],
			bufg[d] -= bufr[d];
		break;
	case 4:
		for( d = 0; d < len; d++)
			bufr[d] -= bufg[d],
			bufg[d] -= bufb[d],
			bufb[d] -= bufr[d];
		break;
	case 5:
		for( d = 0; d < len; d++)
			bufg[d] -= bufb[d],
			bufb[d] -= bufr[d];
		break;
	case 6:
		#undef FILTER_FUNC
		#define FILTER_FUNC(n) \
			bufb[d+n] -= bufg[d+n];
		DO_FILTER
		break;
	case 7:
		#undef FILTER_FUNC
		#define FILTER_FUNC(n) \
			bufg[d+n] -= bufb[d+n];
		DO_FILTER
		break;
	case 8:
		#undef FILTER_FUNC
		#define FILTER_FUNC(n) \
			bufr[d+n] -= bufg[d+n];
		DO_FILTER
		break;
	case 9:
		for( d = 0; d < len; d++)
			bufb[d] -= bufg[d],
			bufg[d] -= bufr[d],
			bufr[d] -= bufb[d];
		break;
	case 10:
		#undef FILTER_FUNC
		#define FILTER_FUNC(n) \
			bufg[d+n] -= bufr[d+n], \
			bufb[d+n] -= bufr[d+n];
		DO_FILTER
		break;
	case 11:
		#undef FILTER_FUNC
		#define FILTER_FUNC(n) \
			bufr[d+n] -= bufb[d+n], \
			bufg[d+n] -= bufb[d+n];
		DO_FILTER
		break;
	case 12:
		for( d = 0; d < len; d++)
			bufg[d] -= bufr[d],
			bufr[d] -= bufb[d];
		break;
	case 13:
		for( d = 0; d < len; d++)
			bufg[d] -= bufr[d],
			bufr[d] -= bufb[d],
			bufb[d] -= bufg[d];
		break;
	case 14:
		for( d = 0; d < len; d++)
			bufr[d] -= bufb[d],
			bufb[d] -= bufg[d],
			bufg[d] -= bufr[d];
		break;
	case 15:
		#undef FILTER_FUNC
		#define FILTER_FUNC(n) \
			t = bufb[d+n]<<1; \
			bufr[d+n] -= t, \
			bufg[d+n] -= t;
		DO_FILTER
		break;
	case 16:
		#undef FILTER_FUNC
		#define FILTER_FUNC(n) \
			bufg[d+n] -= bufr[d+n];
		DO_FILTER
		break;
	case 17:
		for( d = 0; d < len; d++)
			bufr[d] -= bufb[d],
			bufb[d] -= bufg[d];
		break;
	case 18:
		for( d = 0; d < len; d++)
			bufr[d] -= bufb[d];
		break;
	case 19:
		for( d = 0; d < len; d++)
			bufb[d] -= bufr[d],
			bufr[d] -= bufg[d];
		break;
	case 20:
		for( d = 0; d < len; d++)
			bufb[d] -= bufr[d];
		break;
	case 21:
		for( d = 0; d < len; d++)
			bufb[d] -= bufg[d]>>1;
		break;
	case 22:
		#undef FILTER_FUNC
		#define FILTER_FUNC(n) \
			bufg[d+n] -= bufb[d+n]>>1;
		DO_FILTER
		break;
	case 23:
		for( d = 0; d < len; d++)
			bufg[d] -= bufb[d],
			bufb[d] -= bufr[d],
			bufr[d] -= bufg[d];
		break;
	case 24:
		for( d = 0; d < len; d++)
			bufb[d] -= bufr[d],
			bufr[d] -= bufg[d],
			bufg[d] -= bufb[d];
		break;
	case 25:
		#undef FILTER_FUNC
		#define FILTER_FUNC(n) \
			bufg[d+n] -= bufr[d+n]>>1;
		DO_FILTER
		break;
	case 26:
		#undef FILTER_FUNC
		#define FILTER_FUNC(n) \
			bufr[d+n] -= bufg[d+n]>>1;
		DO_FILTER
		break;
	case 27:
		#undef FILTER_FUNC
		#define FILTER_FUNC(n) \
			t = bufr[d+n]>>1; \
			bufg[d+n] -= t, \
			bufb[d+n] -= t;
		DO_FILTER
		break;
	case 28:
		for( d = 0; d < len; d++)
			bufr[d] -= bufb[d]>>1;
		break;
	case 29:
		#undef FILTER_FUNC
		#define FILTER_FUNC(n) \
			t = bufg[d+n]>>1; \
			bufr[d+n] -= t, \
			bufb[d+n] -= t;
		DO_FILTER
		break;
	case 30:
		#undef FILTER_FUNC
		#define FILTER_FUNC(n) \
			t = bufb[d+n]>>1; \
			bufr[d+n] -= t, \
			bufg[d+n] -= t;
		DO_FILTER
		break;
	case 31:
		for( d = 0; d < len; d++)
			bufb[d] -= bufr[d]>>1;
		break;
	case 32:
		for( d = 0; d < len; d++)
			bufr[d] -= bufb[d]<<1;
		break;
	case 33:
		for( d = 0; d < len; d++)
			bufb[d] -= bufg[d]<<1;
		break;
	case 34:
		#undef FILTER_FUNC
		#define FILTER_FUNC(n) \
			t = bufr[d+n]<<1; \
			bufg[d+n] -= t, \
			bufb[d+n] -= t;
		DO_FILTER
		break;
	case 35:
		for( d = 0; d < len; d++)
			bufg[d] -= bufb[d]<<1;
		break;
	case 36:
		for( d = 0; d < len; d++)
			bufr[d] -= bufg[d]<<1;
		break;
	case 37:
		for( d = 0; d < len; d++)
			bufr[d] -= bufg[d]<<1,
			bufb[d] -= bufg[d]<<1;
		break;
	case 38:
		for( d = 0; d < len; d++)
			bufg[d] -= bufr[d]<<1;
		break;
	case 39:
		for( d = 0; d < len; d++)
			bufb[d] -= bufr[d]<<1;
		break;

	}

}
//---------------------------------------------------------------------------
int DetectColorFilter(char *b, char *g, char *r, int size, int &outsize)
{
#ifndef FILTER_TEST
	int minbits = -1;
	int mincode = -1;

	char bbuf[H_BLOCK_SIZE*W_BLOCK_SIZE];
	char gbuf[H_BLOCK_SIZE*W_BLOCK_SIZE];
	char rbuf[H_BLOCK_SIZE*W_BLOCK_SIZE];
	TryCompressGolomb bc, gc, rc;

	for(int code = 0; code < FILTER_TRY_COUNT; code++)   // 17..27 are currently not used
	{
		// copy bbuf, gbuf, rbuf into b, g, r.
		memcpy(bbuf, b, sizeof(char)*size);
		memcpy(gbuf, g, sizeof(char)*size);
		memcpy(rbuf, r, sizeof(char)*size);

		// copy compressor
		bc.Reset();
		gc.Reset();
		rc.Reset();

		// Apply color filter
		ApplyColorFilter(bbuf, gbuf, rbuf, size, code);

		// try to compress
		int bits;
		bits  = (bc.Try(bbuf, size), bc.Flush());
		if(minbits != -1 && minbits < bits) continue;
		bits += (gc.Try(gbuf, size), gc.Flush());
		if(minbits != -1 && minbits < bits) continue;
		bits += (rc.Try(rbuf, size), rc.Flush());

		if(minbits == -1 || minbits > bits)
		{
			minbits = bits, mincode = code;
		}
	}

	outsize = minbits;

	return mincode;
#else
	static int filter = 0;

	int f = filter;

	filter++;
	if(filter >= FILTER_TRY_COUNT) filter = 0;

	outsize = 0;

	return f;
#endif
}
//---------------------------------------------------------------------------
static void WriteInt32(long num, tTJSBinaryStream *out)
{
	char buf[4];
	buf[0] = num & 0xff;
	buf[1] = (num >> 8) & 0xff;
	buf[2] = (num >> 16) & 0xff;
	buf[3] = (num >> 24) & 0xff;
	out->WriteBuffer(buf, 4);
}
//---------------------------------------------------------------------------
static void TLG6InitializeColorFilterCompressor(SlideCompressor &c)
{
	unsigned char code[4096];
	unsigned char dum[4096];
	unsigned char *p = code;
	for(int i = 0; i < 32; i++)
	{
		for(int j = 0; j < 16; j++)
		{
			p[0] = p[1] = p[2] = p[3] = i;
			p += 4;
			p[0] = p[1] = p[2] = p[3] = j;
			p += 4;
		}
	}

	long dumlen;
	c.Encode(code, 4096, dum, dumlen);
}
//---------------------------------------------------------------------------
// int ftfreq[256] = {0};
void SaveTLG6( tTJSBinaryStream* stream, const tTVPBaseBitmap* bmp, bool is24 )
{
	tTJSBinaryStream *out = stream;

	// DWORD medstart, medend;
	tjs_uint reordertick;
#ifdef WRITE_ENTROPY_VALUES
	FILE *vs = fopen("vs.bin", "wb");
#endif

	int colors;

	TVPTLG6InitGolombTable();

	// check pixel format
	if( bmp->Is32BPP() ) {
		if( is24 ) colors = 3;
		else colors = 4;
	} else {
		colors = 1;
	}
	int stride = colors;
	if( stride == 3 ) stride = 4;

	// output stream header
	{
		out->WriteBuffer("TLG6.0\x00raw\x1a\x00", 11);
		out->WriteBuffer(&colors, 1);
		int n = 0;
		out->WriteBuffer(&n, 1); // data flag (0)
		out->WriteBuffer(&n, 1); // color type (0)
		out->WriteBuffer(&n, 1); // external golomb table (0)
		int width = bmp->GetWidth();
		int height = bmp->GetHeight();
		WriteInt32(width, out);
		WriteInt32(height, out);
	}

	// compress
	long max_bit_length = 0;

	unsigned char *buf[MAX_COLOR_COMPONENTS];
	for(int i = 0; i < MAX_COLOR_COMPONENTS; i++) buf[i] = NULL;
	char *block_buf[MAX_COLOR_COMPONENTS];
	for(int i = 0; i < MAX_COLOR_COMPONENTS; i++) block_buf[i] = NULL;
	unsigned char *filtertypes = NULL;
	tTVPMemoryStream *memstream = NULL;

	try
	{
		memstream = new tTVPMemoryStream();

		TLG6BitStream bs(memstream);

//		int buf_size = bmp->Width * bmp->Height;

		// allocate buffer
		for(int c = 0; c < colors; c++)
		{
			buf[c] = new unsigned char [W_BLOCK_SIZE * H_BLOCK_SIZE * 3];
			block_buf[c] = new char [H_BLOCK_SIZE * bmp->GetWidth()];
		}
		int w_block_count = (int)((bmp->GetWidth() - 1) / W_BLOCK_SIZE) + 1;
		int h_block_count = (int)((bmp->GetHeight() - 1) / H_BLOCK_SIZE) + 1;
		filtertypes = new unsigned char [w_block_count * h_block_count];

		int fc = 0;
		for(int y = 0; y < (int)bmp->GetHeight(); y += H_BLOCK_SIZE)
		{
			int ylim = y + H_BLOCK_SIZE;
			if(ylim > (int)bmp->GetHeight()) ylim = bmp->GetHeight();
			int gwp = 0;
			int xp = 0;
			for(int x = 0; x < (int)bmp->GetWidth(); x += W_BLOCK_SIZE, xp++)
			{
				int xlim = x + W_BLOCK_SIZE;
				if(xlim > (int)bmp->GetWidth()) xlim = bmp->GetWidth();
				int bw = xlim - x;

				int p0size; // size of MED method (p=0)
				int minp = 0; // most efficient method (0:MED, 1:AVG)
				int ft; // filter type
				int wp; // write point
				for(int p = 0; p < 2; p++)
				{
					int dbofs = (p+1) * (H_BLOCK_SIZE * W_BLOCK_SIZE);

					// do med(when p=0) or take average of upper and left pixel(p=1)
					for(int c = 0; c < colors; c++)
					{
						int wp = 0;
						for(int yy = y; yy < ylim; yy++)
						{
							const unsigned char * sl = x*stride +
								c + (const unsigned char *)bmp->GetScanLine(yy);
							const unsigned char * usl;
							if(yy >= 1)
								usl = x*stride + c + (const unsigned char *)bmp->GetScanLine(yy-1);
							else
								usl = NULL;
							for(int xx = x; xx < xlim; xx++)
							{
								unsigned char pa = xx > 0 ? sl[-stride] : 0;
								unsigned char pb = usl ? *usl : 0;
								unsigned char px = *sl;

								unsigned char py;

//								py = 0;
								if(p == 0)
								{
									unsigned char pc = (xx > 0 && usl) ? usl[-stride] : 0;
									unsigned char min_a_b = pa>pb?pb:pa;
									unsigned char max_a_b = pa<pb?pb:pa;

									if(pc >= max_a_b)
										py = min_a_b;
									else if(pc < min_a_b)
										py = max_a_b;
									else
										py = pa + pb - pc;
								}
								else
								{
									py = (pa+pb+1)>>1;
								}
								
								buf[c][wp] = (unsigned char)(px - py);

								wp++;
								sl += stride;
								if(usl) usl += stride;
							}
						}
					}

					// reordering
					// Transfer the data into block_buf (block buffer).
					// Even lines are stored forward (left to right),
					// Odd lines are stored backward (right to left).

					wp = 0;
					DWORD reorderstart = GetTickCount();
					for(int yy = y; yy < ylim; yy++)
					{
						int ofs;
						if(!(xp&1))
							ofs = (yy - y)*bw;
						else
							ofs = (ylim - yy - 1) * bw;
						bool dir; // false for forward, true for backward
						if(!((ylim-y)&1))
						{
							// vertical line count per block is even
							dir = ((yy&1) ^ (xp&1)) ? true : false;
						}
						else
						{
							// otherwise;
							if(xp & 1)
							{
								dir = (yy&1);
							}
							else
							{
								dir = ((yy&1) ^ (xp&1)) ? true : false;
							}
						}

						if(!dir)
						{
							// forward
							for(int xx = 0; xx < bw; xx++)
							{
								for(int c = 0; c < colors; c++)
									buf[c][wp + dbofs] =
									buf[c][ofs + xx];
								wp++;
							}
						}
						else
						{
							// backward
							for(int xx = bw - 1; xx >= 0; xx--)
							{
								for(int c = 0; c < colors; c++)
									buf[c][wp + dbofs] =
									buf[c][ofs + xx];
								wp++;
							}
						}
					}
					reordertick += GetTickCount() - reorderstart;
				}


				for(int p = 0; p < 2; p++)
				{
					int dbofs = (p+1) * (H_BLOCK_SIZE * W_BLOCK_SIZE);
					// detect color filter
					int size = 0;
					int ft_;
					if(colors >= 3)
						ft_ = DetectColorFilter(
							reinterpret_cast<char*>(buf[0] + dbofs),
							reinterpret_cast<char*>(buf[1] + dbofs),
							reinterpret_cast<char*>(buf[2] + dbofs), wp, size);
					else
						ft_ = 0;

					// select efficient mode of p (MED or average)
					if(p == 0)
					{
						p0size = size;
						ft = ft_;
					}
					else
					{
						if(p0size >= size)
							minp = 1, ft = ft_;
					}
				}

				// Apply most efficient color filter / prediction method
				wp = 0;
				int dbofs = (minp + 1)  * (H_BLOCK_SIZE * W_BLOCK_SIZE);
				for(int yy = y; yy < ylim; yy++)
				{
					for(int xx = 0; xx < bw; xx++)
					{
						for(int c = 0; c < colors; c++)
							block_buf[c][gwp + wp] = buf[c][wp + dbofs];
						wp++;
					}
				}

				ApplyColorFilter(block_buf[0] + gwp,
					block_buf[1] + gwp, block_buf[2] + gwp, wp, ft);

				filtertypes[fc++] = (ft<<1) + minp;
//				ftfreq[ft]++;
				gwp += wp;
			}

			// compress values (entropy coding)
			for(int c = 0; c < colors; c++)
			{
				int method;
				CompressValuesGolomb(bs, block_buf[c], gwp);
				method = 0;
#ifdef WRITE_ENTROPY_VALUES
				fwrite(block_buf[c], 1, gwp, vs);
#endif
				long bitlength = bs.GetBitLength();
				if(bitlength & 0xc0000000)
					TVPThrowExceptionMessage( TVPTlgTooLargeBitLength );
				// two most significant bits of bitlength are
				// entropy coding method;
				// 00 means Golomb method,
				// 01 means Gamma method (implemented but not used),
				// 10 means modified LZSS method (not yet implemented),
				// 11 means raw (uncompressed) data (not yet implemented).
				if(max_bit_length < bitlength) max_bit_length = bitlength;
				bitlength |= (method << 30);
				WriteInt32(bitlength, memstream);
				bs.Flush();
			}

		}


		// write max bit length
		WriteInt32(max_bit_length, out);

		// output filter types
		{
			SlideCompressor comp;
			TLG6InitializeColorFilterCompressor(comp);
			unsigned char *outbuf = new unsigned char[fc * 2];
			try
			{
				long outlen;
				comp.Encode(filtertypes, fc, outbuf, outlen);
				WriteInt32(outlen, out);
				out->WriteBuffer(outbuf, outlen);
			}
			catch(...)
			{
				delete [] outbuf;
				throw;
			}
			delete [] outbuf;

		}

		// copy memory stream to output stream
		out->WriteBuffer( memstream->GetInternalBuffer(), (tjs_uint)memstream->GetSize() );
	}
	catch(...)
	{
		for(int i = 0; i < MAX_COLOR_COMPONENTS; i++)
		{
			if(buf[i]) delete [] (buf[i]);
			if(block_buf[i]) delete [] (block_buf[i]);
		}
		if(filtertypes) delete [] filtertypes;
		if(memstream) delete memstream;
		throw;
	}

	for(int i = 0; i < MAX_COLOR_COMPONENTS; i++)
	{
		if(buf[i]) delete [] (buf[i]);
		if(block_buf) delete [] (block_buf[i]);
	}
	if(filtertypes) delete [] filtertypes;
	if(memstream) delete memstream;

#ifdef WRITE_ENTROPY_VALUES
	fclose(vs);
#endif
}
//---------------------------------------------------------------------------
extern void SaveTLG5( tTJSBinaryStream* stream, const tTVPBaseBitmap* image, bool is24 );
//---------------------------------------------------------------------------
void TVPSaveAsTLG( const ttstr & storagename, const ttstr & mode, const tTVPBaseBitmap* image, const std::vector<std::string>& tags ) {
	bool istls6 = false;
	if( mode.StartsWith( TJS_W("tlg5") ) ) {
		istls6 = false;
	} else if( mode.StartsWith( TJS_W("tlg6") ) ) {
		istls6 = true;
	} else {
		TVPThrowExceptionMessage(TVPInvalidImageSaveType, mode);
	}
	
	tjs_uint height = image->GetHeight();
	tjs_uint width = image->GetWidth();
	if( height == 0 || width == 0 ) TVPThrowInternalError;

	// open stream
	tTJSBinaryStream *stream = TVPCreateStream(TVPNormalizeStorageName(storagename), TJS_BS_WRITE);
	try {
		if( tags.size() ) {
			// write TLG0.0 Structured Data Stream header
			stream->WriteBuffer("TLG0.0\x00sds\x1a\x00", 11);
			tjs_uint rawlenpos = (tjs_uint)stream->GetPosition();
			stream->WriteBuffer("0000", 4);

			// write raw TLG stream
			if( istls6 ) {
				SaveTLG6( stream, image, mode == TJS_W("tlg624") );
			} else {
				SaveTLG5( stream, image, mode == TJS_W("tlg524") );
			}

			// write raw data size
			tjs_uint pos_save = (tjs_uint)stream->GetPosition();
			stream->SetPosition( rawlenpos );
			tjs_uint size = pos_save - rawlenpos - 4;
			unsigned char bin[4]; // buffer for writing 32bit integer as little endian format
			bin[0] = size & 0xff;
			bin[1] = (size >> 8) & 0xff;
			bin[2] = (size >> 16) & 0xff;
			bin[3] = (size >> 24) & 0xff;
			stream->WriteBuffer(bin, 4);
			stream->SetPosition( pos_save );

			// write "tags" chunk name
			stream->WriteBuffer("tags", 4);

			// build tag data
			std::stringstream tag;
			for( tjs_uint i = 0; i < (tags.size()-1); i+=2 ) {
				std::string name = tags[i];
				if( name.empty() ) continue;
				std::string value = tags[i+1];
				tag << name.length() << ":" << name << "=" << value.length() << ":" << value << ",";
			}

			// write chunk size
			std::string tagstr = tag.str();
			size = tagstr.length();
			bin[0] = size & 0xff;
			bin[1] = (size >> 8) & 0xff;
			bin[2] = (size >> 16) & 0xff;
			bin[3] = (size >> 24) & 0xff;
			stream->WriteBuffer(bin, 4);

			// write chunk data
			stream->WriteBuffer( tagstr.c_str(), tagstr.length() );
		} else {
			if( istls6 ) {
				SaveTLG6( stream, image, mode == TJS_W("tlg624") );
			} else {
				SaveTLG5( stream, image, mode == TJS_W("tlg524") );
			}
		}
	} catch(...) {
		delete stream;
		throw;
	}
	delete stream;
}