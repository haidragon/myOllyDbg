
// SDebuggerDoc.h : CSDebuggerDoc 类的接口
//

#pragma once

#define DISAM_BUF_SIZE 0x80

#include "Disassembly.h"
#include "PT.h"
#include "PETools.h"
#include "DebugCore.h"

class CSDebuggerDoc : public CDocument
{
protected: // 仅从序列化创建
	CSDebuggerDoc();
	DECLARE_DYNCREATE(CSDebuggerDoc)

// 属性
public:
	HANDLE m_hProcess;
	HANDLE m_hThread;
	DWORD m_dProcessId;
// 操作
public:
	void SetDebugThread(HANDLE hThread);
	void SetDebugProcess(HANDLE hProcess);
	void SetDebugProcessId(DWORD ProcessId);
	DWORD GetDebugProcessId();
	BOOL Disassemby(ULONG_PTR Virtual_Address,BOOL IsAdd = FALSE);
	HANDLE GetDebugThread();
	HANDLE GetDebugProcess();
	DWORD GetMainThreadID();
	BOOL GetDebugThreadContext(CONTEXT* pContext);
	void ClearDisasmMap(DisasmMap &disasmm,UInt64 start = 0,UInt64 end = 0);
	DisasmMap& GetDisasmInfoMap();
	ThreadList& GetThreadList();
	UInt64 GetNextCodeAdress(UInt64 codeadress);
	UInt64 GetPrevCodeAdress(UInt64 codeadress);
	void ClearModuleMap();
	void ClearStackDeque();
	ModuleMap& GetModuleMap();
	StackDeque& GetStackDeque();
	NT_TIB& GetCurrentNT_TIB();
	BOOL IsContainedBP(UInt64 adress, BPType type);
// 重写
public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	virtual void OnCloseDocument();
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
// 实现
public:
	virtual ~CSDebuggerDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	BYTE m_disasmbuf[DISAM_BUF_SIZE];
	DisasmMap m_disasmm;
	ThreadList m_threadList;
	ModuleMap m_modulemap;
	StackDeque m_stackdeque;
	NT_TIB m_nt_tib;
	BPVector m_bpvector;
// 生成的消息映射函数
protected:
	DECLARE_MESSAGE_MAP()
};


