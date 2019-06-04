
// SDebuggerDoc.cpp : CSDebuggerDoc 类的实现
//

#include "stdafx.h"
#include "SDebugger.h"

#include "SDebuggerDoc.h"
#include "DebugCore.h"
#include "Message.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CSDebuggerDoc

IMPLEMENT_DYNCREATE(CSDebuggerDoc, CDocument)

BEGIN_MESSAGE_MAP(CSDebuggerDoc, CDocument)
END_MESSAGE_MAP()


// CSDebuggerDoc 构造/析构

CSDebuggerDoc::CSDebuggerDoc()
{
	// TODO: 在此添加一次性构造代码
}

CSDebuggerDoc::~CSDebuggerDoc()
{
	ClearDisasmMap(m_disasmm);
	ClearModuleMap();
}

BOOL CSDebuggerDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: 在此添加重新初始化代码
	// (SDI 文档将重用该文档)

	return TRUE;
}

void CSDebuggerDoc::OnCloseDocument()
{
	CDocument::OnCloseDocument();
	EndBebugThread();
}

BOOL CSDebuggerDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
	BeginBebugThread(lpszPathName);
	return FALSE;
}

void CSDebuggerDoc::SetDebugThread(HANDLE hThread)
{
	m_hThread = hThread;
}

void CSDebuggerDoc::SetDebugProcess(HANDLE hProcess)
{
	m_hProcess = hProcess;
}

HANDLE CSDebuggerDoc::GetDebugProcess()
{
	return m_hProcess;
}

void CSDebuggerDoc::SetDebugProcessId(DWORD ProcessId)
{
	m_dProcessId = ProcessId;
}

DWORD CSDebuggerDoc::GetDebugProcessId()
{
	return m_dProcessId;
}

HANDLE CSDebuggerDoc::GetDebugThread()
{
	return m_hThread;
}

//获取被调试线程的上下文环境。
BOOL CSDebuggerDoc::GetDebugThreadContext(CONTEXT* pContext)
{
	return GetDebuggerContext(m_hThread,pContext);
}

BOOL CSDebuggerDoc::Disassemby(ULONG_PTR Virtual_Address,BOOL IsAdd)
{
	DisasmMap::iterator pos;
	SIZE_T NumberOfBytesRead;
	memset(m_disasmbuf,0,DISAM_BUF_SIZE);
	//读取当前地址之后DISAM_BUF_SIZE长度的内存
	if(ReadProcessMemory(m_hProcess,(LPCVOID)Virtual_Address,(LPVOID)m_disasmbuf,DISAM_BUF_SIZE,&NumberOfBytesRead))
	{
		if(!IsAdd)
		{
			ClearDisasmMap(m_disasmm);//清除反汇编容器
		}
		DisassembleCode((char *)m_disasmbuf,(char *)m_disasmbuf + NumberOfBytesRead, Virtual_Address, m_disasmm);//反汇编代码
		if(!IsAdd)
		{
			pos = m_disasmm.find(Virtual_Address);//查询是否有当前地址
			if(pos!=m_disasmm.end())
			{
				CDisasmInfo *pDI_O,*pDI_N = NULL;
				pDI_O = &(*pos).second;
				ULONG_PTR Prev_Virtual_Address_Start = Virtual_Address - DISAM_BUF_SIZE + pDI_O->m_size; // 构造当前地址之前的汇编起始头地址
				ULONG_PTR Prev_Virtual_Address_End = Virtual_Address + pDI_O->m_size;					//	构造当前地址之前的汇编起结束地址
				memset(m_disasmbuf,0,DISAM_BUF_SIZE);
				if(ReadProcessMemory(m_hProcess,(LPCVOID)Prev_Virtual_Address_Start,(LPVOID)m_disasmbuf,DISAM_BUF_SIZE,&NumberOfBytesRead))
				{
					ULONG_PTR index = 0;
					DisasmMap newdisasmm;
					while((Prev_Virtual_Address_Start+index)<Prev_Virtual_Address_End)
					{	
						DisassembleCode((char *)m_disasmbuf + index,(char *)m_disasmbuf + NumberOfBytesRead, Prev_Virtual_Address_Start + index, newdisasmm);
						pos = newdisasmm.find(Virtual_Address);//查找当前位置是否有反汇编结果
						if(pos!=newdisasmm.end())
						{
							pDI_N = &(*pos).second;
							if(strcmp(pDI_O->m_disasmstr,pDI_N->m_disasmstr)==0)//比对反汇编结果,看是否当前地址符合反汇编的结果
							{
								newdisasmm.erase(pos);	//删掉多余的当前行
								pos = newdisasmm.find(Prev_Virtual_Address_Start + index);//反汇编的第一行删掉保证正确性
								if(pos!=newdisasmm.end())
								{
									newdisasmm.erase(pos);
								}
								for(pos=newdisasmm.begin();pos!=newdisasmm.end();pos++)
								{
									if(Virtual_Address != (*pos).first)
									{
										m_disasmm[(*pos).first] = (*pos).second;
									}
								}
								newdisasmm.clear();
								break;
							}
							else
							{
								ClearDisasmMap(newdisasmm);
							}
						}
						else
						{
							ClearDisasmMap(newdisasmm);
						}
						index++;
					}
				}
			}
		}
		return TRUE;
	}
	return FALSE;
}
//清除当前地址开始的已存在的地址汇编信息
void CSDebuggerDoc::ClearDisasmMap(DisasmMap& disasmm,UInt64 start,UInt64 end)
{
	if(start == 0 && end == 0)
	{
		disasmm.clear();
	}
	else
	{
		UInt64 startadr = 0;
		DisasmMap::iterator pos;
		for(UInt64 i=start;i<=end;i++)
		{
			pos = disasmm.find(i);
			if(pos!=disasmm.end())
			{
				startadr = i;
				break;
			}
		}
		if(startadr)
		{
			UInt64 curadr = startadr;
			while(curadr<=end)
			{
				pos = disasmm.find(curadr);
				if(pos!=disasmm.end())
				{
					CDisasmInfo* di = &(*pos).second;
					curadr+=di->m_size;
					disasmm.erase(pos);
				}
				else
				{
					break;
				}
			}
		}
	}
}

