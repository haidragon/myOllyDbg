#include "stdafx.h"
#include "Message.h"

HWND g_opWnd = NULL;
HWND g_mainWnd = NULL;
HWND g_mainView = NULL;
HWND g_registerWnd = NULL;
HWND g_moduleView = NULL;
HWND g_stackWnd = NULL;
HWND g_executeModuleWnd = NULL;

void InitOPHwnd(HWND hwnd)
{
	g_opWnd = hwnd;
}

void InitMainHwnd(HWND hwnd)
{
	g_mainWnd = hwnd;
}

void InitMainView(HWND hwnd)
{
	g_mainView = hwnd;
}

void InitRegistHwnd(HWND hwnd)
{
	g_registerWnd = hwnd;
}

void InitModuleView(HWND hwnd)
{
	g_moduleView = hwnd;
}

void InitStackWnd(HWND hwnd)
{
	g_stackWnd = hwnd;
}

void InitExecuteModuleWnd(HWND hwnd)
{
	g_executeModuleWnd = hwnd;
}

LRESULT SendUserMsg(
					__in UINT Msg,
					__in WPARAM wParam,
					__in LPARAM lParam
					)
{
	HWND hwnd = NULL;
	switch (Msg)
	{
	case WM_SELFOUTPUT :
	case WM_DEBUGOUTPUT :
		hwnd = g_opWnd;
		break;
	case WM_ATTACHPROCESS :
	case WM_EXECUTE_MODULE_INFO:
		hwnd = g_mainWnd;
		break;
	case WM_DEBUGGERTHREAD :
	case WM_DISASSEMBLY :
	case WM_SETPOINTERADDRESS :
		hwnd = g_mainView;
		break;
	case WM_UPDATEUIREGISTER :
		hwnd = g_registerWnd;
		break;
	case WM_UPDATEMODULEVIEW :
		hwnd = g_moduleView;
		break;
	case WM_UPDATEINVOKESTACK :
		hwnd = g_stackWnd;
		break;
	case WM_UPDATEEXECUTEMODULEVIEW:
		hwnd = g_executeModuleWnd;
		break;
	}
	if(hwnd!=NULL)
	{
		return SendMessage(hwnd,Msg,wParam,lParam);
	}
	return RESULT_ERROR;
}

#define PDSTRBUFF 1024

LRESULT PrintDbgStringW(UINT msg, wchar_t * info, va_list args)
{
	if (NULL == info)
	{
		return RESULT_ERROR;
	}

	wchar_t strData[PDSTRBUFF];

	::_vsnwprintf(strData, PDSTRBUFF, info, args);

	OutputDebugStringW(strData);

	va_end(args);

	return SendUserMsg(msg,(WPARAM)strData,PDSTRBUFF);
}

LRESULT PrintDbgStringA(UINT msg, char * info, va_list args)
{
	if (NULL == info)
	{
		return RESULT_ERROR;
	}

	char strData[PDSTRBUFF];

	::vsprintf(strData, info, args);

	OutputDebugStringA(strData);

	va_end(args);

	wchar_t wstrData[PDSTRBUFF];

	::MultiByteToWideChar(CP_ACP, 0, strData, -1, wstrData, PDSTRBUFF);

	return SendUserMsg(msg,(WPARAM)wstrData,PDSTRBUFF);
}

LRESULT PrintSelfStrW(wchar_t * info, ...)
{
	va_list	args;
	va_start(args, info);
	return PrintDbgStringW(WM_SELFOUTPUT, info, args);	
}

LRESULT PrintSelfStrA(char * info,...)
{
	va_list	args;
	va_start(args, info);
	return PrintDbgStringA(WM_SELFOUTPUT, info, args);	
}

LRESULT PrintDebugStrW(wchar_t * info, ...)
{
	va_list	args;
	va_start(args, info);
	return PrintDbgStringW(WM_DEBUGOUTPUT, info, args);
}

LRESULT PrintDebugStrA(char * info,...)
{
	va_list	args;
	va_start(args, info);
	return PrintDbgStringA(WM_DEBUGOUTPUT, info, args);	
}