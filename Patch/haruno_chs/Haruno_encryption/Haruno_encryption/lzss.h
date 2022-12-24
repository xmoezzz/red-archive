#ifndef _LZSS_
#define _LZSS_

#include <stdio.h>
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
	FILE *fpIn, *fpOut;                                     //��������ļ�ָ��
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
	unsigned long Compress(unsigned char *, unsigned long,
		FILE *);                        //���ڴ��е�����ѹ����д���ļ�
	unsigned long Compress(FILE *, unsigned long, FILE *);   //ѹ���ļ�
	unsigned long UnCompress(unsigned char *, unsigned long,
		unsigned char *);             //�ڴ��е����ݽ�ѹ
	unsigned long UnCompress(FILE *, unsigned long,
		unsigned char *);             //���ļ��е����ݽ�ѹ��д���ڴ�
	unsigned long UnCompress(FILE *, unsigned long, FILE *); //��ѹ�ļ�
};

#endif
