#include "stdafx.h"

#include "Message.h"
#include "Disassembly.h"

#pragma comment(lib,"BeaEngine.lib")

CDisasmInfo::CDisasmInfo()
{

}

CDisasmInfo::CDisasmInfo(char* disasmstr, char* mnemonic, UInt64 vAddress, PBYTE pmem, SIZE_T size)
{
	strcpy(m_disasmstr,disasmstr);
	strcpy(m_mnemonic,mnemonic);
	m_vAddress = vAddress;
	memcpy(m_mem,pmem,size);
	m_size = size;
}

CDisasmInfo::~CDisasmInfo()
{

}

/////////* ===============================================================================*/
/////////*									
/////////*	Disassemble code in the specified buffer using the correct VA		
/////////*									
/////////* ===============================================================================*/
/////////*void DisassembleCode(char *StartCodeSection, 
////////	char *EndCodeSection, 
////////	int (*Virtual_Address)(void))
////////{
////////
////////	/* ============================= Init the Disasm structure (important !)*/
////////	(void) memset (&MyDisasm, 0, sizeof(DISASM));
////////
////////	/* ============================= Init EIP */
////////	MyDisasm.EIP = (int) StartCodeSection;
////////	/* ============================= Init VirtualAddr */
////////	MyDisasm.VirtualAddr = (long long) Virtual_Address;
////////
////////	/* ============================= set IA-32 architecture */
////////	MyDisasm.Archi = 0;
////////	/* ============================= Loop for Disasm */
////////	while ( !Error){
////////		/* ============================= Fix SecurityBlock */
////////		MyDisasm.SecurityBlock = (int) EndCodeSection - MyDisasm.EIP;
////////
////////		len = Disasm(&MyDisasm);
////////		if (len == OUT_OF_BLOCK) {
////////			(void) printf("disasm engine is not allowed to read more memory \n");
////////			Error = 1;
////////		}
////////		else if (len == UNKNOWN_OPCODE) {
////////			(void) printf("unknown opcode");
////////			Error = 1;
////////		}
////////		else {
////////			(void) printf("%.8X %s\n",(int) MyDisasm.VirtualAddr, 
////////				&MyDisasm.CompleteInstr);
////////			MyDisasm.EIP = MyDisasm.EIP + len;
////////			MyDisasm.VirtualAddr = MyDisasm.VirtualAddr + len;
////////			if (MyDisasm.EIP >= (int) EndCodeSection) {
////////				(void) printf("End of buffer reached ! \n");
////////				Error = 1;
////////			}
////////		}
////////	};
////////	return;
////////}

void DisassembleCode(char *StartCodeSection, char *EndCodeSection, UINT_PTR Virtual_Address, DisasmMap &disamm)
{
	int len = 0;
	BOOL Error = 0;
	DISASM MyDisasm;
	BYTE Mem[16];
	/* ============================= Init the Disasm structure (important !)*/
	(void) memset (&MyDisasm, 0, sizeof(DISASM));

	/* ============================= Init EIP */
	MyDisasm.EIP = (int) StartCodeSection;
	/* ============================= Init VirtualAddr */
	MyDisasm.VirtualAddr = (long long) Virtual_Address;

	/* ============================= set IA-32 architecture */
	MyDisasm.Archi = 0;
	/* ============================= Loop for Disasm */
	while ( !Error){
		/* ============================= Fix SecurityBlock */
		MyDisasm.SecurityBlock = (int) EndCodeSection - MyDisasm.EIP;

		len = Disasm(&MyDisasm);
		if (len == OUT_OF_BLOCK) {
			//PrintSelfStrA("disasm engine is not allowed to read more memory \r\n");
			Error = 1;
		}
		else if (len == UNKNOWN_OPCODE) {
			//PrintSelfStrA("unknown opcode \r\n");
			Error = 1;
		}
		else {
			//PrintSelfStrA("%.8X %s\n",(int) MyDisasm.VirtualAddr, 
			//	&MyDisasm.CompleteInstr);
			//size_t size = MyDisasm.Reserved_.EIP_ - MyDisasm.Reserved_.EIP_REAL;
			//size_t index = 0;
			//for(size_t i = MyDisasm.Reserved_.EIP_REAL;i<MyDisasm.Reserved_.EIP_;i++)
			//{
			//	Mem[index] = *(PBYTE)i;
			//	index++;
			//}
			size_t index=0; 
			for (size_t i=MyDisasm.EIP;i<MyDisasm.EIP+len;i++)
			{
				Mem[index]=*(BYTE *)i;
				index++;
			}
			CDisasmInfo DInfo(MyDisasm.CompleteInstr, MyDisasm.Instruction.Mnemonic, MyDisasm.VirtualAddr, Mem, len);
			
			disamm[MyDisasm.VirtualAddr] = DInfo;
			MyDisasm.EIP = MyDisasm.EIP + len;
			MyDisasm.VirtualAddr = MyDisasm.VirtualAddr + len;
			if (MyDisasm.EIP >= (size_t) EndCodeSection) {
				//PrintSelfStrA("End of buffer reached ! \r\n");
				Error = 1;
			}
		}
	};
	return;
}

