#pragma once

class CPropertyGridCtrl : public CMFCPropertyGridCtrl
{
public:
	// Group constructor
	CPropertyGridCtrl();

	virtual ~CPropertyGridCtrl();

	virtual void AdjustLayout();
};