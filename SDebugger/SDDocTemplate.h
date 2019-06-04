#pragma once
#include "afxwin.h"

class SDDocTemplate : public CMultiDocTemplate
{
public:
	SDDocTemplate(UINT nIDResource, CRuntimeClass* pDocClass,CRuntimeClass* pFrameClass, CRuntimeClass* pViewClass);
	~SDDocTemplate(void);
};
