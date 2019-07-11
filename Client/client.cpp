#include "Music\musicPlayer.h"

using namespace musicStreamer;
using namespace music;
using namespace network;

int main()
{	
	MusicPlayer ms(GetDesktopWindow());	
	ms.Play(); 			

	system("pause");
}