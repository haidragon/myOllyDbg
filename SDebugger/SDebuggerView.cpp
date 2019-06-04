
// SDebuggerView.cpp : CSDebuggerView 类的实现
//

#include "stdafx.h"
#include "SDebugger.h"
#include "SDebuggerDoc.h"
#include "SDebuggerView.h"
#include "DebugCore.h"
#include "Message.h"
#include "PT.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CSDebuggerView

IMPLEMENT_DYNCREATE(CSDebuggerView, CView)

BEGIN_MESSAGE_MAP(CSDebuggerView, CView)
	// 标准打印命令
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CSDebuggerView::OnFilePrintPreview)
	ON_MESSAGE(WM_DEBUGGERTHREAD, &CSDebuggerView::OnDebuggerThread)
	ON_MESSAGE(WM_DISASSEMBLY,&CSDebuggerView::OnDisassemby)
	ON_MESSAGE(WM_SETPOINTERADDRESS,&CSDebuggerView::OnSetPointerAdress)
	ON_COMMAND(ID_RUN, &CSDebuggerView::OnRun)
	ON_COMMAND(ID_PAUSE, &CSDebuggerView::OnPause)
	ON_COMMAND(ID_STOP, &CSDebuggerView::OnStop)
	ON_COMMAND(ID_SINGLE_IN, &CSDebuggerView::OnSingleIn)
	ON_UPDATE_COMMAND_UI(ID_RUN, &CSDebuggerView::OnUpdateDebugUI)
	ON_UPDATE_COMMAND_UI(ID_PAUSE, &CSDebuggerView::OnUpdateDebugUI)
	ON_UPDATE_COMMAND_UI(ID_STOP, &CSDebuggerView::OnUpdateDebugUI)
	ON_UPDATE_COMMAND_UI(ID_SINGLE_IN, &CSDebuggerView::OnUpdateDebugUI)
	ON_WM_VSCROLL()
	ON_WM_MOUSEWHEEL()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
END_MESSAGE_MAP()

// CSDebuggerView 构造/析构

CSDebuggerView::CSDebuggerView()
{
	m_background = RGB(255,255,255);
	m_adressBG = RGB(0,0,0);
	m_adressF = RGB(255,255,255);
	m_disasmBG = RGB(0,0,255);
	m_disasmF = RGB(255,255,0);
	m_binaryBG = RGB(0,200,200);
	m_binaryF = RGB(200,0,200);
	m_pointerF = RGB(255,255,0);
	m_pointerBG = RGB(128,128,128);
	m_selecterBG = RGB(0,255,255);
	LOGFONT logFont;
	memset(&logFont, 0, sizeof(LOGFONT));
	logFont.lfCharSet = DEFAULT_CHARSET;
	logFont.lfHeight = 120;
	logFont.lfWeight = FW_BOLD;
	wcscpy(logFont.lfFaceName,L"宋体");
	m_font.CreatePointFontIndirect(&logFont);
	m_marge = 4; //空隙
	m_code_address_width = 0;
	m_line_height = 0;
	m_pointer_width = 10;
	m_binary_width = 0;
	m_disasm_width = 0;
	m_nAllAddress = 0x7FFFFFFF;
	m_nCurrentPageStartAdr = 0;
	m_PointerAddress = 0;
	m_pointer_line = 0;
	m_isMouseLeftDown = FALSE;
	m_dragStartAdr = 0;
}

CSDebuggerView::~CSDebuggerView()
{
}

BOOL CSDebuggerView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: 在此处通过修改
	//  CREATESTRUCT cs 来修改窗口类或样式

	return CView::PreCreateWindow(cs);
}

// CSDebuggerView 绘制

