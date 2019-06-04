#pragma once

#include "ViewTree.h"
#include "PETools.h"
#include "GlobalApi.h"
// CModuleView
class CModuleToolBar : public CMFCToolBar
{
	virtual void OnUpdateCmdUI(CFrameWnd* /*pTarget*/, BOOL bDisableIfNoHndler)
	{
		CMFCToolBar::OnUpdateCmdUI((CFrameWnd*) GetOwner(), bDisableIfNoHndler);
	}

	virtual BOOL AllowShowOnList() const { return FALSE; }
};


class CModuleTree : public CViewTree
{
	// 构造
public:
	CModuleTree();

	// 重写
protected:
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);

	// 实现
public:
	virtual ~CModuleTree();

protected:
	DECLARE_MESSAGE_MAP()
};

class CModuleView : public CDockablePane
{
	DECLARE_DYNAMIC(CModuleView)

public:
	CModuleView();
	virtual ~CModuleView();

	void AdjustLayout();
	void OnChangeVisualStyle();
	void FillModuleView(ModuleMap& modulemap,CTask* pTask);
	void DisasmModuleFunction(CString modulename,CString functionname);
	static void WINAPI UpdateProcess(PVOID param);

protected:
	CModuleToolBar m_wndToolBar;
	CModuleTree m_wndClassView;
	CImageList m_ClassViewImages;
	UINT m_nCurrSort;
	
	//ModuleMap m_modulemap;

	// 重写
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnClassAddMemberFunction();
	afx_msg void OnClassAddMemberVariable();
	afx_msg void OnClassDefinition();
	afx_msg void OnClassProperties();
	afx_msg void OnNewFolder();
	afx_msg void OnPaint();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg LRESULT OnChangeActiveTab(WPARAM, LPARAM);
	afx_msg void OnSort(UINT id);
	afx_msg void OnUpdateSort(CCmdUI* pCmdUI);
	afx_msg LRESULT OnUpdateModuleView(WPARAM wparam,LPARAM lparam);
	afx_msg void OnNMDblclkTree(NMHDR *pNMHDR, LRESULT *pResult);

	DECLARE_MESSAGE_MAP()
};


