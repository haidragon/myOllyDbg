#include "stdafx.h"
#include "DebugCore.h"
#include "Message.h"
#include "process.h"
#include "PT.h"
#include "PETools.h"
#include "DbgHelp.h"
#include "InlineAsm32.h"

BOOL g_bWaitEvent = TRUE;
PROCESSENTRY32 g_CurrentPe = {0};
PROCESS_INFORMATION g_ProcessInfo = {0};
WCHAR g_CurrentFile[MAX_PATH] = {0};
HANDLE g_hProcess = NULL;

DWORD g_continueStatus = DBG_EXCEPTION_NOT_HANDLED;
DEBUGGERSTATUS g_debuggerStatus = STATUS_NONE;
HANDLE g_hUserEvent = NULL;

/**
* 提升当前进程权限函数("SeDebugPrivilege"读、写控制权限)
* @param void
* @return TRUE-成功；FALSE-失败
*/

BOOL EnableDebugPriv()
{
	HANDLE hToken;
	TOKEN_PRIVILEGES tkp;
	LUID Luid;

	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
	{
		return FALSE;
	}

	if (!LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &Luid ))
	{
		CloseHandle(hToken);
		return FALSE;
	}

	tkp.PrivilegeCount = 1;
	tkp.Privileges[0].Luid = Luid;
	tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	if (!AdjustTokenPrivileges(hToken, FALSE, &tkp, sizeof(tkp), NULL, NULL))
	{
		CloseHandle(hToken);
		return FALSE;
	}
	return TRUE;
}

void DebugThreadA(void* param)
{
	LPPROCESSENTRY32 pe =(LPPROCESSENTRY32)param;
	g_hProcess = OpenProcess(PROCESS_ALL_ACCESS,FALSE, pe->th32ProcessID);
	if(g_hProcess && DebugActiveProcess(pe->th32ProcessID))
	{
		PrintSelfStrW(L"Attach Process [%s] Success\r\n",pe->szExeFile);
		if(!CreateUsersEvent())
		{
			PrintSelfStrW(L"CreateUsersEvent failed\r\n");
			goto DTA_ERROR;
		}
		SendUserMsg(WM_ATTACHPROCESS, pe->th32ProcessID ,(LPARAM)g_hProcess);
		g_bWaitEvent = TRUE;
		g_ProcessInfo.dwProcessId = pe->th32ProcessID;
		g_ProcessInfo.hProcess = g_hProcess;
		SendUserMsg(WM_DEBUGGERTHREAD,(WPARAM)g_ProcessInfo.dwProcessId,NULL);
		DEBUG_EVENT debugEvent;
		while (g_bWaitEvent == TRUE && WaitForDebugEvent(&debugEvent, INFINITE)) {
			if(DispatchDebugEvent(&debugEvent))
			{
				g_continueStatus = DBG_EXCEPTION_NOT_HANDLED;
			}
			else
			{
				if(debugEvent.dwDebugEventCode != OUTPUT_DEBUG_STRING_EVENT)
				{
					WaitForSingleObject(g_hUserEvent,INFINITE);
					g_debuggerStatus = STATUS_RUN;
				}
			}
			if (g_bWaitEvent == TRUE) {
				ContinueDebugEvent(debugEvent.dwProcessId, debugEvent.dwThreadId, g_continueStatus);
			}
			else {
				break;
			}
		}
DTA_ERROR:
		g_hProcess = OpenProcess(PROCESS_ALL_ACCESS,FALSE, pe->th32ProcessID);
		if(g_hProcess)
		{
			DebugActiveProcessStop(pe->th32ProcessID);
			CloseHandle(g_hProcess);
		}
		memset(&g_ProcessInfo,0,sizeof(PROCESS_INFORMATION));
		g_hProcess = NULL;
	}
	else
	{
		SendUserMsg(WM_ATTACHPROCESS,NULL,FALSE);
	}
	_endthread();
}

