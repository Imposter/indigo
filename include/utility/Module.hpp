/*
*   This file is part of the Indigo library.
*
*   This program is licensed under the GNU General
*   Public License. To view the full license, check
*   LICENSE in the project root.
*/

#ifndef indigo_module_hpp_
#define indigo_module_hpp_

#include "../Build.hpp"
#include "Memory.hpp"
#include <stdlib.h>
#include <stdio.h>
#include <functional>
#include <utility>

namespace indigo
{
	class INDIGO_API Module
	{
		void *(*mLoadLibrary)(const char *);
		void (*mFreeLibrary)(void *);
		void *(*mGetExport)(void *, const char *);

		static void *loadLibraryInternal(const char *pathName, void *userData);
		static void freeLibraryInternal(void *handle, void *userData);
		static void *getExportInternal(void *handle, const char *exportName, void *userData);

	protected:
		void *mMemory;
		size_t mSize;

		void *mModule;
		bool mLoaded;

	public:
		Module(void *memory, size_t size);
		Module(const Module &) = delete;
		~Module();

		void SetLoadLibrary(void *(*loadLibrary)(const char *));
		void SetFreeLibrary(void (*freeLibrary)(void *));
		void SetGetExport(void *(*getExport)(void *, const char *));

		bool Load();
		bool Unload();

		// This method does not return if successful
		bool Execute() const;
		void *GetExport(const std::string &name) const;
		void *GetImage() const;
	};

	class ModuleMemory : public Module
	{
	public:
		ModuleMemory(void *memory, size_t size)
			: Module(memory, size) { }

		ModuleMemory(const Module &) = delete;
	};

	class ModuleFile : public Module
	{
		std::string path_;

	public:
		ModuleFile(std::string path)
			: Module(nullptr, 0), path_(std::move(path)) { }

		ModuleFile(const ModuleFile &) = delete;

		~ModuleFile()
		{
			Unload();
		}

		bool Load()
		{
			// Check if already open
			if (mLoaded)
			{
				return false;
			}

			// Open the file
			FILE *file = fopen(path_.c_str(), "rb");
			if (file == nullptr)
			{
				return false;
			}

			// Get file size
			fseek(file, 0, SEEK_END);
			size_t size = ftell(file);
			fseek(file, 0, SEEK_SET);

			// Allocate the memory
			mMemory = malloc(size);
			mSize = size;

			// Read the file
			if (fread(mMemory, sizeof(char), mSize, file) != size)
			{
				fclose(file);
				free(mMemory);
				return false;
			}

			return Module::Load();
		}

		bool Unload()
		{
			if (mMemory != nullptr)
			{
				// Free memory
				free(mMemory);
				mMemory = nullptr;
				mSize = 0;
			}

			return Module::Unload();
		}
	};
}

#endif // indigo_module_hpp_
