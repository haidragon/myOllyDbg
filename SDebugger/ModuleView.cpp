// ModuleView.cpp : 实现文件
//

#include "stdafx.h"
#include "SDebugger.h"
#include "ModuleView.h"
#include "Message.h"
#include "Psapi.h"
#include "GlobalApi.h"

class CModuleViewMenuButton : public CMFCToolBarMenuButton
{
	friend class CModuleView;

	DECLARE_SERIAL(CModuleViewMenuButton)

public:
	CModuleViewMenuButton(HMENU hMenu = NULL) : CMFCToolBarMenuButton((UINT)-1, hMenu, -1)
	{
	}

	virtual void OnDraw(CDC* pDC, const CRect& rect, CMFCToolBarImages* pImages, BOOL bHorz = TRUE,
		BOOL bCustomizeMode = FALSE, BOOL bHighlight = FALSE, BOOL bDrawBorder = TRUE, BOOL bGrayDisabledButtons = TRUE)
	{
		pImages = CMFCToolBar::GetImages();

		CAfxDrawState ds;
		pImages->PrepareDrawImage(ds);

		CMFCToolBarMenuButton::OnDraw(pDC, rect, pImages, bHorz, bCustomizeMode, bHighlight, bDrawBorder, bGrayDisabledButtons);

		pImages->EndDrawImage(ds);
	}
};

IMPLEMENT_SERIAL(CModuleViewMenuButton, CMFCToolBarMenuButton, 1)

// CModuleTree

CModuleTree::CModuleTree()
{
}

CModuleTree::~CModuleTree()
{
}

BEGIN_MESSAGE_MAP(CModuleTree, CViewTree)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CModuleTree 消息处理程序

BOOL CModuleTree::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	BOOL bRes = CViewTree::OnNotify(wParam, lParam, pResult);

	NMHDR* pNMHDR = (NMHDR*)lParam;
	ASSERT(pNMHDR != NULL);
	if(TVN_GETDISPINFOW == pNMHDR->code)
	{
		PrintSelfStrA("%d \r\n",pNMHDR->idFrom);
	}

	return bRes;
}


// CModuleView

IMPLEMENT_DYNAMIC(CModuleView, CDockablePane)

CModuleView::CModuleView()
{
	InitDevice2Path();
	m_nCurrSort = ID_SORTING_GROUPBYTYPE;
}

CModuleView::~CModuleView()
{

}


BEGIN_MESSAGE_MAP(CModuleView, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_CLASS_ADD_MEMBER_FUNCTION, OnClassAddMemberFunction)
	ON_COMMAND(ID_CLASS_ADD_MEMBER_VARIABLE, OnClassAddMemberVariable)
	ON_COMMAND(ID_CLASS_DEFINITION, OnClassDefinition)
	ON_COMMAND(ID_CLASS_PROPERTIES, OnClassProperties)
	ON_COMMAND(ID_NEW_FOLDER, OnNewFolder)
	ON_WM_PAINT()
	ON_WM_SETFOCUS()
	ON_COMMAND_RANGE(ID_SORTING_GROUPBYTYPE, ID_SORTING_SORTBYACCESS, OnSort)
	ON_UPDATE_COMMAND_UI_RANGE(ID_SORTING_GROUPBYTYPE, ID_SORTING_SORTBYACCESS, OnUpdateSort)
	ON_MESSAGE(WM_UPDATEMODULEVIEW, OnUpdateModuleView)
	ON_NOTIFY(NM_DBLCLK, ID_TREE_MODULETREE, &CModuleView::OnNMDblclkTree)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CClassView 消息处理程序

int CModuleView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect rectDummy;
	rectDummy.SetRectEmpty();

	// 创建视图:
	const DWORD dwViewStyle = WS_CHILD | WS_VISIBLE | TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

	if (!m_wndClassView.Create(dwViewStyle, rectDummy, this, ID_TREE_MODULETREE))
	{
		TRACE0("未能创建类视图\n");
		return -1;      // 未能创建
	}

	// 加载图像:
	m_wndToolBar.Create(this, AFX_DEFAULT_TOOLBAR_STYLE, IDR_SORT);
	m_wndToolBar.LoadToolBar(IDR_SORT, 0, 0, TRUE /* 已锁定*/);

	OnChangeVisualStyle();

	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() | CBRS_TOOLTIPS | CBRS_FLYBY);
	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() & ~(CBRS_GRIPPER | CBRS_SIZE_DYNAMIC | CBRS_BORDER_TOP | CBRS_BORDER_BOTTOM | CBRS_BORDER_LEFT | CBRS_BORDER_RIGHT));

	m_wndToolBar.SetOwner(this);

	// 所有命令将通过此控件路由，而不是通过主框架路由:
	m_wndToolBar.SetRouteCommandsViaFrame(FALSE);

	CMenu menuSort;
	menuSort.LoadMenu(IDR_POPUP_SORT);

	m_wndToolBar.ReplaceButton(ID_SORT_MENU, CModuleViewMenuButton(menuSort.GetSubMenu(0)->GetSafeHmenu()));

	CModuleViewMenuButton* pButton =  DYNAMIC_DOWNCAST(CModuleViewMenuButton, m_wndToolBar.GetButton(0));

	if (pButton != NULL)
	{
		pButton->m_bText = FALSE;
		pButton->m_bImage = TRUE;
		pButton->SetImage(GetCmdMgr()->GetCmdImage(m_nCurrSort));
		pButton->SetMessageWnd(this);
	}

	InitModuleView(m_hWnd);
	return 0;
}

