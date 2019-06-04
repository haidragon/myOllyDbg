#include "StdAfx.h"
#include "SDDocTemplate.h"

SDDocTemplate::SDDocTemplate(UINT nIDResource, CRuntimeClass* pDocClass,CRuntimeClass* pFrameClass, CRuntimeClass* pViewClass) 
:CMultiDocTemplate(nIDResource,pDocClass,pFrameClass,pViewClass)
{

}

SDDocTemplate::~SDDocTemplate(void)
{
}
