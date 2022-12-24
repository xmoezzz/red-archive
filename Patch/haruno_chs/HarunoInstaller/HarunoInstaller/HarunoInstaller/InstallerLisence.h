#pragma once
#include "InstallerWriting.h"
#include "CCImage.h"

// InstallerLisence �Ի���

class InstallerLisence : public CDialogEx
{
	DECLARE_DYNAMIC(InstallerLisence)

public:
	InstallerLisence(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~InstallerLisence();

// �Ի�������
	enum { IDD = IDD_HARUNOINSTALLER_DIALOG2 };

protected:

	bool bOK;
	HICON m_hIcon;
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnPaint();

	CCImage* bg;
	afx_msg void OnMove(int x, int y);
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnBnClickedRadio1();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
};
