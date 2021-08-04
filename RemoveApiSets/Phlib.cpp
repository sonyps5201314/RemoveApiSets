#include "stdafx.h"
#include "Phlib.h"

#include <ApiSet.h>


//#include <phnative.h>
//#include <ntpsapi.h>


#ifdef __cplusplus
extern "C" {
#endif

PAPI_SET_NAMESPACE GetApiSetNamespace()
{
	ULONG	ReturnLength;
	PROCESS_BASIC_INFORMATION ProcessInformation;
	PAPI_SET_NAMESPACE apiSetMap = NULL;

	//	Retrieve PEB address
	if (!NT_SUCCESS(NtQueryInformationProcess(
		GetCurrentProcess(),
		ProcessBasicInformation,
		&ProcessInformation,
		sizeof(PROCESS_BASIC_INFORMATION),
		&ReturnLength
	)))
	{
		return NULL;
	}

	//	Parsing PEB structure and locating api set map
	PPEB peb = static_cast<PPEB>(ProcessInformation.PebBaseAddress);
	apiSetMap =  static_cast<PAPI_SET_NAMESPACE>(peb->ApiSetMap);
	return apiSetMap;
}

#ifdef __cplusplus
}
#endif

struct ApiSetSchemaImpl
{
    static ApiSetSchema* ParseApiSetSchema(API_SET_NAMESPACE const * apiSetMap);

private:
    // private implementation of ApiSet schema parsing
    static ApiSetSchema* GetApiSetSchemaV2(API_SET_NAMESPACE_V2 const * map);
    static ApiSetSchema* GetApiSetSchemaV4(API_SET_NAMESPACE_V4 const * map);
    static ApiSetSchema* GetApiSetSchemaV6(API_SET_NAMESPACE_V6 const * map);
};

class EmptyApiSetSchema sealed : public ApiSetSchema
{
public:
	CAtlArray<KeyValuePair<CString, ApiSetTarget*>>* GetAll() override { return new CAtlArray<KeyValuePair<CString, ApiSetTarget*>>(); }
    ApiSetTarget* Lookup(CString) override { return nullptr; }
};

class V2V4ApiSetSchema sealed : public ApiSetSchema
{
public:
    CAtlArray<KeyValuePair<CString, ApiSetTarget*>>* const All = new CAtlArray<KeyValuePair<CString, ApiSetTarget*>>();

    CAtlArray<KeyValuePair<CString, ApiSetTarget*>>* GetAll() override { return All; }
    ApiSetTarget* Lookup(CString name) override
    {
		// TODO : check if ext- is not present on win7 and 8.1
        if (_tcsnicmp(name,_T("api-"), 4))
            return nullptr;

		// Force lowercase name
		name = name.MakeLower();

		// remove "api-" or "ext-" prefix
		name = name.Mid(4);

        // Note: The list is initially alphabetically sorted!!!
        auto min = (INT_PTR)0;
        auto max = (INT_PTR)All->GetCount() - 1;
		while (min <= max)
		{
			auto const cur = (min + max) / 2;
			auto pair = All->GetAt(cur);

			if (!_tcsicmp(name, pair.key))
				return pair.value;

			if (_tcscmp(name, pair.key) < 0)
				max = cur - 1;
			else
				min = cur + 1;
		}
        return nullptr;
    }
};

ApiSetSchema* ApiSetSchemaImpl::GetApiSetSchemaV2(API_SET_NAMESPACE_V2 const * const map)
{
	auto const base = reinterpret_cast<ULONG_PTR>(map);
	auto const schema = new V2V4ApiSetSchema();
	for (auto it = map->Array, eit = it + map->Count; it < eit; ++it)
	{
		// Retrieve DLLs names implementing the contract
		auto const targets = new ApiSetTarget();
		auto const value_entry = reinterpret_cast<PAPI_SET_VALUE_ENTRY_V2>(base + it->DataOffset);
		for (auto it2 = value_entry->Redirections, eit2 = it2 + value_entry->NumberOfRedirections; it2 < eit2; ++it2)
		{
			auto const value_buffer = reinterpret_cast<PWCHAR>(base + it2->ValueOffset);
			auto const value = CString(value_buffer, it2->ValueLength / sizeof(WCHAR));
			targets->Add(value);
		}

		// Retrieve api min-win contract name
		auto const name_buffer = reinterpret_cast<PWCHAR>(base + it->NameOffset);
		auto name = CString(name_buffer, it->NameLength / sizeof(WCHAR));

		// force storing lowercase variant for comparison
		auto const lower_name = name.MakeLower();
		
		schema->All->Add(KeyValuePair<CString, ApiSetTarget*>(lower_name, targets));
	}
	return schema;
}

ApiSetSchema* ApiSetSchemaImpl::GetApiSetSchemaV4(API_SET_NAMESPACE_V4 const * const map)
{
	auto const base = reinterpret_cast<ULONG_PTR>(map);
	auto const schema = new V2V4ApiSetSchema();
	for (auto it = map->Array, eit = it + map->Count; it < eit; ++it)
	{
		// Retrieve DLLs names implementing the contract
		auto const targets = new ApiSetTarget();
		auto const value_entry = reinterpret_cast<PAPI_SET_VALUE_ENTRY_V4>(base + it->DataOffset);
		for (auto it2 = value_entry->Redirections, eit2 = it2 + value_entry->NumberOfRedirections; it2 < eit2; ++it2)
		{
			auto const value_buffer = reinterpret_cast<PWCHAR>(base + it2->ValueOffset);
			auto const value = CString(value_buffer, it2->ValueLength / sizeof(WCHAR));
			targets->Add(value);
		}

		// Retrieve api min-win contract name
		auto const name_buffer = reinterpret_cast<PWCHAR>(base + it->NameOffset);
		auto name = CString(name_buffer, it->NameLength / sizeof(WCHAR));

		// force storing lowercase variant for comparison
		auto const lower_name = name.MakeLower();

		schema->All->Add(KeyValuePair<CString, ApiSetTarget*>(lower_name, targets));
	}
	return schema;
}

