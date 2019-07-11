#include "musicStreamer.h"

#include <iostream>

using namespace musicStreamer;
using namespace network;

const ClientID & MusicStreamer::GetClientID()
{
	return m_clientId;
}

void MusicStreamer::StreamMusic(std::map<ClientID, SOCKET>& clients)
{	
	SOCKET clientSocket = clients[m_clientId];

	while (m_currentState != StreamerState::CLOSED)
	{
		switch (m_currentState)
		{
		case StreamerState::IDLE:
			std::cout << "Streamer is IDLE: Client " << m_clientId << std::endl;

			m_currentState = Handshake(clientSocket);
			break;

		case StreamerState::STARTING:
			std::cout << "Streamer is STARTING: Client " << m_clientId << std::endl;

			if (!m_currentSong)
			{
				m_connection->Shutdown(clientSocket);			
				m_currentState = StreamerState::CLOSED;				
#if _DEBUG
				std::cout << "Error no song available for streaming" << std::endl;
#endif
				break;
			}
			m_currentState = StartStreaming(clientSocket, *m_currentSong);
			break;

		case StreamerState::PLAYING:
			std::cout << "Streamer is PLAYING: Client " << m_clientId << std::endl;			
			m_currentState = StreamControl(clientSocket, *m_currentSong);			
			
			break;
		case StreamerState::STOPPED:
			std::cout << "Streamer has STOPPED: Client " << m_clientId << std::endl;
			m_currentState = Handshake(clientSocket);

			break;

		case StreamerState::CLOSED:
			std::cout << "Streamer has CLOSED: Client " << m_clientId << std::endl;
			m_connection->Shutdown(clientSocket);	
			break;

		default:
			break;
		}
	}	
}

StreamerState musicStreamer::MusicStreamer::StreamControl(SOCKET sock, Song & song)
{
	auto header = Wav::ReadHeader(song.GetFileHeader().Get());
	auto avgBytesPerSec = header.SampleRate * header.BlockAlign;
	song.GetFileMusicData().Stream(sock, avgBytesPerSec);	

	return StreamerState::STOPPED;
}

StreamerState MusicStreamer::StartStreaming(SOCKET sock, Song& song)
{		
	MusicBuffer send(SMALL_BUFFER);	
	// Send the header of the file.
	song.GetFileHeader().SentMessage(sock, true);
	
	// Send the size of the music data!
	send.CopyString(std::to_string(song.GetFileMusicData().Size()));
	send.SentMessage(sock);
	send.Clear();
	
	return StreamerState::PLAYING;
}

StreamerState musicStreamer::MusicStreamer::Handshake(SOCKET sock)
{
	MusicBuffer send(SMALL_BUFFER);
	MusicBuffer recv(SMALL_BUFFER);	
	recv.ReceiveMessage(sock);

	if (recv.ToString() != "HELLO")			
		return StreamerState::CLOSED;	
	
	std::cout << "[PROTOCOL] => " << recv.ToString() << std::endl;
	recv.Clear();

	send.CopyString("WELCOME");
	send.SentMessage(sock);
	send.Clear();

	recv.ReceiveMessage(sock);
	if (recv.ToString() != "REQUEST SONG LIST")
		return StreamerState::CLOSED;

	std::cout << "[PROTOCOL] => " << recv.ToString() << std::endl;
	recv.Clear();

	// Read the song list and send it to the client 
	SongList songList;
	send.CopyString(songList.GetSongs());
	send.SentMessage(sock);
	send.Clear();

	m_currentSong = std::make_shared<Song>();

	bool result;
	do
	{
		result = GetSong(sock, songList, m_currentSong);
	} while (!result);
		
	return StreamerState::STARTING;
}

// Support multiple data files.
bool MusicStreamer::GetSong(SOCKET sock, SongList& list, std::shared_ptr<Song> song)
{
	bool found;
	// Create a buffer and receive the request for a song from the client	
	MusicBuffer buf(SMALL_BUFFER);	
	buf.ReceiveMessage(sock);
	std::string songPath = DEFAULT_SONGS_PATH + buf.ToString() + ".wav";	

#ifdef _DEBUG
	std::cout << "[*] File Requested " << buf.ToString() << std::endl;
	std::cout << "[*] File Path " << songPath << std::endl;	
#endif
	if (song->LoadSongFile(songPath))
	{		
#ifdef _DEBUG
		std::cout << "[*] Song was loaded successfully for client" << std::endl;			
#endif		
		found = true;
	}
	else
	{
		found = false;
#ifdef _DEBUG 
		std::cout << "[*] Song failed to load successfully for client" << std::endl;
#endif		
	}	
	return SendLoadOutcome(sock, found);
}

bool MusicStreamer::SendLoadOutcome(SOCKET sock, bool found)
{	
	MusicBuffer send(SMALL_BUFFER);		
	if (found) 
	{
		send.CopyString("SUCCESS LOAD");
		send.SentMessage(sock);

		return true;
	}	
	else
	{
		send.CopyString("FAILED LOAD");
		send.SentMessage(sock);

		return false;
	}
}
