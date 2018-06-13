/*
 *  This file is part of the Indigo library.
*
 *  This program is licensed under the GNU General
 *  Public License. To view the full license, check
 *  LICENSE in the project root.
*/

#include "utility/Module.hpp"
#include "MemoryModule.h"

namespace indigo
{
	typedef int (WINAPI *ExeEntryProc)();
	typedef struct
	{
		PIMAGE_NT_HEADERS headers;
		unsigned char *codeBase;
		HCUSTOMMODULE *modules;
		int numModules;
		BOOL initialized;
		BOOL isDLL;
		BOOL isRelocated;
		CustomAllocFunc alloc;
		CustomFreeFunc free;
		CustomLoadLibraryFunc loadLibrary;
		CustomGetProcAddressFunc getProcAddress;
		CustomFreeLibraryFunc freeLibrary;
		void *userdata;
		ExeEntryProc exeEntry;
		DWORD pageSize;
	} MEMORYMODULE, *PMEMORYMODULE;

	Module::Module(void *memory, size_t size)
		: mLoadLibrary(nullptr), mFreeLibrary(nullptr), mGetExport(nullptr), mMemory(memory), mSize(size), mModule(nullptr), mLoaded(false) {}

	Module::~Module()
	{
		Unload();
	}

	void Module::SetLoadLibrary(void *(*loadLibrary)(const char *))
	{
		mLoadLibrary = loadLibrary;
	}

	void Module::SetFreeLibrary(void (*freeLibrary)(void *))
	{
		mFreeLibrary = freeLibrary;
	}

	void Module::SetGetExport(void *(*getExport)(void *, const char *))
	{
		mGetExport = getExport;
	}

	bool Module::Load()
	{
		// Check if already loaded
		if (mLoaded)
			return false;

		// Try to load the module
		if ((mModule = MemoryLoadLibraryEx(mMemory, mSize, MemoryDefaultAlloc, MemoryDefaultFree,
		                                   loadLibraryInternal, reinterpret_cast<FARPROC (*)(HCUSTOMMODULE, LPCSTR, void *)>(getExportInternal),
		                                   freeLibraryInternal, this)) == nullptr)
			return false;

		// Set as loaded
		mLoaded = true;

		return true;
	}

	bool Module::Unload()
	{
		// Check if not loaded
		if (!mLoaded)
			return false;

		// Unload the module
		MemoryFreeLibrary(mModule);

		// Set as unloaded
		mLoaded = false;

		return true;
	}

	bool Module::Execute() const
	{
		// Check if loaded
		if (!mLoaded)
			return false;

		// Update the entrypoint
		auto *module = reinterpret_cast<MEMORYMODULE *>(mModule);
		IMAGE_NT_HEADERS *ntHeaders = Memory::GetNTHeader(module->codeBase);
		module->exeEntry = reinterpret_cast<ExeEntryProc>(ntHeaders->OptionalHeader.ImageBase + ntHeaders->OptionalHeader.AddressOfEntryPoint);

		// Call the entrypoint
		return MemoryCallEntryPoint(mModule) != -1;
	}

	void *Module::GetExport(const std::string &name) const
	{
		return MemoryGetProcAddress(mModule, name.c_str());
	}

	void *Module::GetImage() const
	{
		return reinterpret_cast<MEMORYMODULE *>(mModule)->codeBase;
	}

	void *Module::loadLibraryInternal(const char *pathName, void *userData)
	{
		auto *module = reinterpret_cast<Module *>(userData);
		if (!module->mLoadLibrary)
			return LoadLibraryA(pathName);

		return module->mLoadLibrary(pathName);
	}

	void Module::freeLibraryInternal(void *handle, void *userData)
	{
		auto *module = reinterpret_cast<Module *>(userData);
		if (!module->mFreeLibrary)
			FreeLibrary(static_cast<HMODULE>(handle));

		module->mFreeLibrary(handle);
	}

	void *Module::getExportInternal(void *handle, const char *exportName, void *userData)
	{
		auto *module = reinterpret_cast<Module *>(userData);
		if (!module->mGetExport)
			return GetProcAddress(static_cast<HMODULE>(handle), exportName);

		return module->mGetExport(handle, exportName);
	}
}
