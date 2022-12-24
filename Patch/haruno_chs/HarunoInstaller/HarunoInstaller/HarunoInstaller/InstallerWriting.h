#pragma once
#include "PathSaver.h"
#include "CCImage.h"
#include "Package.h"
#include "afxcmn.h"
#include <cmath>
#include "InstallerFinal.h"

// InstallerWriting 对话框

class InstallerWriting : public CDialogEx
{
	DECLARE_DYNAMIC(InstallerWriting)

public:
	InstallerWriting(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~InstallerWriting();

// 对话框数据
	enum { IDD = IDD_HARUNOINSTALLER_DIALOG4 };

	Package* pack;

protected:

	HICON m_hIcon;
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnCustomdrawProgress1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	CProgressCtrl m_ctrlProgress;
	afx_msg void OnTimer(UINT_PTR nIDEvent);

	CCImage *bg;
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
};
