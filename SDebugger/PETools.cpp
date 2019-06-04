#include "stdafx.h"
#include "PETools.h"
#include "Message.h"
#include "PT.h"
#include "Psapi.h"


map<wstring, wstring> g_mapDevice2Path;

void ConvertVolumePaths(IN PWCHAR DeviceName, IN PWCHAR VolumeName)
{

	DWORD  CharCount = MAX_PATH + 1;
	PWCHAR Names     = NULL;
	PWCHAR NameIdx      = NULL;
	BOOL   Success      = FALSE;

	for (;;) 
	{
		Names = (PWCHAR) new BYTE [CharCount * sizeof(WCHAR)];
		if ( !Names ) 
		{
			return;
		}


		Success = GetVolumePathNamesForVolumeNameW(VolumeName, Names, CharCount, &CharCount);
		if (Success) 
		{
			break;
		}

		if ( GetLastError() != ERROR_MORE_DATA ) 
		{
			break;
		}

		delete [] Names;

		Names = NULL;
	}

	if ( Success )
	{
		for (NameIdx = Names; NameIdx[0] != L'\0'; NameIdx += wcslen(NameIdx) + 1 ) 
		{
			g_mapDevice2Path[DeviceName] = NameIdx;
		}
	}

	if ( Names != NULL ) 
	{
		delete [] Names;
		Names = NULL;
	}

	return;
}

BOOL InitDevice2Path()
{
	BOOL   bRet               = FALSE;  
	DWORD  CharCount           = 0;
	WCHAR  DeviceName[MAX_PATH] = L"";
	DWORD  Error              = ERROR_SUCCESS;
	HANDLE FindHandle          = INVALID_HANDLE_VALUE;
	BOOL   Found              = FALSE;
	size_t Index              = 0;
	BOOL   Success                = FALSE;
	WCHAR  VolumeName[MAX_PATH] = L"";

	//
	//  Enumerate all volumes in the system.

	FindHandle = FindFirstVolumeW(VolumeName, ARRAYSIZE(VolumeName));

	if (FindHandle == INVALID_HANDLE_VALUE)
	{
		Error = GetLastError();
		PrintSelfStrW(L"FindFirstVolumeW failed with error code %d\r\n", Error);
		return bRet;

	}

	for (;;)
	{
		//
		//  Skip the \\?\ prefix and remove the trailing backslash.

		Index = wcslen(VolumeName) - 1;

		if (VolumeName[0] != L'\\' || VolumeName[1] != L'\\' || VolumeName[2] != L'?' || VolumeName[3] != L'\\' || VolumeName[Index] != L'\\') 
		{
			Error = ERROR_BAD_PATHNAME;
			PrintSelfStrW(L"FindFirstVolumeW/FindNextVolumeW returned a bad path: %s\n", VolumeName);
			break;
		}

		//
		//  QueryDosDeviceW doesn't allow a trailing backslash,
		//  so temporarily remove it.

		VolumeName[Index] = L'\0';
		CharCount = QueryDosDeviceW(&VolumeName[4], DeviceName, ARRAYSIZE(DeviceName)); 
		VolumeName[Index] = L'\\';
		if ( CharCount == 0 ) 
		{
			Error = GetLastError();
			PrintSelfStrW(L"QueryDosDeviceW failed with error code %d\r\n", Error);
			break;
		}

		ConvertVolumePaths(DeviceName, VolumeName);

		//
		//  Move on to the next volume.
		Success = FindNextVolumeW(FindHandle, VolumeName, ARRAYSIZE(VolumeName));

		if ( !Success ) 
		{
			Error = GetLastError();
			if (Error != ERROR_NO_MORE_FILES) 
			{
				PrintSelfStrW(L"FindNextVolumeW failed with error code %d\r\n", Error);
				break;
			}

			//
			//  Finished iterating
			//  through all the volumes.

			Error = ERROR_SUCCESS;
			break;
		}
	}

	FindVolumeClose(FindHandle);
	FindHandle = INVALID_HANDLE_VALUE;
	return bRet;
}

