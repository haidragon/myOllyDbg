#pragma once
#include "SDebugger.h"
#include "SDebuggerDoc.h"
#include "SDebuggerView.h"

#include "ProcessBarDlg.h"

CSDebuggerView* GetActiveView();

CSDebuggerDoc* GetActiveDocument();

class CTask
{
public:
	CProcessBarDlg* ProcBarDlg;
	PVOID param1;
	PVOID param2;
	PVOID param3;
	PVOID param4;
	int value;
public:
	CTask()
	{
		ProcBarDlg = NULL;
		param1 = NULL;
		param2 = NULL;
		param3 = NULL;
		param4 = NULL;
		value = 0;
	}
	void SetProcessValue(int value)
	{
		this->value = value;
		if(ProcBarDlg)
		{
			ProcBarDlg->SetValue(value);
		}
	}
};

void ShowProcessBarDlg(PVOID fun,CTask &task, WCHAR* szTitle);

void SetProcessValue(HWND hwnd,int value);

