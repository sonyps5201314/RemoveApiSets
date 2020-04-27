#pragma once
//´úÂëÐÞ¸Ä×Ôhttps://github.com/lucasg/Dependencies.git

#define PTR_ADD_OFFSET(Pointer, Offset) ((PVOID)((ULONG_PTR)(Pointer) + (ULONG_PTR)(Offset)))

template<typename TKey, typename TValue>
class KeyValuePair
{
public:
	TKey key;
	TValue value;

public:
	KeyValuePair() : key(NULL) /*,value(NULL)*/
	{

	}
	KeyValuePair(TKey key, TValue value)
	{
		this->key = key;
		this->value = value;
	}
};


typedef CAtlArray<CString> ApiSetTarget;

class ApiSetSchema
{
public:
	virtual CAtlArray<KeyValuePair<CString, ApiSetTarget*>>* GetAll() = 0;
	virtual ApiSetTarget* Lookup(LPCTSTR name) = 0;
};

ApiSetSchema* GetApiSetSchema(LPVOID lpImageBase, PIMAGE_NT_HEADERS pNt);
ApiSetSchema* GetApiSetSchema();