void DeviceName2PathName(OUT WCHAR* szPathName, IN const WCHAR* szDeviceName)
{
	memset(szPathName, 0, MAX_PATH * 2);
	wstring strDeviceName = szDeviceName;
	size_t pos = strDeviceName.find(L'\\', 9);
	wstring strTemp1 = strDeviceName.substr(0, pos);
	wstring strTemp2 = strDeviceName.substr(pos + 1);
	wstring strDriverLetter  = g_mapDevice2Path[strTemp1];
	wstring strPathName = strDriverLetter + strTemp2;
	wcscpy_s(szPathName, MAX_PATH, strPathName.c_str());
}

void DeviceName2ModuleName(IN OUT WCHAR* szDeviceName)
{
	wstring strDeviceName = szDeviceName;
	size_t pos = strDeviceName.find_last_of(L'\\');
	wcscpy_s(szDeviceName, MAX_PATH, strDeviceName.substr(pos+1).c_str());
}

void GetExecuteModuleInfo(IN DWORD dwProcessId,ModuleMap& modulemap)
{
	HANDLE hProcess =NULL;
	SIZE_T numberofread;
	MODULEINFO minfo;
	hProcess = OpenProcess(PROCESS_ALL_ACCESS, TRUE, dwProcessId);
	if (hProcess == NULL)
	{
		PrintSelfStrW(L"Open Process %d Error \r\n", dwProcessId);
		return;
	}
	EnumProcessModules(dwProcessId, modulemap);
	ModuleMap::iterator pos,findpos;
	int num = 0;
	int size = modulemap.size();
	for(pos=modulemap.begin();pos!=modulemap.end();pos++)
	{
		wstring name = (*pos).first;
		PMODULE_INFORMATION pmi = &((*pos).second);
		HMODULE hModule = (HMODULE)pmi->AllocationBase;
		if(GetModuleInformation(hProcess,hModule,&minfo,sizeof(MODULEINFO)))
		{
			PVOID hModuleBuff = malloc(minfo.SizeOfImage);
			pmi->SizeOfImage = minfo.SizeOfImage;
			if(hModuleBuff && ReadProcessMemory(hProcess,(PVOID)hModule,hModuleBuff,minfo.SizeOfImage,&numberofread))
			{
				if(!EnumModuleInfo(hModule,(ULONG_PTR)hModuleBuff,pmi))
				{
					pmi->FunctionCount = 0;
					pmi->Functions = NULL;
				}
			}
			free(hModuleBuff);
		}
		num++;
	}
}

