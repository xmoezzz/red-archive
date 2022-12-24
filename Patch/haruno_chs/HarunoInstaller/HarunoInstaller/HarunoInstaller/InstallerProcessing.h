#pragma once

#include "CCImage.h"
#include "PathSaver.h"
// InstallerProcessing �Ի���

class InstallerProcessing : public CDialogEx
{
	DECLARE_DYNAMIC(InstallerProcessing)

public:
	InstallerProcessing(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~InstallerProcessing();

// �Ի�������
	enum { IDD = IDD_HARUNOINSTALLER_DIALOG3 };

	CString lpDic;
	CCImage* bg;

	bool isInvaild;
protected:

	HICON m_hIcon;
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedOk();
	afx_msg void OnPaintClipboard(CWnd* pClipAppWnd, HGLOBAL hPaintStruct);
	afx_msg void OnPaint();
	afx_msg void OnBnClickedCancel();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
};
