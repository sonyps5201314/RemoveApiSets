#pragma once

//代码摘抄自本人私有库

//定义的一些宏
#define NullStringA  ""
#define NullStringW  L""
#define NullString  _T("")

//返回不包含路径的文件名
LPCWSTR GetFileName(LPCWSTR Path);
LPCSTR GetFileName(LPCSTR Path);

//返回不包含拓展名的文件名
CString GetFileTitleName(LPCTSTR FileName);

BOOL IsDir(LPCWSTR Path);
BOOL IsDir(LPCSTR Path);

BOOL IsFile(LPCWSTR Path);
BOOL IsFile(LPCSTR Path);


//EnumDirectory只在你的枚举回调函数lpfn返回FALSE时才返回TRUE,即达到匹配时
typedef BOOL(CALLBACK* DirectoryEnumProcType)(LPWIN32_FIND_DATA lpFfd,
	LPCTSTR FileFullPath, LPARAM lParam);
//注意：即使参数bResultIncludeDir指定为TRUE时也不会在回调函数lpDirectoryEnumProc中返回被枚举的根目录lpDstDir
BOOL WINAPI EnumDirectory(LPCTSTR lpDstDir, DirectoryEnumProcType lpDirectoryEnumProc, LPARAM lParam, BOOL bResultIncludeDir = FALSE);

//返回值为0也有可能代表执行函数失败，如文件不存在的情况。
LARGE_INTEGER FileLen(LPCWSTR PathName);
LARGE_INTEGER FileLen(LPCSTR PathName);

//返回给定错误号的错误信息
//目前只限于报告WIN32函数的错误
CString Error(int ErrorNumber = 0);