void CSDebuggerView::OnDraw(CDC* pDC)
{
	CRect rectClient;
	GetClientRect(&rectClient);

	CDC dc;	
	dc.CreateCompatibleDC(pDC);

	CBitmap bm;
	bm.CreateCompatibleBitmap(pDC, rectClient.Width(), rectClient.Height());
	dc.SelectObject(bm);
	dc.SetBoundsRect(&rectClient, DCB_DISABLE);

	DrawBackground(&dc);
	DrawCodeAddress(&dc);
	DrawBinary(&dc);
	DrawDisassemby(&dc);
	DrawPointer(&dc);
	DrawSelecter(&dc);

	pDC->BitBlt(0, 0, rectClient.Width(), rectClient.Height(), &dc, 0, 0, SRCCOPY);

	dc.DeleteDC();
	// TODO: 在此处为本机数据添加绘制代码
}

void CSDebuggerView::DrawBackground(CDC* pDC)
{
	CRect rectClient;
	GetClientRect(&rectClient);
	CBrush bkBrush(m_background);
	pDC->FillRect(&rectClient,&bkBrush);
}

void CSDebuggerView::DrawCodeAddress(CDC* pDC)
{
	CSDebuggerDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	RECT clientrect;
	GetClientRect(&clientrect);

	CDC dc;	
	dc.CreateCompatibleDC(pDC);
	dc.SelectObject(&m_font);
	dc.SetTextColor(m_adressF);

	CSize size = dc.GetTextExtent(L"00000000",8);
	m_code_address_width = size.cx;
	m_line_height = size.cy;

	m_binary_width = dc.GetTextExtent(L"1234567890123456",16).cx;
	m_disasm_width = 400;

	LONG height = clientrect.bottom - clientrect.top;
	CRect rect(0, 0, m_marge + m_code_address_width + m_marge, height);

	CBitmap bm;
	bm.CreateCompatibleBitmap(pDC, m_marge + m_code_address_width + m_marge, height);
	dc.SelectObject(bm);
	dc.SetBoundsRect(&rect, DCB_DISABLE);
	dc.SetBkMode(TRANSPARENT);

	CBrush bkBrush(m_adressBG);
	dc.FillRect(&rect, &bkBrush);

	DisasmMap disasmm = pDoc->GetDisasmInfoMap();
	int count = 0;
	UInt64 prevadr = 0;
	UInt64 curadr = m_nCurrentPageStartAdr;
	m_hasPointer = FALSE;
	DisasmMap::iterator pos;
	while(count*m_line_height<height)
	{
		pos = disasmm.find(curadr);
		if(pos!=disasmm.end())
		{
			prevadr = curadr;
			CDisasmInfo* pdinfo=&(*pos).second;
			CString adrstr;
			adrstr.Format(L"%08p",pdinfo->m_vAddress);
			dc.TextOut(m_marge,count*m_line_height,adrstr);
			vector<Selecter>::iterator pos;
			for(pos=m_SelecterList.begin();pos!=m_SelecterList.end();pos++)
			{
				PSelecter selecter = (PSelecter)&(*pos);
				if(selecter->address == pdinfo->m_vAddress)
				{
					selecter->line = count;
					selecter->show = TRUE;
				}
			}
			if(m_PointerAddress == pdinfo->m_vAddress)
			{
				m_pointer_line = count;
				m_hasPointer = TRUE;
			}
			curadr+=pdinfo->m_size;
			count++;
		}
		else
		{
			//如果当前行反汇编不成功
			if(!curadr || !GetDocument()->Disassemby(curadr,TRUE))
			{
				break;
			}
			//清除掉当上一行
			if(prevadr)
			{
				GetDocument()->ClearDisasmMap(GetDocument()->GetDisasmInfoMap(),prevadr,prevadr);
			}
			//反汇编上一行
			if(!prevadr || !GetDocument()->Disassemby(prevadr,TRUE))
				break;
			//设置当前行
			curadr = prevadr;
			disasmm = pDoc->GetDisasmInfoMap();
		}
	}
	m_nCurPageAdrCount = curadr - m_nCurrentPageStartAdr;
	int start = m_marge + m_pointer_width + m_marge;
	pDC->BitBlt(start, 0, rect.Width(), rect.Height(), &dc, 0, 0, SRCCOPY);
	dc.DeleteDC();
	SetVertScrollBar();
}

