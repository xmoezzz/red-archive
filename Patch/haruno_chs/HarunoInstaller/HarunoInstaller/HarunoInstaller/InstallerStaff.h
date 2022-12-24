#pragma once
#include "CCImage.h"
#include "InstallerLisence.h"

// InstallerStaff 对话框

class InstallerStaff : public CDialogEx
{
	DECLARE_DYNAMIC(InstallerStaff)

public:
	InstallerStaff(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~InstallerStaff();

// 对话框数据
	enum { IDD = IDD_HARUNOINSTALLER_DIALOG1 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	HICON m_hIcon;
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedOk();
	afx_msg void OnDestroy();
	afx_msg void OnInitMenu(CMenu* pMenu);
	afx_msg void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);
	afx_msg void OnPaint();
	afx_msg void OnEnChangeEdit1();
	CString mText;
	CCImage* bg;
	afx_msg void OnMove(int x, int y);
	afx_msg void OnMoving(UINT fwSide, LPRECT pRect);
//	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnNcPaint();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg HCURSOR OnQueryDragIcon();
};
