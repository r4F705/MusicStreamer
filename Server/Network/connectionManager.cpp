#include "connectionManager.h"

musicStreamer::network::ConnectionManager::ConnectionManager()
{
}

void musicStreamer::network::ConnectionManager::AcceptConnections()
{
	m_serverNetwork = std::make_unique<ServerNetwork>(m_address.c_str(), m_port.c_str());

	while (true)
	{
		m_mutex.lock();
		if (m_serverNetwork->AcceptClient(m_clientId, m_clients))
		{
			printf("Client %d has been connected to the server\n", m_clientId);					
			// Create a new music streamer object and create a new thread to stream the music to the clients
			std::shared_ptr<MusicStreamer> streamer = std::make_shared<MusicStreamer>(m_clientId);
			std::thread thread(StreamMusic, *streamer, m_clients);
			thread.detach();

			m_clientId++;
		}
		m_mutex.unlock();
	}
}

void musicStreamer::network::ConnectionManager::ReadConfigurationFile(std::string path)
{
	std::ifstream myfile(path);	
	if (myfile.is_open())
	{		
		myfile >> m_address;
		myfile >> m_port;				

		myfile.close();
	}
	else
	{
		std::cout << "Failed to locate config file. Default address and port used." << std::endl;
		m_address = DEFAULT_ADDRESS;
		m_port = DEFAULT_PORT;
	}
}



void musicStreamer::network::StreamMusic(musicStreamer::MusicStreamer & musicStreamer, std::map<ClientID, SOCKET> clients)
{		
	musicStreamer.StreamMusic(clients);
}
