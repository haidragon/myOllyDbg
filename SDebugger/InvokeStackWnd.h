#pragma once
#include "afxdockablepane.h"
#include "CSTabCtrl.h"

class CStackList : public  CListCtrl
{
public:
	CStackList(void);
	~CStackList(void);
	void ScrollV(int itemindex);
protected:
	DECLARE_MESSAGE_MAP()
};

class CInvokeStackWnd;

class CStackTabCtrl : public CSTabCtrl
{
public:
	CStackTabCtrl(void);
	~CStackTabCtrl(void);
	void SetInvokeStackWnd(CInvokeStackWnd* invokeStackWnd)
	{
		m_InvokeStackWnd = invokeStackWnd;
	}
protected:
	CInvokeStackWnd* m_InvokeStackWnd;
	afx_msg void OnNMCustomdrawThreadStackTable(NMHDR *pNMHDR, LRESULT *pResult);
	DECLARE_MESSAGE_MAP()
};

class CInvokeStackWnd : public CDockablePane
{
	friend CStackTabCtrl;
public:
	CInvokeStackWnd(void);
	~CInvokeStackWnd(void);

protected:
	CFont m_Font;
	CStackTabCtrl m_wndTabs;

	CStackList m_cInvokeList;
	CStackList m_cThreadList;

	ULONG_PTR m_current_esp;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV Ö§³Ö

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg LRESULT OnUpdateInvokeStack(WPARAM wparam,LPARAM lparam);
	DECLARE_MESSAGE_MAP()
};
