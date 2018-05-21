/*
*   This file is part of the jChatSystem project.
*
*   This program is licensed under the GNU General
*   Public License. To view the full license, check
*   LICENSE in the project root.
*/

#ifndef indigo_tcp_server_hpp_
#define indigo_tcp_server_hpp_

// Required libraries
#include "TcpClient.hpp"

#ifndef INDIGO_TCP_SERVER_BACKLOG
#define INDIGO_TCP_SERVER_BACKLOG 50
#endif // INDIGO_TCP_SERVER_BACKLOG

namespace indigo
{
	class TcpServer
	{
		const char *mHostname;
		uint16_t mPort;
		bool mIsListening;
		SOCKET mListenSocket;
		IPEndpoint mListenEndpoint;
		std::vector<TcpClient *> mAcceptedClients;
		std::mutex mAcceptedClientsMutex;
		std::thread mWorkerThread;

#if defined(OS_WIN)
		WSADATA mWsaData;
#endif

		void workerLoop();

	public:
		TcpServer(const char *hostname, uint16_t port);
		~TcpServer();

		bool Start();
		bool Stop();

		bool DisconnectClient(TcpClient &tcp_client);
		bool Send(TcpClient &tcp_client, Buffer &buffer) const;

		IPEndpoint GetListenEndpoint() const;

		Event<TcpClient &> OnClientConnected;
		Event<TcpClient &> OnClientDisconnected;
		Event<TcpClient &, Buffer &> OnDataReceived;
	};
}

#endif // indigo_tcp_server_hpp_
