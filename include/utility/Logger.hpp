/*
*   This file is part of the Indigo library.
*
*   This program is licensed under the GNU General
*   Public License. To view the full license, check
*   LICENSE in the project root.
*/

#ifndef indigo_logger_hpp_
#define indigo_logger_hpp_

#include <ostream>
#include <iomanip>
#include <vector>
#include <mutex>
#include <stdarg.h>

namespace indigo
{
	enum LogType
	{
		kLogType_Error,
		kLogType_Warning,
		kLogType_Info,
		kLogType_Trace
	};

	class Logger
	{
		LogType mLevel;
		std::vector<std::ostream *> mStreams;
		std::mutex mStreamsMutex;

	public:
		Logger() : mLevel(kLogType_Error) {}
		Logger(LogType level) : mLevel(level) {}

		void AddStream(std::ostream &stream)
		{
			mStreamsMutex.lock();
			mStreams.push_back(&stream);
			mStreamsMutex.unlock();

			time_t currentTime;
			time(&currentTime);
			tm localTime;
			localtime_s(&localTime, &currentTime);

			stream << "==================================================" << std::endl;
			stream << "Log created on "
				<< std::setw(2) << std::setfill('0') << localTime.tm_mday << "/"
				<< std::setw(2) << std::setfill('0') << localTime.tm_mon + 1 << "/"
				<< std::setw(2) << std::setfill('0') << localTime.tm_year + 1900 << " "
				<< std::setw(2) << std::setfill('0') << localTime.tm_hour << ":"
				<< std::setw(2) << std::setfill('0') << localTime.tm_min << ":"
				<< std::setw(2) << std::setfill('0') << localTime.tm_sec
				<< std::endl;
			stream << "==================================================" << std::endl;
			stream << std::endl;
		}

		void RemoveStream(std::ostream &stream)
		{
			mStreamsMutex.lock();
			for (auto it = mStreams.begin(); it != mStreams.end();)
			{
				if (*it == &stream)
				{
					it = mStreams.erase(it);
				}
				else
				{
					++it;
				}
			}
			mStreamsMutex.unlock();
		}

		void SetLevel(LogType level)
		{
			mLevel = level;
		}

		void Write(LogType type, const std::string &className, std::string format, ...)
		{
			va_list arguments;
			va_start(arguments, format);

			const int length = _vscprintf(format.c_str(), arguments) + 1;

			std::vector<char> message;
			message.resize(length);
			fill(message.begin(), message.end(), 0);

			vsprintf_s(message.data(), length, format.c_str(), arguments);

			va_end(arguments);

			const char *typeString;
			switch (type)
			{
			case kLogType_Error:
				typeString = "ERROR";
				break;
			case kLogType_Warning:
				typeString = "WARNING";
				break;
			case kLogType_Trace:
				typeString = "TRACE";
				break;
			case kLogType_Info:
				typeString = "INFO";
				break;
			default:
				typeString = "UNKN";
			}

			time_t currentTime;
			time(&currentTime);
			tm localTime{};
			localtime_s(&localTime, &currentTime);

			mStreamsMutex.lock();
			for (auto &stream : mStreams)
			{
				*stream << "[" << std::setw(2) << std::setfill('0') << localTime.tm_hour
					<< ":" << std::setw(2) << std::setfill('0') << localTime.tm_min
					<< ":" << std::setw(2) << std::setfill('0') << localTime.tm_sec
					<< "][" << typeString << ":" << className.c_str()
					<< "]: " << message.data() << std::endl;
			}
			mStreamsMutex.unlock();
		}
	};
}

#endif // indigo_logger_hpp_
