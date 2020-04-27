// RemoveApiSets.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "Phlib.h"

//https://www.51hint.com/rva-to-file-offset.html
//RVA 转 文件偏移地址
DWORD RvaToOffset(PIMAGE_NT_HEADERS pNt, DWORD dwRva)
{
	DWORD nCount = pNt->FileHeader.NumberOfSections;
	PIMAGE_SECTION_HEADER pSection = IMAGE_FIRST_SECTION(pNt);
	if (pSection == NULL)
	{
		return 0;
	}
	for (DWORD i = 0; i < nCount; i++, pSection++)
	{
		if ((dwRva >= pSection->VirtualAddress) && (dwRva <= (pSection->VirtualAddress + pSection->Misc.VirtualSize)))
		{
			return dwRva - (pSection->VirtualAddress - pSection->PointerToRawData);
		}
	}
	return 0;
}

BOOL TryDoReplaceDllNameItem(PCHAR pDllName, ApiSetSchema* pApiSetSchema, CStringA strNewCrtDllName)
{
	BOOL bResult = FALSE;
	CStringA strOldDllTitleName = GetFileTitleName(pDllName);
	ApiSetTarget* pApiSetTarget = pApiSetSchema->Lookup(strOldDllTitleName);
	if (pApiSetTarget)
	{
		CStringA strNewDllName = pApiSetTarget->GetAt(0);
		if (strNewDllName.GetLength() <= (int)strlen(pDllName))
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
		if (!_strnicmp(pDllName, "api-ms-win-crt-", _countof("api-ms-win-crt-") - 1))
		{
			if (strNewCrtDllName.GetLength() <= (int)strlen(pDllName))
			{
				lstrcpynA(pDllName, strNewCrtDllName, strNewCrtDllName.GetLength() + 1);
				bResult = TRUE;
			}
			else
			{
				printf("new dll length to long!!!(%s->%s)\r\n", pDllName, (LPCSTR)strNewCrtDllName);
				ATLASSERT(FALSE);
			}
		}
	}
	return bResult;
}

BOOL RemoveApiSets(LPCTSTR szFileName, ApiSetSchema* pApiSetSchema, CStringA strNewCrtDllName)
{
	BOOL bRet = FALSE;
	HANDLE hFile;
	HANDLE hMapping;
	LPVOID lpImageBase;

	hFile = CreateFile(szFileName, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

	if (!hFile)
	{
		return FALSE;
	}

	hMapping = CreateFileMapping(hFile, NULL, PAGE_READWRITE, 0, 0, NULL);
	if (!hMapping)
	{
		CloseHandle(hFile);
		return FALSE;
	}

	lpImageBase = MapViewOfFile(hMapping, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);
	if (!lpImageBase)
	{
		CloseHandle(hFile);
		CloseHandle(hMapping);
		return false;
	}

	PIMAGE_DOS_HEADER dos_header;
	PIMAGE_NT_HEADERS Nt_headers;

	dos_header = (PIMAGE_DOS_HEADER)lpImageBase;
	if (dos_header->e_magic != IMAGE_DOS_SIGNATURE)
	{
		SetLastError(ERROR_BAD_EXE_FORMAT);
		return FALSE;
	}

	Nt_headers = (PIMAGE_NT_HEADERS) & ((const unsigned char *)(lpImageBase))[dos_header->e_lfanew];
	if (Nt_headers->Signature != IMAGE_NT_SIGNATURE)
	{
		SetLastError(ERROR_BAD_EXE_FORMAT);
		return FALSE;
	}

	//替换IAT中的条目
	IMAGE_DATA_DIRECTORY ImageDataDirectory_Import = Nt_headers->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
	PIMAGE_IMPORT_DESCRIPTOR pImageImport = (PIMAGE_IMPORT_DESCRIPTOR)(ImageDataDirectory_Import.VirtualAddress == 0 ? NULL : (PBYTE)dos_header + RvaToOffset(Nt_headers, ImageDataDirectory_Import.VirtualAddress));

	if (pImageImport)
	{
		while (pImageImport->Name)
		{
			PCHAR pDllName = (PCHAR)((PBYTE)lpImageBase + RvaToOffset(Nt_headers, pImageImport->Name));
			TryDoReplaceDllNameItem(pDllName, pApiSetSchema, strNewCrtDllName);
			++pImageImport;
		}
	}

	//替换DIAT中的条目
	IMAGE_DATA_DIRECTORY ImageDataDirectory_DelayImport = Nt_headers->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT];
	ULONG dwSize = ImageDataDirectory_DelayImport.Size;
	PImgDelayDescr pDelay = (PImgDelayDescr)(ImageDataDirectory_DelayImport.VirtualAddress == 0 ? NULL : (PBYTE)dos_header + RvaToOffset(Nt_headers, ImageDataDirectory_DelayImport.VirtualAddress));

	if (pDelay && dwSize)
	{
		// 遍历延迟导入表
		while (pDelay->rvaDLLName && pDelay->rvaIAT && pDelay->rvaINT)
		{
			PCHAR pDllName = (PCHAR)((PBYTE)lpImageBase + RvaToOffset(Nt_headers, pDelay->rvaDLLName));
			TryDoReplaceDllNameItem(pDllName, pApiSetSchema, strNewCrtDllName);

			++pDelay;
		}
	}

	if (lpImageBase)
	{
		UnmapViewOfFile(lpImageBase);
	}

	if (hMapping)
	{
		CloseHandle(hMapping);
	}

	if (hFile)
	{
		CloseHandle(hFile);
	}

	return bRet;
}


int _tmain(int argc, _TCHAR *argv[])
{
	if (argc < 2)
	{
		return 1;
	}

	ApiSetSchema* pApiSetSchema = GetApiSetSchema();

	RemoveApiSets(argv[1], pApiSetSchema, _T("MSVCR140_.dll"));

	CAtlArray<KeyValuePair<CString, ApiSetTarget*>>* pInfos = pApiSetSchema->GetAll();
	delete pInfos;
	pInfos = NULL;
	delete pApiSetSchema;
	pApiSetSchema = NULL;
	return 0;
}

