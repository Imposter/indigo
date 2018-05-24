/*
*   This file is part of the Indigo library.
*
*   This program is licensed under the GNU General
*   Public License. To view the full license, check
*   LICENSE in the project root.
*/

#ifndef indigo_command_line_hpp_
#define indigo_command_line_hpp_

#include "../Build.hpp"
#include <map>
#include <stdint.h>
#include <utility>
#include <vector>
#include <string>

namespace indigo
{
	class INDIGO_API CommandLine
	{
		static std::map<std::string, std::string> mArguments;

		static std::vector<std::string> convertToArgv(std::string commandLine)
		{
			std::vector<std::string> arguments;
			std::string string;
			for (size_t i = 0; i < commandLine.size(); i++)
			{
				char c = commandLine[i];
				if (c == '"')
				{
					size_t quotePosition = commandLine.find('"', i + 1);
					arguments.push_back(commandLine.substr(i + 1, quotePosition - i - 1));
					i = quotePosition + 1;
					continue;
				}

				if (c == ' ')
				{
					arguments.push_back(string);
					string.clear();
					continue;
				}

				string += c;
			}

			return arguments;
		}

	public:
		static void Parse(int argc, char **argv)
		{
			for (int i = 0; i < argc; i++)
			{
				std::string argument = argv[i];
				if (!argument.empty() && argument[0] == '-')
				{
					bool inserted = false;
					argument = argument.substr(1);
					if (i + 1 < argc)
					{
						std::string value = argv[i + 1];
						if (!value.empty() && value[0] != '-')
						{
							mArguments.insert(make_pair(argument, value));
							inserted = true;
						}
					}

					if (!inserted)
						mArguments.insert(make_pair(argument, ""));
				}
			}
		}

		static void Parse(std::vector<std::string> arguments)
		{
			for (size_t i = 0; i < arguments.size(); i++)
			{
				std::string argument = arguments[i];
				if (!argument.empty() && argument[0] == '-')
				{
					bool inserted = false;
					argument = argument.substr(1);
					if (i + 1 < arguments.size())
					{
						std::string value = arguments[i + 1];
						if (!value.empty() && value[0] != '-')
						{
							mArguments.insert(make_pair(argument, value));
							inserted = true;
						}
					}

					if (!inserted)
						mArguments.insert(make_pair(argument, ""));
				}
			}
		}

		static void Parse(std::string commandLine)
		{
			Parse(convertToArgv(std::move(commandLine)));
		}

		static bool FlagExists(const std::string &keyName)
		{
			return mArguments.find(keyName) != mArguments.end();
		}

		static int64_t GetInteger(const std::string &keyName, int64_t defaultValue)
		{
			auto value = mArguments.find(keyName);
			if (value == mArguments.end())
				return defaultValue;

			return stoull(value->second, nullptr, 0);
		}

		static std::string GetString(const std::string &keyName, std::string defaultValue)
		{
			auto value = mArguments.find(keyName);
			if (value == mArguments.end())
				return defaultValue;

			return value->second;
		}
	};
}

#endif // indigo_command_line_hpp_