void CModuleView::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);
	AdjustLayout();
}

WCHAR szModuleList[10] = L"模块列表";  

void CModuleView::FillModuleView(ModuleMap& modulemap,CTask* pTask)
{
	m_wndClassView.DeleteAllItems();
	HTREEITEM hRoot = m_wndClassView.InsertItem(szModuleList, 0, 0);
	m_wndClassView.SetItemState(hRoot, TVIS_BOLD, TVIS_BOLD);
	int process = pTask->value;
	int num = 0;
	int size = modulemap.size();
	ModuleMap::iterator pos;
	for(pos=modulemap.begin();pos!=modulemap.end();pos++)
	{
		pTask->SetProcessValue(process+num*80/size);
		wstring name = (*pos).first;
		MODULE_INFORMATION mi = (*pos).second;
		HTREEITEM hModule = m_wndClassView.InsertItem(name.c_str(), 1, 1, hRoot);
		for(DWORD i=0;i<mi.FunctionCount;i++)
		{
			PFUNCTION_INFORMATION pfi =	mi.Functions + i;
			CString tmp;
			tmp.Format(L"%S()",pfi->name);
			HTREEITEM hFunction = m_wndClassView.InsertItem(tmp.GetBuffer(), 3, 3, hModule);
		}
		num++;
	}
	m_wndClassView.Expand(hRoot,TVE_EXPAND);
}

void CModuleView::OnContextMenu(CWnd* pWnd, CPoint point)
{
	CTreeCtrl* pWndTree = (CTreeCtrl*)&m_wndClassView;
	ASSERT_VALID(pWndTree);

	if (pWnd != pWndTree)
	{
		CDockablePane::OnContextMenu(pWnd, point);
		return;
	}

	if (point != CPoint(-1, -1))
	{
		// 选择已单击的项:
		CPoint ptTree = point;
		pWndTree->ScreenToClient(&ptTree);

		UINT flags = 0;
		HTREEITEM hTreeItem = pWndTree->HitTest(ptTree, &flags);
		if (hTreeItem != NULL)
		{
			pWndTree->SelectItem(hTreeItem);
		}
	}

	pWndTree->SetFocus();
	CMenu menu;
	menu.LoadMenu(IDR_POPUP_SORT);

	CMenu* pSumMenu = menu.GetSubMenu(0);

	if (AfxGetMainWnd()->IsKindOf(RUNTIME_CLASS(CMDIFrameWndEx)))
	{
		CMFCPopupMenu* pPopupMenu = new CMFCPopupMenu;

		if (!pPopupMenu->Create(this, point.x, point.y, (HMENU)pSumMenu->m_hMenu, FALSE, TRUE))
			return;

		((CMDIFrameWndEx*)AfxGetMainWnd())->OnShowPopupMenu(pPopupMenu);
		UpdateDialogControls(this, FALSE);
	}
}

void CModuleView::AdjustLayout()
{
	if (GetSafeHwnd() == NULL)
	{
		return;
	}

	CRect rectClient;
	GetClientRect(rectClient);

	int cyTlb = m_wndToolBar.CalcFixedLayout(FALSE, TRUE).cy;

	m_wndToolBar.SetWindowPos(NULL, rectClient.left, rectClient.top, rectClient.Width(), cyTlb, SWP_NOACTIVATE | SWP_NOZORDER);
	m_wndClassView.SetWindowPos(NULL, rectClient.left + 1, rectClient.top + cyTlb + 1, rectClient.Width() - 2, rectClient.Height() - cyTlb - 2, SWP_NOACTIVATE | SWP_NOZORDER);
}

BOOL CModuleView::PreTranslateMessage(MSG* pMsg)
{
	return CDockablePane::PreTranslateMessage(pMsg);
}

void CModuleView::OnSort(UINT id)
{
	if (m_nCurrSort == id)
	{
		return;
	}

	m_nCurrSort = id;

	CModuleViewMenuButton* pButton =  DYNAMIC_DOWNCAST(CModuleViewMenuButton, m_wndToolBar.GetButton(0));

	if (pButton != NULL)
	{
		pButton->SetImage(GetCmdMgr()->GetCmdImage(id));
		m_wndToolBar.Invalidate();
		m_wndToolBar.UpdateWindow();
	}
}