void CSDebuggerView::DrawBinary(CDC* pDC)
{
	CSDebuggerDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	RECT clientrect;
	GetClientRect(&clientrect);

	CDC dc;	
	dc.CreateCompatibleDC(pDC);
	dc.SelectObject(&m_font);
	dc.SetTextColor(m_binaryF);

	LONG height = clientrect.bottom - clientrect.top;
	CRect rect(0, 0,m_marge + m_binary_width + m_marge, height);

	CBitmap bm;
	bm.CreateCompatibleBitmap(pDC, m_marge + m_binary_width + m_marge, height);
	dc.SelectObject(bm);
	dc.SetBoundsRect(&rect, DCB_DISABLE);
	dc.SetBkMode(TRANSPARENT);

	CBrush bkBrush(m_binaryBG);
	dc.FillRect(&rect, &bkBrush);

	DisasmMap disasmm = pDoc->GetDisasmInfoMap();
	int count = 0;
	UInt64 curadr = m_nCurrentPageStartAdr;
	DisasmMap::iterator pos;
	while(count*m_line_height<height)
	{
		pos = disasmm.find(curadr);
		if(pos!=disasmm.end())
		{
			CDisasmInfo* pdinfo=&(*pos).second;
			CString bytestr;
			for(SIZE_T i=0;i<pdinfo->m_size;i++)
			{
				CString bstr;
				bstr.Format(L"%X",pdinfo->m_mem[i]);
				bytestr+=bstr;
			}
			dc.TextOut(m_marge,count*m_line_height,bytestr);
			curadr+=pdinfo->m_size;
			count++;
		}
		else
		{
			break;
		}
	}
	int start = m_marge + m_pointer_width + m_marge + m_marge + m_code_address_width + m_marge;
	pDC->BitBlt(start, 0, rect.Width(), rect.Height(), &dc, 0, 0, SRCCOPY);
	dc.DeleteDC();
}

void CSDebuggerView::DrawDisassemby(CDC* pDC)
{
	CSDebuggerDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	RECT clientrect;
	GetClientRect(&clientrect);

	CDC dc;	
	dc.CreateCompatibleDC(pDC);
	dc.SelectObject(&m_font);
	dc.SetTextColor(m_disasmF);

	LONG height = clientrect.bottom - clientrect.top;
	CRect rect(0, 0, m_marge + m_disasm_width + m_marge, height);

	CBitmap bm;
	bm.CreateCompatibleBitmap(pDC, m_marge + m_disasm_width + m_marge, height);
	dc.SelectObject(bm);
	dc.SetBoundsRect(&rect, DCB_DISABLE);
	dc.SetBkMode(TRANSPARENT);

	CBrush bkBrush(m_disasmBG);
	dc.FillRect(&rect, &bkBrush);

	DisasmMap disasmm = pDoc->GetDisasmInfoMap();
	int count = 0;
	UInt64 curadr = m_nCurrentPageStartAdr;
	DisasmMap::iterator pos;
	while(count*m_line_height<height)
	{
		pos = disasmm.find(curadr);
		if(pos!=disasmm.end())
		{
			CDisasmInfo* pdinfo=&(*pos).second;
			CString codestr;
			codestr.Format(L"%S",pdinfo->m_disasmstr);
			dc.TextOut(m_marge,count*m_line_height,codestr);
			curadr+=pdinfo->m_size;
			count++;
		}
		else
		{
			break;
		}
	}
	int start = m_marge + m_pointer_width + m_marge + m_marge + m_code_address_width + m_marge + m_marge +m_binary_width + m_marge;
	pDC->BitBlt(start, 0, rect.Width(), rect.Height(), &dc, 0, 0, SRCCOPY);
	dc.DeleteDC();
}

