/*
*   This file is part of the Indigo library.
*
*   This program is licensed under the GNU General
*   Public License. To view the full license, check
*   LICENSE in the project root.
*/

#ifndef indigo_ip_endpoint_hpp_
#define indigo_ip_endpoint_hpp_

// Required libraries
#include "../Platform.hpp"
#include "../Build.hpp"
#if defined(OS_LINUX)
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#elif defined(OS_WIN)
#include <WS2tcpip.h>
#endif
#include <string>
#include <stdint.h>

namespace indigo
{
	class INDIGO_API IPEndpoint
	{
		sockaddr_in mEndpoint;
		std::string mAddress;
		uint16_t mPort;

	public:
		IPEndpoint();
		IPEndpoint(std::string address, uint16_t port);
		IPEndpoint(sockaddr_in endpoint);

		void SetAddress(std::string address);
		void SetAddress(uint32_t address);

		uint32_t GetAddress() const;
		std::string GetAddressString() const;

		uint16_t GetPort() const;
		void SetPort(uint16_t port);

		void SetSocketEndpoint(sockaddr_in endpoint);

		sockaddr_in GetSocketEndpoint() const;

		std::string ToString() const;
	};
}

#endif // indigo_ip_endpoint_hpp_
