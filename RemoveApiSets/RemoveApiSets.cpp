// RemoveApiSets.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "Phlib.h"
#include "Misc.h"
#include <delayimp.h>

//https://www.51hint.com/rva-to-file-offset.html
//RVA ת �ļ�ƫ�Ƶ�ַ
DWORD_PTR RvaToOffset(PIMAGE_NT_HEADERS pNt, DWORD_PTR dwRva)
{
	DWORD nCount = pNt->FileHeader.NumberOfSections;
	PIMAGE_SECTION_HEADER pSection = IMAGE_FIRST_SECTION(pNt);
	if (pSection == NULL)
	{
		ATLASSERT(FALSE);
		return 0;
	}
	for (DWORD i = 0; i < nCount; i++, pSection++)
	{
		if ((dwRva >= pSection->VirtualAddress) && (dwRva < (pSection->VirtualAddress + pSection->Misc.VirtualSize)))
		{
			return dwRva - (pSection->VirtualAddress - pSection->PointerToRawData);
		}
	}
	ATLASSERT(FALSE);
	return 0;
}

BOOL TryDoReplaceDllNameItem(PCHAR pDllName, ApiSetSchema* pApiSetSchema, CStringA strNewVcrDllName, CStringA strNewVcpDllsName[5], CStringA strNewConCrtDllName, CStringA strNewMfcDllsName[2], LPCSTR pFirstFuncName)
{
	BOOL bResult = FALSE;

	int nDllNameLen = (int)strlen(pDllName);
	CStringA strOldDllTitleName = GetFileTitleName(pDllName);
	ApiSetTarget* pApiSetTarget = pApiSetSchema->Lookup(strOldDllTitleName);
	if (pApiSetTarget)
	{
		CStringA strNewDllName = pApiSetTarget->GetAt(0);
		if (!strNewDllName.CompareNoCase("ucrtbase.dll"))
		{
			strNewDllName = strNewVcrDllName;
		}
		else if (!strNewDllName.CompareNoCase("kernelbase.dll"))
		{
			if (pApiSetTarget->GetCount() == 1)
			{
				BOOL bFindNewDllName = FALSE;
				static const LPCSTR pszNewDllNames[] = { "kernel32.dll","advapi32.dll" };
				for (int i = 0; i < _countof(pszNewDllNames); i++)
				{
					HMODULE hMod = LoadLibrary(pszNewDllNames[i]);
					if (hMod)
					{
						if (GetProcAddress(hMod, pFirstFuncName))
						{
							strNewDllName = pszNewDllNames[i];
							bFindNewDllName = TRUE;
						}

						FreeLibrary(hMod);
						hMod = NULL;

						if (bFindNewDllName)
						{
							break;
						}
					}
				}

				if (!bFindNewDllName)
				{
					ATLASSERT(FALSE);
					printf("can not find new dll name for replace \"kernelbase.dll\"!\r\n");
				}
			}
			else
			{
				strNewDllName = pApiSetTarget->GetAt(1);
			}
		}

		if (strNewDllName.GetLength() <= nDllNameLen)
		{
			lstrcpynA(pDllName, strNewDllName, strNewDllName.GetLength() + 1);
			bResult = TRUE;
		}
		else
		{
			printf("new dll length to long!!!(%s->%s)\r\n", pDllName, (LPCSTR)strNewDllName);
			ATLASSERT(FALSE);
		}
	}
	else
	{
		static const LPCSTR pszDllNames[] = { "api-ms-win-crt-","vcruntime140.dll"
#if _M_AMD64
			,"vcruntime140_1.dll"
#endif
		};
		for (int i = 0; i < _countof(pszDllNames); i++)
		{
			if (!_strnicmp(pDllName, pszDllNames[i], strlen(pszDllNames[i])))
			{
				if (strNewVcrDllName.GetLength() <= nDllNameLen)
				{
					lstrcpynA(pDllName, strNewVcrDllName, strNewVcrDllName.GetLength() + 1);
					bResult = TRUE;
					break;
				}
				else
				{
					printf("new dll length to long!!!(%s->%s)\r\n", pDllName, (LPCSTR)strNewVcrDllName);
					ATLASSERT(FALSE);
				}
			}
		}

		if (bResult == FALSE)
		{
			static const LPCSTR pszDllNames[] = { "msvcp140.dll","msvcp140_1.dll","msvcp140_2.dll","msvcp140_atomic_wait.dll","msvcp140_codecvt_ids.dll" };
			for (int i = 0; i < _countof(pszDllNames); i++)
			{
				if (!_stricmp(pDllName, pszDllNames[i]))
				{
					CStringA strNewVcpDllName = strNewVcpDllsName[i];
					if (strNewVcpDllName.GetLength() <= nDllNameLen)
					{
						lstrcpynA(pDllName, strNewVcpDllName, strNewVcpDllName.GetLength() + 1);
						bResult = TRUE;
						break;
					}
					else
					{
						printf("new dll length to long!!!(%s->%s)\r\n", pDllName, (LPCSTR)strNewVcpDllName);
						ATLASSERT(FALSE);
					}
				}
			}
		}

		if (bResult == FALSE)
		{
			if (!_stricmp(pDllName, "concrt140.dll"))
			{
				if (strNewConCrtDllName.GetLength() <= nDllNameLen)
				{
					lstrcpynA(pDllName, strNewConCrtDllName, strNewConCrtDllName.GetLength() + 1);
					bResult = TRUE;
				}
				else
				{
					printf("new dll length to long!!!(%s->%s)\r\n", pDllName, (LPCSTR)strNewConCrtDllName);
					ATLASSERT(FALSE);
				}
			}
		}

		if (bResult == FALSE)
		{
			static const LPCSTR pszDllNames[] = { "mfc140u.dll","mfc140.dll" };
			for (int i = 0; i < _countof(pszDllNames); i++)
			{
				if (!_stricmp(pDllName, pszDllNames[i]))
				{
					CStringA strNewMfcDllName = strNewMfcDllsName[i];
					if (strNewMfcDllName.GetLength() <= nDllNameLen)
					{
						lstrcpynA(pDllName, strNewMfcDllName, strNewMfcDllName.GetLength() + 1);
						bResult = TRUE;
						break;
					}
					else
					{
						printf("new dll length to long!!!(%s->%s)\r\n", pDllName, (LPCSTR)strNewMfcDllName);
						ATLASSERT(FALSE);
					}
				}
			}
		}
	}
	return bResult;
}

