.\amd64\ml64.exe /c InlineAsm64.asm
.ml.exe /c InlineAsm32.asm
:link.exe /lib InlineAsm32.obj
:.\amd64\link.exe /lib InlineAsm64.obj