void CSDebuggerView::DrawPointer(CDC* pDC)
{
	RECT clientrect;
	GetClientRect(&clientrect);

	CDC dc;	
	dc.CreateCompatibleDC(pDC);

	LONG height = clientrect.bottom - clientrect.top;
	CRect rect(0, 0, m_marge + m_pointer_width + m_marge, height);

	CBitmap bm;
	bm.CreateCompatibleBitmap(pDC, m_marge + m_pointer_width + m_marge, height);
	dc.SelectObject(bm);
	dc.SetBoundsRect(&rect, DCB_DISABLE);
	dc.SetBkMode(TRANSPARENT);

	CBrush bkBrush(m_pointerBG);
	dc.FillRect(&rect, &bkBrush);
	if(m_hasPointer)
	{	
		CPen pencolor(PS_SOLID, 1, m_pointerF);
		dc.SelectObject(&pencolor);
		CBrush penBrush(m_pointerF);
		dc.SelectObject(penBrush);
		int pointerY = m_pointer_line * m_line_height;
		POINT p[7];
		GetPointerPoly(p,m_marge,pointerY);
		dc.Polygon(p,7);
		dc.FillPath();
	}
	int start = 0;
	pDC->BitBlt(start, 0, rect.Width(), rect.Height(), &dc, 0, 0, SRCCOPY);
	dc.DeleteDC();
}

void CSDebuggerView::GetPointerPoly(POINT* p,int x,int y)
{
	p[0].y = y + m_line_height/4;
	p[0].x = x;

	p[1].y = y + m_line_height/4;
	p[1].x = x + m_pointer_width*2/3;

	p[2].y = y;
	p[2].x = x + m_pointer_width*2/3;

	p[3].y = y + m_line_height/2;
	p[3].x = x + m_pointer_width;

	p[4].y = y + m_line_height;
	p[4].x = x + m_pointer_width*2/3;

	p[5].y = y + m_line_height*3/4;
	p[5].x = x + m_pointer_width*2/3;

	p[6].y = y + m_line_height*3/4;
	p[6].x = x;
}

void CSDebuggerView::DrawSelecter(CDC* pDC)
{
	CSDebuggerDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	RECT clientrect;
	GetClientRect(&clientrect);

	LONG width = clientrect.right - clientrect.left;
	LONG height = clientrect.bottom - clientrect.top;

	CRect rect(0, 0, width, height);

	CDC dc;	
	dc.CreateCompatibleDC(pDC);

	CBitmap bm;
	bm.CreateCompatibleBitmap(pDC, width, height);
	dc.SelectObject(bm);
	dc.SetBoundsRect(&rect, DCB_DISABLE);
	dc.SetBkMode(TRANSPARENT);

	BLENDFUNCTION   bf; 
	bf.BlendOp   =   AC_SRC_OVER; 
	bf.BlendFlags   =   0; 
	bf.SourceConstantAlpha   =   0x7f;   //半透明 
	bf.AlphaFormat   =  AC_SRC_ALPHA; 

	CBrush bkBrush(m_selecterBG);
	vector<Selecter>::iterator pos;
	for(pos=m_SelecterList.begin();pos!=m_SelecterList.end();pos++)
	{
		PSelecter selecter = (PSelecter)&(*pos);
		if(selecter->show)
		{
			CRect rectselecter(0,selecter->line * m_line_height, width,selecter->line * m_line_height + m_line_height + 1);
			dc.FillRect(&rectselecter,&bkBrush);
			selecter->show = FALSE;
		}
	}
	pDC-> AlphaBlend(0,0,rect.Width(),rect.Height(),&dc,0,0,rect.Width(),rect.Height(),bf); 
	dc.DeleteDC();
}

// CSDebuggerView 打印


