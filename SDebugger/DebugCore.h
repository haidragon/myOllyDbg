
#pragma once

#include<Tlhelp32.h>
#include<deque>
#include<vector>
#include "Disassembly.h"
using namespace std;
//��Ȩ����
BOOL EnableDebugPriv();
//�����¼��ص�
BOOL OnProcessCreated(const CREATE_PROCESS_DEBUG_INFO*);
BOOL OnThreadCreated(const CREATE_THREAD_DEBUG_INFO*);
BOOL OnException(const EXCEPTION_DEBUG_INFO*);
BOOL OnProcessExited(const EXIT_PROCESS_DEBUG_INFO*);
BOOL OnThreadExited(const EXIT_THREAD_DEBUG_INFO*);
BOOL OnOutputDebugString(const OUTPUT_DEBUG_STRING_INFO*);
BOOL OnRipEvent(const RIP_INFO*);
BOOL OnDllLoaded(const LOAD_DLL_DEBUG_INFO*);
BOOL OnDllUnloaded(const UNLOAD_DLL_DEBUG_INFO*);
//����ʱ�䴦��
BOOL DispatchDebugEvent(const DEBUG_EVENT* pDebugEvent);
//�����߳�
void DebugThreadA(void* param);
void DebugThreadG(void* param);
//���ӵ��Խ���
void BeginBebugThread(LPPROCESSENTRY32 pe);
//���������߳�
void BeginBebugThread(LPCTSTR lpszPathName);
//���������߳�
void EndBebugThread();
//�û��¼�
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
//��ȡ�����߳��豸������
BOOL GetDebuggerContext(HANDLE hThread, CONTEXT* pContext);
//���õ����߳��豸������
BOOL SetDebuggerContext(HANDLE hThread, CONTEXT* pContext);
//��������
BOOL SingleDebug(HANDLE hThread);

typedef struct _InvokeStack
{
	UInt64 AddrPC;
	UInt64 AddrStack;
}InvokeStack;

typedef deque<InvokeStack> StackDeque;
//�������ö�ջ
void EmunInvokeStack(HANDLE hProcess, HANDLE hThread, StackDeque& stackdeque);

//��ȡ��ǰ�̶߳�ջ��Ϣ
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