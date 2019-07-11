#pragma once
#include "../../resource.h"
#include "../song.h"
#include "../songList.h"


#include <map>
namespace musicStreamer{

	using namespace network;
	using namespace music;

	class MusicStreamer 
	{
	private:			
		StreamerState m_currentState;
		ClientID m_clientId;		
		std::shared_ptr<Song> m_currentSong;		
		std::shared_ptr<ServerNetwork> m_connection;					

	public:
		MusicStreamer(ClientID clientId)
			:m_clientId(clientId), m_connection(std::make_shared<ServerNetwork>()) , m_currentState(StreamerState::IDLE){};

		const ClientID& GetClientID();

		void StreamMusic(std::map<ClientID, SOCKET>& clients);

	private:
		StreamerState StreamControl(SOCKET sock, Song& song);
		StreamerState StartStreaming(SOCKET sock, Song& song);

		StreamerState Handshake(SOCKET sock);

		bool GetSong(SOCKET sock, SongList& list, std::shared_ptr<Song> song);
		bool SendLoadOutcome(SOCKET sock, bool song);
	};
}



