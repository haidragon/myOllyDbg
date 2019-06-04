.386
.model flat,stdcall
option casemap:none
assume fs:nothing

.data

; all data variables in your asm code goes here


.code

; all assembly routines go here

ASM_GetFSImageAdress PROC
	mov eax,fs:[18h]
	ret
ASM_GetFSImageAdress ENDP

END ; end of assembly file