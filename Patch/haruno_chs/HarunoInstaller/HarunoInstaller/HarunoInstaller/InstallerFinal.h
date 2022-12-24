#pragma once

#include "CCImage.h"

// InstallerFinal 对话框

class InstallerFinal : public CDialogEx
{
	DECLARE_DYNAMIC(InstallerFinal)

public:
	InstallerFinal(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~InstallerFinal();

// 对话框数据
	enum { IDD = IDD_HARUNOINSTALLER_DIALOG5 };

	CCImage* bg;

protected:
	HICON m_hIcon;

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
};
