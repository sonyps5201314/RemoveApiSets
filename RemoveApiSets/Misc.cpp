#include "stdafx.h"
#include "Misc.h"

//返回不包含路径的文件名
LPCWSTR GetFileName(LPCWSTR Path)
{
	return PathFindFileNameW(Path);
}
LPCSTR GetFileName(LPCSTR Path)
{
	return PathFindFileNameA(Path);
}

//返回不包含拓展名的文件名
CString GetFileTitleName(LPCTSTR FileName)
{
	FileName = GetFileName(FileName);
	CString Ret(FileName);
	int DPos = Ret.ReverseFind(_T('.'));
	if (DPos > -1)
	{
		Ret = Ret.Left(DPos);
	}
	return Ret;
}


BOOL IsDir(LPCWSTR Path)
{
	DWORD dwFileAttib = GetFileAttributesW(Path);
	return (dwFileAttib != INVALID_FILE_ATTRIBUTES) && (dwFileAttib & FILE_ATTRIBUTE_DIRECTORY);
}
BOOL IsDir(LPCSTR Path)
{
	DWORD dwFileAttib = GetFileAttributesA(Path);
	return (dwFileAttib != INVALID_FILE_ATTRIBUTES) && (dwFileAttib & FILE_ATTRIBUTE_DIRECTORY);
}

BOOL IsFile(LPCWSTR Path)
{
	DWORD dwFileAttib = GetFileAttributesW(Path);
	return (dwFileAttib != INVALID_FILE_ATTRIBUTES) && (!(dwFileAttib & FILE_ATTRIBUTE_DIRECTORY));
}
BOOL IsFile(LPCSTR Path)
{
	DWORD dwFileAttib = GetFileAttributesA(Path);
	return (dwFileAttib != INVALID_FILE_ATTRIBUTES) && (!(dwFileAttib & FILE_ATTRIBUTE_DIRECTORY));
}

//EnumDirectory只在你的枚举回调函数lpfn返回FALSE时才返回TRUE,即达到匹配时
BOOL WINAPI EnumDirectory(LPCTSTR lpDstDir, DirectoryEnumProcType lpDirectoryEnumProc, LPARAM lParam, BOOL bResultIncludeDir)
{
	WIN32_FIND_DATA ffd;
	TCHAR szDir[MAX_PATH];
	HANDLE hFind = INVALID_HANDLE_VALUE;
	DWORD dwError = 0;
	TCHAR FileFullPath[MAX_PATH];

	int Xstrlen = lstrlen(lpDstDir);
	ATLASSERT(lpDstDir && lpDirectoryEnumProc && lpDstDir[lstrlen(lpDstDir) - 1] == _T('\\'));
	if (!(lpDstDir && lpDirectoryEnumProc && lpDstDir[lstrlen(lpDstDir) - 1] == _T('\\')))
		return FALSE;

	// Prepare string for use with FindFile functions.  First, copy the
	// string to a buffer, then append '*' to the directory name.
	lstrcpyn(szDir, lpDstDir, MAX_PATH);
	lstrcat(szDir, TEXT("*"));

	// Find the first file in the directory.

	hFind = FindFirstFile(szDir, &ffd);

	if (INVALID_HANDLE_VALUE == hFind)
	{
		ATLTRACE(TEXT("FindFirstFile\r\n"));
		return FALSE;
	}

	// List all the files in the directory with some info about them.

	do
	{
		if (lstrcmp(ffd.cFileName, _T(".")) == 0 || lstrcmp(ffd.cFileName, _T("..")) == 0)
		{
			//忽略
			continue;
		}
		lstrcpyn(FileFullPath, lpDstDir, MAX_PATH);
		lstrcat(FileFullPath, ffd.cFileName);
		if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)//文件夹
		{
			if (bResultIncludeDir)
			{
				if (lpDirectoryEnumProc(&ffd, FileFullPath, lParam) == FALSE)
				{
					//条件匹配了
					break;
				}
			}

			lstrcat(FileFullPath, _T("\\"));
			EnumDirectory(FileFullPath, lpDirectoryEnumProc, lParam);
		}
		else//文件
		{
			if (lpDirectoryEnumProc(&ffd, FileFullPath, lParam) == FALSE)
			{
				//条件匹配了
				break;
			}
		}
	} while (FindNextFile(hFind, &ffd) != 0);

	dwError = GetLastError();
	if (dwError != ERROR_NO_MORE_FILES)
	{
		ATLTRACE(TEXT("FindFirstFile\r\n"));
	}

	FindClose(hFind);
	return TRUE;
}

//返回给定错误号的错误信息
//目前只限于报告WIN32函数的错误
CString Error(int ErrorNumber)
{
	if (ErrorNumber == 0)//获取最后一次错误
	{
		ErrorNumber = GetLastError();
	}
	if (ErrorNumber == 0)
	{
		ATL::CComQIPtr <IErrorInfo> spErrInfo;	// 声明IErrorInfo接口
		HRESULT hr = GetErrorInfo(0, &spErrInfo);	// 取得接口
		if (hr == S_OK)
		{
			ATL::CComBSTR bstrDes;
			spErrInfo->GetDescription(&bstrDes);	// 取得错误描述
			//......
			// 还可以取得其它的信息
#ifndef _ATL_CSTRING_EXPLICIT_CONSTRUCTORS
			return (BSTR)bstrDes;
#else
			return CComVariant(bstrDes);
#endif
		}
	}
	if (ErrorNumber == ERROR_SUCCESS)
	{
		return NullString;
	}
	LPTSTR lpMsgBuf;
	int nRet = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, ErrorNumber, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		(LPTSTR)&lpMsgBuf, 0, NULL);
	if (nRet > 2)
	{
		//去掉末尾的换行符
		if (!_tcsncmp(&lpMsgBuf[nRet - 2], _T("\r\n"), 2))
		{
			lpMsgBuf[nRet - 2] = 0;
		}
	}
	CString RetStr = (LPTSTR)lpMsgBuf;
	LocalFree(lpMsgBuf);
	return RetStr;
}