//对齐粒度算法
ULONG_PTR Alignment(ULONG_PTR size,ULONG_PTR align)
{
	if(size%align!=0)
		return (size/align+1)*align;
	else 
		return size;
}
//快速遍历模块
void EmumModulesFaster(IN DWORD dwProcessId,ModuleMap& modulemap)
{
	HANDLE hModuleSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwProcessId);

	if(INVALID_HANDLE_VALUE == hModuleSnap)
	{
		PrintSelfStrW(L"EmumModulesFaster Error[%d] \r\n",GetLastError());
		return;
	}
	
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS,FALSE, dwProcessId);

	MODULEENTRY32 me32;
	BYTE szBuffer[MAX_PATH * 2 + 4] = {0};
	WCHAR szModuleName[MAX_PATH] = {0};
	WCHAR szPathName[MAX_PATH] = {0};
	MEMORY_BASIC_INFORMATION mbi;
	MODULE_INFORMATION mi;
	PUNICODE_STRING usSectionName;
	ULONG_PTR dwStartAddr;
	me32.dwSize = sizeof( MODULEENTRY32 );

	ZWQUERYVIRTUALMEMORY fnZwQueryVirtualMemory;
	fnZwQueryVirtualMemory = (ZWQUERYVIRTUALMEMORY)::GetProcAddress(GetModuleHandleA("ntdll.dll"),"ZwQueryVirtualMemory" );

	BOOL bReault =  Module32First( hModuleSnap, &me32);
	if(bReault)
	{
		dwStartAddr = (ULONG_PTR)me32.modBaseAddr;
		if(hProcess && fnZwQueryVirtualMemory)
		{
			if(fnZwQueryVirtualMemory(hProcess, (PVOID)dwStartAddr, MemoryBasicInformation,&mbi,sizeof(mbi),0) >= 0)
			{
				if(mbi.Type == MEM_IMAGE)
				{
					if(fnZwQueryVirtualMemory(hProcess,(PVOID)dwStartAddr,MemorySectionName,szBuffer,sizeof(szBuffer),0) >= 0)
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
		while(Module32Next(hModuleSnap,&me32))
		{
			dwStartAddr = (ULONG_PTR)me32.modBaseAddr;
			if(hProcess && fnZwQueryVirtualMemory)
			{
				if(fnZwQueryVirtualMemory(hProcess, (PVOID)dwStartAddr, MemoryBasicInformation,&mbi,sizeof(mbi),0) >= 0)
				{
					if(mbi.Type == MEM_IMAGE)
					{
						if(fnZwQueryVirtualMemory(hProcess,(PVOID)dwStartAddr,MemorySectionName,szBuffer,sizeof(szBuffer),0) >= 0)
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
		}
	}
}

/**
* 枚举指定进程加载的模块
* @param dwProcessId 进程Id
* @return void
*/

void UpdateProcessModules(IN DWORD dwProcessId,ModuleMap& modulemap)
{
	ULONG_PTR dwStartAddr = 0x00000000;
	BYTE szBuffer[MAX_PATH * 2 + 4] = {0};
	WCHAR szModuleName[MAX_PATH] = {0};
	WCHAR szPathName[MAX_PATH] = {0};
	SIZE_T numberofread;
	MEMORY_BASIC_INFORMATION mbi;
	MODULE_INFORMATION mi;
	PUNICODE_STRING usSectionName;    
	ZWQUERYVIRTUALMEMORY fnZwQueryVirtualMemory;
	HANDLE hProcess =NULL;
	ModuleMap::iterator iter;
	hProcess = OpenProcess(PROCESS_ALL_ACCESS, TRUE, dwProcessId);
	if (hProcess == NULL)
	{
		PrintSelfStrW(L"Open Process %d Error \r\n", dwProcessId);
		return;
	}
	dwStartAddr = 0x00000000;
	fnZwQueryVirtualMemory = (ZWQUERYVIRTUALMEMORY)::GetProcAddress(GetModuleHandleA("ntdll.dll"),"ZwQueryVirtualMemory" );
	if(fnZwQueryVirtualMemory)
	{
		do
		{
			if(fnZwQueryVirtualMemory(hProcess, (PVOID)dwStartAddr, MemoryBasicInformation,&mbi,sizeof(mbi),0) >= 0)
			{
				if(mbi.Type == MEM_IMAGE)
				{
					if(fnZwQueryVirtualMemory(hProcess,(PVOID)dwStartAddr,MemorySectionName,szBuffer,sizeof(szBuffer),0) >= 0 )
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
							iter = modulemap.find(szModuleName);
							if(iter != modulemap.end())
							{
								MODULE_INFORMATION mi = (*iter).second;
								dwStartAddr = Alignment((ULONG_PTR)mi.AllocationBase + (ULONG_PTR)mi.SizeOfImage,0x1000);
							}
							else
							{
								MODULEINFO minfo;
								HMODULE hModule = (HMODULE)mi.AllocationBase;
								if(GetModuleInformation(hProcess,hModule,&minfo,sizeof(MODULEINFO)))
								{
									PVOID hModuleBuff = malloc(minfo.SizeOfImage);
									mi.SizeOfImage = minfo.SizeOfImage;
									if(hModuleBuff && ReadProcessMemory(hProcess,(PVOID)hModule,hModuleBuff,minfo.SizeOfImage,&numberofread))
									{
										if(!EnumModuleInfo(hModule,(ULONG_PTR)hModuleBuff,&mi))
										{
											mi.FunctionCount = 0;
											mi.Functions = NULL;
										}
									}
									free(hModuleBuff);
									modulemap.insert(ModuleMap::value_type(szModuleName,mi));
									dwStartAddr = Alignment((ULONG_PTR)mi.AllocationBase + (ULONG_PTR)mi.SizeOfImage,0x1000);
								}
							}
						}
					}
				}
			}
			// 递增基址,开始下一轮查询!
			dwStartAddr += 0x1000;
		}while( dwStartAddr < 0x80000000 );
	}

	CloseHandle(hProcess);
}

/**
* 枚举指定进程加载的模块
* @param dwProcessId 进程Id
* @return void
*/

void EnumProcessModules(IN DWORD dwProcessId,ModuleMap& modulemap)
{
	ULONG_PTR dwStartAddr = 0x00000000;
	BYTE szBuffer[MAX_PATH * 2 + 4] = {0};
	WCHAR szModuleName[MAX_PATH] = {0};
	WCHAR szPathName[MAX_PATH] = {0};
	MEMORY_BASIC_INFORMATION mbi;
	MODULE_INFORMATION mi;
	PUNICODE_STRING usSectionName;    
	ZWQUERYVIRTUALMEMORY fnZwQueryVirtualMemory;
	HANDLE hProcess =NULL;
	hProcess = OpenProcess(PROCESS_ALL_ACCESS, TRUE, dwProcessId);
	if (hProcess == NULL)
	{
		PrintSelfStrW(L"Open Process %d Error \r\n", dwProcessId);
		return;
	}
	dwStartAddr = 0x00000000;
	fnZwQueryVirtualMemory = (ZWQUERYVIRTUALMEMORY)::GetProcAddress(GetModuleHandleA("ntdll.dll"),"ZwQueryVirtualMemory" );
	if(fnZwQueryVirtualMemory)
	{
		do
		{
			if(fnZwQueryVirtualMemory(hProcess, (PVOID)dwStartAddr, MemoryBasicInformation,&mbi,sizeof(mbi),0) >= 0)
			{
				if(mbi.Type == MEM_IMAGE)
				{
					if(fnZwQueryVirtualMemory(hProcess,(PVOID)dwStartAddr,MemorySectionName,szBuffer,sizeof(szBuffer),0) >= 0 )
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
							modulemap.insert(ModuleMap::value_type(szModuleName,mi));
						}
					}
				}
			}
			// 递增基址,开始下一轮查询!
			dwStartAddr += 0x1000;
		}while( dwStartAddr < 0x80000000 );
	}

	CloseHandle(hProcess);
}

#define GET_HEADER_DICTIONARY(nt_header, idx) &(nt_header->OptionalHeader.DataDirectory[idx])

//遍历导出表
BOOL EnumModuleInfo(HMODULE hModule,ULONG_PTR hModuleBuff,PMODULE_INFORMATION pmi)
{
	PrintSelfStrW(L"获取[%s]模块信息\r\n",pmi->szPathName);
	DWORD i, *nameRef;
	WORD *ordinal;
	ULONG_PTR codeBase = (ULONG_PTR)hModule;

	PIMAGE_DOS_HEADER dos_header;
	PIMAGE_NT_HEADERS nt_header;
	dos_header = (PIMAGE_DOS_HEADER)hModuleBuff;
	if (dos_header->e_magic != IMAGE_DOS_SIGNATURE) 
	{
		PrintSelfStrA("DOS Header can't be found \r\n");
		return FALSE;
	}
	nt_header = (PIMAGE_NT_HEADERS)&((const unsigned char *)(hModuleBuff))[dos_header->e_lfanew];
	if (nt_header->Signature != IMAGE_NT_SIGNATURE) 
	{
		PrintSelfStrA("NT Header can't be found \r\n");
		return FALSE;
	}

	pmi->EntryAddress = (PVOID)(codeBase + nt_header->OptionalHeader.AddressOfEntryPoint);

	PIMAGE_EXPORT_DIRECTORY exports;
	PIMAGE_DATA_DIRECTORY directory = GET_HEADER_DICTIONARY(nt_header, IMAGE_DIRECTORY_ENTRY_EXPORT);
	if (directory->Size == 0) {
		// no export table found
		return FALSE;
	}

	exports = (PIMAGE_EXPORT_DIRECTORY) (hModuleBuff + directory->VirtualAddress);
	if (exports->NumberOfNames == 0 || exports->NumberOfFunctions == 0) {
		// DLL doesn't export anything
		return FALSE;
	}

	// search function name in list of exported names
	nameRef = (DWORD *) (hModuleBuff + exports->AddressOfNames);
	ordinal = (WORD *) (hModuleBuff + exports->AddressOfNameOrdinals);

	pmi->FunctionCount = exports->NumberOfNames;
	pmi->Functions = (PFUNCTION_INFORMATION)malloc(pmi->FunctionCount*sizeof(FUNCTION_INFORMATION));
	if(pmi->Functions==NULL)
	{
		PrintSelfStrA("Allocate Functions Failed \r\n");
		return FALSE;
	}
	DWORD idx = 0;
	for (i=0; i<exports->NumberOfNames; i++, nameRef++, ordinal++) {
		idx = *ordinal;
		char* name = (char *)(hModuleBuff + (*nameRef));
		PFUNCTION_INFORMATION pfi = pmi->Functions + i;
		strcpy(pfi->name,name);
		pfi->VirtualAddress = ULONG_PTR(codeBase + (*(DWORD *) (hModuleBuff + exports->AddressOfFunctions + (idx*4))));
	}

	return TRUE;
}

#define FILEBUFSIZE 512

BOOL GetFileNameFromHandle(HANDLE hFile) 
{
	BOOL bSuccess = FALSE;
	TCHAR pszFilename[MAX_PATH+1];
	HANDLE hFileMap;

	// Get the file size.
	DWORD dwFileSizeHi = 0;
	DWORD dwFileSizeLo = GetFileSize(hFile, &dwFileSizeHi); 

	if( dwFileSizeLo == 0 && dwFileSizeHi == 0 )
	{
		printf("Cannot map a file with a length of zero./n");
		return FALSE;
	}

	// Create a file mapping object.
	hFileMap = CreateFileMapping(hFile, 
		NULL, 
		PAGE_READONLY,
		0, 
		1,
		NULL);

	if (hFileMap) 
	{
		// Create a file mapping to get the file name.
		void* pMem = MapViewOfFile(hFileMap, FILE_MAP_READ, 0, 0, 1);

		if (pMem) 
		{
			if (GetMappedFileName (GetCurrentProcess(), 
				pMem, 
				pszFilename,
				MAX_PATH)) 
			{

				// Translate path with device name to drive letters.
				TCHAR szTemp[FILEBUFSIZE];
				szTemp[0] = '/0';

				if (GetLogicalDriveStrings(FILEBUFSIZE-1, szTemp)) 
				{
					TCHAR szName[MAX_PATH];
					TCHAR szDrive[3] = TEXT(" :");
					BOOL bFound = FALSE;
					TCHAR* p = szTemp;

					do 
					{
						// Copy the drive letter to the template string
						*szDrive = *p;

						// Look up each device name
						if (QueryDosDevice(szDrive, szName, FILEBUFSIZE))
						{
							UINT uNameLen = _tcslen(szName);

							if (uNameLen < MAX_PATH) 
							{
								bFound = _tcsnicmp(pszFilename, szName, 
									uNameLen) == 0;

								if (bFound) 
								{
									// Reconstruct pszFilename using szTemp
									// Replace device path with DOS path
									TCHAR szTempFile[MAX_PATH];
									_stprintf(szTempFile,
										TEXT("%s%s"),
										szDrive,
										pszFilename+uNameLen);
									_tcsncpy(pszFilename, szTempFile, MAX_PATH);
								}
							}
						}

						// Go to the next NULL character.
						while (*p++);
					} while (!bFound && *p); // end of string
				}
			}
			bSuccess = TRUE;
			UnmapViewOfFile(pMem);
		} 

		CloseHandle(hFileMap);
	}
	printf("File name is %s/n", pszFilename);
	return(bSuccess);
}

