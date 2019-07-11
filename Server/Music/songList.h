#pragma once
#include <vector>
#include <string>
#include <sstream>

namespace musicStreamer {
	namespace music {

#define DEFAULT_SONGS_PATH "../Server/Songs/"	

		class SongList 
		{
		private:
			std::string m_songsPath;
			std::vector<std::string> m_songsList;
		private:			
			void ReadSongList();
			std::string Split(const std::string &s, char delim);
		public:
			SongList();
			SongList(std::string path);
			~SongList();

		public:
			std::string GetSongs();
			
			const std::string GetSongsPath();
		};
	}
}