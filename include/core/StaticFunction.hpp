/*
*   This file is part of the Indigo library.
*
*   This program is licensed under the GNU General
*   Public License. To view the full license, check
*   LICENSE in the project root.
*/

#ifndef indigo_sys_function_hpp_
#define indigo_sys_function_hpp_

#include <functional>

namespace indigo
{
	class StaticFunction
	{
		std::function<void()> mShutdownFunction;

	public:
		StaticFunction(const std::function<void()> &startupFunction)
		{
			startupFunction();
		}

		StaticFunction(const std::function<void()> &startup_function, std::function<void()> shutdownFunction)
			: mShutdownFunction(shutdownFunction)
		{
			startup_function();
		}

		~StaticFunction()
		{
			if (mShutdownFunction)
				mShutdownFunction();
		}
	};
}

#endif // indigo_sys_function_hpp_
