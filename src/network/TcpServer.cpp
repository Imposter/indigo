/*
*   This file is part of the jChatSystem project.
*
*   This program is licensed under the GNU General
*   Public License. To view the full license, check
*   LICENSE in the project root.
*/

// Required libraries
#include "Network/TcpServer.hpp"

namespace indigo
{
	void TcpServer::workerLoop()
	{
		while (mIsListening)
		{
			// Clear the socket set
			fd_set socketSet;
			FD_ZERO(&socketSet);

			// Add the listener to the set
			FD_SET(mListenSocket, &socketSet);
			SOCKET maxSocket = mListenSocket;

			// Add all clients to the set
			mAcceptedClientsMutex.lock();
			for (auto tcp_client : mAcceptedClients)
			{
				if (tcp_client->mIsConnected)
				{
					FD_SET(tcp_client->mClientSocket, &socketSet);

					// If the client socket is the largest socket, set it so
					if (tcp_client->mClientSocket > maxSocket)
						maxSocket = tcp_client->mClientSocket;
				}
			}
			mAcceptedClientsMutex.unlock();

			// Check if an activity was completed on any of those sockets
			int32_t socketActivity = select(maxSocket + 1, &socketSet, nullptr, nullptr,
			                                 nullptr);

			// Ensure select didn't fail
			if (socketActivity == SOCKET_ERROR && errno == EINTR)
				continue;

			// Check if a new connection is awaiting
			if (FD_ISSET(mListenSocket, &socketSet))
			{
				sockaddr_in clientEndpoint;
#if defined(OS_LINUX) || defined(OS_OSX) || defined(OS_UNIX)
			uint32_t client_endpoint_size = sizeof(client_endpoint);
#elif defined(OS_WIN)
				int32_t clientEndpointSize = sizeof clientEndpoint;
#endif
				SOCKET clientSocket = accept(mListenSocket,
				                              reinterpret_cast<sockaddr *>(&clientEndpoint), &clientEndpointSize);
				if (clientSocket != SOCKET_ERROR)
				{
					TcpClient *tcpClient = new TcpClient(clientSocket, clientEndpoint,
					                                      mListenEndpoint.GetSocketEndpoint());
					mAcceptedClientsMutex.lock();
					mAcceptedClients.push_back(tcpClient);
					mAcceptedClientsMutex.unlock();

					OnClientConnected(*tcpClient);
				}
			}

			// Check if there was some operation completed on another socket
			mAcceptedClientsMutex.lock();
			for (auto tcpClient = mAcceptedClients.begin();
			     tcpClient != mAcceptedClients.end();)
			{
				if (FD_ISSET((*tcpClient)->mClientSocket, &socketSet))
				{
					int32_t read_bytes = recv((*tcpClient)->mClientSocket,
					                          reinterpret_cast<char *>((*tcpClient)->mReadBuffer.data()),
					                          (*tcpClient)->mReadBuffer.size(), 0);
					bool disconnect_client = false;
					if (read_bytes > 0 && read_bytes < INDIGO_TCP_BUFFER_SIZE)
					{
						Buffer buffer((*tcpClient)->mReadBuffer.data(), read_bytes);
						if (!OnDataReceived(**tcpClient, buffer))
							disconnect_client = true;
					}
					else
					{
						disconnect_client = true;
					}

					if (disconnect_client)
					{
						(*tcpClient)->mIsConnected = false;
						closesocket((*tcpClient)->mClientSocket);
						OnClientDisconnected(**tcpClient);
						delete *tcpClient;
						tcpClient = mAcceptedClients.erase(tcpClient);
						continue;
					}
				}
				++tcpClient;
			}
			mAcceptedClientsMutex.unlock();

			// Sleep
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
	}

	TcpServer::TcpServer(const char *hostname, uint16_t port)
		: mHostname(hostname), mPort(port), mIsListening(false),
		  mListenSocket(0), mListenEndpoint("0.0.0.0", port)
	{
#if defined(OS_WIN)
		// Initialize Winsock
		WSAStartup(MAKEWORD(2, 2), &mWsaData);
#endif
	}

	TcpServer::~TcpServer()
	{
		if (mIsListening)
		{
			mIsListening = false;
			mWorkerThread.join();
			closesocket(mListenSocket);

#if defined(OS_WIN)
			// Cleanup Winsock
			WSACleanup();
#endif
		}

		if (!mAcceptedClients.empty())
		{
			for (auto tcpClient : mAcceptedClients)
				delete tcpClient;

			mAcceptedClients.clear();
		}
	}

	bool TcpServer::Start()
	{
		if (mIsListening)
			return false;

		// Get remote address info
		addrinfo *result = nullptr;
		addrinfo hints;

		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;
		hints.ai_flags = AI_PASSIVE;

		sockaddr_in *endpointInfo = nullptr;
		int returnValue = getaddrinfo(mHostname, std::to_string(mPort).c_str(),
		                               &hints, &result);
		if (returnValue != SOCKET_ERROR)
		{
			for (addrinfo *ptr = result; ptr != nullptr; ptr = ptr->ai_next)
			{
				if (ptr->ai_family == AF_INET)
				{
					endpointInfo = reinterpret_cast<sockaddr_in *>(ptr->ai_addr);
					mListenEndpoint.SetAddress(ntohl(endpointInfo->sin_addr.s_addr));
					break;
				}
			}
		}

		freeaddrinfo(result);

		// Check if we resolved an address
		if (endpointInfo == nullptr)
			return false;

		// Create socket
		if ((mListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP))
			== SOCKET_ERROR)
			return false;

		if (bind(mListenSocket, reinterpret_cast<const sockaddr *>(endpointInfo),
		         sizeof*endpointInfo) == SOCKET_ERROR)
		{
			closesocket(mListenSocket);
			return false;
		}

		if (listen(mListenSocket, INDIGO_TCP_SERVER_BACKLOG) == SOCKET_ERROR)
		{
			closesocket(mListenSocket);
			return false;
		}

#if defined(OS_LINUX) || defined(OS_OSX) || defined(OS_UNIX)
		uint32_t flags = fcntl(mListenSocket, F_GETFL, 0);
		if (flags != SOCKET_ERROR) {
			flags |= O_NONBLOCK;
			if (fcntl(mListenSocket, F_SETFL, flags) == SOCKET_ERROR) {
				closesocket(mListenSocket);
				return false;
			}
		} else {
#elif defined(OS_WIN)
#if defined(__CYGWIN__) || defined(__MINGW32__)
		unsigned int blocking = 1;
#else
		u_long blocking = 1;
#endif
		if (ioctlsocket(mListenSocket, FIONBIO, &blocking) == SOCKET_ERROR)
		{
#endif
			closesocket(mListenSocket);
			return false;
		}

		mIsListening = true;

		mWorkerThread = std::thread(&TcpServer::workerLoop, this);

		return true;
	}

	bool TcpServer::Stop()
	{
		if (!mIsListening)
			return false;

		mIsListening = false;
		mWorkerThread.join();
		closesocket(mListenSocket);

		mAcceptedClientsMutex.lock();
		if (!mAcceptedClients.empty())
		{
			for (auto tcp_client : mAcceptedClients)
				delete tcp_client;

			mAcceptedClients.clear();
		}
		mAcceptedClientsMutex.unlock();

		return true;
	}

	bool TcpServer::DisconnectClient(TcpClient &tcp_client)
	{
		mAcceptedClientsMutex.lock();
		for (auto client = mAcceptedClients.begin();
		     client != mAcceptedClients.end();)
		{
			if (*client == &tcp_client)
			{
				(*client)->mIsConnected = false;
				closesocket((*client)->mClientSocket);
				OnClientDisconnected(**client);
				mAcceptedClients.erase(client);
				mAcceptedClientsMutex.unlock();
				return true;
			}
			++client;
		}
		mAcceptedClientsMutex.unlock();

		return false;
	}

	bool TcpServer::Send(TcpClient &tcp_client, Buffer &buffer) const
	{
		if (!mIsListening || !tcp_client.mIsInternal || !tcp_client.mIsConnected)
			return false;

		return send(tcp_client.mClientSocket, reinterpret_cast<const char *>(buffer.GetBuffer()),
		            buffer.GetSize(), 0) != SOCKET_ERROR;
	}

	IPEndpoint TcpServer::GetListenEndpoint() const
	{
		return mListenEndpoint;
	}
}
