/*
*   This file is part of the Indigo library.
*
*   This program is licensed under the GNU General
*   Public License. To view the full license, check
*   LICENSE in the project root.
*/

#include "Platform.hpp"
#if !defined(OS_WIN)
#error "Unsupported platform!"
#endif

#include "Utility/Hook.hpp"
#include "Utility/Memory.hpp"
#include "Core/String.hpp"
#include <minhook/MinHook.h>

namespace indigo
{
	bool Hook::Install(void *target, void *function, void **original)
	{
		MH_STATUS status = MH_CreateHook(target, function, original);
		if (status == MH_ERROR_NOT_INITIALIZED)
		{
			MH_Initialize();
			MH_CreateHook(target, function, original);
		}

		status = MH_EnableHook(reinterpret_cast<void *>(target));

		return status == MH_OK;
	}

	bool Hook::Remove(void **original)
	{
		if (MH_DisableHook(original) != MH_OK)
			return false;

		return MH_RemoveHook(original) == MH_OK;
	}

	bool Hook::Install(const char *moduleName, const char *exportName, void *function, void **original)
	{
		HMODULE module = LoadLibraryA(moduleName);
		if (module == nullptr)
			return false;

		void *target = GetProcAddress(module, exportName);
		if (target == nullptr)
			return false;

		return Install(target, function, original);
	}

	bool Hook::Install(void *module, const char *moduleName, const char *importName, bool ordinal, void *function, void **original)
	{
		IMAGE_NT_HEADERS *ntHeaders = Memory::GetNTHeader(module);
		IMAGE_DATA_DIRECTORY *importDirectory = &ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
		auto *descriptor = Memory::GetRVA<IMAGE_IMPORT_DESCRIPTOR>(module, importDirectory->VirtualAddress);

		// Search imports
		for (; descriptor->Name; descriptor++)
		{
			// Check the module name
			const char *currentModuleName = Memory::GetRVA<char>(module, descriptor->Name);
			if (!String::Equals(currentModuleName, moduleName, true))
				continue;

			// Pointer to name and address entries
			auto *originalFirstThunk = Memory::GetRVA<IMAGE_THUNK_DATA>(module, descriptor->OriginalFirstThunk);
			auto *firstThunk = Memory::GetRVA<IMAGE_THUNK_DATA>(module, descriptor->FirstThunk);

			// Check if we have hints
			if (!descriptor->OriginalFirstThunk)
				originalFirstThunk = firstThunk;

			// Loop through all the functions
			bool replace = false;
			for (; originalFirstThunk->u1.Function; originalFirstThunk++ , firstThunk++)
			{
				if (ordinal && originalFirstThunk->u1.Ordinal & IMAGE_ORDINAL_FLAG)
				{
					replace = reinterpret_cast<char *>(IMAGE_ORDINAL(originalFirstThunk->u1.Ordinal)) == importName;
				}
				else if (!ordinal)
				{
					auto *importByName = Memory::GetRVA<IMAGE_IMPORT_BY_NAME>(module, *reinterpret_cast<uintptr_t *>(originalFirstThunk));
					replace = strcmp(importByName->Name, importName) == 0;
				}

				if (replace)
				{
					// Unprotect function
					DWORD funcProt = Memory::SetProtection(firstThunk, sizeof firstThunk, PAGE_EXECUTE_READWRITE);

					// Store original
					if (original != nullptr)
						*original = *reinterpret_cast<FARPROC *>(firstThunk);

					// Change reference to our function
					*reinterpret_cast<FARPROC *>(firstThunk) = static_cast<FARPROC>(function);

					// Protect function
					Memory::SetProtection(firstThunk, sizeof firstThunk, funcProt);

					return true;
				}
			}
		}

		return false;
	}
}
