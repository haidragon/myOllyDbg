// DebugTest.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

void check(void* param)
{
	while(TRUE)
	{
		if(IsDebuggerPresent())
		{
			_asm int 3;
		}
		Sleep(10);
	}
}

int _tmain(int argc, _TCHAR* argv[])
{
	//OutputDebugString(TEXT("Warning! An exception will be thrown!"));

	//__try {
	//	int a = 0;
	//	int b = 10 / a;

	//} __except(EXCEPTION_EXECUTE_HANDLER) {

	//	OutputDebugString(TEXT("Entered exception handler."));
	//}
	//OutputDebugString(TEXT("Warning! An exception will be thrown!"));

	//try {
	//	throw 9;

	//}
	//catch(int ex) {

	//	OutputDebugString(TEXT("Entered exception handler."));
	//}

	CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)check,NULL,0,0);
	Sleep(10);
	 OutputDebugString(TEXT("Warning! An exception will be thrown! \r\n"));
	 try {
		 throw 9;
		 OutputDebugString(TEXT("Will this message be shown? \r\n"));
	 }
	 catch(int ex) {
		 OutputDebugString(TEXT("Entered exception handler. \r\n"));
	 }

	 while(true)
	 {
		 Sleep(10);
	 }

	return 0;
}

