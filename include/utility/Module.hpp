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
#include "memory.hpp"
#include <stdlib.h>
#include <stdio.h>
#include <functional>

namespace indigo
{
	class INDIGO_API Module
	{
		std::function<void *(const char *)> mLoadLibrary;
		std::function<void (void *)> mFreeLibrary;
		std::function<void *(void *, const char *)> mGetExport;

		static void *loadLibraryInternal(const char *path_name, void *user_data);
		static void freeLibraryInternal(void *handle, void *user_data);
		static void *getExportInternal(void *handle, const char *export_name, void *user_data);

	protected:
		void *mMemory;
		size_t mSize;

		void *mModule;
		bool mLoaded;

	public:
		Module(void *memory, size_t size);
		Module(const Module &) = delete;
		~Module();

		void SetLoadLibrary(std::function<void *(const char *)> loadLibrary);
		void SetFreeLibrary(std::function<void(void *)> freeLibrary);
		void SetGetExport(std::function<void *(void *, const char *)> getExport);

		bool Load();
		bool Unload();

		// This method does not return if successful
		bool Execute() const;

		void *GetExport(std::string name) const;
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
			: Module(nullptr, 0), path_(path) { }

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
