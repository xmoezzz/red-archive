// ���� ifdef ���Ǵ���ʹ�� DLL �������򵥵�
// ��ı�׼�������� DLL �е������ļ��������������϶���� XMOE_HARUNO_EXPORTS
// ���ű���ġ���ʹ�ô� DLL ��
// �κ�������Ŀ�ϲ�Ӧ����˷��š�������Դ�ļ��а������ļ����κ�������Ŀ���Ὣ
// XMOE_HARUNO_API ������Ϊ�Ǵ� DLL ����ģ����� DLL ���ô˺궨���
// ������Ϊ�Ǳ������ġ�
#ifdef XMOE_HARUNO_EXPORTS
#define XMOE_HARUNO_API __declspec(dllexport)
#else
#define XMOE_HARUNO_API __declspec(dllimport)
#endif

// �����Ǵ� xmoe_haruno.dll ������
class XMOE_HARUNO_API Cxmoe_haruno {
public:
	Cxmoe_haruno(void);
	// TODO:  �ڴ�������ķ�����
};

extern XMOE_HARUNO_API int nxmoe_haruno;

XMOE_HARUNO_API int fnxmoe_haruno(void);