void DebugThreadG(void* param)
{
	STARTUPINFO si = { 0 };
	si.cb = sizeof(si);

	PROCESS_INFORMATION pi = { 0 };

	if (CreateProcess(
		g_CurrentFile,
		NULL,
		NULL,
		NULL,
		FALSE,
		DEBUG_ONLY_THIS_PROCESS | CREATE_NEW_CONSOLE | CREATE_SUSPENDED,
		NULL,
		NULL,
		&si,
		&pi) == FALSE) {

			PrintSelfStrW(L"CreateProcess failed: [%d] \r\n", GetLastError());
			SendUserMsg(WM_ATTACHPROCESS,(WPARAM)g_CurrentFile,FALSE);
			return;
	}
	g_debuggerStatus = STATUS_SUSPENDED;
	g_hProcess = pi.hProcess;
	memcpy(&g_ProcessInfo,&pi,sizeof(PROCESS_INFORMATION));
	PrintSelfStrW(L"Create Process [%s] Success\r\n",g_CurrentFile);
	if(!CreateUsersEvent())
	{
		PrintSelfStrW(L"CreateUsersEvent failed\r\n");
		goto DTG_ERROR;
	}
	SendUserMsg(WM_ATTACHPROCESS,(WPARAM)pi.dwProcessId, (LPARAM)g_hProcess);
	g_bWaitEvent = TRUE;
	SendUserMsg(WM_DEBUGGERTHREAD,(WPARAM)pi.dwProcessId, (LPARAM)pi.dwThreadId);
	DEBUG_EVENT debugEvent;
	while (g_bWaitEvent == TRUE && WaitForDebugEvent(&debugEvent, INFINITE)) {
		if(DispatchDebugEvent(&debugEvent))
		{
			g_continueStatus = DBG_EXCEPTION_NOT_HANDLED;
		}
		else
		{
			if(debugEvent.dwDebugEventCode != OUTPUT_DEBUG_STRING_EVENT)
			{
				WaitForSingleObject(g_hUserEvent,INFINITE);
				g_debuggerStatus = STATUS_RUN;
			}
		}

		if (g_bWaitEvent == TRUE) {
			ContinueDebugEvent(debugEvent.dwProcessId, debugEvent.dwThreadId, g_continueStatus);
		}
		else {
			break;
		}
	}
DTG_ERROR:
	g_hProcess = NULL;
	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);
	_endthread();
}

void BeginBebugThread(LPPROCESSENTRY32 pe)
{
	memcpy(&g_CurrentPe,pe,sizeof(PROCESSENTRY32));
	_beginthread(DebugThreadA,0,(void *)&g_CurrentPe);
}

void BeginBebugThread(LPCTSTR lpszPathName)
{
	wcscpy(g_CurrentFile,lpszPathName);
	_beginthread(DebugThreadG,0,(void *)g_CurrentFile);
}

//结束调试线程
void EndBebugThread()
{
	g_bWaitEvent = FALSE;
	SetUserEvent();
	HANDLE hThread = CreateRemoteThread(g_hProcess,
		NULL,
		0,
		(LPTHREAD_START_ROUTINE)GetLastError,
		NULL,
		0,
		NULL);
	CloseHandle(hThread);
	//启动一个远程线程试调试线程结束而不结束调试进程
}

BOOL OnProcessCreated(const CREATE_PROCESS_DEBUG_INFO* pInfo) 
{
	CloseHandle(pInfo->hFile);
	CloseHandle(pInfo->hThread);
	CloseHandle(pInfo->hProcess);
	PrintSelfStrW(L"Debugger was created.\n");
	return TRUE;
}

BOOL OnThreadCreated(const CREATE_THREAD_DEBUG_INFO* pInfo) 
{
	CloseHandle(pInfo->hThread);
	PrintSelfStrW(L"A new thread was created.\n");
	return TRUE;
}

BOOL OnException(const EXCEPTION_DEBUG_INFO* pInfo, DWORD ProcessId, DWORD ThreadId) {

	PrintSelfStrW(L"An exception was occured.\r\n");
	PrintSelfStrW(L"Exception code: %X \r\n",pInfo->ExceptionRecord.ExceptionCode);
	PrintSelfStrW(L"Address: %X \r\n",pInfo->ExceptionRecord.ExceptionAddress);

	if (pInfo->dwFirstChance == TRUE)
	{
		if(STATUS_BREAKPOINT == pInfo->ExceptionRecord.ExceptionCode)//int 3 断点
		{
			g_continueStatus = DBG_EXCEPTION_HANDLED;
		}
		else
		{
			g_continueStatus = DBG_EXCEPTION_NOT_HANDLED;
		}
		if(STATUS_SINGLE_STEP == pInfo->ExceptionRecord.ExceptionCode)
		{
			g_continueStatus = DBG_CONTINUE;
		}
		SendUserMsg(WM_SETPOINTERADDRESS, (WPARAM)pInfo->ExceptionRecord.ExceptionAddress,0);
		SendUserMsg(WM_DISASSEMBLY, (LPARAM)pInfo->ExceptionRecord.ExceptionAddress, (WPARAM)pInfo->ExceptionRecord.ExceptionCode);
		SendUserMsg(WM_DEBUGGERTHREAD,(WPARAM)ProcessId, (LPARAM)ThreadId);
		PrintSelfStrW(L"First chance.\r\n");
	}
	else 
	{
		g_continueStatus = DBG_EXCEPTION_HANDLED;
		PrintSelfStrW(L"Second chance.\r\n");
	}
	g_debuggerStatus = STATUS_INTERRUPTED;
	return FALSE;
}

