
#pragma once

/////////////////////////////////////////////////////////////////////////////
// COutputList ����
#include "CSTabCtrl.h"

class COutputList : public CListBox
{
// ����
public:
	COutputList();

// ʵ��
public:
	virtual ~COutputList();

protected:
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnEditCopy();
	afx_msg void OnEditClear();
	afx_msg void OnViewOutput();

	DECLARE_MESSAGE_MAP()
};

class COutputWnd : public CDockablePane
{
// ����
public:
	COutputWnd();

// ����
protected:
	CFont m_Font;

	CSTabCtrl	m_wndTabs;

	COutputList m_wndOutputSelf;
	COutputList m_wndOutputDebug;
	COutputList m_wndOutputAll;

protected:
	void AdjustHorzScroll(CListBox& wndListBox);

// ʵ��
public:
	virtual ~COutputWnd();

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg LRESULT SelfOutPut(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT DebugOutPut(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT AllOutPut(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
};

