#include "musicPlayer.h"
#include "../Music/Formats/wav.h"
#include "../resource.h"

#include <thread>

namespace musicStreamer {
	namespace music {

		MusicPlayer::MusicPlayer(HWND handle) :
			m_currentBuffer(0), m_status("460"), m_handle(handle), 
			m_directSound(nullptr), m_primaryBuffer(nullptr), m_secondaryBuffer(nullptr), m_primaryMusicBuffer(nullptr), m_secondaryMusicBuffer(nullptr),
			m_volume(0), m_averageBytesPerSecond(0), m_musicDataSize(0), m_connection(std::make_shared<network::ClientNetwork>(DEFAULT_ADDRESS, DEFAULT_PORT))
		{
			m_buffers = { std::make_shared<DoubleBuffer>(), std::make_shared<DoubleBuffer>() };
			Handshake();
			if (Initialise())
				std::cout << "MusicPlayer has been initialised!" << std::endl;
		}

		MusicPlayer::~MusicPlayer()
		{
			std::cout << "MusicPlayer shutdown!" << std::endl;
			ReleaseSecondaryBuffer();
			Shutdown();
		}

		void MusicPlayer::Handshake()
		{			
			MusicBuffer send(SMALL_BUFFER);
			MusicBuffer recv(SMALL_BUFFER);

			// Hello server
			send.CopyString("HELLO");
			send.SentMessage(*m_connection);
			send.Clear();

			// Recv welcome message from server.
			recv.ReceiveMessage(*m_connection);
			if (recv.ToString() == "WELCOME") // Successful handshake
			{
				std::cout << "[PROTOCOL] => " << recv.ToString() << std::endl;

				// Request the song list.
				send.CopyString("REQUEST SONG LIST");
				send.SentMessage(*m_connection);
				send.Clear();

				// Recv the songs list.
				recv.ReceiveMessage(*m_connection);
				std::cout << "[Available Songs]:" << std::endl << recv.ToString() << std::endl;

				do
				{
					// Await input to indicate the song that will be requested. This will change into a function.
					std::string songTitle = "";
					std::cin >> songTitle;

					if (songTitle == "")
						exit(0);

					// Send select song
					send.CopyString(songTitle);
					send.SentMessage(*m_connection);
					send.Clear();

					// Recv whether or not the server found your song.
					recv.ReceiveMessage(*m_connection);
					std::cout << "[PROTOCOL] => " << recv.ToString() << std::endl;

				} while (recv.ToString() != "SUCCESS LOAD");
			}
			else
				m_connection->Shutdown(m_connection->GetSocket());

			// Recv the file header.
			std::shared_ptr<MusicBuffer> header = std::make_shared<MusicBuffer>(WAVE_HEADER_SIZE);
			header->ReceiveMessage(*m_connection);

			m_wavHeader = std::make_unique<WavHeader>(Wav::ReadHeader(header->Get()));

			// Recv file size			
			recv.Clear();
			recv.ReceiveMessage(*m_connection);
			// Check for exceptions here.
			m_musicDataSize = std::stoi(recv.ToString());
			m_fileInitialSize = m_musicDataSize;
		}

		bool MusicPlayer::Initialise()
		{
			HRESULT result;
			DSBUFFERDESC bufferDesc;
			WAVEFORMATEX waveFormat;

			// Initialize the direct sound interface pointer for the default sound device.
			result = DirectSoundCreate8(NULL, &m_directSound, NULL);
			if (FAILED(result))
			{
				return false;
			}

			// Set the cooperative level to priority so the format of the primary sound buffer can be modified.
			// We use the handle of the desktop window since we are a console application.  If you do write a 
			// graphical application, you should use the HWnd of the graphical application. 
			result = m_directSound->SetCooperativeLevel(m_handle, DSSCL_PRIORITY);
			if (FAILED(result))
			{
				return false;
			}

			// Setup the primary buffer description.
			bufferDesc.dwSize = sizeof(DSBUFFERDESC);
			bufferDesc.dwFlags = DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRLVOLUME;
			bufferDesc.dwBufferBytes = 0;
			bufferDesc.dwReserved = 0;
			bufferDesc.lpwfxFormat = NULL;
			bufferDesc.guid3DAlgorithm = GUID_NULL;

			// Get control of the primary sound buffer on the default sound device.
			result = m_directSound->CreateSoundBuffer(&bufferDesc, &m_primaryBuffer, NULL);
			if (FAILED(result))
			{
				return false;
			}

			waveFormat.nSamplesPerSec = m_wavHeader->SampleRate;
			waveFormat.wBitsPerSample = m_wavHeader->BitsPerSample;
			waveFormat.nChannels = m_wavHeader->NumChannels;
			waveFormat.nBlockAlign = m_wavHeader->BlockAlign;
			waveFormat.nAvgBytesPerSec = m_wavHeader->SampleRate * m_wavHeader->BlockAlign;
			waveFormat.cbSize = 0;			

			m_averageBytesPerSecond = waveFormat.nAvgBytesPerSec;
			std::cout << m_averageBytesPerSecond << std::endl;

			// Set the primary buffer to be the wave format specified.
			result = m_primaryBuffer->SetFormat(&waveFormat);
			if (FAILED(result))
			{
				return false;
			}
			return true;
		}