BOOL OnProcessExited(const EXIT_PROCESS_DEBUG_INFO* pInfo) 
{	
	PrintSelfStrW(L"Process was exited. ExitCode [%d]\n", pInfo->dwExitCode);
	g_bWaitEvent = FALSE;
	g_debuggerStatus = STATUS_NONE;
	g_continueStatus = DBG_EXCEPTION_NOT_HANDLED;
	return TRUE;
}

BOOL OnThreadExited(const EXIT_THREAD_DEBUG_INFO* pInfo) 
{
	PrintSelfStrW(L"A thread was exited. ExitCode [%d]\n", pInfo->dwExitCode);
	return TRUE;
}

BOOL OnOutputDebugString(const OUTPUT_DEBUG_STRING_INFO* pInfo) {

	PrintSelfStrW(L"Debuggee outputed debug string.\r\n");
	BYTE* pBuffer = (BYTE*)malloc(pInfo->nDebugStringLength);

	SIZE_T bytesRead;

	ReadProcessMemory(
		g_hProcess,
		pInfo->lpDebugStringData,
		pBuffer, 
		pInfo->nDebugStringLength,
		&bytesRead);

	PrintDebugStrA("%s \r\n", pBuffer);
	free(pBuffer);
	g_continueStatus = DBG_EXCEPTION_HANDLED;
	return FALSE;
}

BOOL OnRipEvent(const RIP_INFO* pInfo) 
{
	PrintSelfStrW(L"A RIP_EVENT occured.\r\n");
	return TRUE;
}

BOOL OnDllLoaded(const LOAD_DLL_DEBUG_INFO* pInfo) 
{
	BYTE szBuffer[MAX_PATH * 2 + 4] = {0};
	WCHAR szModuleName[MAX_PATH] = {0};
	WCHAR szPathName[MAX_PATH] = {0};
	MEMORY_BASIC_INFORMATION mbi;
	MODULE_INFORMATION mi;
	PUNICODE_STRING usSectionName;
	ULONG_PTR dwStartAddr = (ULONG_PTR)pInfo->lpBaseOfDll;
	ZWQUERYVIRTUALMEMORY fnZwQueryVirtualMemory;
	fnZwQueryVirtualMemory = (ZWQUERYVIRTUALMEMORY)::GetProcAddress(GetModuleHandleA("ntdll.dll"),"ZwQueryVirtualMemory" );
	if(g_hProcess && fnZwQueryVirtualMemory)
	{
		if(fnZwQueryVirtualMemory(g_hProcess, (PVOID)dwStartAddr, MemoryBasicInformation,&mbi,sizeof(mbi),0) >= 0)
		{
			if(mbi.Type == MEM_IMAGE)
			{
				if(fnZwQueryVirtualMemory(g_hProcess,(PVOID)dwStartAddr,MemorySectionName,szBuffer,sizeof(szBuffer),0) >= 0)
				{
					memset(&mi,0,sizeof(MODULE_INFORMATION));
					memcpy(&mi,&mbi,sizeof(MEMORY_BASIC_INFORMATION));
					usSectionName = (PUNICODE_STRING)szBuffer;
					if( _wcsnicmp(szModuleName, usSectionName->Buffer, usSectionName->Length / sizeof(WCHAR)) )
					{
						wcsncpy_s(szModuleName, usSectionName->Buffer, usSectionName->Length / sizeof(WCHAR) );
						szModuleName[usSectionName->Length / sizeof(WCHAR)] = UNICODE_NULL;
						DeviceName2PathName(szPathName, szModuleName);
						wcscpy_s(mi.szPathName,szPathName);
						DeviceName2ModuleName(szModuleName);
						SendUserMsg(WM_EXECUTE_MODULE_INFO,(WPARAM)&szModuleName,(LPARAM)&mi);
					}
				}
			}
		}
	}
	PrintSelfStrW(L"A dll was loaded. \r\n");
	return TRUE;
}

BOOL OnDllUnloaded(const UNLOAD_DLL_DEBUG_INFO* pInfo) 
{
	PrintSelfStrW(L"A dll was unloaded.\r\n");
	return TRUE;
}

