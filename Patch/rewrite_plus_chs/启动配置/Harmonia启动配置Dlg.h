
// Harmonia��������Dlg.h : ͷ�ļ�
//

#pragma once

#include <vector>
#include <string>
#include "afxwin.h"

// CHarmonia��������Dlg �Ի���
class CHarmonia��������Dlg : public CDialogEx
{
// ����
public:
	CHarmonia��������Dlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_HARMONIA_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��

public:
	std::vector<std::wstring> FontList;

// ʵ��
protected:
	HICON m_hIcon;


	// ���ɵ���Ϣӳ�亯��
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
