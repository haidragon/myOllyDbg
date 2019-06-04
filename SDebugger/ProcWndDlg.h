#pragma once

#include "ViewTree.h"
#include "afxcmn.h"
#include "PT.h"

// CProcWndDlg 对话框
class CProcWndDlg : public CDialog
{
	DECLARE_DYNAMIC(CProcWndDlg)

public:
	CProcWndDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CProcWndDlg();

// 对话框数据
	enum { IDD = IDD_PROC_WND_DLG };

	void AdjustLayout();
	virtual BOOL OnInitDialog();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual void OnOK();
	virtual void OnCancel();
	DECLARE_MESSAGE_MAP()
public:
	CViewTree m_Proc_Wnd_Tree;
	ProcessList m_ProcessList;
};
