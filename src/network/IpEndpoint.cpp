/*
*   This file is part of the Indigo library.
*
*   This program is licensed under the GNU General
*   Public License. To view the full license, check
*   LICENSE in the project root.
*/

#include "Network/IpEndpoint.hpp"

namespace indigo
{
	IPEndpoint::IPEndpoint() : mAddress("0.0.0.0"), mPort(0)
	{
		mEndpoint.sin_family = AF_INET;
		mEndpoint.sin_addr.s_addr = inet_addr(mAddress.c_str());
		mEndpoint.sin_port = htons(mPort);
	}

	IPEndpoint::IPEndpoint(std::string address, uint16_t port) : mAddress(address),
	                                                             mPort(port)
	{
		mEndpoint.sin_family = AF_INET;
		mEndpoint.sin_addr.s_addr = inet_addr(mAddress.c_str());
		mEndpoint.sin_port = htons(mPort);
	}

	IPEndpoint::IPEndpoint(sockaddr_in endpoint) : mEndpoint(endpoint)
	{
		mAddress = inet_ntoa(endpoint.sin_addr);
		mPort = ntohs(endpoint.sin_port);
	}

	void IPEndpoint::SetAddress(std::string address)
	{
		mAddress = address;
		mEndpoint.sin_addr.s_addr = inet_addr(mAddress.c_str());
	}

	void IPEndpoint::SetAddress(uint32_t address)
	{
		mEndpoint.sin_addr.s_addr = htonl(address);
		mAddress = inet_ntoa(mEndpoint.sin_addr);
	}

	uint32_t IPEndpoint::GetAddress() const
	{
		return ntohl(mEndpoint.sin_addr.s_addr);
	}

	std::string IPEndpoint::GetAddressString() const
	{
		return mAddress;
	}

	void IPEndpoint::SetPort(uint16_t port)
	{
		mPort = port;
		mEndpoint.sin_port = htons(mPort);
	}

	uint16_t IPEndpoint::GetPort() const
	{
		return mPort;
	}

	void IPEndpoint::SetSocketEndpoint(sockaddr_in endpoint)
	{
		mEndpoint = endpoint;
		mAddress = inet_ntoa(endpoint.sin_addr);
		mPort = ntohs(endpoint.sin_port);
	}

	sockaddr_in IPEndpoint::GetSocketEndpoint() const
	{
		return mEndpoint;
	}

	std::string IPEndpoint::ToString() const
	{
		return mAddress + ":" + std::to_string(mPort);
	}
}