void CSDebuggerView::OnFilePrintPreview()
{
	AFXPrintPreview(this);
}

BOOL CSDebuggerView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// 默认准备
	return DoPreparePrinting(pInfo);
}

void CSDebuggerView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 添加额外的打印前进行的初始化过程
}

void CSDebuggerView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 添加打印后进行的清理过程
}

void CSDebuggerView::OnRButtonUp(UINT nFlags, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void CSDebuggerView::OnContextMenu(CWnd* pWnd, CPoint point)
{
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
}

void CSDebuggerView::OnInitialUpdate()
{
	InitMainView(m_hWnd);
}

void CSDebuggerView::OnRun()
{
	DEBUGGERSTATUS status = GetDebuggerStatus();
	if(status == STATUS_SUSPENDED)
	{
		ResumeThread(GetDocument()->GetDebugThread());
	}
	if(status == STATUS_INTERRUPTED)
	{
		SetUserEvent();
	}
	if(status == STATUS_PAUSE)
	{
		CSDebuggerDoc* pDoc = GetDocument();
		ASSERT_VALID(pDoc);
		if (!pDoc)
			return;
		size_t size = pDoc->GetThreadList().size();
		for(size_t i=0;i<size;i++)
		{
			PSYSTEM_THREAD_INFORMATION pSTI = &(pDoc->GetThreadList()[i]);
			if(pSTI->State == StateWait && pSTI->WaitReason == Suspended)
			{
				//挂起状态的，不处理
			}
			else
			{
				HANDLE hThread = OpenThread(THREAD_ALL_ACCESS,FALSE,pSTI->ClientId.UniqueThread);
				ResumeThread(hThread);
				CloseHandle(hThread);
			}
		}
		SetDebuggerStatus(STATUS_RUN);
	}
}

void CSDebuggerView::OnStop()
{

}

void CSDebuggerView::OnPause()
{
	CSDebuggerDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;
	HANDLE hThread;
	pDoc->GetThreadList().clear();
	if(GetThreadList(pDoc->GetDebugProcessId(),pDoc->GetThreadList()))
	{
		size_t size = pDoc->GetThreadList().size();
		for(size_t i=0;i<size;i++)
		{
			PSYSTEM_THREAD_INFORMATION pSTI = &(pDoc->GetThreadList()[i]);
			if(pSTI->State == StateWait && pSTI->WaitReason == Suspended)
			{
				//挂起状态的，不处理
			}
			else
			{
				hThread = OpenThread(THREAD_ALL_ACCESS,FALSE,pSTI->ClientId.UniqueThread);
				SuspendThread(hThread);
				CloseHandle(hThread);
			}
		}
	}
	DWORD mainthreadid = pDoc->GetMainThreadID();
	hThread = OpenThread(THREAD_ALL_ACCESS,FALSE,mainthreadid);
	pDoc->SetDebugThread(hThread);
	CONTEXT context;
	if(pDoc->GetDebugThreadContext(&context))
	{
		OnDisassemby((WPARAM)context.Eip,NULL);
	}
	SendUserMsg(WM_UPDATEUIREGISTER, 0, 0);
	SetDebuggerStatus(STATUS_PAUSE);
}

void CSDebuggerView::OnRestart()
{

}

void CSDebuggerView::OnSingleIn()
{
	CSDebuggerDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;
	HANDLE hThread = pDoc->GetDebugThread();
	SingleDebug(hThread);
	OnRun();
}

void CSDebuggerView::OnUpdateDebugUI(CCmdUI* pCui)
{
	DEBUGGERSTATUS status = GetDebuggerStatus();
	switch(status)
	{
	case STATUS_NONE:
		{
			if(pCui->m_nID == ID_RUN)
			{
				pCui->Enable(FALSE);
			}
			if(pCui->m_nID == ID_PAUSE)
			{
				pCui->Enable(FALSE);
			}
			if(pCui->m_nID == ID_STOP)
			{
				pCui->Enable(FALSE);
			}
			if(pCui->m_nID == ID_SINGLE_IN)
			{
				pCui->Enable(FALSE);
			}
		}
		break;
	case STATUS_SUSPENDED:
		{
			if(pCui->m_nID == ID_RUN)
			{
				pCui->Enable(TRUE);
			}
			if(pCui->m_nID == ID_PAUSE)
			{
				pCui->Enable(FALSE);
			}
			if(pCui->m_nID == ID_STOP)
			{
				pCui->Enable(TRUE);
			}
			if(pCui->m_nID == ID_SINGLE_IN)
			{
				pCui->Enable(FALSE);
			}
		}
		break;
	case STATUS_INTERRUPTED:
		{
			if(pCui->m_nID == ID_RUN)
			{
				pCui->Enable(TRUE);
			}
			if(pCui->m_nID == ID_PAUSE)
			{
				pCui->Enable(FALSE);
			}
			if(pCui->m_nID == ID_STOP)
			{
				pCui->Enable(TRUE);
			}
			if(pCui->m_nID == ID_SINGLE_IN)
			{
				pCui->Enable(TRUE);
			}
		}
		break;
	case STATUS_RUN:
		{
			if(pCui->m_nID == ID_RUN)
			{
				pCui->Enable(FALSE);
			}
			if(pCui->m_nID == ID_PAUSE)
			{
				pCui->Enable(TRUE);
			}
			if(pCui->m_nID == ID_STOP)
			{
				pCui->Enable(TRUE);
			}
			if(pCui->m_nID == ID_SINGLE_IN)
			{
				pCui->Enable(FALSE);
			}
		}
		break;
	case STATUS_PAUSE:
		{
			if(pCui->m_nID == ID_RUN)
			{
				pCui->Enable(TRUE);
			}
			if(pCui->m_nID == ID_PAUSE)
			{
				pCui->Enable(FALSE);
			}
			if(pCui->m_nID == ID_STOP)
			{
				pCui->Enable(TRUE);
			}
			if(pCui->m_nID == ID_SINGLE_IN)
			{
				pCui->Enable(FALSE);
			}
		}
		break;
	}
}

LRESULT CSDebuggerView::OnDebuggerThread(WPARAM wparam,LPARAM lparam)
{
	
	DWORD ProcessID = (DWORD)wparam;
	DWORD ThreadID = (DWORD)lparam;
	HANDLE hThread = NULL;
	if(ThreadID)
	{
		 hThread = OpenThread(THREAD_ALL_ACCESS,FALSE,ThreadID);
	}
	else
	{
		ThreadList threadlist;
		if(GetThreadList(ProcessID,threadlist))
		{
			size_t size = threadlist.size();
			for(size_t i=0;i<size;i++)
			{
				PSYSTEM_THREAD_INFORMATION pSTI = &threadlist[i];
				if(pSTI->State == StateWait && pSTI->WaitReason == Suspended)
				{
					//非调试线程
				}
				else
				{
					//调试线程
					hThread = OpenThread(THREAD_ALL_ACCESS,FALSE,pSTI->ClientId.UniqueThread);
					break;
				}
			}
		}
		threadlist.clear();
	}
	GetDocument()->SetDebugThread(hThread);
	GetDocument()->ClearStackDeque();
	EmunInvokeStack(GetDocument()->GetDebugProcess(),GetDocument()->GetDebugThread(),GetDocument()->GetStackDeque());
	THREAD_BASIC_INFORMATION TBI;
	if(GetThreadBaseInformation(hThread,&TBI))
	{
		GetStackInformation(GetDocument()->GetDebugProcess(),TBI.TebBaseAddress,&GetDocument()->GetCurrentNT_TIB());
	}
	SendUserMsg(WM_UPDATEUIREGISTER, 0, 0);
	SendUserMsg(WM_UPDATEINVOKESTACK,0, 0);
	return TRUE;
}

LRESULT CSDebuggerView::OnDisassemby(WPARAM wparam,LPARAM lparam)
{
	ULONG_PTR ExceptionAddress = (ULONG_PTR)wparam;
	DWORD ExceptionCode = (DWORD)lparam;
	BOOL ret = TRUE;
	if(ExceptionCode == STATUS_SINGLE_STEP)
	{
		DisasmMap *pDisasmMap = &GetDocument()->GetDisasmInfoMap();
		DisasmMap::iterator pos;
		pos = pDisasmMap->find(ExceptionAddress);
		if(pos!=pDisasmMap->end() && ExceptionAddress>=m_nCurrentPageStartAdr && ExceptionAddress<m_nCurrentPageStartAdr+m_nCurPageAdrCount)
		{
			Invalidate(FALSE);
			return ret;
		}
	}
	ret = GetDocument()->Disassemby(ExceptionAddress);
	m_nCurrentPageStartAdr = ExceptionAddress;
	Invalidate(FALSE);
	return ret;
}

LRESULT CSDebuggerView::OnSetPointerAdress(WPARAM wparam,LPARAM lparam)
{
	m_PointerAddress = (ULONG_PTR)wparam;
	return m_PointerAddress;
}

void CSDebuggerView::SetVertScrollBar(void)
{
	SCROLLINFO si;
	ZeroMemory(&si,sizeof(si));
	si.cbSize = sizeof(si);
	si.fMask = SIF_ALL;
	si.nMin = 0;
	si.nMax = m_nAllAddress;
	si.nPage = m_nCurPageAdrCount;
	si.nPos = m_nCurrentPageStartAdr;
	VERIFY(SetScrollInfo(SB_VERT, &si, TRUE));
}

void CSDebuggerView::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	CView::OnVScroll(nSBCode, nPos, pScrollBar);
	CSDebuggerDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;
	UInt64 curadr;
	SCROLLINFO si;
	si.cbSize = sizeof(si);
	si.fMask = SIF_ALL;
	VERIFY(GetScrollInfo(SB_VERT, &si));

	switch (nSBCode)
	{
	case SB_TOP:
		break;
	case SB_BOTTOM:
		break;
	case SB_LINEUP:
		{
			curadr = pDoc->GetPrevCodeAdress(m_nCurrentPageStartAdr);
			if(curadr)
			{
				m_nCurrentPageStartAdr = curadr;
				Invalidate(FALSE);
			}
			else
			{
				OnDisassemby(m_nCurrentPageStartAdr,0);
			}
		}
		break;
	case SB_LINEDOWN:
		{
			curadr = pDoc->GetNextCodeAdress(m_nCurrentPageStartAdr);
			m_nCurrentPageStartAdr = curadr ? curadr : m_nCurrentPageStartAdr;
			Invalidate(FALSE);
		}
		break;
	case SB_PAGEUP:
		si.nPage;
		break;
	case SB_PAGEDOWN:
		break;
	case SB_THUMBTRACK:
		si.nTrackPos;
		break;
	}
}

