
#pragma once

#include<Tlhelp32.h>
#include<deque>
#include<vector>
#include "Disassembly.h"
using namespace std;
//提权代码
BOOL EnableDebugPriv();
//调试事件回调
BOOL OnProcessCreated(const CREATE_PROCESS_DEBUG_INFO*);
BOOL OnThreadCreated(const CREATE_THREAD_DEBUG_INFO*);
BOOL OnException(const EXCEPTION_DEBUG_INFO*);
BOOL OnProcessExited(const EXIT_PROCESS_DEBUG_INFO*);
BOOL OnThreadExited(const EXIT_THREAD_DEBUG_INFO*);
BOOL OnOutputDebugString(const OUTPUT_DEBUG_STRING_INFO*);
BOOL OnRipEvent(const RIP_INFO*);
BOOL OnDllLoaded(const LOAD_DLL_DEBUG_INFO*);
BOOL OnDllUnloaded(const UNLOAD_DLL_DEBUG_INFO*);
//调试时间处理
BOOL DispatchDebugEvent(const DEBUG_EVENT* pDebugEvent);
//调试线程
void DebugThreadA(void* param);
void DebugThreadG(void* param);
//附加调试进程
void BeginBebugThread(LPPROCESSENTRY32 pe);
//创建调试线程
void BeginBebugThread(LPCTSTR lpszPathName);
//结束调试线程
void EndBebugThread();
//用户事件
BOOL CreateUsersEvent();
void SetUserEvent();

enum DEBUGGERSTATUS
{
	STATUS_NONE,
	STATUS_RUN,
	STATUS_SUSPENDED,
	STATUS_PAUSE,
	STATUS_INTERRUPTED
};

DEBUGGERSTATUS GetDebuggerStatus();
void SetDebuggerStatus(DEBUGGERSTATUS status);
//获取调试线程设备上下文
BOOL GetDebuggerContext(HANDLE hThread, CONTEXT* pContext);
//设置调试线程设备上下文
BOOL SetDebuggerContext(HANDLE hThread, CONTEXT* pContext);
//单步调试
BOOL SingleDebug(HANDLE hThread);

typedef struct _InvokeStack
{
	UInt64 AddrPC;
	UInt64 AddrStack;
}InvokeStack;

typedef deque<InvokeStack> StackDeque;
//遍历调用堆栈
void EmunInvokeStack(HANDLE hProcess, HANDLE hThread, StackDeque& stackdeque);

//获取当前线程堆栈信息
void GetStackInformation(HANDLE hProcess, ULONG_PTR TebBaseAddress, OUT PNT_TIB pNTB);

enum BPType
{
	SoftBreakPoint,
	HardBreakPoint
};

typedef struct _BreakPoint
{
	BPType type;
	UInt64 address;
}BreakPoint,*PBreakPoint;

typedef vector<BreakPoint> BPVector;

BOOL SetBreakPoint(IN OUT PBreakPoint pBP);