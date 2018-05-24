/*
*   This file is part of the Indigo library.
*
*   This program is licensed under the GNU General
*   Public License. To view the full license, check
*   LICENSE in the project root.
*/

#ifndef indigo_config_hpp_
#define indigo_config_hpp_

#include "../Core/String.hpp"
#include <stdint.h>
#include <map>
#include <mutex>
#include <utility>

namespace indigo
{
	class Config
	{
		std::string mBuffer;
		std::mutex mMutex;
		std::map<std::string, std::map<std::string, std::string>> mData;

	public:
		bool Open(std::string buffer)
		{
			mBuffer = std::move(buffer);
			mBuffer = String::Replace(mBuffer, "\r\n", "\n");

			std::vector<std::string> lines;
			if (String::Contains(mBuffer, "\n"))
				lines = String::Split(mBuffer, "\n");
			else
				lines = String::Split(mBuffer, "\n");

			std::string sectionName;
			std::map<std::string, std::string> section;
			for (auto &line : lines)
			{
				if (line.size() > 0)
				{
					if (line.find_first_of('[') == 0 && line.find_last_of(']') == line.size() - 1)
					{
						if (sectionName.size() != 0)
						{
							mData.insert(make_pair(sectionName, section));
							section.clear();
						}

						sectionName = line.substr(1, line.size() - 2);
					}
					else if (line.find_first_of('=') != std::string::npos)
					{
						size_t keyEnd = line.find_first_of('=');
						std::string key = line.substr(0, keyEnd);
						std::string value = line.substr(keyEnd + 1);
						if (!key.empty())
							section.insert(make_pair(key, value));
					}
				}
			}

			if (!section.empty())
			{
				mData.insert(make_pair(sectionName, section));
				section.clear();
			}

			return true;
		}

		void Close()
		{
			mBuffer.clear();
			mData.clear();
		}

		bool KeyExists(const std::string &section, const std::string &key)
		{
			return !(mData.find(section) == mData.end() || mData[section].find(key) == mData[section].end());
		}

		std::string GetString(const std::string &section, const std::string &key, std::string defaultValue = std::string())
		{
			if (mData.find(section) == mData.end() || mData[section].find(key) == mData[section].end())
			{
				SetString(section, key, defaultValue);
				return defaultValue;
			}
			
			return mData[section][key];
		}

		int64_t GetInteger(const std::string &section, const std::string &key, int64_t defaultValue = 0)
		{
			if (mData.find(section) == mData.end() || mData[section].find(key) == mData[section].end())
			{
				SetInteger(section, key, defaultValue);
				return defaultValue;
			}

			return stoull(mData[section][key], nullptr, 0);
		}

		float GetFloat(const std::string &section, const std::string &key, float defaultValue = 0)
		{
			if (mData.find(section) == mData.end() || mData[section].find(key) == mData[section].end())
			{
				SetFloat(section, key, defaultValue);
				return defaultValue;
			}

			return stof(mData[section][key]);
		}

		std::vector<std::string> GetStringList(const std::string &section, const std::string &key, std::vector<std::string> defaultValue = std::vector<std::string>())
		{
			if (mData.find(section) == mData.end() || mData[section].find(key) == mData[section].end())
			{
				SetStringList(section, key, defaultValue);
				return defaultValue;
			}

			int size = atoi(mData[section][key].c_str());
			std::vector<std::string> result;
			for (int i = 0; i < size; i++)
				result.push_back(mData[section][key + "." + std::to_string(i)]);

			return result;
		}

		std::vector<int64_t> GetIntegerList(const std::string &section, const std::string &key, std::vector<int64_t> defaultValue = std::vector<int64_t>())
		{
			if (mData.find(section) == mData.end() || mData[section].find(key) == mData[section].end())
			{
				SetIntegerList(section, key, defaultValue);
				return defaultValue;
			}

			int size = atoi(mData[section][key].c_str());
			std::vector<int64_t> result;
			for (int i = 0; i < size; i++)
				result.push_back(std::stoull(mData[section][key + "." + std::to_string(i)].c_str(), nullptr, 0));

			return result;
		}

		std::vector<float> GetFloatList(const std::string &section, const std::string &key, std::vector<float> defaultValue = std::vector<float>())
		{
			if (mData.find(section) == mData.end() || mData[section].find(key) == mData[section].end())
			{
				SetFloatList(section, key, defaultValue);				
				return defaultValue;
			}
			int size = atoi(mData[section][key].c_str());
			std::vector<float> result;
			for (int i = 0; i < size; i++)
				result.push_back(std::stof(mData[section][key + "." + std::to_string(i)].c_str()));

			return result;
		}

