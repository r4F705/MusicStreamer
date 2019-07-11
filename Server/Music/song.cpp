#include "song.h"

using namespace musicStreamer;
using namespace music;

Song::Song() :
	m_filename(""), m_headerBuffer(nullptr), m_dataBuffer(nullptr), m_soundFormat(SoundFormat::WAV)
{

}

Song::Song(SoundFormat soundFormat) :
	m_filename(""), m_headerBuffer(nullptr), m_dataBuffer(nullptr), m_soundFormat(soundFormat)
{

}

musicStreamer::music::Song::~Song()
{
#ifdef _DEBUG
	std::cout << "Song " << m_filename << " has been deallocated" << std::endl;
#endif 
}

Song & musicStreamer::music::Song::Empty()
{
	Song song;
	return song;
}

bool Song::LoadWaveFile(std::string filename)
{
	m_headerBuffer = std::make_shared<MusicBuffer>(WAVE_HEADER_SIZE);	
	Wav::ReadData(filename.c_str(), *m_headerBuffer, m_headerBuffer->Size(), true);	
	
	if(!Wav::ReadHeader(m_headerBuffer->Get()).AudioFormat == fourccWAVE)
		return false;
	
	auto fileSize = Wav::GetFileSize(m_filename.c_str());
	m_dataBuffer = std::make_shared<MusicBuffer>(fileSize - WAVE_HEADER_SIZE);
	 
	Wav::ReadData(filename.c_str(), *m_dataBuffer, m_dataBuffer->Size(), false);

	return true;
}

bool Song::LoadSongFile(std::string filename)
{
	m_filename = filename;
	try
	{
		switch (m_soundFormat)
		{
		case SoundFormat::WAV:
			return LoadWaveFile(m_filename);
		default:
			return false;
		}
	}
	catch (const std::exception& ex)
	{
		std::cout << "LoadSongException: " << ex.what() << std::endl;
	}
	return false;
}
 
MusicBuffer& Song::GetFileHeader() 
{
	return *m_headerBuffer;
}

MusicBuffer& Song::GetFileMusicData()
{
	return *m_dataBuffer;
}

