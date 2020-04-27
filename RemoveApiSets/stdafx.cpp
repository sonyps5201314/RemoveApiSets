// stdafx.cpp : 只包括标准包含文件的源文件
// RemoveApiSets.pch 将作为预编译头
// stdafx.obj 将包含预编译类型信息

#include "stdafx.h"

// TODO: 在 STDAFX.H 中
// 引用任何所需的附加头文件，而不是在此文件中引用

//JKSDK
#include "F:\MyCppProjects\JKSDK\Lib\JKSDK.CPP"
#ifdef _M_IX86
#pragma comment(lib,"F:\\MyCppProjects\\JKSDK\\Lib\\JKSDK_ASM_LIB.lib")
#elif defined(_M_AMD64)
#pragma comment(lib,"F:\\MyCppProjects\\JKSDK\\Lib\\x64\\JKSDK_ASM_LIB.lib")
#endif