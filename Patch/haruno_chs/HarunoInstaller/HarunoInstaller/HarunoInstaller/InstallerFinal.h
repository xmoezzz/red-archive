#pragma once

#include "CCImage.h"

// InstallerFinal �Ի���

class InstallerFinal : public CDialogEx
{
	DECLARE_DYNAMIC(InstallerFinal)

public:
	InstallerFinal(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~InstallerFinal();

// �Ի�������
	enum { IDD = IDD_HARUNOINSTALLER_DIALOG5 };

	CCImage* bg;

protected:
	HICON m_hIcon;

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
};
