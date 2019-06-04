#pragma once

#include "ViewTree.h"
#include "afxcmn.h"
#include "PT.h"

// CProcWndDlg �Ի���
class CProcWndDlg : public CDialog
{
	DECLARE_DYNAMIC(CProcWndDlg)

public:
	CProcWndDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CProcWndDlg();

// �Ի�������
	enum { IDD = IDD_PROC_WND_DLG };

	void AdjustLayout();
	virtual BOOL OnInitDialog();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
	virtual void OnOK();
	virtual void OnCancel();
	DECLARE_MESSAGE_MAP()
public:
	CViewTree m_Proc_Wnd_Tree;
	ProcessList m_ProcessList;
};
