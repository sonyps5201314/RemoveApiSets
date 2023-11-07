#include "stdafx.h"
#include "Misc.h"

//���ز�����·�����ļ���
LPCWSTR GetFileName(LPCWSTR Path)
{
	return PathFindFileNameW(Path);
}
LPCSTR GetFileName(LPCSTR Path)
{
	return PathFindFileNameA(Path);
}

//���ز�������չ�����ļ���
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

//EnumDirectoryֻ�����ö�ٻص�����lpfn����FALSEʱ�ŷ���TRUE,���ﵽƥ��ʱ
BOOL WINAPI EnumDirectory(LPCTSTR lpDstDir, DirectoryEnumProcType lpDirectoryEnumProc, LPARAM lParam, BOOL bResultIncludeDir)
{
	WIN32_FIND_DATA ffd;
	TCHAR szDir[MAX_PATH];
	HANDLE hFind = INVALID_HANDLE_VALUE;
	DWORD dwError = 0;
	TCHAR FileFullPath[MAX_PATH];

	int Xstrlen = lstrlen(lpDstDir);
	ATLASSERT(lpDstDir && lpDirectoryEnumProc && lpDstDir[Xstrlen - 1] == _T('\\'));
	if (!(lpDstDir && lpDirectoryEnumProc && lpDstDir[Xstrlen - 1] == _T('\\')))
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
			//����
			continue;
		}
		lstrcpyn(FileFullPath, lpDstDir, MAX_PATH);
		lstrcat(FileFullPath, ffd.cFileName);
		if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)//�ļ���
		{
			if (bResultIncludeDir)
			{
				if (lpDirectoryEnumProc(&ffd, FileFullPath, lParam) == FALSE)
				{
					//����ƥ����
					break;
				}
			}

			lstrcat(FileFullPath, _T("\\"));
			EnumDirectory(FileFullPath, lpDirectoryEnumProc, lParam);
		}
		else//�ļ�
		{
			if (lpDirectoryEnumProc(&ffd, FileFullPath, lParam) == FALSE)
			{
				//����ƥ����
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

//����ֵΪ0Ҳ�п��ܴ���ִ�к���ʧ�ܣ����ļ������ڵ������
LARGE_INTEGER FileLen(LPCWSTR PathName)
{
	//Ҳ������statʵ��
	WIN32_FILE_ATTRIBUTE_DATA FileAttribData =
	{
		0
	};
	GetFileAttributesExW(PathName, GetFileExInfoStandard, &FileAttribData);
	LARGE_INTEGER RetValue =
	{
		FileAttribData.nFileSizeLow, (LONG)FileAttribData.nFileSizeHigh
	};
	return RetValue;
}
LARGE_INTEGER FileLen(LPCSTR PathName)
{
	//Ҳ������statʵ��
	WIN32_FILE_ATTRIBUTE_DATA FileAttribData =
	{
		0
	};
	GetFileAttributesExA(PathName, GetFileExInfoStandard, &FileAttribData);
	LARGE_INTEGER RetValue =
	{
		FileAttribData.nFileSizeLow, (LONG)FileAttribData.nFileSizeHigh
	};
	return RetValue;
}

//���ظ�������ŵĴ�����Ϣ
//Ŀǰֻ���ڱ���WIN32�����Ĵ���
CString Error(int ErrorNumber)
{
	if (ErrorNumber == 0)//��ȡ���һ�δ���
	{
		ErrorNumber = GetLastError();
	}
	if (ErrorNumber == 0)
	{
		ATL::CComQIPtr <IErrorInfo> spErrInfo;	// ����IErrorInfo�ӿ�
		HRESULT hr = GetErrorInfo(0, &spErrInfo);	// ȡ�ýӿ�
		if (hr == S_OK)
		{
			ATL::CComBSTR bstrDes;
			spErrInfo->GetDescription(&bstrDes);	// ȡ�ô�������
			//......
			// ������ȡ����������Ϣ
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
		//ȥ��ĩβ�Ļ��з�
		if (!_tcsncmp(&lpMsgBuf[nRet - 2], _T("\r\n"), 2))
		{
			lpMsgBuf[nRet - 2] = 0;
		}
	}
	CString RetStr = (LPTSTR)lpMsgBuf;
	LocalFree(lpMsgBuf);
	return RetStr;
}

//https://blog.csdn.net/MoreWindows/article/details/6789206
BOOL SetConsoleColor(DWORD nStdHandle, WORD wAttributes)
{
	HANDLE hConsole = GetStdHandle(nStdHandle);
	if (hConsole == INVALID_HANDLE_VALUE)
		return FALSE;

	return SetConsoleTextAttribute(hConsole, wAttributes);
}
BOOL SetConsoleInputColor(WORD wAttributes)
{
	return SetConsoleColor(STD_INPUT_HANDLE, wAttributes);
}
BOOL SetConsoleOutputColor(WORD wAttributes)
{
	return SetConsoleColor(STD_OUTPUT_HANDLE, wAttributes);
}
BOOL SetConsoleErrorColor(WORD wAttributes)
{
	return SetConsoleColor(STD_ERROR_HANDLE, wAttributes);
}