		std::map<std::string, std::string> GetStringMap(const std::string &section, const std::string &key, std::map<std::string, std::string> defaultValue
			                                                = std::map<std::string, std::string>())
		{
			if (mData.find(section) == mData.end() || mData[section].find(key) == mData[section].end())
			{
				SetStringMap(section, key, defaultValue);
				return defaultValue;
			}

			int32_t size = atoi(mData[section][key].c_str());
			std::map<std::string, std::string> result;
			do
			{
				for (auto &key_value_pair : mData[section])
					if (key_value_pair.first.find_first_of(key + ".") == 0)
						result[key_value_pair.first.substr(key_value_pair.first.find_first_of('.'))] = key_value_pair.second;
			}
			while (result.size() != size);

			return result;
		}

		std::map<std::string, int64_t> GetIntegerMap(const std::string &section, const std::string &key, std::map<std::string, int64_t> defaultValue
			                                             = std::map<std::string, int64_t>())
		{
			if (mData.find(section) == mData.end() || mData[section].find(key) == mData[section].end())
			{
				SetIntegerMap(section, key, defaultValue);
				return defaultValue;
			}

			int32_t size = atoi(mData[section][key].c_str());
			std::map<std::string, int64_t> result;
			do
			{
				for (auto &key_value_pair : mData[section])
					if (key_value_pair.first.find_first_of(key + ".") == 0)
						result[key_value_pair.first.substr(key_value_pair.first.find_first_of('.'))] = stoull(key_value_pair.second, nullptr, 0);
			}
			while (result.size() != size);

			return result;
		}

		std::map<std::string, float> GetFloatMap(const std::string &section, const std::string &key, std::map<std::string, float> defaultValue = std::map<std::string, float>())
		{
			if (mData.find(section) == mData.end() || mData[section].find(key) == mData[section].end())
			{
				SetFloatMap(section, key, defaultValue);
				return defaultValue;
			}

			int32_t size = atoi(mData[section][key].c_str());
			std::map<std::string, float> result;
			do
			{
				for (auto &key_value_pair : mData[section])
					if (key_value_pair.first.find_first_of(key + ".") == 0)
						result[key_value_pair.first.substr(key_value_pair.first.find_first_of('.'))] = stof(key_value_pair.second);
			}
			while (result.size() != size);

			return result;
		}

		void SetString(const std::string &section, const std::string &key, std::string value)
		{
			mData[section][key] = std::move(value);
		}

		void SetInteger(const std::string &section, const std::string &key, int64_t value)
		{
			mData[section][key] = std::to_string(value);
		}

		void SetFloat(const std::string &section, const std::string &key, float value)
		{
			mData[section][key] = std::to_string(value);
		}

		void SetStringList(const std::string &section, const std::string &key, std::vector<std::string> value)
		{
			mData[section][key] = std::to_string(value.size());
			for (size_t i = 0; i < value.size(); i++)
				mData[section][key + "." + std::to_string(i)] = value[i];
		}

		void SetIntegerList(const std::string &section, const std::string &key, std::vector<int64_t> value)
		{
			mData[section][key] = std::to_string(value.size());
			for (size_t i = 0; i < value.size(); i++)
				mData[section][key + "." + std::to_string(i)] = std::to_string(value[i]);
		}

		void SetFloatList(const std::string &section, const std::string &key, std::vector<float> value)
		{
			mData[section][key] = std::to_string(value.size());
			for (size_t i = 0; i < value.size(); i++)
				mData[section][key + "." + std::to_string(i)] = std::to_string(value[i]);
		}

		void SetStringMap(const std::string &section, const std::string &key, std::map<std::string, std::string> value)
		{
			mData[section][key] = std::to_string(value.size());
			for (auto &pair : value)
				mData[section][key + "." + pair.first] = pair.second;
		}

		void SetIntegerMap(const std::string &section, const std::string &key, std::map<std::string, int64_t> value)
		{
			mData[section][key] = std::to_string(value.size());
			for (auto &pair : value)
				mData[section][key + "." + pair.first] = std::to_string(pair.second);
		}

		void SetFloatMap(const std::string &section, const std::string &key, std::map<std::string, float> value)
		{
			mData[section][key] = std::to_string(value.size());
			for (auto &pair : value)
				mData[section][key + "." + pair.first] = std::to_string(pair.second);
		}

		std::string GetBuffer()
		{
			mBuffer.clear();
			for (auto &section : mData)
			{
				mBuffer.append("[" + section.first + "]\n");
				for (auto &data : section.second)
					mBuffer.append(data.first + "=" + data.second + "\n");
				mBuffer.append("\n");
			}

			for (auto it = mBuffer.begin(); it != mBuffer.end();)
			{
				if (*it == '\0')
					it = mBuffer.erase(it);
				else
					++it;
			}

			return mBuffer;
		}
	};
}

#endif // indigo_config_hpp_
