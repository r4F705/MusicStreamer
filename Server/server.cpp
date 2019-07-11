#include "Network\serverNetwork.h"
#include "Network\connectionManager.h"
#include "Music\songList.h"
#include "Music\song.h"
#include "Music\Formats\wav.h"

#include <iostream>

using namespace musicStreamer;
using namespace network;

void AcceptConnections()
{
	ConnectionManager::Instance().AcceptConnections();
}

int main()
{	
	ConnectionManager::Instance().ReadConfigurationFile("config.ini");
	ConnectionManager::Instance().AcceptConnections();	

	system("pause");
}