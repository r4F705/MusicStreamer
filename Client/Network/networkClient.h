#pragma once

#include <cstdint>

// Networking libraries
#include <winsock2.h>
#include <Windows.h>
#include <ws2tcpip.h>
#include <stdio.h> 
#include <iostream>

// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#define DEFAULT_PORT "4000"
#define DEFAULT_ADDRESS "127.0.0.1"

#define SMALL_BUFFER 1024
#define MEDIUM_BUFFER 10240
#define LARGE_BUFFER 102400

namespace musicStreamer {
	namespace network {

		class ClientNetwork {

		private:
			const char* m_address;
			const char* m_port;

			int m_iResult;			

			WSADATA wsaData;
			// We can use a raw pointer here because this will never be used outside the 
			// class and because it is easier to use with WINAPI.
			struct addrinfo *m_result = nullptr;
			struct addrinfo *m_ptr = nullptr;
			struct addrinfo m_hints;
			
			int m_iSendResult;

			SOCKET m_connectSocket;
		private:
			void Init();
			static void CleanSocketConnection(SOCKET sock);

		public:
			ClientNetwork(const char* address, const char* port);
			~ClientNetwork();

			void Shutdown(SOCKET curSocket);

			// Sents a message of specified length and return the number of bytes sent
			static int SendMessageA(SOCKET sock, const char* message, const int messageSize, bool isStream);
			// Puts the data received in the buffer and returns the number of bytes received
			static int ReceiveMessage(SOCKET sock, char* buffer, int bufSize, bool isStream);

			const SOCKET GetSocket(); 
		};
	}
}