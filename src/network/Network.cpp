/*
*   This file is part of the Indigo library.
*
*   This program is licensed under the GNU General
*   Public License. To view the full license, check
*   LICENSE in the project root.
*/

// Required libraries
#include "network/Network.hpp"
#include "Platform.hpp"
#if !defined(OS_WIN)
#error "Unsupported platform!"
#endif

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <SensAPI.h>

namespace indigo
{
	bool Network::IsWANConnected()
	{
		DWORD flags = NETWORK_ALIVE_WAN;
		return IsNetworkAlive(&flags) && GetLastError() == NO_ERROR;
	}
}
