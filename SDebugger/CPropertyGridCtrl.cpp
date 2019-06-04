#include "stdafx.h"

#include "CPropertyGridCtrl.h"
#include "Resource.h"


CPropertyGridCtrl::CPropertyGridCtrl()
{

}

CPropertyGridCtrl::~CPropertyGridCtrl()
{
	
}

void CPropertyGridCtrl::AdjustLayout()
{
	RECT box;
	GetWindowRect(&box);
	int width = box.right - box.left;
	if (width > 50 )
		m_nLeftColumnWidth = 50;  //set property column to one third of total width
	CMFCPropertyGridCtrl::AdjustLayout();

}