void CModuleView::OnUpdateSort(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(pCmdUI->m_nID == m_nCurrSort);
}

void CModuleView::OnClassAddMemberFunction()
{
	AfxMessageBox(_T("添加成员函数..."));
}

void CModuleView::OnClassAddMemberVariable()
{
	// TODO: 在此处添加命令处理程序代码
}

void CModuleView::OnClassDefinition()
{
	// TODO: 在此处添加命令处理程序代码
}

void CModuleView::OnClassProperties()
{
	// TODO: 在此处添加命令处理程序代码
}

void WINAPI CModuleView::UpdateProcess(PVOID param)
{
	CTask *pTask = (CTask*)param;
	CSDebuggerDoc *pDoc = (CSDebuggerDoc*)pTask->param2;
	CModuleView* view = (CModuleView*)pTask->param1;
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

void CModuleView::OnNewFolder()
{
	CTask task;
	task.param1 = this;
	task.param2 = GetActiveDocument();
	ShowProcessBarDlg((PVOID)UpdateProcess,task,L"查询模块");
}

void CModuleView::OnPaint()
{
	CPaintDC dc(this); // 用于绘制的设备上下文

	CRect rectTree;
	m_wndClassView.GetWindowRect(rectTree);
	ScreenToClient(rectTree);

	rectTree.InflateRect(1, 1);
	dc.Draw3dRect(rectTree, ::GetSysColor(COLOR_3DSHADOW), ::GetSysColor(COLOR_3DSHADOW));
}

void CModuleView::OnSetFocus(CWnd* pOldWnd)
{
	CDockablePane::OnSetFocus(pOldWnd);

	m_wndClassView.SetFocus();
}

void CModuleView::OnChangeVisualStyle()
{
	m_ClassViewImages.DeleteImageList();

	UINT uiBmpId = theApp.m_bHiColorIcons ? IDB_CLASS_VIEW_24 : IDB_CLASS_VIEW;

	CBitmap bmp;
	if (!bmp.LoadBitmap(uiBmpId))
	{
		TRACE(_T("无法加载位图: %x\n"), uiBmpId);
		ASSERT(FALSE);
		return;
	}

	BITMAP bmpObj;
	bmp.GetBitmap(&bmpObj);

	UINT nFlags = ILC_MASK;

	nFlags |= (theApp.m_bHiColorIcons) ? ILC_COLOR24 : ILC_COLOR4;

	m_ClassViewImages.Create(16, bmpObj.bmHeight, nFlags, 0, 0);
	m_ClassViewImages.Add(&bmp, RGB(255, 0, 0));

	m_wndClassView.SetImageList(&m_ClassViewImages, TVSIL_NORMAL);

	m_wndToolBar.CleanUpLockedImages();
	m_wndToolBar.LoadBitmap(theApp.m_bHiColorIcons ? IDB_SORT_24 : IDR_SORT, 0, 0, TRUE /* 锁定*/);
}

LRESULT CModuleView::OnUpdateModuleView(WPARAM wparam,LPARAM lparam)
{
	CTask task;
	task.param1 = this;
	task.param2 = GetActiveDocument();
	ShowProcessBarDlg((PVOID)UpdateProcess,task,L"查询模块");
	return TRUE;
}

void CModuleView::OnNMDblclkTree(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: 在此添加控件通知处理程序代码
	HTREEITEM hSelectItem = m_wndClassView.GetSelectedItem();
	HTREEITEM hParentItem = m_wndClassView.GetParentItem(hSelectItem);
	CString functionname = m_wndClassView.GetItemText(hSelectItem);
	CString modulename = m_wndClassView.GetItemText(hParentItem);
	if(modulename.IsEmpty() || wcscmp(modulename.GetBuffer(),szModuleList)==0)
	{
		return;
	}
	DisasmModuleFunction(modulename,functionname);
	*pResult = 0; 
}

void CModuleView::DisasmModuleFunction(CString modulename,CString functionname)
{
	wstring key = modulename.GetBuffer();
	ModuleMap::iterator pos;
	CSDebuggerDoc* pDoc = GetActiveDocument();
	pos = pDoc->GetModuleMap().find(key);
	if(pos!=pDoc->GetModuleMap().end())
	{
		CString fntmp;
		MODULE_INFORMATION mi = (*pos).second;
		for(DWORD i=0;i<mi.FunctionCount;i++)
		{
			PFUNCTION_INFORMATION pfi = mi.Functions + i;
			fntmp.Format(L"%S()",pfi->name);
			if(functionname==fntmp)
			{
				SendUserMsg(WM_DISASSEMBLY,(WPARAM)pfi->VirtualAddress,(LPARAM)NULL);
				break;
			}
		}
	}
}
