#include "resource.h"

#include <iostream>

using namespace musicStreamer::network;

namespace musicStreamer {

	BytePtr MusicBuffer::CreateMusicBuffer(int bufsize)
	{
		musicStreamer::BytePtr musicBuffer = new Byte[bufsize];			

		return musicBuffer;
	}

	void MusicBuffer::DestroyMusicBuffer(BytePtr buf)
	{
		if (buf != nullptr)
			delete[] buf;
		m_bufsize = 0;
#if	_DEBUG
		std::cout << "MusicBuffer has been deleted" << std::endl;
#endif
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

	MusicBuffer::~MusicBuffer()
	{
		DestroyMusicBuffer(m_musicBuffer);
	}

	int MusicBuffer::Size()
	{
		return m_bufsize;
	}

	MusicBuffer::MusicBuffer(MusicBuffer && other)
	{
		m_musicBuffer = other.m_musicBuffer;
		m_bufsize = other.m_bufsize;
		DestroyMusicBuffer(other.m_musicBuffer);
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

	const int MusicBuffer::ReceiveMessage(network::ClientNetwork& sock, bool isStream)
	{		
		return sock.ReceiveMessage(sock.GetSocket(), (char*)m_musicBuffer, m_bufsize, isStream);
	}

	const int MusicBuffer::SentMessage(network::ClientNetwork& sock, bool isStream)
	{	
		return sock.SendMessageA(sock.GetSocket(), (char*)m_musicBuffer, m_bufsize, isStream);
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
}