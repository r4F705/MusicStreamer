#pragma once
#include "./enums.h"
#include "Network/serverNetwork.h"

#define Kilobyte 131072 * 2

namespace musicStreamer {

	using Byte = int8_t;
	using BytePtr = int8_t*;

	enum class StreamerState;

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
		~MusicBuffer();

		int Size();

		MusicBuffer(MusicBuffer&& other);
		MusicBuffer& operator= (MusicBuffer&& other);

		const int Stream(SOCKET sock, int bytesPerSec);

		const int ReceiveMessage(SOCKET sock, bool isStream = true);
		const int SentMessage(SOCKET sock, bool isStream = true);

		BytePtr Get() const;

		void CopyString(std::string str);
		std::string ToString();

		void Clear();
	};
}
