#include "stdafx.h"
#include "SDebugger.h"
#include "CPropertyGridCtrl.h"
#include "RegisterWnd.h"
#include "Resource.h"
#include "Message.h"
#include "SDebuggerDoc.h"
#include "GlobalApi.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CResourceViewBar

CRegisterWnd::CRegisterWnd()
{
}

CRegisterWnd::~CRegisterWnd()
{
}

BEGIN_MESSAGE_MAP(CRegisterWnd, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_COMMAND(ID_EXPAND_ALL, OnExpandAllProperties)
	ON_UPDATE_COMMAND_UI(ID_EXPAND_ALL, OnUpdateExpandAllProperties)
	ON_COMMAND(ID_SORTPROPERTIES, OnSortProperties)
	ON_UPDATE_COMMAND_UI(ID_SORTPROPERTIES, OnUpdateSortProperties)
	ON_COMMAND(ID_PROPERTIES1, OnProperties1)
	ON_UPDATE_COMMAND_UI(ID_PROPERTIES1, OnUpdateProperties1)
	ON_COMMAND(ID_PROPERTIES2, OnProperties2)
	ON_UPDATE_COMMAND_UI(ID_PROPERTIES2, OnUpdateProperties2)
	ON_WM_SETFOCUS()
	ON_WM_SETTINGCHANGE()
	ON_MESSAGE(WM_UPDATEUIREGISTER, OnUpdateRegisterUI)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CResourceViewBar 消息处理程序

void CRegisterWnd::AdjustLayout()
{
	if (GetSafeHwnd() == NULL)
	{
		return;
	}

	CRect rectClient,rectCombo;
	GetClientRect(rectClient);

	m_wndPropList.SetWindowPos(NULL, rectClient.left, rectClient.top, rectClient.Width(), rectClient.Height() , SWP_NOACTIVATE | SWP_NOZORDER);
}

int CRegisterWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect rectDummy;
	rectDummy.SetRectEmpty();

	// 创建组合:
	const DWORD dwViewStyle = WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_BORDER | CBS_SORT | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

	if (!m_wndPropList.Create(WS_VISIBLE | WS_CHILD, rectDummy, this, 2))
	{
		TRACE0("未能创建属性网格\n");
		return -1;      // 未能创建
	}

	InitPropList();

	AdjustLayout();

	InitRegistHwnd(m_hWnd);
	return 0;
}

void CRegisterWnd::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);
	AdjustLayout();
}

void CRegisterWnd::OnExpandAllProperties()
{
	m_wndPropList.ExpandAll();
}

void CRegisterWnd::OnUpdateExpandAllProperties(CCmdUI* pCmdUI)
{
}

void CRegisterWnd::OnSortProperties()
{
	m_wndPropList.SetAlphabeticMode(!m_wndPropList.IsAlphabeticMode());
}

void CRegisterWnd::OnUpdateSortProperties(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_wndPropList.IsAlphabeticMode());
}

void CRegisterWnd::OnProperties1()
{
	// TODO: 在此处添加命令处理程序代码
}

void CRegisterWnd::OnUpdateProperties1(CCmdUI* /*pCmdUI*/)
{
	// TODO: 在此处添加命令更新 UI 处理程序代码
}

void CRegisterWnd::OnProperties2()
{
	// TODO: 在此处添加命令处理程序代码
}

void CRegisterWnd::OnUpdateProperties2(CCmdUI* /*pCmdUI*/)
{
	// TODO: 在此处添加命令更新 UI 处理程序代码
}

