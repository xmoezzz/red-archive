#ifndef _LZSS_
#define _LZSS_

class LZSS
{
	enum LZSSDATA
	{
		inMEM = 1, inFILE
	};
	unsigned char *buffer;
	int mpos, mlen;
	int *lson, *rson, *dad;

	unsigned char InType, OutType;                          //���������������,ָ�����ļ������ڴ��е�����
	unsigned char *InData, *OutData;                        //�����������ָ��
	unsigned long InDataSize;                              //�������ݳ���
	unsigned long InSize, OutSize;                          //������������ݳ���
	int GetByte();                                         //��ȡһ���ֽڵ�����
	void PutByte(unsigned char);                           //д��һ���ֽڵ�����

	void InitTree();						//��ʼ������
	void InsertNode(int);					//����һ������
	void DeleteNode(int);					//ɾ��һ������
	void Encode();                                         //ѹ������
	void Decode();                                         //��ѹ����
public:
	LZSS();                                                //���๹�캯��
	~LZSS();                                               //������������
	unsigned long Compress(unsigned char *, unsigned long,
		unsigned char *);               //�ڴ��е�����ѹ��
	unsigned long UnCompress(unsigned char *, unsigned long,
		unsigned char *);             //�ڴ��е����ݽ�ѹ
};

#endif
