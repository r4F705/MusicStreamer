#pragma once

#undef UNICODE
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#include <cstdint>
#include <string>
#include <thread>
#include <map>

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")

#define DEFAULT_ADDRESS "127.0.0.1"
#define DEFAULT_PORT "4000"

#define SMALL_BUFFER 1024
#define MEDIUM_BUFFER 10240
#define LARGE_BUFFER 102400


using ClientID = unsigned int;
namespace musicStreamer 
{
	namespace network 
	{
		class ServerNetwork 
		{
		private:			
			const char* m_address;
			const char* m_port;
			
			// Move to local scope for each function
			WSADATA m_wsaData;
			int m_iResult;

			// We can use a raw pointer here because this will never be used outside the 
			// class and because it is easier to use with WINAPI.
			struct addrinfo *m_result = nullptr;
			struct addrinfo m_hints;

			// Remove
			int m_iSendResult;

			SOCKET m_listenSocket;
			SOCKET m_clientSocket;

		private:
			void Init();
			SOCKET Accept();
			void CleanSocketConnection(SOCKET curSocket);
		
		public:
			ServerNetwork() {};
			ServerNetwork(const char* address, const char* port);
			~ServerNetwork();

			bool AcceptClient(ClientID& id, std::map<ClientID, SOCKET>& clients);
			// Sents a message of specified length and return the number of bytes sent
			int SendMessageA(SOCKET socket, const char* message, int messageSize, bool isSteam);
			// Puts the data received in the buffer and returns the number of bytes received
		 	int ReceiveMessage(SOCKET socket, char* buffer, int bufSize, bool isStream); 

			void Shutdown(SOCKET curSocket);

			SOCKET GetClientSocket();
		};

	}
}
