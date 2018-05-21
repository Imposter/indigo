/*
*   This file is part of the Indigo library.
*
*   This program is licensed under the GNU General
*   Public License. To view the full license, check
*   LICENSE in the project root.
*/

#ifndef indigo_network_hpp_
#define indigo_network_hpp_

#include "../Build.hpp"

namespace indigo
{
	class INDIGO_API Network
	{
	public:
		static bool IsWANConnected();
	};
}

#endif // indigo_network_hpp_
