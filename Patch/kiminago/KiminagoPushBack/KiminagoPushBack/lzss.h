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

	unsigned char InType, OutType;                          //输入输出数据类型,指明是文件还是内存中的数据
	unsigned char *InData, *OutData;                        //输入输出数据指针
	unsigned long InDataSize;                              //输入数据长度
	unsigned long InSize, OutSize;                          //已输入输出数据长度
	int GetByte();                                         //获取一个字节的数据
	void PutByte(unsigned char);                           //写入一个字节的数据

	void InitTree();						//初始化串表
	void InsertNode(int);					//插入一个表项
	void DeleteNode(int);					//删除一个表项
	void Encode();                                         //压缩数据
	void Decode();                                         //解压数据
public:
	LZSS();                                                //本类构造函数
	~LZSS();                                               //本类析构函数
	unsigned long Compress(unsigned char *, unsigned long,
		unsigned char *);               //内存中的数据压缩
	unsigned long UnCompress(unsigned char *, unsigned long,
		unsigned char *);             //内存中的数据解压
};

#endif