void CRegisterWnd::InitPropList()
{
	SetPropListFont();

	m_wndPropList.EnableHeaderCtrl(FALSE);
	m_wndPropList.EnableDescriptionArea();
	m_wndPropList.SetVSDotNetLook();
	m_wndPropList.MarkModifiedProperties();

	CMFCPropertyGridProperty* pGroupStandRegister = new CMFCPropertyGridProperty(_T("标准寄存器"));
	
	CMFCPropertyGridProperty* pPropEAX = new CMFCPropertyGridProperty(_T("EAX"), (_variant_t) _T("00000000"), _T("EAX寄存器以称为累加器,AX寄存器是算术运算的主要寄存器,所有的输入、输出只使用AL或AX人作为数据寄存器")); 

	pGroupStandRegister->AddSubItem(pPropEAX);

	CMFCPropertyGridProperty* pPropEBX = new CMFCPropertyGridProperty(_T("EBX"), (_variant_t) _T("00000000"), _T("EBX寄存器是基地址(base)寄存器, 在内存寻址时存放基地址。")); 

	pGroupStandRegister->AddSubItem(pPropEBX);

	CMFCPropertyGridProperty* pPropECX = new CMFCPropertyGridProperty(_T("ECX"), (_variant_t) _T("00000000"), _T("ECX寄存器是计数器(counter),是重复(REP)前缀指令和LOOP指令的内定计数器")); 

	pGroupStandRegister->AddSubItem(pPropECX);

	CMFCPropertyGridProperty* pPropEDX = new CMFCPropertyGridProperty(_T("EDX"), (_variant_t) _T("00000000"), _T("EDX寄存器总是被用来放整数除法产生的余数")); 

	pGroupStandRegister->AddSubItem(pPropEDX);

	CMFCPropertyGridProperty* pPropESP = new CMFCPropertyGridProperty(_T("ESP"), (_variant_t) _T("00000000"), _T("ESP寄存器专门用作堆栈指针,被形象地称为栈顶指针,堆栈的顶部是地址小的区域，压入堆栈的数据越多,ESP也就越来越小,在32位平台上,ESP每次减少4字节")); 

	pGroupStandRegister->AddSubItem(pPropESP);

	CMFCPropertyGridProperty* pPropEBP = new CMFCPropertyGridProperty(_T("EBP"), (_variant_t) _T("00000000"), _T("EBP寄存器EBP是基址指针(BASE POINTER),它最经常被用作高级语言函数调用的框架指针(frame pointer)")); 

	pGroupStandRegister->AddSubItem(pPropEBP);

	CMFCPropertyGridProperty* pPropESI = new CMFCPropertyGridProperty(_T("ESI"), (_variant_t) _T("00000000"), _T("ESI/EDI寄存器分别叫做源/目标索引寄存器(source/destination index),因为在很多字符串操作指令中,DS:ESI指向源串,而ES:EDI指向目标串")); 

	pGroupStandRegister->AddSubItem(pPropESI);

	CMFCPropertyGridProperty* pPropEDI = new CMFCPropertyGridProperty(_T("EDI"), (_variant_t) _T("00000000"), _T("ESI/EDI寄存器分别叫做源/目标索引寄存器(source/destination index),因为在很多字符串操作指令中,DS:ESI指向源串,而ES:EDI指向目标串")); 

	pGroupStandRegister->AddSubItem(pPropEDI);

	CMFCPropertyGridProperty* pPropEIP = new CMFCPropertyGridProperty(_T("EIP"), (_variant_t) _T("00000000"), _T("EIP寄存器寄存器存放下一个CPU指令存放的内存地址,当CPU执行完当前的指令后,从EIP寄存器中读取下一条指令的内存地址,然后继续执行")); 

	pGroupStandRegister->AddSubItem(pPropEIP);

	m_wndPropList.AddProperty(pGroupStandRegister);

	CMFCPropertyGridProperty* pGroupFlagRegister = new CMFCPropertyGridProperty(_T("标志寄存器"));

	CMFCPropertyGridProperty* pPropCF = new CMFCPropertyGridProperty(_T("CF"), (_variant_t) _T("0"), _T("进位标志.无符号数发生溢出时,该标志为1,否则为0")); 

	pGroupFlagRegister->AddSubItem(pPropCF);

	CMFCPropertyGridProperty* pPropPF = new CMFCPropertyGridProperty(_T("PF"), (_variant_t) _T("0"), _T("奇偶标志.运算结果的最低字节中包含偶数个1时,该标志为1,否则为0.")); 

	pGroupFlagRegister->AddSubItem(pPropPF);

	CMFCPropertyGridProperty* pPropAF = new CMFCPropertyGridProperty(_T("AF"), (_variant_t) _T("0"), _T("辅助进位标志,运算结果的最低字节的第三位向高位进位时,该标志为1,否则为0.")); 

	pGroupFlagRegister->AddSubItem(pPropAF);

	CMFCPropertyGridProperty* pPropZF = new CMFCPropertyGridProperty(_T("ZF"), (_variant_t) _T("0"), _T("0标志,运算结果未0时,该标志为1,否则为0.")); 

	pGroupFlagRegister->AddSubItem(pPropZF);

	CMFCPropertyGridProperty* pPropSF = new CMFCPropertyGridProperty(_T("SF"), (_variant_t) _T("0"), _T("符号标志,运算结果未负数时,该标志为1,否则为0.")); 

	pGroupFlagRegister->AddSubItem(pPropSF);

	CMFCPropertyGridProperty* pPropDF = new CMFCPropertyGridProperty(_T("DF"), (_variant_t) _T("0"), _T("方向标志,该标志为1时,字符串指令每次操作后递减ESI和EDI,为0时递增.")); 

	pGroupFlagRegister->AddSubItem(pPropDF);

	CMFCPropertyGridProperty* pPropOF = new CMFCPropertyGridProperty(_T("OF"), (_variant_t) _T("0"), _T("溢出标志,有符号数发生溢出时,该标志为1,否则为0.")); 

	pGroupFlagRegister->AddSubItem(pPropOF);

	m_wndPropList.AddProperty(pGroupFlagRegister);
}

void CRegisterWnd::OnSetFocus(CWnd* pOldWnd)
{
	CDockablePane::OnSetFocus(pOldWnd);
	m_wndPropList.SetFocus();
}

void CRegisterWnd::OnSettingChange(UINT uFlags, LPCTSTR lpszSection)
{
	CDockablePane::OnSettingChange(uFlags, lpszSection);
	SetPropListFont();
}

