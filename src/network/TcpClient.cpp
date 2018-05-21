/*
*   This file is part of the jChatSystem project.
*
*   This program is licensed under the GNU General
*   Public License. To view the full license, check
*   LICENSE in the project root.
*/

// Required libraries
#include "Network/TcpClient.hpp"
#if defined(OS_LINUX) || defined(OS_OSX) || defined(OS_UNIX)
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#ifndef __SOCKET__
#define __SOCKET__
typedef int SOCKET;
#endif // __SOCKET__

#ifndef SOCKET_ERROR
#define SOCKET_ERROR -1
#endif // SOCKET_ERROR

#ifndef __CLOSE_SOCKET__
#define __CLOSE_SOCKET__
#define closesocket(socket_fd) close(socket_fd)
#endif // __CLOSE_SOCKET__

#elif defined(OS_WIN)
#include <WinSock2.h>

// Platform/Compiler patches
#if defined(__CYGWIN__) || defined(__MINGW32__)
#if defined(FIONBIO)
#undef FIONBIO
#define FIONBIO 0x8004667E
#endif
#endif
#endif

namespace indigo
{
	void TcpClient::workerLoop()
	{
		fd_set socketSet;
		while (mIsConnected)
		{
			// Clear the socket set
			FD_ZERO(&socketSet);

			// Add the client to the set
			FD_SET(mClientSocket, &socketSet);

			// Check if an activity was completed on the client socket
			int32_t socketActivity = select(mClientSocket + 1, &socketSet, nullptr,
			                                nullptr, nullptr);

			// Ensure select didn't fail
			if (socketActivity == SOCKET_ERROR && errno == EINTR)
				continue;

			// Check if there was some operation completed on the client socket
			if (FD_ISSET(mClientSocket, &socketSet))
			{
				int32_t readBytes = recv(mClientSocket, reinterpret_cast<char *>(mReadBuffer.data()),
				                         mReadBuffer.size(), 0);
				bool disconnectClient = false;
				if (readBytes > 0 && readBytes < INDIGO_TCP_BUFFER_SIZE)
				{
					Buffer buffer(mReadBuffer.data(), readBytes);
					if (!OnDataReceived(buffer))
						disconnectClient = true;
				}
				else 
				{
					disconnectClient = true;
				}

				if (disconnectClient)
				{
					mIsConnected = false;
					closesocket(mClientSocket);
					OnDisconnected();
				}
			}

			// Sleep
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
	}

	TcpClient::TcpClient()
		: mHostname(nullptr), mPort(0), mIsConnected(false),
		  mIsInternal(false), mClientSocket(0),
		  mRemoteEndpoint(nullptr, 0)
	{
		mReadBuffer.resize(INDIGO_TCP_BUFFER_SIZE);

#if defined(OS_WIN)
		// Initialize Winsock
		WSAStartup(MAKEWORD(2, 2), &mWsaData);
#endif
	}

	TcpClient::TcpClient(SOCKET clientSocket, sockaddr_in clientEndpoint,
	                     sockaddr_in serverEndpoint) : mHostname(nullptr), mPort(0),
	                                                    mIsConnected(true), mIsInternal(true),
	                                                    mClientSocket(clientSocket), mClientEndpoint(clientEndpoint),
	                                                    mRemoteEndpoint(serverEndpoint)
	{
		mReadBuffer.resize(INDIGO_TCP_BUFFER_SIZE);

#if defined(OS_LINUX) || defined(OS_OSX) || defined(OS_UNIX)
		uint32_t flags = fcntl(clientSocket, F_GETFL, 0);
		if (flags != SOCKET_ERROR) {
			flags |= O_NONBLOCK;
			fcntl(clientSocket, F_SETFL, flags);
		}
#elif defined(OS_WIN)
#if defined(__CYGWIN__) || defined(__MINGW32__)
		unsigned int blocking = 1;
#else
		u_long blocking = 1;
#endif
		ioctlsocket(clientSocket, FIONBIO, &blocking);
#endif
	}

	TcpClient::~TcpClient()
	{
		if (mIsConnected)
		{
			mIsConnected = false;
			if (!mIsInternal)
				mWorkerThread.join();

			closesocket(mClientSocket);
#if defined(OS_WIN)
			// Cleanup Winsock
			WSACleanup();
#endif
		}
	}

	bool TcpClient::Connect(const char *hostname, uint16_t port)
	{
		if (mIsConnected || mIsInternal)
			return false;

		// Get remote address info
		addrinfo *result = nullptr;
		addrinfo hints;

		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;
		hints.ai_flags = AI_PASSIVE;

		sockaddr_in *endpointInfo = nullptr;
		int returnValue = getaddrinfo(hostname, std::to_string(port).c_str(),
		                               &hints, &result);
		if (returnValue != SOCKET_ERROR)
		{
			for (addrinfo *ptr = result; ptr != nullptr; ptr = ptr->ai_next)
			{
				if (ptr->ai_family == AF_INET)
				{
					endpointInfo = reinterpret_cast<sockaddr_in *>(ptr->ai_addr);
					mRemoteEndpoint.SetAddress(ntohl(endpointInfo->sin_addr.s_addr));
					break;
				}
			}
		}

		freeaddrinfo(result);

		// Failed to resolve
		if (endpointInfo == nullptr)
			return false;

		// Create socket
		if ((mClientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP))
			== SOCKET_ERROR)
			return false;

		// Try to connect
		if (connect(mClientSocket, reinterpret_cast<const sockaddr *>(endpointInfo),
		            sizeof*endpointInfo) == SOCKET_ERROR)
		{
			closesocket(mClientSocket);
			return false;
		}

		sockaddr_in clientEndpoint;
		socklen_t clientEndpointSize = sizeof clientEndpoint;
		if (getsockname(mClientSocket, reinterpret_cast<sockaddr *>(&clientEndpoint),
		                &clientEndpointSize) == SOCKET_ERROR)
		{
			closesocket(mClientSocket);
			return false;
		}
		mClientEndpoint.SetSocketEndpoint(clientEndpoint);

#if defined(OS_LINUX) || defined(OS_OSX) || defined(OS_UNIX)
		uint32_t flags = fcntl(mClientSocket, F_GETFL, 0);
		if (flags != SOCKET_ERROR) {
			flags |= O_NONBLOCK;
			if (fcntl(mClientSocket, F_SETFL, flags) == SOCKET_ERROR) {
				closesocket(mClientSocket);
				return false;
			}
		} else {
#elif defined(OS_WIN)
#if defined(__CYGWIN__) || defined(__MINGW32__)
		unsigned int blocking = 1;
#else
		u_long blocking = 1;
#endif
		if (ioctlsocket(mClientSocket, FIONBIO, &blocking) == SOCKET_ERROR)
		{
#endif
			closesocket(mClientSocket);
			return false;
		}

		mIsConnected = true;
		mWorkerThread = std::thread(&TcpClient::workerLoop, this);

		OnConnected();

		return true;
	}

	bool TcpClient::Disconnect()
	{
		if (!mIsConnected || mIsInternal)
			return false;

		mIsConnected = false;
		mWorkerThread.join();
		closesocket(mClientSocket);

		OnDisconnected();

		return true;
	}

	bool TcpClient::Send(Buffer &buffer) const
	{
		if (mIsInternal || !mIsConnected)
			return false;

		return send(mClientSocket, reinterpret_cast<const char *>(buffer.GetBuffer()),
		            buffer.GetSize(), 0) != SOCKET_ERROR;
	}

	IPEndpoint TcpClient::GetLocalEndpoint() const
	{
		if (mIsInternal)
			return mRemoteEndpoint;

		return mClientEndpoint;
	}

	IPEndpoint TcpClient::GetRemoteEndpoint() const
	{
		if (mIsInternal)
			return mClientEndpoint;

		return mRemoteEndpoint;
	}
}
