/*
*   This file is part of the jChatSystem project.
*
*   This program is licensed under the GNU General
*   Public License. To view the full license, check
*   LICENSE in the project root.
*/

#ifndef indigo_tcp_client_hpp_
#define indigo_tcp_client_hpp_

// Required libraries
#include "../Platform.hpp"
#include "../Build.hpp"
#include "../Core/Event.hpp"
#include "../Core/Buffer.hpp"
#include "IpEndpoint.hpp"
#include <thread>

#ifndef INDIGO_TCP_BUFFER_SIZE
#define INDIGO_TCP_BUFFER_SIZE 8192
#endif // INDIGO_TCP_BUFFER_SIZE

namespace indigo
{
	class TcpServer;

	class INDIGO_API TcpClient
	{
		friend class TcpServer;

		const char *mHostname;
		uint16_t mPort;
		bool mIsConnected;
		bool mIsInternal;
		SOCKET mClientSocket;
		IPEndpoint mClientEndpoint;
		IPEndpoint mRemoteEndpoint;
		std::thread mWorkerThread;
		std::vector<uint8_t> mReadBuffer;

#if defined(OS_WIN)
		WSADATA mWsaData;
#endif

		void workerLoop();

	public:
		TcpClient();
		TcpClient(SOCKET clientSocket, sockaddr_in clientEndpoint,
		          sockaddr_in serverEndpoint);
		~TcpClient();

		bool Connect(const char *hostname, uint16_t port);
		bool Disconnect();
		bool Send(Buffer &buffer) const;

		IPEndpoint GetLocalEndpoint() const;
		IPEndpoint GetRemoteEndpoint() const;

		// NOTE: These are not intended to be used in combination with TcpServer
		Event<> OnConnected;
		Event<> OnDisconnected;
		Event<Buffer &> OnDataReceived;
	};
}

#endif // indigo_tcp_client_hpp_
