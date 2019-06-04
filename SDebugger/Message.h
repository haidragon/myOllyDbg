#ifndef _MESSAGE_H_
#define _MESSAGE_H_

void InitOPHwnd(HWND hwnd);
void InitMainHwnd(HWND hwnd);
void InitMainView(HWND hwnd);
void InitRegistHwnd(HWND hwnd);
void InitModuleView(HWND hwnd);
void InitStackWnd(HWND hwnd);
void InitExecuteModuleWnd(HWND hwnd);

#define RESULT_ERROR (LRESULT)-3

#define USER_MESSAGE WM_USER + 0x100

#define WM_ALLOUTPUT				USER_MESSAGE + 0
#define WM_SELFOUTPUT				USER_MESSAGE + 1
#define WM_DEBUGOUTPUT				USER_MESSAGE + 2

#define WM_ATTACHPROCESS			USER_MESSAGE + 3
#define WM_DEBUGGERTHREAD			USER_MESSAGE + 4

#define WM_UPDATEUIREGISTER			USER_MESSAGE + 5
#define WM_DISASSEMBLY				USER_MESSAGE + 7

#define WM_UPDATEMODULEVIEW			USER_MESSAGE + 8

#define WM_UPDATEINVOKESTACK		USER_MESSAGE + 9

#define WM_SETPOINTERADDRESS		USER_MESSAGE + 10

#define WM_EXECUTE_MODULE_INFO		USER_MESSAGE + 11

#define WM_UPDATEEXECUTEMODULEVIEW	USER_MESSAGE + 12

LRESULT SendUserMsg(
					__in UINT Msg,
					__in WPARAM wParam,
					__in LPARAM lParam
					);

LRESULT PrintDbgStringW(UINT msg, wchar_t * info, va_list args);
LRESULT PrintDbgStringA(UINT msg, char * info, va_list args);

LRESULT PrintDebugStrA(char * info,...);
LRESULT PrintDebugStrW(wchar_t * info, ...);
LRESULT PrintSelfStrA(char * info,...);
LRESULT PrintSelfStrW(wchar_t * info, ...);

#endif



