#include "StdAfx.h"
#include "Resource.h"
#include "ExecuteModuleWnd.h"
#include "Message.h"
#include "GlobalApi.h"

BEGIN_MESSAGE_MAP(CExecuteModuleWnd, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_MESSAGE(WM_UPDATEEXECUTEMODULEVIEW,OnUpdateExecuteModuleView)
END_MESSAGE_MAP()

CExecuteModuleWnd::CExecuteModuleWnd(void)
{

}

CExecuteModuleWnd::~CExecuteModuleWnd(void)
{
}

void CExecuteModuleWnd::DoDataExchange(CDataExchange* pDX)
{
	CDockablePane::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EXECUTE_MODULE_TABLE, m_cExeModuleList);
}

int CExecuteModuleWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	m_Font.CreateStockObject(DEFAULT_GUI_FONT);

	CRect rectDummy;
	rectDummy.SetRectEmpty();

	GetClientRect(&rectDummy);

	const DWORD dwViewStyle = WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL | LVS_EX_GRIDLINES;
	if(!m_cExeModuleList.Create(dwViewStyle,rectDummy,this,IDC_INVOKE_STACK_TABLE))
	{
		TRACE("创建调用函数堆栈表控件失败");
		return -1;
	}
	DWORD ExStyle = m_cExeModuleList.GetExtendedStyle();
	m_cExeModuleList.SetExtendedStyle(ExStyle | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);//设置属性整行选择 | 显示网格
	m_cExeModuleList.InsertColumn(0,_T("基址"),LVCFMT_IMAGE|LVCFMT_LEFT); //添加第0列,可以制定允许显示图标等属性
	m_cExeModuleList.InsertColumn(1,_T("大小"));
	m_cExeModuleList.InsertColumn(2,_T("入口"));
	m_cExeModuleList.InsertColumn(3,_T("名称"));
	m_cExeModuleList.InsertColumn(4,_T("路径"));
	m_cExeModuleList.SetColumnWidth(0, 100);
	m_cExeModuleList.SetColumnWidth(1, 100);
	m_cExeModuleList.SetColumnWidth(2, 100);
	m_cExeModuleList.SetColumnWidth(3, 100);
	m_cExeModuleList.SetColumnWidth(4, 500);

	InitExecuteModuleWnd(m_hWnd);
	return 0;
}

void CExecuteModuleWnd::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);
	m_cExeModuleList.SetWindowPos(NULL, -1, -1, cx, cy, SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);
	// 选项卡控件应覆盖整个工作区:
}

void WINAPI CExecuteModuleWnd::UpdateProcess(PVOID param)
{
	CTask *pTask = (CTask*)param;
	CSDebuggerDoc *pDoc = (CSDebuggerDoc*)pTask->param2;
	CExecuteModuleWnd* view = (CExecuteModuleWnd*)pTask->param1;
	if(pDoc)
	{
		int process = 0;
		process+=10;
		pTask->SetProcessValue(process);
		view->FillModuleView(pDoc->GetModuleMap(),pTask);
		process+=10;
		pTask->SetProcessValue(process);
	}
	pTask->SetProcessValue(100);
}

LRESULT CExecuteModuleWnd::OnUpdateExecuteModuleView(WPARAM wparam,LPARAM lparam)
{
	CSDebuggerDoc * pDoc = GetActiveDocument();
	CTask task;
	task.param1 = this;
	task.param2 = GetActiveDocument();
	ShowProcessBarDlg((PVOID)UpdateProcess,task,L"查询模块");
	return TRUE;
}

void CExecuteModuleWnd::FillModuleView(ModuleMap& modulemap,CTask* pTask)
{
	m_cExeModuleList.DeleteAllItems();
	int process = pTask->value;
	int num = 0;
	int size = modulemap.size();
	ModuleMap::iterator pos;
	for(pos=modulemap.begin();pos!=modulemap.end();pos++)
	{
		pTask->SetProcessValue(process+num*80/size);
		CString tmp;
		wstring name = (*pos).first;
		MODULE_INFORMATION mi = (*pos).second;
		tmp.Format(L"%p",(UINT_PTR)mi.BaseAddress);
		m_cExeModuleList.InsertItem(num,tmp);
		tmp.Format(L"%d",mi.SizeOfImage);
		m_cExeModuleList.SetItemText(num,1,tmp);
		tmp.Format(L"%p",(UINT_PTR)mi.EntryAddress);
		m_cExeModuleList.SetItemText(num,2,tmp);
		m_cExeModuleList.SetItemText(num,3,name.c_str());
		m_cExeModuleList.SetItemText(num,4,mi.szPathName);
		num++;
	}
}