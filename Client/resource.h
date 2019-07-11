#pragma once
#include "Network\networkClient.h"

// A simple MusicBuffer class using the RAII principle
namespace musicStreamer {

	using Byte = int8_t;
	using BytePtr = int8_t*;

	class MusicBuffer 
	{

	private:
		BytePtr m_musicBuffer;
		int m_bufsize;

	private:
		BytePtr CreateMusicBuffer(int bufsize);
		void DestroyMusicBuffer(BytePtr buf);
		
		// Remove the copy constructor and assignment operator.
		MusicBuffer(MusicBuffer const &) = delete;
		MusicBuffer& operator = (MusicBuffer const &) = delete;		

	public:
		MusicBuffer() :
			m_musicBuffer(nullptr), m_bufsize(0) {/*Default Constructor*/};
		MusicBuffer(std::string);
		MusicBuffer(int bufsize);
		MusicBuffer(int8_t* buf, int size) 
		{
			m_bufsize = size;
			memcpy(m_musicBuffer, buf, size);
		};

		~MusicBuffer();

		int Size();

		MusicBuffer(MusicBuffer&& other);
		MusicBuffer& operator= (MusicBuffer&& other);

		const int ReceiveMessage(network::ClientNetwork& sock, bool isStream = true);
		const int SentMessage(network::ClientNetwork& sock, bool isStream = true);

		BytePtr Get() const;

		void CopyString(std::string str);
		std::string ToString();

		void Clear();		
	};
}
