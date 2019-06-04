#include "stdafx.h"
#include "CSTabCtrl.h"

CSTabCtrl::CSTabCtrl()
{

}

BOOL CSTabCtrl::OnCommand(WPARAM wParam, LPARAM lParam)
{
	if ((HWND)lParam == m_btnScrollLeft.GetSafeHwnd())
	{
		int tab = GetActiveTab();
		if(tab>0)
		{
			SetActiveTab(--tab);
		}
	}
	else if ((HWND)lParam == m_btnScrollRight.GetSafeHwnd())
	{
		int tab = GetActiveTab();
		int size = GetTabsNum();
		if(tab<size-1)
		{
			SetActiveTab(++tab);
		}
	}
	else if ((HWND)lParam == m_btnScrollFirst.GetSafeHwnd())
	{
		SetActiveTab(0);
	}
	else if ((HWND)lParam == m_btnScrollLast.GetSafeHwnd())
	{
		SetActiveTab(GetTabsNum()-1);
	}

	return CMFCTabCtrl::OnCommand(wParam, lParam);
}