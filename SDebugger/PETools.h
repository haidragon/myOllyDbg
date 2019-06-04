#ifndef _PETOOLS_H_
#define _PETOOLS_H_

#include <map>
#include <string>

using namespace std;

typedef struct _FUNCTION_INFORMATION
{
	char name[64];
	ULONG_PTR VirtualAddress;
}FUNCTION_INFORMATION, *PFUNCTION_INFORMATION;

typedef struct _MODULE_INFORMATION
{
	PVOID BaseAddress;
	PVOID AllocationBase;
	DWORD AllocationProtect;
	SIZE_T RegionSize;
	DWORD State;
	DWORD Protect;
	DWORD Type;
	WCHAR szPathName[MAX_PATH];
	PVOID EntryAddress;
	PFUNCTION_INFORMATION Functions;
	DWORD FunctionCount;
	DWORD SizeOfImage;
}MODULE_INFORMATION, *PMODULE_INFORMATION;

typedef	map<wstring, MODULE_INFORMATION> ModuleMap;

typedef enum _MEMORY_INFORMATION_CLASS 
{
	MemoryBasicInformation,
	MemoryWorkingSetList,
	MemorySectionName
}MEMORY_INFORMATION_CLASS;

typedef LONG (WINAPI *ZWQUERYVIRTUALMEMORY)(
	IN HANDLE ProcessHandle, 
	IN PVOID BaseAddress, 
	IN MEMORY_INFORMATION_CLASS MemoryInformationClass, 
	OUT PVOID MemoryInformation, 
	IN ULONG MemoryInformationLength, 
	OUT PULONG ReturnLength OPTIONAL 
	);

BOOL InitDevice2Path();
void GetExecuteModuleInfo(IN DWORD dwProcessId,ModuleMap& modulemap);
void UpdateProcessModules(IN DWORD dwProcessId,ModuleMap& modulemap);
void EnumProcessModules(IN DWORD dwProcessId,ModuleMap& modulemap);
BOOL EnumModuleInfo(HMODULE hModule, ULONG_PTR hModuleBuff, PMODULE_INFORMATION pmi);
BOOL GetFileNameFromHandle(HANDLE hFile);
void DeviceName2PathName(OUT WCHAR* szPathName, IN const WCHAR* szDeviceName);
void DeviceName2ModuleName(IN OUT WCHAR* szDeviceName);
void EmumModulesFaster(IN DWORD dwProcessId,ModuleMap& modulemap);


#endif
