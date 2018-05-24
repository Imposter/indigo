/*
*   This file is part of the Indigo library.
*
*   This program is licensed under the GNU General
*   Public License. To view the full license, check
*   LICENSE in the project root.
*/

#ifndef indigo_console_hpp_
#define indigo_console_hpp_

#include "../Build.hpp"
#include <string>

namespace indigo
{
	class INDIGO_API Console
	{
		static bool mAllocated;

	public:
		static void Show(const std::string &title = "Console");
		static void Hide();
	};
}

#endif // indigo_console_hpp_
