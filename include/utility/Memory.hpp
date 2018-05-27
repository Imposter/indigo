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

#include "../core/String.hpp"
#include <utility>
#include <vector>
#include <cstdint>
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
			return {mAddress + count};
		}

		MemoryPointer Sub(int count) const
		{
			return {mAddress - count};
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
			: mPointers(std::move(pointers)) { }

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
				return {nullptr};

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
			IMAGE_NT_HEADERS *ntHeaders = GetNTHeader(GetModuleHandle(nullptr));
			return reinterpret_cast<void *>(ntHeaders->OptionalHeader.ImageBase);
		}

		static IMAGE_NT_HEADERS *GetNTHeader(void *address)
		{
			IMAGE_DOS_HEADER *prgDosHeader = reinterpret_cast<IMAGE_DOS_HEADER *>(address);
			IMAGE_NT_HEADERS *prgNtHeader = reinterpret_cast<IMAGE_NT_HEADERS *>(reinterpret_cast<int *>(prgDosHeader) + static_cast<int>(prgDosHeader->e_lfanew / 4));

			if (prgNtHeader->Signature != IMAGE_NT_SIGNATURE)
			{
				return nullptr;
			}

			return prgNtHeader;
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

		static void *Find(void *startAddress, size_t searchLength, const char *pattern, const char *mask)
		{
			const size_t maskLength = strlen(mask);

			for (size_t i = 0; i < searchLength - maskLength; ++i)
			{
				size_t foundBytes = 0;

				for (size_t j = 0; j < maskLength; ++j)
				{
					const char byteRead = *reinterpret_cast<char *>(static_cast<char *>(startAddress) + i + j);

					if (byteRead != pattern[j] && mask[j] != '?')
					{
						break;
					}

					++foundBytes;

					if (foundBytes == maskLength)
					{
						return static_cast<char *>(startAddress) + i;
					}
				}
			}

			return nullptr;
		}

		static MemoryPointerList Find(void *startAddress, size_t searchLength, const std::string &pattern)
		{
			std::vector<MemoryPointer> pointers;

			// Generate pattern
			std::vector<std::string> splitPattern = String::Split(pattern, " ");
			std::string mask;
			const auto cleanPattern = new uint8_t[splitPattern.size()];
			for (size_t i = 0; i < splitPattern.size(); i++)
			{
				const char *byteString = splitPattern[i].c_str();

				// Convert to number
				const long byte = strtol(byteString, nullptr, 16);
				cleanPattern[i] = static_cast<uint8_t>(byte);

				if (strcmp(byteString, "??") == 0)
				{
					mask.append("?");
				}
				else
				{
					mask.append("x");
				}
			}

			void *currentAddress = Find(startAddress, searchLength, reinterpret_cast<const char *>(const_cast<const uint8_t *>(cleanPattern)), mask.c_str());
			pointers.emplace_back(currentAddress);
			while (currentAddress != nullptr && currentAddress < static_cast<char *>(startAddress) + searchLength)
			{
				currentAddress = Find(static_cast<char *>(currentAddress) + 1,
				                       searchLength - (reinterpret_cast<size_t>(currentAddress) - reinterpret_cast<size_t>(startAddress)),
				                       reinterpret_cast<const char *>(const_cast<const uint8_t *>(cleanPattern)), mask.c_str());
				if (currentAddress != nullptr)
				{
					pointers.emplace_back(currentAddress);
				}
			}

			delete[] cleanPattern;

			return pointers;
		}

		static MemoryPointerList Find(const std::string &pattern)
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
