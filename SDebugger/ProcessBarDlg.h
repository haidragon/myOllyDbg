#pragma once
#include "afxcmn.h"


// CProcessBarDlg 对话框

class CProcessBarDlg : public CDialog
{
	DECLARE_DYNAMIC(CProcessBarDlg)

public:
	CProcessBarDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CProcessBarDlg();

// 对话框数据
	enum { IDD = IDD_PROCESS_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CProgressCtrl m_ProcCtrl;
	int m_value;
	WCHAR m_szTitle[MAX_PATH];

	virtual BOOL OnInitDialog();

	void SetValue(int value);

	void SetText(WCHAR* szTitle);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};
