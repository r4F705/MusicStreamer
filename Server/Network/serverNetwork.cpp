#include "serverNetwork.h"
#include <iostream>
#include <vector>

musicStreamer::network::ServerNetwork::ServerNetwork(const char* address, const char* port)
	: m_address(address), m_port(port), m_listenSocket(INVALID_SOCKET), m_clientSocket(INVALID_SOCKET)
{
	std::cout << "Starting Connection for " << address << ":" << port << std::endl;
	Init();
}

void musicStreamer::network::ServerNetwork::Init()
{
	// Initialize Windows sockets 
	m_iResult = WSAStartup(MAKEWORD(2, 2), &m_wsaData);
	// Check result
	if (m_iResult != 0)
	{
		std::cout << "Windows sockets failed to initialize with error:\n " << m_iResult << std::endl;
		return;
	}

	ZeroMemory(&m_hints, sizeof(m_hints));

	m_hints.ai_family = AF_INET;
	m_hints.ai_family = AF_INET;
	m_hints.ai_socktype = SOCK_STREAM;
	m_hints.ai_protocol = IPPROTO_TCP;
	m_hints.ai_flags = AI_PASSIVE;

	// Resolve the server address and port
	m_iResult = GetAddrInfo(m_address, m_port, &m_hints, &m_result);
	if (m_iResult != 0)
	{
		std::cout << "getaddrinfo failed with error:\n" << m_iResult << std::endl;
		// Clean windows socket
		WSACleanup();
		return;
	}

	// Create the listening socket
	m_listenSocket = socket(m_result->ai_family, m_result->ai_socktype, m_result->ai_protocol);
	if (m_listenSocket == INVALID_SOCKET)
	{
		std::cout << "socket failed with error:\n" << WSAGetLastError() << std::endl;
		freeaddrinfo(m_result);
		WSACleanup();
		return;
	}

	// Bind a socket for listening
	m_iResult = bind(m_listenSocket, m_result->ai_addr, (int)m_result->ai_addrlen);
	// Error check
	if (m_iResult == SOCKET_ERROR)
	{
		std::cout << "Bind failed, error:\n" << WSAGetLastError() << std::endl;
		freeaddrinfo(m_result);

		CleanSocketConnection(m_listenSocket);
		return;
	}

	freeaddrinfo(m_result);

	// Start listening for a client socket
	m_iResult = listen(m_listenSocket, SOMAXCONN);
	if (m_iResult == SOCKET_ERROR)
	{
		std::cout << "listen failed with error:\n" << WSAGetLastError() << std::endl;
		CleanSocketConnection(m_listenSocket);
		return;
	}
	std::cout << "Server Listening..." << std::endl;
}

musicStreamer::network::ServerNetwork::~ServerNetwork()
{

}

bool musicStreamer::network::ServerNetwork::AcceptClient(ClientID& id, std::map<ClientID, SOCKET>& clients)
{
	m_clientSocket = Accept();
	if (m_clientSocket != INVALID_SOCKET)
	{
		// disable nagle 
		char value = 1;
		setsockopt(m_clientSocket, IPPROTO_TCP, TCP_NODELAY, &value, sizeof(value));

		// add new client connection into the session id table
		clients.insert(std::pair<ClientID, SOCKET>(id, m_clientSocket));

		return true;
	}
	return false;
}

SOCKET musicStreamer::network::ServerNetwork::Accept()
{
	// Accept client sockets
	m_clientSocket = accept(m_listenSocket, NULL, NULL);
	if (m_clientSocket == INVALID_SOCKET)
	{
		std::cout << "accept failed with error: %d\n" << WSAGetLastError() << std::endl;
		CleanSocketConnection(m_listenSocket);
		return INVALID_SOCKET;
	}
	std::cout << "\nConnection made." << std::endl;

	return m_clientSocket;
}

void musicStreamer::network::ServerNetwork::Shutdown(SOCKET curSocket)
{
	int m_iResult;

	// shutdown the connection since we're done
	m_iResult = shutdown(curSocket, SD_SEND);
	if (m_iResult == SOCKET_ERROR)
	{
		printf("shutdown failed with error: %d\n", WSAGetLastError());
		CleanSocketConnection(curSocket);
		return;
	}
}

SOCKET musicStreamer::network::ServerNetwork::GetClientSocket()
{
	return m_clientSocket;
}

void musicStreamer::network::ServerNetwork::CleanSocketConnection(SOCKET curSocket)
{
	closesocket(curSocket);
	WSACleanup();
}

int musicStreamer::network::ServerNetwork::SendMessageA(SOCKET socket, const char* message, int messageSize, bool isStream)
{	
	if (isStream)
	{
		int messageLength = messageSize;
		int sendPosition = 0;

		while (messageLength)
		{
			int chunksize = messageLength > SMALL_BUFFER ? SMALL_BUFFER : messageLength;
			chunksize = send(socket, message + sendPosition, chunksize, NULL);			

			if (chunksize == SOCKET_ERROR)
			{
				std::cout << "send failed with error:\n" << WSAGetLastError() << std::endl;
				CleanSocketConnection(socket);
			}
			messageLength -= chunksize;
			sendPosition += chunksize;
		}
	#ifdef _DEBUG
		std::cout << "Server socket bytes send: " << sendPosition << std::endl;
	#endif
		return sendPosition;	
	}
	else 
	{
		auto result = send(socket, message, messageSize, NULL);
		if (result == SOCKET_ERROR)
		{
			std::cout << "send failed with error:\n" << WSAGetLastError() << std::endl;
			CleanSocketConnection(socket);
		}
		return result;
	}
}

int musicStreamer::network::ServerNetwork::ReceiveMessage(SOCKET socket, char* buffer, int bufSize, bool isStream)
{	
	if (isStream)
	{
		int messageLength = bufSize;
		int recvPosition = 0;

		while (messageLength)
		{
			int chunkSize = messageLength > SMALL_BUFFER ? SMALL_BUFFER : messageLength;
			chunkSize = recv(socket, (char*)buffer + recvPosition, chunkSize, NULL);			

			if (chunkSize == SOCKET_ERROR)
			{
				std::cout << "shutdown failed with error :\n" << WSAGetLastError() << std::endl;
				WSACleanup();
			}
			messageLength -= chunkSize;
			recvPosition += chunkSize;
		}
	#ifdef _DEBUG
		std::cout << "Server socket bytes received: " << recvPosition << std::endl;
	#endif // _DEBUG
		return recvPosition;
	}
	else
	{
		auto result = recv(socket, buffer, bufSize, NULL);
		if (result == SOCKET_ERROR)
		{
			std::cout << "send failed with error:\n" << WSAGetLastError() << std::endl;
			CleanSocketConnection(socket);
		}
		return result;
	}
}