BOOL RemoveApiSets(LPCTSTR szFileName, ApiSetSchema* pApiSetSchema, CStringA strNewVcrDllName, CStringA strNewVcpDllsName[5], CStringA strNewConCrtDllName, CStringA strNewMfcDllsName[2])
{
	BOOL bRet = FALSE;
	HANDLE hFile = INVALID_HANDLE_VALUE;
	HANDLE hMapping = NULL;
	LPVOID lpImageBase = NULL;

	hFile = CreateFile(szFileName, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

	if (hFile == INVALID_HANDLE_VALUE)
	{
		goto __finish__;
	}

	hMapping = CreateFileMapping(hFile, NULL, PAGE_READWRITE, 0, 0, NULL);
	if (!hMapping)
	{
		goto __finish__;
	}

	lpImageBase = MapViewOfFile(hMapping, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);
	if (!lpImageBase)
	{
		goto __finish__;
	}

	PIMAGE_DOS_HEADER dos_header;
	PIMAGE_NT_HEADERS Nt_headers;

	dos_header = (PIMAGE_DOS_HEADER)lpImageBase;
	if (dos_header->e_magic != IMAGE_DOS_SIGNATURE)
	{
		SetLastError(ERROR_BAD_EXE_FORMAT);
		goto __finish__;
	}

	Nt_headers = (PIMAGE_NT_HEADERS) & ((const unsigned char*)(lpImageBase))[dos_header->e_lfanew];
	if (Nt_headers->Signature != IMAGE_NT_SIGNATURE)
	{
		SetLastError(ERROR_BAD_EXE_FORMAT);
		goto __finish__;
	}

#if _M_IX86
	if (Nt_headers->FileHeader.Machine != IMAGE_FILE_MACHINE_I386)
	{
		SetLastError(ERROR_INVALID_MODULETYPE);
		goto __finish__;
	}
#elif _M_AMD64
	if (Nt_headers->FileHeader.Machine != IMAGE_FILE_MACHINE_AMD64)
	{
		SetLastError(ERROR_INVALID_MODULETYPE);
		goto __finish__;
	}
#else
	ATLASSERT(FALSE);//δ֧�ִ���ļܹ�����
#endif

	//�滻IAT�е���Ŀ
	IMAGE_DATA_DIRECTORY ImageDataDirectory_Import = Nt_headers->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
	PIMAGE_IMPORT_DESCRIPTOR pImageImport = (PIMAGE_IMPORT_DESCRIPTOR)(ImageDataDirectory_Import.VirtualAddress == 0 ? NULL : (PBYTE)dos_header + RvaToOffset(Nt_headers, ImageDataDirectory_Import.VirtualAddress));

	if (pImageImport)
	{
		while (pImageImport->Name)
		{
			PCHAR pDllName = (PCHAR)((PBYTE)lpImageBase + RvaToOffset(Nt_headers, pImageImport->Name));

			if (pImageImport->OriginalFirstThunk)
			{
				PIMAGE_THUNK_DATA pFunctionNameThunk = (PIMAGE_THUNK_DATA)((PBYTE)lpImageBase + RvaToOffset(Nt_headers, pImageImport->OriginalFirstThunk));
				if (pFunctionNameThunk[0].u1.Ordinal & IMAGE_ORDINAL_FLAG)
				{
					TryDoReplaceDllNameItem(pDllName, pApiSetSchema, strNewVcrDllName, strNewVcpDllsName, strNewConCrtDllName, strNewMfcDllsName, (LPCSTR)IMAGE_ORDINAL(pFunctionNameThunk[0].u1.Ordinal));
				}
				else
				{
					PIMAGE_IMPORT_BY_NAME pByName = (PIMAGE_IMPORT_BY_NAME)((DWORD_PTR)lpImageBase + RvaToOffset(Nt_headers, pFunctionNameThunk[0].u1.AddressOfData));
					TryDoReplaceDllNameItem(pDllName, pApiSetSchema, strNewVcrDllName, strNewVcpDllsName, strNewConCrtDllName, strNewMfcDllsName, (LPCSTR)pByName->Name);
				}
			}

			++pImageImport;
		}
	}

	//�滻DIAT�е���Ŀ
	IMAGE_DATA_DIRECTORY ImageDataDirectory_DelayImport = Nt_headers->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT];
	ULONG dwSize = ImageDataDirectory_DelayImport.Size;
	PImgDelayDescr pDelay = (PImgDelayDescr)(ImageDataDirectory_DelayImport.VirtualAddress == 0 ? NULL : (PBYTE)dos_header + RvaToOffset(Nt_headers, ImageDataDirectory_DelayImport.VirtualAddress));

	if (pDelay && dwSize)
	{
		// �����ӳٵ����
		while (pDelay->rvaDLLName && pDelay->rvaIAT && pDelay->rvaINT)
		{
			PCHAR pDllName = (PCHAR)((PBYTE)lpImageBase + RvaToOffset(Nt_headers, pDelay->rvaDLLName));

			PIMAGE_THUNK_DATA pThunk = (PIMAGE_THUNK_DATA)((PBYTE)lpImageBase + RvaToOffset(Nt_headers, pDelay->rvaINT));

			// ѭ��IAT��
			if (pThunk->u1.Function)
			{
				if (!IMAGE_SNAP_BY_ORDINAL(pThunk->u1.Ordinal))
				{
					// ��ȡ���������
					PIMAGE_IMPORT_BY_NAME pImportName = (PIMAGE_IMPORT_BY_NAME)((PBYTE)lpImageBase + RvaToOffset(Nt_headers, pThunk->u1.AddressOfData));
					//ATLTRACE(_T("VA: %08X Hint: %08X, FunctionName: %s\n"), pThunkIAT->u1.Function,pImportName->Hint, pImportName->Name);

					TryDoReplaceDllNameItem(pDllName, pApiSetSchema, strNewVcrDllName, strNewVcpDllsName, strNewConCrtDllName, strNewMfcDllsName, (LPCSTR)pImportName->Name);
				}
				else
				{
					DWORD Ordinal = DWORD(IMAGE_ORDINAL(pThunk->u1.Ordinal));
					//ATLTRACE(_T("VA: %08X Hint:Ord FunctionName:%s.%d\n"), pThunkIAT->u1.Function,DllTitle, Ordinal);

					TryDoReplaceDllNameItem(pDllName, pApiSetSchema, strNewVcrDllName, strNewVcpDllsName, strNewConCrtDllName, strNewMfcDllsName, MAKEINTRESOURCEA(Ordinal));
				}
			}

			++pDelay;
		}
	}

	bRet = TRUE;

__finish__:
	if (lpImageBase)
	{
		UnmapViewOfFile(lpImageBase);
		lpImageBase = NULL;
	}

	if (hMapping)
	{
		CloseHandle(hMapping);
		hMapping = NULL;
	}

	if (hFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hFile);
		hFile = INVALID_HANDLE_VALUE;
	}

	return bRet;
}

