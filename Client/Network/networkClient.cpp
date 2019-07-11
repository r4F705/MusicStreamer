#include "networkClient.h"

musicStreamer::network::ClientNetwork::ClientNetwork(const char* address, const char* port):
	m_address(address), m_port(port), m_connectSocket(INVALID_SOCKET)
{
	Init();
}

void musicStreamer::network::ClientNetwork::Init()
{
	m_iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);

	if (m_iResult != 0)
	{
		std::cout << "WSAStartup failed with error:" << m_iResult << std::endl;
		exit(1);
	}

	// set address info
	ZeroMemory(&m_hints, sizeof(m_hints));
	m_hints.ai_family = AF_UNSPEC;
	m_hints.ai_socktype = SOCK_STREAM;
	m_hints.ai_protocol = IPPROTO_TCP;  //TCP connection!!!

	m_iResult = getaddrinfo(m_address, m_port, &m_hints, &m_result);
	if (m_iResult != 0)
	{
		std::cout << "getaddrinfo failed with error: " << m_iResult << std::endl;
		WSACleanup();
		exit(1);
	}

	// Attempt to connect to an adress until one succeeds
	for (m_ptr = m_result; m_ptr != nullptr; m_ptr = m_ptr->ai_next)
	{
		m_connectSocket = socket(m_ptr->ai_family, m_ptr->ai_socktype, m_ptr->ai_protocol);
		if (m_connectSocket == INVALID_SOCKET) {
			std::cout << "Socket failed with error: " << WSAGetLastError() << std::endl;
			WSACleanup();
			exit(1);
		}

		m_iResult = connect(m_connectSocket, m_ptr->ai_addr, (int)m_ptr->ai_addrlen);
		if (m_iResult == SOCKET_ERROR)
		{
			closesocket(m_connectSocket);
			m_connectSocket = INVALID_SOCKET;
			std::cout << "Server could not be reached" << std::endl;
		}
	}

	freeaddrinfo(m_result);
	if (m_connectSocket == INVALID_SOCKET)
	{
		std::cout << "Unable to connect to server" << std::endl;
		WSACleanup();
		exit(1);
	}

	// Disable nagle 
	char value = 1;
	setsockopt(m_connectSocket, IPPROTO_TCP, TCP_NODELAY, &value, sizeof(value));
}

musicStreamer::network::ClientNetwork::~ClientNetwork()
{
	Shutdown(m_connectSocket);
}

void musicStreamer::network::ClientNetwork::Shutdown(SOCKET curSocket)
{
	// shutdown the connection since we're done
	m_iResult = shutdown(curSocket, SD_SEND);
	if (m_iResult == SOCKET_ERROR)
	{
		printf("shutdown failed with error: %d\n", WSAGetLastError());
		CleanSocketConnection(curSocket);
		return;
	}
}

void musicStreamer::network::ClientNetwork::CleanSocketConnection(SOCKET curSocket)
{
	closesocket(curSocket);
	WSACleanup();
}

int musicStreamer::network::ClientNetwork::SendMessageA(SOCKET sock, const char* message, const int messageSize, bool isStream)
{
	if (isStream)
	{
		int messageLength = messageSize;
		int sendPosition = 0;

		while (messageLength)
		{
			int chunksize = messageLength > SMALL_BUFFER ? SMALL_BUFFER : messageLength;
			chunksize = send(sock, message + sendPosition, chunksize, NULL);

			if (chunksize == SOCKET_ERROR)
			{
				//std::cout << "send failed with error:\n" << WSAGetLastError() << std::endl;
				CleanSocketConnection(sock);
			}
			messageLength -= chunksize;
			sendPosition += chunksize;
		}
#ifdef _DEBUG
		//std::cout << "Bytes send: " << sendPosition << std::endl;
#endif
		return sendPosition;
	}
	else
	{
		auto result = send(sock, message, messageSize, NULL);
		if (result == SOCKET_ERROR)
		{
			//std::cout << "send failed with error:\n" << WSAGetLastError() << std::endl;
			CleanSocketConnection(sock);
		}
		return result;
	}
}

int musicStreamer::network::ClientNetwork::ReceiveMessage(SOCKET sock, char * buffer, int bufSize, bool isStream)
{
	if (isStream)
	{
		int messageLength = bufSize;
		int recvPosition = 0;

		while (messageLength)
		{
			int chunkSize = messageLength > SMALL_BUFFER ? SMALL_BUFFER : messageLength;
			chunkSize = recv(sock, (char*)buffer + recvPosition, chunkSize, NULL);						

			if (chunkSize == SOCKET_ERROR)
			{
				//std::cout << "shutdown failed with error :\n" << WSAGetLastError() << std::endl;
				WSACleanup();
			}
			messageLength -= chunkSize;
			recvPosition += chunkSize;
		}
#ifdef _DEBUG
		//std::cout << "Bytes received: " << recvPosition << std::endl;
#endif // _DEBUG
		return recvPosition;
	}
	else
	{
		auto result = recv(sock, buffer, bufSize, NULL);
		if (result == SOCKET_ERROR)
		{
			std::cout << "send failed with error:\n" << WSAGetLastError() << std::endl;
			CleanSocketConnection(sock);
		}
		return result;
	}
}

const SOCKET musicStreamer::network::ClientNetwork::GetSocket()
{
	return m_connectSocket;
}