BOOL DispatchDebugEvent(const DEBUG_EVENT* pDebugEvent) 
{
	switch (pDebugEvent->dwDebugEventCode) {

		case CREATE_PROCESS_DEBUG_EVENT:
			return OnProcessCreated(&pDebugEvent->u.CreateProcessInfo);

		case CREATE_THREAD_DEBUG_EVENT:
			return OnThreadCreated(&pDebugEvent->u.CreateThread);

		case EXCEPTION_DEBUG_EVENT:
			return OnException(&pDebugEvent->u.Exception,pDebugEvent->dwProcessId,pDebugEvent->dwThreadId);

		case EXIT_PROCESS_DEBUG_EVENT:
			return OnProcessExited(&pDebugEvent->u.ExitProcess);

		case EXIT_THREAD_DEBUG_EVENT:
			return OnThreadExited(&pDebugEvent->u.ExitThread);

		case LOAD_DLL_DEBUG_EVENT:
			return OnDllLoaded(&pDebugEvent->u.LoadDll);

		case OUTPUT_DEBUG_STRING_EVENT:
			return OnOutputDebugString(&pDebugEvent->u.DebugString);

		case RIP_EVENT:
			return OnRipEvent(&pDebugEvent->u.RipInfo);

		case UNLOAD_DLL_DEBUG_EVENT:
			return OnDllUnloaded(&pDebugEvent->u.UnloadDll);

		default:
			PrintSelfStrW(L"Unknown debug event.\n");
			return FALSE;
	}
}

BOOL CreateUsersEvent()
{
	g_hUserEvent = CreateEvent(NULL, FALSE, FALSE, L"User");
	if(g_hUserEvent!=NULL){
		return TRUE;
	}
	return FALSE;
}

void SetUserEvent()
{
	SetEvent(g_hUserEvent);
}

void SetDebuggerStatus(DEBUGGERSTATUS status)
{
	g_debuggerStatus = status;
}

DEBUGGERSTATUS GetDebuggerStatus()
{
	return g_debuggerStatus;
}

//获取被调试线程的上下文环境。
BOOL GetDebuggerContext(HANDLE hThread, CONTEXT* pContext) 
{
	pContext->ContextFlags = CONTEXT_FULL;

	if (GetThreadContext(hThread, pContext) == FALSE) {

		PrintSelfStrW(L"GetThreadContext failed: %X \r\n", GetLastError());
		return FALSE;
	}

	return TRUE;
}

BOOL SetDebuggerContext(HANDLE hThread, CONTEXT* pContext)
{
	if(!SetThreadContext(hThread,pContext))
	{
		PrintSelfStrW(L"SetThreadContext failed: %X \r\n", GetLastError());
		return FALSE;
	}
	return TRUE;
}

BOOL SingleDebug(HANDLE hThread)
{
	CONTEXT context;
	if(GetDebuggerContext(hThread,&context))
	{
		context.EFlags |= 0x100;
		if(SetDebuggerContext(hThread,&context))
		{
			return TRUE;
		}
	}
	return FALSE;
}

void EmunInvokeStack(HANDLE hProcess, HANDLE hThread, StackDeque& stackdeque)
{
	CONTEXT context;
	if(!GetDebuggerContext(hThread,&context))
	{
		return;
	}

	STACKFRAME64 stackFrame = { 0 };
	stackFrame.AddrPC.Mode = AddrModeFlat;
	stackFrame.AddrPC.Offset = context.Eip;
	stackFrame.AddrStack.Mode = AddrModeFlat;
	stackFrame.AddrStack.Offset = context.Esp;
	stackFrame.AddrFrame.Mode = AddrModeFlat;
	stackFrame.AddrFrame.Offset = context.Ebp;

	while (true) {

		//获取栈帧
		if (StackWalk64(
			IMAGE_FILE_MACHINE_I386,
			hProcess,
			hThread,
			&stackFrame,
			&context,
			NULL,
			SymFunctionTableAccess64,
			SymGetModuleBase64,
			NULL) == FALSE) {

				break;
		}
		InvokeStack is;
		is.AddrPC = stackFrame.AddrPC.Offset;
		is.AddrStack = stackFrame.AddrStack.Offset;
		stackdeque.push_back(is);
	}
}

void GetStackInformation(HANDLE hProcess, ULONG_PTR TebBaseAddress, OUT PNT_TIB pNTB)
{
	SIZE_T numberofread;
	if(ReadProcessMemory(hProcess,(LPVOID)TebBaseAddress,pNTB,sizeof(NT_TIB),&numberofread))
	{
		PrintSelfStrA("top %p bottom %p \r\n",pNTB->StackBase,pNTB->StackLimit);
	}
}

BOOL SetBreakPoint(IN OUT PBreakPoint pBP)
{
	switch(pBP->type)
	{
	case 1:
break;
	}
	return TRUE;
}