BOOL CSDebuggerView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	CSDebuggerDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return CView::OnMouseWheel(nFlags, zDelta, pt);;
	if(zDelta>0)
	{
		UInt64 curadr= pDoc->GetPrevCodeAdress(m_nCurrentPageStartAdr);
		if(curadr)
		{
			m_nCurrentPageStartAdr = curadr;
			Invalidate(FALSE);
		}
		else
		{
			OnDisassemby(m_nCurrentPageStartAdr,0);
		}
	}
	else
	{
		UInt64 curadr = pDoc->GetNextCodeAdress(m_nCurrentPageStartAdr);
		m_nCurrentPageStartAdr = curadr ? curadr : m_nCurrentPageStartAdr;
		Invalidate(FALSE);
	}
	return CView::OnMouseWheel(nFlags, zDelta, pt);
}

void CSDebuggerView::OnLButtonDown(UINT nFlags, CPoint point)
{
	CView::OnLButtonDown(nFlags, point);
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	CSDebuggerDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;
	m_isMouseLeftDown = TRUE;
	m_SelecterList.clear();

	RECT clientrect;
	GetClientRect(&clientrect);
	LONG height = clientrect.bottom - clientrect.top;
	UInt64 curadr = m_nCurrentPageStartAdr;
	DisasmMap disasmm = pDoc->GetDisasmInfoMap();
	DisasmMap::iterator pos;
	int count = 0;
	while(count*m_line_height<height)
	{
		pos = disasmm.find(curadr);
		if(pos!=disasmm.end())
		{
			CDisasmInfo* pdinfo=&(*pos).second;
			if(point.y>count*m_line_height && point.y<=(count+1)*m_line_height)
			{
				Selecter selecter;
				selecter.address = pdinfo->m_vAddress;
				selecter.line = count;
				m_SelecterList.push_back(selecter);
				m_dragStartAdr = pdinfo->m_vAddress;
			}
			curadr+=pdinfo->m_size;
			count++;
		}
	}
	Invalidate(FALSE);
}