UInt64 CSDebuggerDoc::GetNextCodeAdress(UInt64 codeadress)
{
	DisasmMap::iterator pos;
	pos = m_disasmm.find(codeadress);
	UInt64 curadr = codeadress;
	if(pos!=m_disasmm.end())
	{
		CDisasmInfo* di = &(*pos).second;
		curadr+=di->m_size;
		pos = m_disasmm.find(curadr);
		if(pos!=m_disasmm.end())
			return curadr;
	}
	return 0;
}

UInt64 CSDebuggerDoc::GetPrevCodeAdress(UInt64 codeadress)
{
	DisasmMap::iterator pos;
	pos = m_disasmm.find(codeadress);
	UInt64 curadr = codeadress;
	if(pos!=m_disasmm.end())
	{
		for(curadr--;curadr>=codeadress-16;curadr--)
		{
			pos = m_disasmm.find(curadr);
			if(pos!=m_disasmm.end())
				return curadr;
		}
		pos = m_disasmm.find(curadr);
		if(pos!=m_disasmm.end())
			return curadr;
	}
	return 0;
}

DisasmMap& CSDebuggerDoc::GetDisasmInfoMap()
{
	return m_disasmm;
}

ThreadList& CSDebuggerDoc::GetThreadList()
{
	return m_threadList;
}

DWORD CSDebuggerDoc::GetMainThreadID()
{
	UInt64 maincreatetime = 0;
	DWORD mainthreadid = 0;
	ThreadList::iterator pos;
	for(pos = m_threadList.begin();pos!=m_threadList.end();pos++)
	{
		UInt64 createtime = *(UInt64*)&((*pos).CreateTime);
		if(maincreatetime==0)
		{
			maincreatetime = createtime;
			mainthreadid = (*pos).ClientId.UniqueThread;
		}
		if(maincreatetime>createtime)
		{
			maincreatetime = createtime;
			mainthreadid = (*pos).ClientId.UniqueThread;
		}
	}
	return mainthreadid;
}

void CSDebuggerDoc::ClearModuleMap()
{
	ModuleMap::iterator pos;
	for(pos=m_modulemap.begin();pos!=m_modulemap.end();pos++)
	{
		MODULE_INFORMATION mi = (*pos).second;
		if(mi.FunctionCount!=0)
		{
			free(mi.Functions);
		}
	}
	m_modulemap.clear();
}

void CSDebuggerDoc::ClearStackDeque()
{
	m_stackdeque.clear();
}

ModuleMap& CSDebuggerDoc::GetModuleMap()
{
	return m_modulemap;
}

StackDeque& CSDebuggerDoc::GetStackDeque()
{
	return m_stackdeque;
}

NT_TIB& CSDebuggerDoc::GetCurrentNT_TIB()
{
	return m_nt_tib;
}

BOOL CSDebuggerDoc::IsContainedBP(UInt64 adress, BPType type)
{
	BPVector::iterator pos;
	for(pos=m_bpvector.begin();pos!=m_bpvector.end();pos++)
	{
		if((*pos).type==type && (*pos).address == adress)
		{
			return TRUE;
		}
	}
	return FALSE;
}


// CSDebuggerDoc 序列化

void CSDebuggerDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: 在此添加存储代码
	}
	else
	{
		// TODO: 在此添加加载代码
	}
}


// CSDebuggerDoc 诊断

#ifdef _DEBUG
void CSDebuggerDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CSDebuggerDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CSDebuggerDoc 命令
