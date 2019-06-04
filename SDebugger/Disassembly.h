#ifndef _DISASSEMBLY_H_
#define _DISASSEMBLY_H_

#define BEA_ENGINE_STATIC
#define BEA_USE_STDCALL
#include "BeaEngine.h"
#include <map>

using namespace std;


class CDisasmInfo
{
public:
	char m_disasmstr[64];
	char m_mnemonic[16];
	UInt64 m_vAddress;
	BYTE m_mem[16];
	SIZE_T m_size;
public:
	CDisasmInfo(char* disasmstr, char* mnemonic, UInt64 vAddress, PBYTE pmem, SIZE_T size);
	CDisasmInfo();
	~CDisasmInfo();
};

typedef map<UInt64,CDisasmInfo> DisasmMap;

void DisassembleCode(char *StartCodeSection, char *EndCodeSection, UINT_PTR Virtual_Address, DisasmMap &disamm);

#endif