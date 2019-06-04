// ProcessBarDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "SDebugger.h"
#include "ProcessBarDlg.h"
#include "Message.h"


// CProcessBarDlg 对话框

IMPLEMENT_DYNAMIC(CProcessBarDlg, CDialog)

CProcessBarDlg::CProcessBarDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CProcessBarDlg::IDD, pParent)
{
	wcscpy(m_szTitle,L"执行进度...");
	m_value = 0;
}

CProcessBarDlg::~CProcessBarDlg()
{
}

void CProcessBarDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROGRESS_BAR, m_ProcCtrl);
}


BEGIN_MESSAGE_MAP(CProcessBarDlg, CDialog)
	ON_WM_TIMER()
END_MESSAGE_MAP()

BOOL CProcessBarDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	m_ProcCtrl.SetRange(0,100);
	SetWindowText(m_szTitle);
	SetTimer(1,100,NULL);
	return TRUE;
}


void CProcessBarDlg::SetValue(int value)
{
	m_value = value;
	if(m_ProcCtrl.GetSafeHwnd())
	{
		m_ProcCtrl.SetPos(value);
	}
}

void CProcessBarDlg::SetText(WCHAR* szTitle)
{
	if(szTitle)
		wcscpy(m_szTitle,szTitle);
}


// CProcessBarDlg 消息处理程序

void CProcessBarDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	CDialog::OnTimer(nIDEvent);
	m_ProcCtrl.SetPos(m_value);
	if(m_value>=100)
	{
		KillTimer(1);
		CDialog::OnCancel();
	}
}
