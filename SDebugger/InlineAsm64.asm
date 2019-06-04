.data

; all data variables in your asm code goes here


.code

; all assembly routines go here

ASM_GetFSImageAdress PROC
	int 3
	ret 
ASM_GetFSImageAdress ENDP

END ; end of assembly file