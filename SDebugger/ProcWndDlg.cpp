// PROCWNDDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "SDebugger.h"
#include "ProcWndDlg.h"
#include "Message.h"
#include "DebugCore.h"

// CProcWndDlg 对话框

IMPLEMENT_DYNAMIC(CProcWndDlg, CDialog)

CProcWndDlg::CProcWndDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CProcWndDlg::IDD, pParent)
{

}

CProcWndDlg::~CProcWndDlg()
{
}

void CProcWndDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROC_WND_TREE, m_Proc_Wnd_Tree);
}


BEGIN_MESSAGE_MAP(CProcWndDlg, CDialog)
END_MESSAGE_MAP()


BOOL CProcWndDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	m_ProcessList.clear();
	ProcessList::iterator pos;
	if(GetProcessList(m_ProcessList))
	{
		size_t size = m_ProcessList.size();
		for(size_t i=0;i<size;i++)
		{
			CString lab;
			PROCESSENTRY32 pe = m_ProcessList[i];
			lab.Format(L"ProcessID %d : %ws",pe.th32ProcessID,pe.szExeFile);
			TVINSERTSTRUCT item;
			item.hParent = NULL;
			item.hInsertAfter = NULL;
			item.item.mask = TVIF_TEXT|TVIF_PARAM;
			item.item.pszText = lab.GetBuffer();
			item.item.lParam = (LPARAM)i;
			m_Proc_Wnd_Tree.InsertItem(&item);
		}
	}
	return TRUE;
}

void CProcWndDlg::OnOK()
{
	HTREEITEM hTreeItem = m_Proc_Wnd_Tree.GetSelectedItem();
	if(hTreeItem)
	{
		size_t index = (size_t)m_Proc_Wnd_Tree.GetItemData(hTreeItem);
		LPPROCESSENTRY32 pe = (LPPROCESSENTRY32)&m_ProcessList[index];
		BeginBebugThread(pe);
	}
	CDialog::OnOK();
}

void CProcWndDlg::OnCancel()
{
	CDialog::OnCancel();
}

// CProcWndDlg 消息处理程序
