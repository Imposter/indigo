/*
*   This file is part of the Indigo library.
*
*   This program is licensed under the GNU General
*   Public License. To view the full license, check
*   LICENSE in the project root.
*/

#ifndef indigo_credentials_box_hpp_
#define indigo_credentials_box_hpp_

#include "../Build.hpp"
#include <string>

namespace indigo
{
	class INDIGO_API CredentialsBox
	{
	public:
		static bool Show(std::string &username, std::string &password, bool &save, std::string title, std::string format, ...);
	};
}

#endif // indigo_credentials_box_hpp_
