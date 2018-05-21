/*
*   This file is part of the Indigo library.
*
*   This program is licensed under the GNU General
*   Public License. To view the full license, check
*   LICENSE in the project root.
*/

#ifndef indigo_memory_hpp_
#define indigo_memory_hpp_

#include "../Platform.hpp"
#if !defined(OS_WIN)
#error "Unsupported platform!"
#endif

#include "../Core/String.hpp"
#include <vector>
#include <stdint.h>
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>

namespace indigo
{
	class MemoryPointer
	{
		char *mAddress;

	public:
		MemoryPointer(void *address)
			: mAddress(static_cast<char *>(address)) { }

		MemoryPointer Add(int count) const
		{
			return MemoryPointer(mAddress + count);
		}

		MemoryPointer Sub(int count) const
		{
			return MemoryPointer(mAddress - count);
		}

		void *Get() const
		{
			return mAddress;
		}

		template <typename _TValue>
		_TValue As() const
		{
			return reinterpret_cast<_TValue>(mAddress);
		}
	};

	class MemoryPointerList
	{
		std::vector<MemoryPointer> mPointers;

	public:
		MemoryPointerList(std::vector<MemoryPointer> pointers)
			: mPointers(pointers) { }

		size_t Count() const
		{
			return mPointers.size();
		}

		MemoryPointerList Skip(int count) const
		{
			return MemoryPointerList(std::vector<MemoryPointer>(mPointers.begin() + count, mPointers.end()));
		}

		MemoryPointerList Take(int count) const
		{
			return MemoryPointerList(std::vector<MemoryPointer>(mPointers.begin(), mPointers.end() - count));
		}

		MemoryPointer Select(int index) const
		{
			if (index >= mPointers.size())
				return MemoryPointer(nullptr);

			return mPointers[index];
		}
	};

	class Memory
	{
	public:
		template <typename _TRVA>
		static _TRVA *GetRVA(void *module, size_t rva)
		{
			return reinterpret_cast<_TRVA *>(static_cast<char *>(module) + rva);
		}

		static void *GetProcessBaseAddress()
		{
			IMAGE_NT_HEADERS *nt_headers = GetNTHeader(GetModuleHandle(nullptr));
			return reinterpret_cast<void *>(nt_headers->OptionalHeader.ImageBase);
		}

		static IMAGE_NT_HEADERS *GetNTHeader(void *address)
		{
			IMAGE_DOS_HEADER *prg_dos_header = reinterpret_cast<IMAGE_DOS_HEADER *>(address);
			IMAGE_NT_HEADERS *prg_nt_header = reinterpret_cast<IMAGE_NT_HEADERS *>(reinterpret_cast<int *>(prg_dos_header) + static_cast<int>(prg_dos_header->e_lfanew / 4));

			if (prg_nt_header->Signature != IMAGE_NT_SIGNATURE)
			{
				return nullptr;
			}

			return prg_nt_header;
		}

		static void *GetProcessTextSectionStart(void *address = GetProcessBaseAddress())
		{
			return reinterpret_cast<void *>(reinterpret_cast<char *>(address) + GetNTHeader(address)->OptionalHeader.BaseOfCode);
		}

		static size_t GetProcessTextSectionSize(void *address = GetProcessBaseAddress())
		{
			return GetNTHeader(address)->OptionalHeader.SizeOfCode;
		}

		static size_t GetProcessImageSize(void *address = GetProcessBaseAddress())
		{
			return GetNTHeader(address)->OptionalHeader.SizeOfImage;
		}

		static void *GetModuleEntrypoint(void *address = GetProcessBaseAddress())
		{
			return reinterpret_cast<void *>(reinterpret_cast<char *>(address) + GetNTHeader(address)->OptionalHeader.AddressOfEntryPoint);
		}

		static void *Find(void *start_address, size_t search_length, const char *pattern, const char *mask)
		{
			size_t mask_length = strlen(mask);

			for (size_t i = 0; i < search_length - mask_length; ++i)
			{
				size_t found_bytes = 0;

				for (size_t j = 0; j < mask_length; ++j)
				{
					char byte_read = *reinterpret_cast<char *>(static_cast<char *>(start_address) + i + j);

					if (byte_read != pattern[j] && mask[j] != '?')
					{
						break;
					}

					++found_bytes;

					if (found_bytes == mask_length)
					{
						return static_cast<char *>(start_address) + i;
					}
				}
			}

			return nullptr;
		}

		static MemoryPointerList Find(void *start_address, size_t search_length, const char *pattern)
		{
			std::vector<MemoryPointer> pointers;

			// Generate pattern
			std::vector<std::string> split_pattern = String::Split(pattern, " ");
			std::string mask;
			uint8_t *clean_pattern = new uint8_t[split_pattern.size()];
			for (size_t i = 0; i < split_pattern.size(); i++)
			{
				const char *byte_string = split_pattern[i].c_str();

				// Convert to number
				long byte = strtol(byte_string, nullptr, 16);
				clean_pattern[i] = static_cast<uint8_t>(byte);

				if (strcmp(byte_string, "??") == 0)
				{
					mask.append("?");
				}
				else
				{
					mask.append("x");
				}
			}

			void *current_address = Find(start_address, search_length, reinterpret_cast<const char *>(const_cast<const uint8_t *>(clean_pattern)), mask.c_str());
			pointers.push_back(current_address);
			while (current_address != nullptr && current_address < static_cast<char *>(start_address) + search_length)
			{
				current_address = Find(static_cast<char *>(current_address) + 1,
				                       search_length - (reinterpret_cast<size_t>(current_address) - reinterpret_cast<size_t>(start_address)),
				                       reinterpret_cast<const char *>(const_cast<const uint8_t *>(clean_pattern)), mask.c_str());
				if (current_address != nullptr)
				{
					pointers.push_back(current_address);
				}
			}

			delete[] clean_pattern;

			return pointers;
		}

		static MemoryPointerList Find(const char *pattern)
		{
			return Find(GetProcessBaseAddress(), GetProcessImageSize(), pattern);
		}

		template <typename _TAddress>
		static void Patch(_TAddress address, void *data, size_t size)
		{
			DWORD protection = SetProtection(address, size, PAGE_EXECUTE_READWRITE);
			memcpy(reinterpret_cast<void *>(address), data, size);
			SetProtection(address, size, protection);
		}

		template <typename _TAddress, typename _TValue>
		static void Patch(_TAddress address, _TValue value)
		{
			Patch(address, &value, sizeof(value));
		}

		template <typename _TAddress>
		static DWORD SetProtection(_TAddress address, size_t size, DWORD flag)
		{
			DWORD old_protect;
			VirtualProtect(reinterpret_cast<void *>(address), size, flag, &old_protect);
			return old_protect;
		}

		template <typename _TAddress>
		static void Nop(_TAddress address, size_t size)
		{
			DWORD protection = SetProtection(address, size, PAGE_EXECUTE_READWRITE);
			memset(reinterpret_cast<void *>(address), 0x90, size);
			SetProtection(address, size, protection);
		}
	};
}

#endif // indigo_memory_hpp_