		void MusicPlayer::Shutdown()
		{
			// Release the primary sound buffer pointer.
			if (m_primaryBuffer != nullptr)
			{
				m_primaryBuffer->Release();
				m_primaryBuffer = nullptr;
			}

			// Release the direct sound interface pointer.
			if (m_directSound != nullptr)
			{
				m_directSound->Release();
				m_directSound = nullptr;
			}
		}

		void MusicPlayer::ReleaseSecondaryBuffer()
		{
			// Release the secondary sound buffer.
			if (m_secondaryBuffer != nullptr)
			{
				(m_secondaryBuffer)->Release();
				m_secondaryBuffer = nullptr;
			}
		}

		void MusicPlayer::Play()
		{
			if (m_status == "460")
			{
				bool firstTime = true;
				do
				{
					if (firstTime)
					{
						firstTime = false;
						DownloadDouble(m_status, 0);
						DownloadDouble(m_status, 1);				
					}										
					else
					{					
						m_currentBuffer == 0 ? std::thread(&MusicPlayer::DownloadDouble, this, m_status, 1).detach() : std::thread(&MusicPlayer::DownloadDouble, this, m_status, 0).detach();
					}
					SwapDouble();
					LoadMusicData(m_buffers[m_currentBuffer]->m_playBuffer);
					PlayWaveData(m_buffers[m_currentBuffer]->m_playBuffer);
					UpdateProgress(m_buffers[m_currentBuffer]->m_playBuffer->Size());
					SwapCurrentBuffer(m_currentBuffer);					
				} while (m_musicDataSize > 0);

				// Music Data ended close connection.
				MusicBuffer send(SMALL_BUFFER);
				send.CopyString("326");
				send.SentMessage(*m_connection);
				send.Clear();
				std::cout << "Data Ended!" << std::endl;
				ResetProgress();		

				// Give server some tome to set up.
				Sleep(500);
				Handshake();
			}
			else m_status = "460";							
		}

		void MusicPlayer::Pause()
		{
			m_status = "382";
		}
		
		void MusicPlayer::Stop()
		{
			m_status = "326";
			ResetProgress();
		}

		const int MusicPlayer::Progress()
		{
			return m_progress * 100;
		}

		void MusicPlayer::SetVolume(int volume)
		{
			m_volume = volume;
		}

		void MusicPlayer::DownloadDouble(std::string status, int currentBuffer)
		{			
			auto curBuffer = m_buffers[currentBuffer];

			int recvChunk = m_musicDataSize > m_averageBytesPerSecond ? m_averageBytesPerSecond : m_musicDataSize;
			curBuffer->m_fillBuffer = std::make_shared<MusicBuffer>(recvChunk);

			if (status == "460")
			{
				MusicBuffer send(SMALL_BUFFER);
				send.CopyString("460");
				send.SentMessage(*m_connection);
				send.Clear();
				std::cout << "Stream" << std::endl;

				m_musicDataSize -= curBuffer->m_fillBuffer->ReceiveMessage(*m_connection, true);
				std::cout << m_musicDataSize << " Bytes remain to be streamed" << std::endl;				
			}
			else if (status == "382")
			{
				MusicBuffer send(SMALL_BUFFER);
				send.CopyString("382");
				send.SentMessage(*m_connection);
				send.Clear();
				std::cout << "Pause" << std::endl;
				return;
			}
			else
			{
				MusicBuffer send(SMALL_BUFFER);
				send.CopyString("326");
				send.SentMessage(*m_connection);
				send.Clear();
				std::cout << "Stop" << std::endl;
				return;
			}			

		}

		void MusicPlayer::SwapDouble()
		{			
			auto curBuffer = m_buffers[m_currentBuffer];

			std::shared_ptr<MusicBuffer> temp = curBuffer->m_fillBuffer;
			curBuffer->m_fillBuffer = curBuffer->m_playBuffer;
			curBuffer->m_playBuffer = temp;						
		}

		void MusicPlayer::SwapCurrentBuffer(int currentBuffer)
		{
			m_currentBuffer = m_currentBuffer == 0 ? 1 : 0;
		}

		double MusicPlayer::UpdateProgress(int numBytes)
		{		
			m_progress += ((double)numBytes / (double)m_fileInitialSize);
			
			return m_progress;
		}

		void MusicPlayer::ResetProgress()
		{
			m_progress = 0;
		}

		bool MusicPlayer::LoadMusicData(std::shared_ptr<MusicBuffer> buf)
		{
			// Create and initialize the header of the wave file in the Initialize function
			HRESULT result;
			
			WAVEFORMATEX waveFormat;
			IDirectSoundBuffer * tempBuffer;
			DSBUFFERDESC bufferDesc;

			m_dataBufferSize = buf->Size();

			bufferDesc.dwSize = sizeof(DSBUFFERDESC);
			bufferDesc.dwFlags = DSBCAPS_CTRLVOLUME | DSBCAPS_GLOBALFOCUS | DSBCAPS_CTRLPOSITIONNOTIFY;
			bufferDesc.dwBufferBytes = m_dataBufferSize;
			bufferDesc.dwReserved = 0;
			bufferDesc.lpwfxFormat = &waveFormat;
			bufferDesc.guid3DAlgorithm = GUID_NULL;

			waveFormat.wFormatTag = m_wavHeader->AudioFormat;
			waveFormat.nSamplesPerSec = m_wavHeader->SampleRate;
			waveFormat.wBitsPerSample = m_wavHeader->BitsPerSample;
			waveFormat.nChannels = m_wavHeader->NumChannels;
			waveFormat.nBlockAlign = m_wavHeader->BlockAlign;
			waveFormat.nAvgBytesPerSec = m_wavHeader->SampleRate * m_wavHeader->BlockAlign;
			waveFormat.cbSize = 0;
			// Create a temporary sound buffer with the specific buffer settings.
			result = m_directSound->CreateSoundBuffer(&bufferDesc, &tempBuffer, NULL);
			if (FAILED(result))
			{
				return false;
			}

			// Test the buffer format against the direct sound 8 interface and create the secondary buffer.
			result = tempBuffer->QueryInterface(IID_IDirectSoundBuffer8, (void**)&m_secondaryBuffer);
			if (FAILED(result))
			{
				return false;
			}

			// Release the temporary buffer.
			tempBuffer->Release();
			tempBuffer = nullptr;

			return true;
		}

		bool MusicPlayer::PlayWaveData(std::shared_ptr<MusicBuffer> buf)
		{
			HRESULT result;
			unsigned char * bufferPtr1;
			unsigned long   bufferSize1;
			unsigned char * bufferPtr2;
			unsigned long   bufferSize2;
			DWORD soundBytesOutput = 0;
			bool fillFirstHalf = true;
			LPDIRECTSOUNDNOTIFY8 directSoundNotify;
			DSBPOSITIONNOTIFY positionNotify[2];

			// Set position of playback at the beginning of the sound buffer.
			result = m_secondaryBuffer->SetCurrentPosition(0);
			if (FAILED(result))
			{
				return false;
			}

			// Set volume of the buffer to 100%.
			result = m_secondaryBuffer->SetVolume(m_volume);
			if (FAILED(result))
			{
				return false;
			}

			HANDLE playEventHandles[1];			
			playEventHandles[0] = CreateEvent(NULL, FALSE, FALSE, NULL);
			result = m_secondaryBuffer->QueryInterface(IID_IDirectSoundNotify8, (LPVOID*)&directSoundNotify);
			if (FAILED(result))
			{
				return false;
			}
			positionNotify[0].dwOffset = DSBPN_OFFSETSTOP;
			positionNotify[0].hEventNotify = playEventHandles[0];
			directSoundNotify->SetNotificationPositions(1, positionNotify);
			directSoundNotify->Release();			
			result = m_secondaryBuffer->Lock(0, m_dataBufferSize, (void**)&bufferPtr1, (DWORD*)&bufferSize1, (void**)&bufferPtr2, (DWORD*)&bufferSize2, 0);
			if (FAILED(result))
			{
				return false;
			}						
	
			memcpy(bufferPtr1, buf->Get(), bufferSize1);			
			if (bufferPtr2 != NULL)
			{				
				memcpy(bufferPtr1, buf->Get(), bufferSize1);
			}			
			result = m_secondaryBuffer->Unlock((void*)bufferPtr1, bufferSize1, (void*)bufferPtr2, bufferSize2);
			if (FAILED(result))
			{
				return false;
			}			
			result = m_secondaryBuffer->Play(0, 0, 0);			
			if (FAILED(result))
			{
				return false;
			}			
			result = WaitForMultipleObjects(1, playEventHandles, FALSE, INFINITE);					
			CloseHandle(playEventHandles[0]);
			return true;
		}
	}
}


