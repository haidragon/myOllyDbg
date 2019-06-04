#include "stdafx.h"
#include "SDebugger.h"
#include "SDebuggerDoc.h"
#include "SDebuggerView.h"
#include "GlobalApi.h"
#include "Message.h"

#include "ProcessBarDlg.h"

CSDebuggerView* GetActiveView()
{
	CMDIFrameWnd *pFrame = (CMDIFrameWnd*)AfxGetApp()->GetMainWnd();

	// Get the active MDI child window.
	CMDIChildWnd *pChild = (CMDIChildWnd*)pFrame->GetActiveFrame();
	if(pChild)
	{
		CSDebuggerView *pView = (CSDebuggerView*)pChild->GetActiveView();
		return pView;
	}

	return NULL;
}

CSDebuggerDoc* GetActiveDocument()
{
	CSDebuggerView* pView = GetActiveView();
	if(pView)
	{
		return pView->GetDocument();
	}
	return NULL;
}

void ShowProcessBarDlg(PVOID fun,CTask &task, WCHAR* szTitle)
{
	HANDLE hThread;
	DWORD dThreadId;
	CProcessBarDlg ProcBarDlg(GetActiveView());
	ProcBarDlg.SetText(szTitle);
	task.ProcBarDlg = &ProcBarDlg;
	hThread = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)fun,&task,0,&dThreadId);
	CloseHandle(hThread);
	ProcBarDlg.DoModal();
}
