// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件
//

#pragma once

//让编译器进行严格类型检查，比如表示对 Windows   应用程序中使用的句柄进行严格的类型检查。
#ifndef STRICT
#define STRICT
#endif

//https://www.gamedev.net/forums/topic/367942-win32_lean_and_mean-vs-vc_extralean/
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>
#include <locale.h>

#include <Windows.h>
#include <ntdll.h>
#pragma comment(lib,"ntdll.lib")


// TODO: 在此处引用程序需要的其他头文件

#include <atlstr.h>
#include <atlcoll.h>

//JKSDK
#define __DO_NOT_USE_JKSDK_OUTPUTDEBUGSTRING__
#define __DO_NOT_USE_JKSDK_TRACE__
#define __DO_NOT_USE_JKSDK_ASSERT__
#define __DO_NOT_USE_COM__
#define __DO_NOT_USE_ATL_CSTRING__
#define __DO_NOT_USE_JKSDK_CDLG__
#define __DO_NOT_USE_JKSDK_AUTOLOCK__
#define __DO_NOT_USE_JKSDK_SHOWCALLSTACKTRACK_SOURCEFILEPATHMAPPINGS__
#include "F:\MyCppProjects\JKSDK\Lib\JKSDK.H"