class V6ApiSetSchema sealed : public ApiSetSchema
{
public:
    CAtlArray<KeyValuePair<CString, ApiSetTarget*>>* const All = new CAtlArray<KeyValuePair<CString, ApiSetTarget*>>();
    CAtlArray<KeyValuePair<CString, ApiSetTarget*>>* HashedAll = new CAtlArray<KeyValuePair<CString, ApiSetTarget*>>();

    CAtlArray<KeyValuePair<CString, ApiSetTarget*>>* GetAll() override { return All; }
    ApiSetTarget* Lookup(CString name) override
    {
		// Force lowercase name
		name = name.MakeLower();

        // Note: The list is initially alphabetically sorted!!!
        auto min = (INT_PTR)0;
        auto max = (INT_PTR)HashedAll->GetCount() - 1;
        while (min <= max)
        {
            auto const cur = (min + max) / 2;
            auto pair = HashedAll->GetAt(cur);
            
			if (!_tcsnicmp(name, pair.key, pair.key.GetLength()))
				return pair.value;

            if (_tcscmp(name, pair.key) < 0)
                max = cur - 1;
            else
                min = cur + 1;
        }
        return nullptr;
    }
};

ApiSetSchema* ApiSetSchemaImpl::GetApiSetSchemaV6(API_SET_NAMESPACE_V6 const * const map)
{
	auto const base = reinterpret_cast<ULONG_PTR>(map);
	auto const schema = new V6ApiSetSchema();
	for (auto it = reinterpret_cast<PAPI_SET_NAMESPACE_ENTRY_V6>(map->EntryOffset + base), eit = it + map->Count; it < eit; ++it)
	{
		// Iterate over all the host dll for this contract
		auto const targets = new ApiSetTarget();
		for (auto it2 = static_cast<_API_SET_VALUE_ENTRY_V6*const>(reinterpret_cast<PAPI_SET_VALUE_ENTRY_V6>(base + it->ValueOffset)), eit2 = it2 + it->ValueCount; it2 < eit2; ++it2)
		{
			// Retrieve DLLs name implementing the contract
			auto const value_buffer = reinterpret_cast<PWCHAR>(base + it2->ValueOffset);
			auto const value = CString(value_buffer, it2->ValueLength / sizeof(WCHAR));
			targets->Add(value);
		}

		// Retrieve api min-win contract name
		auto const name_buffer = reinterpret_cast<PWCHAR>(base + it->NameOffset);
		auto name = CString(name_buffer, it->NameLength / sizeof(WCHAR));
		auto hash_name = CString(name_buffer, it->HashedLength / sizeof(WCHAR));

		// force storing lowercase variant for comparison
		auto const lower_name = name.MakeLower();
		auto const lower_hash_name = hash_name.MakeLower();

		schema->All->Add(KeyValuePair<CString, ApiSetTarget*>(lower_name, targets));
		schema->HashedAll->Add(KeyValuePair<CString, ApiSetTarget*>(lower_hash_name, targets));
	}
	return schema;
}

ApiSetSchema* GetApiSetSchema()
{
    // Api set schema resolution adapted from https://github.com/zodiacon/WindowsInternals/blob/master/APISetMap/APISetMap.cpp
    // References :
    // 		* Windows Internals v7
    // 		* @aionescu's slides on "Hooking Nirvana" (RECON 2015)
    //		* Quarkslab blog posts : 
    // 				https://blog.quarkslab.com/runtime-dll-name-resolution-apisetschema-part-i.html
    // 				https://blog.quarkslab.com/runtime-dll-name-resolution-apisetschema-part-ii.html
    return ApiSetSchemaImpl::ParseApiSetSchema(GetApiSetNamespace());
}


ApiSetSchema* GetApiSetSchema(LPVOID lpMappedImageBase, PIMAGE_NT_HEADERS pNt)
{
	PIMAGE_SECTION_HEADER pSection = IMAGE_FIRST_SECTION(pNt);
	if (pSection == NULL)
	{
		return new EmptyApiSetSchema();
	}

    for (auto n = 0u; n < pNt->FileHeader.NumberOfSections; ++n)
    {
        IMAGE_SECTION_HEADER const & section = *pSection;
        if (strncmp(".apiset", reinterpret_cast<char const*>(section.Name), IMAGE_SIZEOF_SHORT_NAME) == 0)
            return ApiSetSchemaImpl::ParseApiSetSchema(reinterpret_cast<PAPI_SET_NAMESPACE>(PTR_ADD_OFFSET(lpMappedImageBase, section.PointerToRawData)));
    }
    return new EmptyApiSetSchema();
}


ApiSetSchema* ApiSetSchemaImpl::ParseApiSetSchema(API_SET_NAMESPACE const * const apiSetMap)
{
	// Check the returned api namespace is correct
	if (!apiSetMap)
		return new EmptyApiSetSchema();

	switch (apiSetMap->Version) 
	{
		case 2: // Win7
			return GetApiSetSchemaV2(&apiSetMap->ApiSetNameSpaceV2);

		case 4: // Win8.1
			return GetApiSetSchemaV4(&apiSetMap->ApiSetNameSpaceV4);

		case 6: // Win10
			return GetApiSetSchemaV6(&apiSetMap->ApiSetNameSpaceV6);

		default: // unsupported
			return new EmptyApiSetSchema();
	}
}