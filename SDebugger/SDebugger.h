
// SDebugger.h : SDebugger Ӧ�ó������ͷ�ļ�
//
#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"       // ������


// CSDebuggerApp:
// �йش����ʵ�֣������ SDebugger.cpp
//

class CSDebuggerApp : public CWinAppEx
{
public:
	CSDebuggerApp();


// ��д
public:
	virtual BOOL InitInstance();

// ʵ��
	BOOL  m_bHiColorIcons;

	virtual void PreLoadState();
	virtual void LoadCustomState();
	virtual void SaveCustomState();

	afx_msg void OnAppAbout();
	afx_msg void OnProcessNew();
	afx_msg void OnProcessOpen();

	DECLARE_MESSAGE_MAP()
};

extern CSDebuggerApp theApp;