int nRootDirLength;
BOOL DirectoryEnumProc_Internal(LPCTSTR FileFullPath, LPCTSTR cFileName, LPARAM lParam)
{
	ApiSetSchema* pApiSetSchema = (ApiSetSchema*)lParam;
	static CStringA strNewVcpDllsName[] = { "MSVCP14X.DLL","MSVCP14X_1.DLL","MSVCP14X_2.DLL","MSVCP14X_ATOMIC_WAIT.DLL","MSVCP14X_CODECVT_IDS.DLL" };
	static CStringA strNewMfcDllsName[] = { "MFC14XU.dll","MFC14X.dll" };
	BOOL bResult = RemoveApiSets(FileFullPath, pApiSetSchema, _T("MSVCR14X.dll"), strNewVcpDllsName, _T("CONCRT14X.dll"), strNewMfcDllsName);
	if (!bResult)
	{
		DWORD dwError = GetLastError();
		if (dwError != ERROR_BAD_EXE_FORMAT && dwError != ERROR_INVALID_MODULETYPE)
		{
			_tprintf(_T("RemoveApiSets failed! %s %s\r\n"), (LPCTSTR)Error(dwError), &FileFullPath[nRootDirLength]);
		}
	}
	return TRUE;
}

BOOL CALLBACK DirectoryEnumProc(LPWIN32_FIND_DATA lpFfd, LPCTSTR FileFullPath, LPARAM lParam)
{
	if (lpFfd->nFileSizeLow == 0 && lpFfd->nFileSizeHigh == 0)
	{
		return TRUE;
	}
	return DirectoryEnumProc_Internal(FileFullPath, lpFfd->cFileName, lParam);
}

