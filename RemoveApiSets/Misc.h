#pragma once

//����ժ���Ա���˽�п�

//�����һЩ��
#define NullStringA  ""
#define NullStringW  L""
#define NullString  _T("")

//��ȡ�����ַ����ĳ���
#define CONST_STRING_LENGTH(s) (_countof(s)-1)

//���ز�����·�����ļ���
LPCWSTR GetFileName(LPCWSTR Path);
LPCSTR GetFileName(LPCSTR Path);

//���ز�������չ�����ļ���
CString GetFileTitleName(LPCTSTR FileName);

BOOL IsDir(LPCWSTR Path);
BOOL IsDir(LPCSTR Path);

BOOL IsFile(LPCWSTR Path);
BOOL IsFile(LPCSTR Path);


//EnumDirectoryֻ�����ö�ٻص�����lpfn����FALSEʱ�ŷ���TRUE,���ﵽƥ��ʱ
typedef BOOL(CALLBACK* DirectoryEnumProcType)(LPWIN32_FIND_DATA lpFfd,
	LPCTSTR FileFullPath, LPARAM lParam);
//ע�⣺��ʹ����bResultIncludeDirָ��ΪTRUEʱҲ�����ڻص�����lpDirectoryEnumProc�з��ر�ö�ٵĸ�Ŀ¼lpDstDir
BOOL WINAPI EnumDirectory(LPCTSTR lpDstDir, DirectoryEnumProcType lpDirectoryEnumProc, LPARAM lParam, BOOL bResultIncludeDir = FALSE);

//����ֵΪ0Ҳ�п��ܴ���ִ�к���ʧ�ܣ����ļ������ڵ������
LARGE_INTEGER FileLen(LPCWSTR PathName);
LARGE_INTEGER FileLen(LPCSTR PathName);

//���ظ�������ŵĴ�����Ϣ
//Ŀǰֻ���ڱ���WIN32�����Ĵ���
CString Error(int ErrorNumber = 0);

//����̨

//���ÿ���̨��ɫ
//һ����16��������ɫ��16�ֱ�����ɫ�������256�֡������ֵӦ��С��256
#define CONSOLE_DEFAULT_COLOR_ATTRIBUTES (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE)
BOOL SetConsoleInputColor(WORD wAttributes);
BOOL SetConsoleOutputColor(WORD wAttributes);
BOOL SetConsoleErrorColor(WORD wAttributes);