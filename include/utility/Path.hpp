/*
*   This file is part of the Indigo library.
*
*   This program is licensed under the GNU General
*   Public License. To view the full license, check
*   LICENSE in the project root.
*/

#ifndef indigo_file_hpp_
#define indigo_file_hpp_

#include "../Build.hpp"
#include <string>

namespace indigo
{
	class INDIGO_API Path
	{
	public:
		static std::string CurrentDirectory();
		static std::string Normalize(const std::string &path);
		static std::string RelativePath(std::string path, std::string relativeTo);
	};
}

#endif // indigo_file_hpp_