int _tmain(int argc, _TCHAR* argv[])
{
	CString strDir_or_File;

	_tsetlocale(LC_ALL, _T(".ACP"));
	if (argc == 2 && (IsDir(argv[1]) || IsFile(argv[1])))
	{
		strDir_or_File = argv[1];
	}
	else
	{
		_tprintf(_T("Please input the file (folder) path:\r\n"));
		TCHAR szBuf[1024];
		_getts_s(szBuf);
		PathUnquoteSpaces(szBuf);
		if (IsDir(szBuf) || IsFile(szBuf))
		{
			strDir_or_File = szBuf;
		}
		else
		{
			return 1;
		}
	}

	if (IsDir(strDir_or_File))
	{
		if (strDir_or_File.Right(1) != _T('\\'))
		{
			strDir_or_File.AppendChar(_T('\\'));
		}
	}

	ApiSetSchema* pApiSetSchema = GetApiSetSchema();

	if (IsDir(strDir_or_File))
	{
		nRootDirLength = strDir_or_File.GetLength();
		EnumDirectory(strDir_or_File, DirectoryEnumProc, (LPARAM)pApiSetSchema);
	}
	else
	{
		if (FileLen(strDir_or_File).QuadPart > 0)
		{
			DirectoryEnumProc_Internal(strDir_or_File, GetFileName(strDir_or_File), (LPARAM)pApiSetSchema);
		}
	}

	CAtlArray<KeyValuePair<CString, ApiSetTarget*>>* pInfos = pApiSetSchema->GetAll();
	delete pInfos;
	pInfos = NULL;
	delete pApiSetSchema;
	pApiSetSchema = NULL;

	_tprintf(_T("All eligible files have been converted!\r\n"));
	getchar();
	return 0;
}

