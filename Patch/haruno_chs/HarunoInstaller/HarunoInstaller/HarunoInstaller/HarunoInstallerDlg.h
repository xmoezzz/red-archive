
// HarunoInstallerDlg.h : ͷ�ļ�
//

#pragma once
#include "CCImage.h"

// CHarunoInstallerDlg �Ի���
class CHarunoInstallerDlg : public CDialogEx
{
// ����
public:
	CHarunoInstallerDlg(CWnd* pParent = NULL);	// ��׼���캯��
	~CHarunoInstallerDlg();

// �Ի�������
	enum { IDD = IDD_HARUNOINSTALLER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();

	void FreeMem();
	void UpdataAnimation();
	void UpdataAnimation2();
private:

	bool bOK;
	int alpha;
	RECT imageRect;
	RECT backRect;
	CCImage *image;
	CCImage *Back;
	CCImage *Logo;
public:
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
//	afx_msg void OnInitMenu(CMenu* pMenu);
	afx_msg void OnDestroy();
	afx_msg void OnMove(int x, int y);
};
