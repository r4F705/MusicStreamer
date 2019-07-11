#include "resource.h"

#include <iostream>

using namespace musicStreamer;
using namespace network;

	BytePtr MusicBuffer::CreateMusicBuffer(int bufsize)
	{
		musicStreamer::BytePtr musicBuffer = new Byte[bufsize];				

		return musicBuffer;
	}

	void MusicBuffer::DestroyMusicBuffer(BytePtr buf)
	{
		if (buf != nullptr) {
			delete[] buf;
			m_bufsize = 0;
#if _DEBUG
		std::cout << "MusicBuffer has been deleted" << std::endl;
#endif // _DEBUG			
		}		
	}

	MusicBuffer::MusicBuffer(std::string str)
	{	
		CopyString(str);
	}

	MusicBuffer::MusicBuffer(int bufsize) :
		m_bufsize(bufsize)
	{	
		m_musicBuffer = CreateMusicBuffer(bufsize);
		Clear();
	}

	MusicBuffer::MusicBuffer(MusicBuffer && other)
	{		
		m_musicBuffer = other.m_musicBuffer;
		m_bufsize = other.m_bufsize;
		DestroyMusicBuffer(other.m_musicBuffer);
	}

	MusicBuffer::~MusicBuffer()
	{
		DestroyMusicBuffer(m_musicBuffer);
	}

	int MusicBuffer::Size()
	{
		return m_bufsize;
	}

	MusicBuffer & MusicBuffer::operator=(MusicBuffer && other)
	{
		DestroyMusicBuffer(m_musicBuffer);

		m_musicBuffer = other.m_musicBuffer;
		m_bufsize = other.m_bufsize;

		other.m_musicBuffer = nullptr;
		other.m_bufsize = 0;

		return *this;
	}

	const int MusicBuffer::Stream(SOCKET sock, int bytesPerSec)
	{				
		bool endOfStream = false;
		int messageLength = m_bufsize;
		int sendPosition = 0;
		int chunkSize = 0;
		ServerNetwork connection;
		try
		{
			while (endOfStream != true || messageLength)
			{
				MusicBuffer recv(SMALL_BUFFER);				
				recv.ReceiveMessage(sock);
				Command command = Command(std::stoi(recv.ToString()));
				recv.Clear();				
				// Take in a command send from the client.
				switch (command)
				{
					// Case the command is to stream then steam. \\(oo)//
				case Command::STREAM:		
					chunkSize = messageLength > bytesPerSec ? bytesPerSec : messageLength;
					chunkSize = connection.SendMessageA(sock, (char*)m_musicBuffer + sendPosition, chunkSize, true);
					messageLength -= chunkSize;
					sendPosition += chunkSize;
#if _DEBUG
					std::cout << "Bytes Remaining " << messageLength << std::endl;
#endif
					break;
					// Case the command is to pause break the switch and wait for a new command.
				case Command::PAUSE:					
					break;
					// Case the command is to stop then terminate the loop and break the switch.
				case Command::STOP:					
					endOfStream = true;					
					break;
				}
			}
		}
		catch (const std::exception& ex)
		{
			std::cout << "Error during streaming: " << ex.what() << std::endl;
			connection.Shutdown(connection.GetClientSocket());
		}
		
		std::cout << "Data Ended!" << std::endl;
		return sendPosition;
	}

	const int MusicBuffer::ReceiveMessage(SOCKET sock, bool isStream)
	{
		ServerNetwork connection;
		return connection.ReceiveMessage(sock, (char*)m_musicBuffer, m_bufsize, isStream);
	}

	const int MusicBuffer::SentMessage(SOCKET sock, bool isStream)
	{
		ServerNetwork connection;
		return connection.SendMessageA(sock, (char*)m_musicBuffer, m_bufsize, isStream);
	}

	BytePtr MusicBuffer::Get() const
	{
		return m_musicBuffer;
	}

	void MusicBuffer::CopyString(std::string str)
	{		
		m_musicBuffer = CreateMusicBuffer(m_bufsize);
		Clear();
		memcpy(m_musicBuffer, str.c_str(), str.length());
	}

	std::string MusicBuffer::ToString()
	{
		if (m_musicBuffer && m_bufsize > 0) 
		{			
			 std::string s(reinterpret_cast<char const*>(m_musicBuffer), m_bufsize);
			 return s.data();
		}
		return std::string();
	}

	void MusicBuffer::Clear()
	{
		if (m_bufsize > 0)
			for (size_t i = 0; i < m_bufsize; i++)
				m_musicBuffer[i] = '\0';
	}