#pragma once
#include "../Music/Streamer/musicStreamer.h"

#include <map>
#include <thread>
#include <memory>
#include <mutex>

namespace musicStreamer
{
	namespace network
	{
		class ConnectionManager
		{
		private:
			unsigned int m_clientId;
			std::string m_address;
			std::string m_port;

			std::mutex m_mutex;
			std::map<ClientID, SOCKET> m_clients;
			std::unique_ptr<ServerNetwork> m_serverNetwork;			// Holds a server network to listen for incoming connections


		private:
			ConnectionManager();

			// Delete all the copy and move constructors for safety
			ConnectionManager(ConnectionManager const&) = delete;
			ConnectionManager(ConnectionManager&&) = delete;
			ConnectionManager& operator=(ConnectionManager const&) = delete;
			ConnectionManager& operator=(ConnectionManager&&) = delete;

		public:
			static ConnectionManager& Instance()
			{
				// As long as instance is a static var, it will only be 
				// created once. And as of C++11 this is also thread safe.
				static ConnectionManager instance;
				return instance;
			}

			void AcceptConnections();			
			void ReadConfigurationFile(std::string path);
		};	
		
		void StreamMusic(musicStreamer::MusicStreamer& musicStreamer, std::map<ClientID, SOCKET> clients);
	}
}

