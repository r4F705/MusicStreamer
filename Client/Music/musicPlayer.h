#pragma once
#include "../resource.h"
#include "../Music/Formats/wav.h"

#include <fstream>
#include <memory>
#include <string>
#include <array>

#include <windows.h>
#include <mmsystem.h>
#include <mmreg.h>
#include <dsound.h>

#pragma comment(lib, "dsound.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "winmm.lib")

#define Kilobyte 131072

namespace musicStreamer {
	namespace music {

		struct DoubleBuffer 
		{
			std::shared_ptr<MusicBuffer>			m_playBuffer;
			std::shared_ptr<MusicBuffer>			m_fillBuffer;
		};

		__declspec(dllexport) class MusicPlayer
		{			
		private:					
			std::string									  m_status;

			int											  m_fileInitialSize;	// Worst name EVER!.
			int											  m_musicDataSize;		
			int											  m_volume;
			int											  m_averageBytesPerSecond;
			int											  m_currentBuffer;			
														  
			double										  m_progress;
														  
			HWND										  m_handle;
			DWORD										  m_dataBufferSize;			
			IDirectSound8 *								  m_directSound;
			IDirectSoundBuffer *						  m_primaryBuffer;
			IDirectSoundBuffer8 *						  m_secondaryBuffer;
						
			std::array<std::shared_ptr<DoubleBuffer>, 2>  m_buffers;
			std::shared_ptr<MusicBuffer>				  m_playBuffer;		
			std::shared_ptr<MusicBuffer>				  m_fillBuffer;	

			std::shared_ptr<WavHeader>					  m_wavHeader;
			std::shared_ptr<network::ClientNetwork>		  m_connection;
			std::shared_ptr<MusicBuffer>				  m_primaryMusicBuffer;
			std::shared_ptr<MusicBuffer>				  m_secondaryMusicBuffer;


		private:

			void SwapDouble();
			void DownloadDouble(std::string status, int currentBuffer);
			void SwapCurrentBuffer(int currentBuffer);
			
			void Handshake();
			bool Initialise();

			void Shutdown();
			void ReleaseSecondaryBuffer();
			
			// Convert it to use the double buffers.
			double UpdateProgress(int numBytes);
			void ResetProgress();
			bool LoadMusicData(std::shared_ptr<MusicBuffer> buf);
			bool PlayWaveData(std::shared_ptr<MusicBuffer> buf);
		public:		
			MusicPlayer(HWND handle);
			~MusicPlayer();

			void Play();
			void Pause();
			void Stop();
			const int Progress();

			// Min value {-10000}, Max Value {0}
			void SetVolume(int volume);
		};
	}
}


