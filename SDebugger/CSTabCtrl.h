#pragma once

class CSTabCtrl : public CMFCTabCtrl
{
public:
	CSTabCtrl();
protected:
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
};