void CRegisterWnd::SetPropListFont()
{
	::DeleteObject(m_fntPropList.Detach());

	LOGFONT lf;
	afxGlobalData.fontRegular.GetLogFont(&lf);

	NONCLIENTMETRICS info;
	info.cbSize = sizeof(info);

	afxGlobalData.GetNonClientMetrics(info);

	lf.lfHeight = info.lfMenuFont.lfHeight;
	lf.lfWeight = info.lfMenuFont.lfWeight;
	lf.lfItalic = info.lfMenuFont.lfItalic;

	m_fntPropList.CreateFontIndirect(&lf);

	m_wndPropList.SetFont(&m_fntPropList);
}

LRESULT CRegisterWnd::OnUpdateRegisterUI(WPARAM wparam,LPARAM lparam)
{
	CSDebuggerDoc* pDoc = GetActiveDocument();
	if(pDoc)
	{
		CONTEXT context;
		if(pDoc->GetDebugThreadContext(&context))
		{
			CMFCPropertyGridProperty *pGroupStandRegister = m_wndPropList.GetProperty(0);
			int subcount = pGroupStandRegister->GetSubItemsCount();
			for(int i=0;i<subcount;i++)
			{
				CMFCPropertyGridProperty *regProperty = pGroupStandRegister->GetSubItem(i);
				if(wcscmp(regProperty->GetName(),L"EAX")==0)
				{
					CString val;
					val.Format(L"%08X",context.Eax);
					regProperty->SetValue(val.GetBuffer());
				}
				if(wcscmp(regProperty->GetName(),L"EBX")==0)
				{
					CString val;
					val.Format(L"%08X",context.Ebx);
					regProperty->SetValue(val.GetBuffer());
				}
				if(wcscmp(regProperty->GetName(),L"ECX")==0)
				{
					CString val;
					val.Format(L"%08X",context.Ecx);
					regProperty->SetValue(val.GetBuffer());
				}
				if(wcscmp(regProperty->GetName(),L"EDX")==0)
				{
					CString val;
					val.Format(L"%08X",context.Edx);
					regProperty->SetValue(val.GetBuffer());
				}
				if(wcscmp(regProperty->GetName(),L"ESP")==0)
				{
					CString val;
					val.Format(L"%08X",context.Esp);
					regProperty->SetValue(val.GetBuffer());
				}
				if(wcscmp(regProperty->GetName(),L"EBP")==0)
				{
					CString val;
					val.Format(L"%08X",context.Ebp);
					regProperty->SetValue(val.GetBuffer());
				}
				if(wcscmp(regProperty->GetName(),L"ESI")==0)
				{
					CString val;
					val.Format(L"%08X",context.Esi);
					regProperty->SetValue(val.GetBuffer());
				}
				if(wcscmp(regProperty->GetName(),L"EDI")==0)
				{
					CString val;
					val.Format(L"%08X",context.Edi);
					regProperty->SetValue(val.GetBuffer());
				}
				if(wcscmp(regProperty->GetName(),L"EIP")==0)
				{
					CString val;
					val.Format(L"%08X",context.Eip);
					regProperty->SetValue(val.GetBuffer());
				}
			}

			CMFCPropertyGridProperty* pGroupFlagRegister = m_wndPropList.GetProperty(1);
			subcount = pGroupFlagRegister->GetSubItemsCount();
			for(int i=0;i<subcount;i++)
			{
				CMFCPropertyGridProperty *regProperty = pGroupFlagRegister->GetSubItem(i);
				if(wcscmp(regProperty->GetName(),L"CF")==0)
				{
					CString val;
					val.Format(L"%1d",(context.EFlags) & 0x1);
					regProperty->SetValue(val.GetBuffer());
				}
				if(wcscmp(regProperty->GetName(),L"PF")==0)
				{
					CString val;
					val.Format(L"%1d",(context.EFlags>>2) & 0x1);
					regProperty->SetValue(val.GetBuffer());
				}
				if(wcscmp(regProperty->GetName(),L"AF")==0)
				{
					CString val;
					val.Format(L"%1d",(context.EFlags>>4) & 0x1);
					regProperty->SetValue(val.GetBuffer());
				}
				if(wcscmp(regProperty->GetName(),L"ZF")==0)
				{
					CString val;
					val.Format(L"%1d",(context.EFlags>>6) & 0x1);
					regProperty->SetValue(val.GetBuffer());
				}
				if(wcscmp(regProperty->GetName(),L"SF")==0)
				{
					CString val;
					val.Format(L"%1d",(context.EFlags>>7) & 0x1);
					regProperty->SetValue(val.GetBuffer());
				}
				if(wcscmp(regProperty->GetName(),L"DF")==0)
				{
					CString val;
					val.Format(L"%1d",(context.EFlags>>10) & 0x1);
					regProperty->SetValue(val.GetBuffer());
				}
				if(wcscmp(regProperty->GetName(),L"0F")==0)
				{
					CString val;
					val.Format(L"%1d",(context.EFlags>>11) & 0x1);
					regProperty->SetValue(val.GetBuffer());
				}
			}
		}
	}
	return TRUE;
}
