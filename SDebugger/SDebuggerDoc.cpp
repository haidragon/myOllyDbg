
// SDebuggerDoc.cpp : CSDebuggerDoc ���ʵ��
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


// CSDebuggerDoc ����/����

CSDebuggerDoc::CSDebuggerDoc()
{
	// TODO: �ڴ����һ���Թ������
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

	// TODO: �ڴ�������³�ʼ������
	// (SDI �ĵ������ø��ĵ�)

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

//��ȡ�������̵߳������Ļ�����
BOOL CSDebuggerDoc::GetDebugThreadContext(CONTEXT* pContext)
{
	return GetDebuggerContext(m_hThread,pContext);
}

BOOL CSDebuggerDoc::Disassemby(ULONG_PTR Virtual_Address,BOOL IsAdd)
{
	DisasmMap::iterator pos;
	SIZE_T NumberOfBytesRead;
	memset(m_disasmbuf,0,DISAM_BUF_SIZE);
	//��ȡ��ǰ��ַ֮��DISAM_BUF_SIZE���ȵ��ڴ�
	if(ReadProcessMemory(m_hProcess,(LPCVOID)Virtual_Address,(LPVOID)m_disasmbuf,DISAM_BUF_SIZE,&NumberOfBytesRead))
	{
		if(!IsAdd)
		{
			ClearDisasmMap(m_disasmm);//������������
		}
		DisassembleCode((char *)m_disasmbuf,(char *)m_disasmbuf + NumberOfBytesRead, Virtual_Address, m_disasmm);//��������
		if(!IsAdd)
		{
			pos = m_disasmm.find(Virtual_Address);//��ѯ�Ƿ��е�ǰ��ַ
			if(pos!=m_disasmm.end())
			{
				CDisasmInfo *pDI_O,*pDI_N = NULL;
				pDI_O = &(*pos).second;
				ULONG_PTR Prev_Virtual_Address_Start = Virtual_Address - DISAM_BUF_SIZE + pDI_O->m_size; // ���쵱ǰ��ַ֮ǰ�Ļ����ʼͷ��ַ
				ULONG_PTR Prev_Virtual_Address_End = Virtual_Address + pDI_O->m_size;					//	���쵱ǰ��ַ֮ǰ�Ļ���������ַ
				memset(m_disasmbuf,0,DISAM_BUF_SIZE);
				if(ReadProcessMemory(m_hProcess,(LPCVOID)Prev_Virtual_Address_Start,(LPVOID)m_disasmbuf,DISAM_BUF_SIZE,&NumberOfBytesRead))
				{
					ULONG_PTR index = 0;
					DisasmMap newdisasmm;
					while((Prev_Virtual_Address_Start+index)<Prev_Virtual_Address_End)
					{	
						DisassembleCode((char *)m_disasmbuf + index,(char *)m_disasmbuf + NumberOfBytesRead, Prev_Virtual_Address_Start + index, newdisasmm);
						pos = newdisasmm.find(Virtual_Address);//���ҵ�ǰλ���Ƿ��з������
						if(pos!=newdisasmm.end())
						{
							pDI_N = &(*pos).second;
							if(strcmp(pDI_O->m_disasmstr,pDI_N->m_disasmstr)==0)//�ȶԷ������,���Ƿ�ǰ��ַ���Ϸ����Ľ��
							{
								newdisasmm.erase(pos);	//ɾ������ĵ�ǰ��
								pos = newdisasmm.find(Prev_Virtual_Address_Start + index);//�����ĵ�һ��ɾ����֤��ȷ��
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
//�����ǰ��ַ��ʼ���Ѵ��ڵĵ�ַ�����Ϣ
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


// CSDebuggerDoc ���л�

void CSDebuggerDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: �ڴ���Ӵ洢����
	}
	else
	{
		// TODO: �ڴ���Ӽ��ش���
	}
}


// CSDebuggerDoc ���

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


// CSDebuggerDoc ����
