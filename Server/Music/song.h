#pragma once
#include "Formats\wav.h"

#include <string>
#include <fstream>

#include <iostream>

#include <windows.h>
#include <mmsystem.h>
#include <mmreg.h>

#define fourccRIFF MAKEFOURCC('R', 'I', 'F', 'F')
#define fourccDATA MAKEFOURCC('d', 'a', 't', 'a')
#define fourccFMT  MAKEFOURCC('f', 'm', 't', ' ')
#define fourccWAVE MAKEFOURCC('W', 'A', 'V', 'E')
#define fourccXWMA MAKEFOURCC('X', 'W', 'M', 'A')		

namespace musicStreamer {
	namespace music {

		enum class SoundFormat {
			WAV,
		};

		class Song
		{			
		private:
			SoundFormat m_soundFormat;
			std::string m_filename;

			std::shared_ptr<MusicBuffer> m_headerBuffer;
			std::shared_ptr<MusicBuffer> m_dataBuffer;

			bool LoadWaveFile(std::string filename);			

		public:
			Song();
			Song(SoundFormat soundFormat);
			~Song();

			static Song& Empty();

			bool LoadSongFile(std::string filename);
			MusicBuffer& GetFileHeader();			
			MusicBuffer& GetFileMusicData();				
		};
	}
}


