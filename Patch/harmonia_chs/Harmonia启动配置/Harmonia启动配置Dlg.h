
// Harmonia启动配置Dlg.h : 头文件
//

#pragma once

#include <vector>
#include <string>
#include "afxwin.h"

// CHarmonia启动配置Dlg 对话框
class CHarmonia启动配置Dlg : public CDialogEx
{
// 构造
public:
	CHarmonia启动配置Dlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_HARMONIA_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

public:
	std::vector<std::wstring> FontList;

// 实现
protected:
	HICON m_hIcon;


	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedChsButton();
	afx_msg void OnBnClickedOriButton();
	afx_msg void OnBnClickedFontButton();
	CListBox m_ListBox;
};
