/*
*   This file is part of the Indigo library.
*
*   This program is licensed under the GNU General
*   Public License. To view the full license, check
*   LICENSE in the project root.
*/

#ifndef indigo_typed_buffer_hpp_
#define indigo_typed_buffer_hpp_

// Required libraries
#include "../Core/Buffer.hpp"

namespace indigo
{
	class TypedBuffer : Buffer
	{
		enum DataType : uint8_t
		{
			kDataType_Bool,
			kDataType_Char,
			kDataType_Int8,
			kDataType_UInt8,
			kDataType_Int16,
			kDataType_UInt16,
			kDataType_Int32,
			kDataType_UInt32,
			kDataType_Int64,
			kDataType_UInt64,
			kDataType_Float,
			kDataType_String,
			kDataType_Blob,
		};

		bool verifyDataType(DataType expectedType)
		{
			// Check to see if we're not going to be reading past the end of the buffer
			if (GetPosition() == Buffer::GetSize())
				return false;

			// Peek ahead instead of reading
			uint8_t type = Buffer::GetBuffer()[GetPosition()];

			// Verify the data type
			if (type != static_cast<uint8_t>(expectedType))
				return false;

			// Increase the current position if the read was successful
			SetPosition(GetPosition() + sizeof type);

			return true;
		}

	public:
		TypedBuffer(bool flipEndian = false) : Buffer(flipEndian) { }

		TypedBuffer(const uint8_t *buffer, size_t size, bool flipEndian = false)
			: Buffer(buffer, size, flipEndian) { }

		bool ReadBoolean(bool &obj)
		{
			if (!verifyDataType(kDataType_Bool))
				return false;

			return Read(&obj);
		}

		bool ReadChar(char &obj)
		{
			if (!verifyDataType(kDataType_Char))
				return false;

			return Read(&obj);
		}

		bool ReadInt8(int8_t &obj)
		{
			if (!verifyDataType(kDataType_Int8))
				return false;

			return Read(&obj);
		}

		bool ReadUInt8(uint8_t &obj)
		{
			if (!verifyDataType(kDataType_UInt8))
				return false;

			return Read(&obj);
		}

		bool ReadInt16(int16_t &obj)
		{
			if (!verifyDataType(kDataType_Int16))
				return false;

			return Read(&obj);
		}

		bool ReadUInt16(uint16_t &obj)
		{
			if (!verifyDataType(kDataType_UInt16))
				return false;

			return Read(&obj);
		}

		bool ReadInt32(int32_t &obj)
		{
			if (!verifyDataType(kDataType_Int32))
				return false;

			return Read(&obj);
		}

		bool ReadUInt32(uint32_t &obj)
		{
			if (!verifyDataType(kDataType_UInt32))
				return false;

			return Read(&obj);
		}

		bool ReadInt64(int64_t &obj)
		{
			if (!verifyDataType(kDataType_Int64))
				return false;

			return Read(&obj);
		}

		bool ReadUInt64(uint64_t &obj)
		{
			if (!verifyDataType(kDataType_UInt64))
				return false;

			return Read(&obj);
		}

		bool ReadFloat(float &obj)
		{
			if (!verifyDataType(kDataType_Float))
				return false;

			return Read(&obj);
		}

		bool ReadString(std::string &obj)
		{
			if (!verifyDataType(kDataType_String))
				return false;

			uint32_t length = 0;
			if (!Read(&length))
				return false;

			obj.resize(length);
			return ReadArray<char>(const_cast<char *>(obj.c_str()), length);
		}

		bool ReadBlob(std::basic_string<uint8_t> &obj)
		{
			if (!verifyDataType(kDataType_Blob))
				return false;

			uint32_t length = 0;
			if (!Read(&length))
				return false;

			obj.resize(length);
			return ReadArray<uint8_t>(const_cast<uint8_t *>(obj.c_str()),
				length);
		}

		void WriteBoolean(bool obj)
		{
			Write<uint8_t>(kDataType_Bool);
			Write<bool>(obj);
		}

		void WriteChar(char obj)
		{
			Write<uint8_t>(kDataType_Char);
			Write<char>(obj);
		}

		void WriteInt8(int8_t obj)
		{
			Write<uint8_t>(kDataType_Int8);
			Write<int8_t>(obj);
		}

		void WriteUInt8(uint8_t obj)
		{
			Write<uint8_t>(kDataType_UInt8);
			Write<uint8_t>(obj);
		}

		void WriteInt16(int16_t obj)
		{
			Write<uint8_t>(kDataType_Int16);
			Write<int16_t>(obj);
		}

		void WriteUInt16(uint16_t obj)
		{
			Write<uint8_t>(kDataType_UInt16);
			Write<uint16_t>(obj);
		}

		void WriteInt32(int32_t obj)
		{
			Write<uint8_t>(kDataType_Int32);
			Write<int32_t>(obj);
		}

		void WriteUInt32(uint32_t obj)
		{
			Write<uint8_t>(kDataType_UInt32);
			Write<uint32_t>(obj);
		}

		void WriteInt64(int64_t obj)
		{
			Write<uint8_t>(kDataType_Int64);
			Write<int64_t>(obj);
		}

		void WriteUInt64(uint64_t obj)
		{
			Write<uint8_t>(kDataType_UInt64);
			Write<uint64_t>(obj);
		}

		void WriteFloat(float obj)
		{
			Write<uint8_t>(kDataType_Float);
			Write<float>(obj);
		}

		void WriteString(std::string obj)
		{
			Write<uint8_t>(kDataType_String);
			uint32_t length = obj.size();
			Write(length);
			WriteArray<char>(obj.c_str(), length);
		}

		void WriteBlob(std::basic_string<uint8_t> obj)
		{
			Write<uint8_t>(kDataType_Blob);
			uint32_t length = obj.size();
			Write(length);
			WriteArray<uint8_t>(obj.c_str(), length);
		}

		bool IsFlippingEndian() const
		{
			return Buffer::IsFlippingEndian();
		}

		void SetFlipEndian(bool flipEndian)
		{
			Buffer::SetFlipEndian(flipEndian);
		}

		void Rewind()
		{
			Buffer::Rewind();
		}

		void Resize(size_t size)
		{
			Buffer::Resize(size);
		}

		const uint8_t *GetBuffer() const
		{
			return Buffer::GetBuffer();
		}

		size_t GetSize() const
		{
			return Buffer::GetSize();
		}

		void Clear()
		{
			Buffer::Clear();
		}
	};
}

#endif // indigo_typed_buffer_hpp_
