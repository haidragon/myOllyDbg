#include "stdafx.h"
#include "PT.h"

#include <tlhelp32.h> 

BOOL GetProcessList(ProcessList& proclist)
{
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
	if(hSnapshot==INVALID_HANDLE_VALUE){
		return FALSE;
	}
	PROCESSENTRY32 pe={sizeof(pe)};
	DWORD dwProcessID=0;
	for(BOOL fOK = Process32First(hSnapshot,&pe);fOK;fOK = Process32Next(hSnapshot,&pe))
	{
		proclist.push_back(pe);
	}
	CloseHandle(hSnapshot);
	return TRUE;
}

BOOL GetThreadList(DWORD th32ProcessID, ThreadList& threadlist)
{
	UINT_PTR pBuffer;

	PSYSTEM_PROCESS_INFORMATION pInfo;

	DWORD b=0;

	pBuffer = GetInfoTable(SystemProcessesAndThreadsInformation);

	if(!pBuffer)
		return FALSE;

	pInfo = (PSYSTEM_PROCESS_INFORMATION)pBuffer;

	for (;;)
	{
		if(pInfo->ProcessId == th32ProcessID)
		{
			for(b=0;b<pInfo->ThreadCount;b++)
			{
				threadlist.push_back(pInfo->Threads[b]);
			}
			break;
		}
		if (pInfo->NextEntryDelta == 0)
			break;
		pInfo = (PSYSTEM_PROCESS_INFORMATION)(((PUCHAR)pInfo) + pInfo->NextEntryDelta);
	}

	free((PVOID)pBuffer);
	return TRUE;
}   

ULONG_PTR GetInfoTable(SYSTEM_INFORMATION_CLASS SystemInformationClass)
{
	ZWQUERYSYSTEMINFORMATION fnZwQuerySystemInformation = (ZWQUERYSYSTEMINFORMATION)GetProcAddress(GetModuleHandleA("ntdll.dll"),"ZwQuerySystemInformation");
	if(fnZwQuerySystemInformation)
	{
		ULONG mSize = 0x4000;
		PVOID mPtr = NULL;
		LONG St;
		do
		{
			mPtr = malloc(mSize);
			memset(mPtr, 0, mSize);
			if (mPtr)
			{
				St = fnZwQuerySystemInformation(SystemInformationClass, mPtr, mSize, NULL);
			} else return NULL;
			if (St == STATUS_INFO_LENGTH_MISMATCH)
			{
				free(mPtr);
				mSize = mSize * 2;
			}
		} while (St == STATUS_INFO_LENGTH_MISMATCH);
		if (St == STATUS_SUCCESS) return (ULONG_PTR)mPtr;
		free(mPtr);
	}
	return NULL;
}

BOOL GetThreadBaseInformation(HANDLE ThreadHandle,OUT PTHREAD_BASIC_INFORMATION pbi)
{
	ZWQUERYINFORMATIONTHREAD fnZwQueryInformationThread = (ZWQUERYINFORMATIONTHREAD)GetProcAddress(GetModuleHandleA("ntdll.dll"),"ZwQueryInformationThread");
	if(fnZwQueryInformationThread)
	{
		ULONG ReturnLength;
		NTSTATUS status = fnZwQueryInformationThread(ThreadHandle, ThreadBasicInformation, pbi, sizeof(THREAD_BASIC_INFORMATION), &ReturnLength);
		if(NT_SUCCESS(status))
		{
			return TRUE;
		}
	}
	return FALSE;
}

/**************************************************************/
/*判断线程是否被终止 , 如果终止返回FALSE,如果还活着返回TRUE
/**************************************************************/

BOOL IsThreadAlive(DWORD dwThreadID)
{
	BOOL bRet = FALSE;
	DWORD ExitCode = 0;

	HANDLE hThread = OpenThread(THREAD_QUERY_INFORMATION,FALSE,dwThreadID);
	if(hThread != NULL)
	{
		if(GetExitCodeThread(hThread,&ExitCode))
		{
			if( ExitCode == STILL_ACTIVE)
				bRet = TRUE;
		}

		CloseHandle(hThread);
	}

	return bRet;
}

