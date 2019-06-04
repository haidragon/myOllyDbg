#pragma once
#include "afxdockablepane.h"
#include "CSTabCtrl.h"
#include "GlobalApi.h"

class CExecuteModuleWnd : public CDockablePane
{
public:
	CExecuteModuleWnd(void);
	~CExecuteModuleWnd(void);
	void FillModuleView(ModuleMap& modulemap,CTask* pTask);
	static void WINAPI UpdateProcess(PVOID param);
protected:
	CFont m_Font;
	CListCtrl m_cExeModuleList;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV Ö§³Ö
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg LRESULT OnUpdateExecuteModuleView(WPARAM wparam,LPARAM lparam);
	DECLARE_MESSAGE_MAP()

public:
};