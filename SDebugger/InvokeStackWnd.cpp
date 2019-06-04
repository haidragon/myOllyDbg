#include "StdAfx.h"
#include "Resource.h"
#include "InvokeStackWnd.h"
#include "Message.h"
#include "GlobalApi.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

BEGIN_MESSAGE_MAP(CStackList, CListCtrl)
	ON_WM_WINDOWPOSCHANGING()
END_MESSAGE_MAP()

CStackList::CStackList()
{
	
}

CStackList::~CStackList()
{

}

void CStackList::ScrollV(int itemindex)
{
	RECT iRect,clientRect;
	GetItemRect(itemindex,&iRect,LVIR_BOUNDS);
	LONG height = (iRect.bottom - iRect.top) * itemindex;
	int count = GetItemCount();
	LONG maxheight = count * (iRect.bottom - iRect.top);
	GetClientRect(&clientRect);
	LONG clientHeight = clientRect.bottom - clientRect.top;
	if(height > maxheight - clientHeight)
	{
		EnsureVisible(count - 1,FALSE);
		return;
	}
	CSize scrollsize(0,height);
	Scroll(scrollsize);
}

BEGIN_MESSAGE_MAP(CStackTabCtrl, CSTabCtrl)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_THREAD_STACK_TABLE, OnNMCustomdrawThreadStackTable)
END_MESSAGE_MAP()

CStackTabCtrl::CStackTabCtrl()
{

}

CStackTabCtrl::~CStackTabCtrl()
{

}

void CStackTabCtrl::OnNMCustomdrawThreadStackTable(NMHDR *pNMHDR, LRESULT *pResult)
{
	NMLVCUSTOMDRAW* pLVCD = reinterpret_cast<NMLVCUSTOMDRAW*>( pNMHDR );
	*pResult = CDRF_DODEFAULT;

	if ( CDDS_PREPAINT == pLVCD->nmcd.dwDrawStage )
	{
		*pResult = CDRF_NOTIFYITEMDRAW;
	}
	else if ( CDDS_ITEMPREPAINT == pLVCD->nmcd.dwDrawStage )
	{
		*pResult = CDRF_NOTIFYSUBITEMDRAW;
	}
	else if ( (CDDS_ITEMPREPAINT | CDDS_SUBITEM) == pLVCD->nmcd.dwDrawStage )
	{
		COLORREF clrNewTextColor, clrNewBkColor;
		int nItem = static_cast<int>( pLVCD->nmcd.dwItemSpec );
		wchar_t szText[MAX_PATH];
		LVITEMW lvitem;
		lvitem.mask = LVIF_TEXT;
		lvitem.iItem = nItem;
		lvitem.iSubItem = 0;
		lvitem.pszText = szText; 
		lvitem.cchTextMax = MAX_PATH; 
		m_InvokeStackWnd->m_cThreadList.GetItem(&lvitem);
		UINT_PTR address = wcstoul(szText,0,16);
		if(address == m_InvokeStackWnd->m_current_esp)
		{
			clrNewTextColor = RGB(255,255,0);
			clrNewBkColor = RGB(0, 0, 255);
		}
		else
		{
			clrNewTextColor = RGB(0,0,0);
			clrNewBkColor = RGB(255, 255, 255);
		}
		pLVCD->clrText = clrNewTextColor;
		pLVCD->clrTextBk = clrNewBkColor;
		*pResult = CDRF_DODEFAULT;
	}
}

BEGIN_MESSAGE_MAP(CInvokeStackWnd, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_MESSAGE(WM_UPDATEINVOKESTACK,OnUpdateInvokeStack)
END_MESSAGE_MAP()

CInvokeStackWnd::CInvokeStackWnd(void)
{
	m_current_esp = 0;
}

CInvokeStackWnd::~CInvokeStackWnd(void)
{
}

void CInvokeStackWnd::DoDataExchange(CDataExchange* pDX)
{
	CDockablePane::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_THREAD_STACK_TABLE, m_cThreadList);
}

int CInvokeStackWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	m_Font.CreateStockObject(DEFAULT_GUI_FONT);

	CRect rectDummy;
	rectDummy.SetRectEmpty();

	// 创建选项卡窗口:
	if (!m_wndTabs.Create(CMFCTabCtrl::STYLE_FLAT, rectDummy, this, 1))
	{
		TRACE0("未能创建输出选项卡窗口\n");
		return -1;      // 未能创建
	}

	m_wndTabs.SetInvokeStackWnd(this);

	RECT clientrect;
	GetClientRect(&clientrect);
	const DWORD dwViewStyle = WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL | LVS_EX_GRIDLINES;
	if(!m_cInvokeList.Create(dwViewStyle,clientrect,&m_wndTabs,IDC_INVOKE_STACK_TABLE))
	{
		TRACE("创建调用函数堆栈表控件失败");
		return -1;
	}
	DWORD ExStyle = m_cInvokeList.GetExtendedStyle();
	m_cInvokeList.SetExtendedStyle(ExStyle | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);//设置属性整行选择 | 显示网格
	m_cInvokeList.InsertColumn(0,_T("堆栈地址"),LVCFMT_IMAGE|LVCFMT_LEFT); //添加第0列,可以制定允许显示图标等属性
	m_cInvokeList.InsertColumn(1,_T("调用函数"));
	m_cInvokeList.SetColumnWidth(0, 100);
	m_cInvokeList.SetColumnWidth(1, 100);

	if(!m_cThreadList.Create(dwViewStyle,clientrect,&m_wndTabs,IDC_THREAD_STACK_TABLE))
	{
		TRACE("创建线程堆栈表控件失败");
		return -1;
	}
	ExStyle = m_cThreadList.GetExtendedStyle();
	m_cThreadList.SetExtendedStyle(ExStyle | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);//设置属性整行选择 | 显示网格
	m_cThreadList.InsertColumn(0,_T("堆栈地址"),LVCFMT_IMAGE|LVCFMT_LEFT); //添加第0列,可以制定允许显示图标等属性
	m_cThreadList.InsertColumn(1,_T("堆栈值"));
	m_cThreadList.SetColumnWidth(0, 100);
	m_cThreadList.SetColumnWidth(1,100);

	CString strTabName;
	BOOL bNameValid;

	// 将列表窗口附加到选项卡:

	bNameValid = strTabName.LoadString(IDS_THREAD_STACK_TAB);
	ASSERT(bNameValid);
	m_wndTabs.AddTab(&m_cThreadList, strTabName, (UINT)0);

	bNameValid = strTabName.LoadString(IDS_INVOKE_STACK_TAB);
	ASSERT(bNameValid);
	m_wndTabs.AddTab(&m_cInvokeList, strTabName, (UINT)0);

	InitStackWnd(m_hWnd);

	return 0;
}

void CInvokeStackWnd::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);
	// 选项卡控件应覆盖整个工作区:
	m_wndTabs.SetWindowPos(NULL, -1, -1, cx, cy, SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);
}

//调用堆栈的读取
LRESULT CInvokeStackWnd::OnUpdateInvokeStack(WPARAM wparam,LPARAM lparam)
{
	m_cInvokeList.DeleteAllItems();
	CSDebuggerDoc *pDoc = GetActiveDocument();
	if(!pDoc)
	{
		return -1;
	}
	StackDeque sd = pDoc->GetStackDeque();
	StackDeque::iterator pos;
	size_t size = sd.size();
	for(size_t i=0;i<size;i++)
	{
		InvokeStack is = sd[i];
		CString adrstr;
		adrstr.Format(L"%08p",is.AddrStack);
		m_cInvokeList.InsertItem(i,adrstr);
		adrstr.Format(L"%08p",is.AddrPC);
		m_cInvokeList.SetItemText(i,1,adrstr);
	}

	CONTEXT context;
	if(!pDoc->GetDebugThreadContext(&context))
	{
		return -1;
	}
	m_current_esp = 0;
	m_current_esp = context.Esp;
	ULONG_PTR scrollindex = 0;
	m_cThreadList.DeleteAllItems();
	NT_TIB nt_tib = pDoc->GetCurrentNT_TIB();
	if((ULONG_PTR)nt_tib.StackBase!=0xffffffff)
	{
		SIZE_T size = (ULONG_PTR)nt_tib.StackBase - (ULONG_PTR)nt_tib.StackLimit;
		PBYTE buff = (PBYTE)malloc(size);
		if(buff)
		{
			SIZE_T numberofread;
			if(ReadProcessMemory(pDoc->GetDebugProcess(),nt_tib.StackLimit,buff,size,&numberofread))
			{
				ULONG_PTR index = 0;
				for(ULONG_PTR i=(ULONG_PTR)nt_tib.StackLimit;i<(ULONG_PTR)nt_tib.StackBase;i+=4,index++)
				{
					CString adrstr;
					adrstr.Format(L"%08p",i);
					m_cThreadList.InsertItem(index,adrstr);
					ULONG_PTR val = *PULONG_PTR(buff+index*4);
					adrstr.Format(L"%08p",val);
					m_cThreadList.SetItemText(index,1,adrstr);
					if(m_current_esp == i)
					{
						scrollindex = index;
					}
				}
			}
			free(buff);
			m_cThreadList.ScrollV(scrollindex);
		}
	}
	return 0;
}