void CSDebuggerView::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	CView::OnLButtonDblClk(nFlags, point);
}

BOOL CSDebuggerView::IsSelected(UInt64 vAddress)
{
	vector<Selecter>::iterator pos;
	for(pos=m_SelecterList.begin();pos!=m_SelecterList.end();pos++)
	{
		if((*pos).address==vAddress)
		{
			return TRUE;
		}
	}
	return FALSE;
}

void CSDebuggerView::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	CView::OnMouseMove(nFlags, point);
	CSDebuggerDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;
	if(m_isMouseLeftDown)
	{
		ULONG_PTR dragEndAdr = 0;
		RECT clientrect;
		GetClientRect(&clientrect);
		LONG height = clientrect.bottom - clientrect.top;
		UInt64 curadr = m_nCurrentPageStartAdr;
		DisasmMap disasmm = pDoc->GetDisasmInfoMap();
		DisasmMap::iterator pos;
		int count = 0;
		while(count*m_line_height<height)
		{
			pos = disasmm.find(curadr);
			if(pos!=disasmm.end())
			{
				CDisasmInfo* pdinfo=&(*pos).second;
				if(point.y>count*m_line_height && point.y<=(count+1)*m_line_height)
				{
					dragEndAdr = curadr;
					break;
				}
				curadr+=pdinfo->m_size;
				count++;
			}
		}
		if(dragEndAdr)
		{
			if(dragEndAdr>m_dragStartAdr)
			{
				curadr = m_dragStartAdr;
			}
			else
			{
				curadr = dragEndAdr;
				dragEndAdr = m_dragStartAdr;
			}
		}
		m_SelecterList.clear();
		count = 0;
		while(curadr<=dragEndAdr)
		{
			pos = disasmm.find(curadr);
			if(pos!=disasmm.end())
			{
				CDisasmInfo* pdinfo=&(*pos).second;
				Selecter selecter;
				selecter.address = pdinfo->m_vAddress;
				selecter.line = count;
				m_SelecterList.push_back(selecter);
				curadr+=pdinfo->m_size;
				count++;
			}
		}

		Invalidate(FALSE);
	}
}

void CSDebuggerView::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	CView::OnLButtonUp(nFlags, point);
	m_isMouseLeftDown = FALSE;
}

// CSDebuggerView 诊断

#ifdef _DEBUG
void CSDebuggerView::AssertValid() const
{
	CView::AssertValid();
}

void CSDebuggerView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CSDebuggerDoc* CSDebuggerView::GetDocument() const // 非调试版本是内联的
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CSDebuggerDoc)));
	return (CSDebuggerDoc*)m_pDocument;
}
#endif //_DEBUG


// CSDebuggerView 消息处理程序
