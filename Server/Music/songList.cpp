#include "songList.h"
#include <Windows.h>

void musicStreamer::music::SongList::ReadSongList()
{	
	std::string directoryPath(m_songsPath+"*");

	WIN32_FIND_DATA FindFileData;
	HANDLE hFind = FindFirstFile(directoryPath.c_str(), &FindFileData);

	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (FindFileData.dwFileAttributes != FILE_ATTRIBUTE_DIRECTORY)
				m_songsList.push_back(FindFileData.cFileName);
		} while (FindNextFile(hFind, &FindFileData));

		FindClose(hFind);
	}

	DWORD dwError = GetLastError();
	if (dwError != ERROR_NO_MORE_FILES)
	{
		return;
	}
}



musicStreamer::music::SongList::SongList()
	: m_songsPath(DEFAULT_SONGS_PATH) 
{
	ReadSongList();
}

musicStreamer::music::SongList::SongList(std::string path) :
	m_songsPath(path)
{
	ReadSongList();
}

musicStreamer::music::SongList::~SongList()
{
}

const std::string musicStreamer::music::SongList::GetSongsPath()
{
	return m_songsPath != "" ? m_songsPath : DEFAULT_SONGS_PATH;
}

std::string musicStreamer::music::SongList::GetSongs()
{
	std::string songs;
	for (size_t i = 0; i < m_songsList.size(); i++)
		songs += Split(m_songsList[i], '.') + "\r\n";		

	return songs;
}

std::string musicStreamer::music::SongList::Split(const std::string & s, char delim)
{
	std::stringstream ss;
	ss.str(s);
	std::string item;	
	std::getline(ss, item, delim);
	return item;
}


