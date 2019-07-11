#include "wav.h"
#include <vector>
#include <mutex>

namespace musicStreamer {
	namespace music {

		WavHeader Wav::ReadHeader(const BytePtr buffer)
		{
			// Maybe it will be better to move them instead of copying.

			// This takes for granted that the header will be 44 bytes
			WavHeader wav;
			// Read RIFF
			wav.RIFF[0] = buffer[0]; wav.RIFF[1] = buffer[1]; wav.RIFF[2] = buffer[2]; wav.RIFF[3] = buffer[3];
			// Read chunk size
			memcpy(&wav.ChunkSize, &buffer[4], 4);
			// Read wave
			wav.WAVE[0] = buffer[8]; wav.WAVE[1] = buffer[9]; wav.WAVE[2] = buffer[10]; wav.WAVE[3] = buffer[11];
			// Read Subchunk1
			wav.Subchunk1ID[0] = buffer[12]; wav.Subchunk1ID[1] = buffer[13]; wav.Subchunk1ID[2] = buffer[14]; wav.Subchunk1ID[3] = buffer[15];
			// Read Subchunk1Size
			memcpy(&wav.Subchunk1Size, &buffer[16], 4);
			// Read Audio format
			memcpy(&wav.AudioFormat, &buffer[20], 2);
			// Read Number of channels
			memcpy(&wav.NumChannels, &buffer[22], 2);
			// Read Sample rate
			memcpy(&wav.SampleRate, &buffer[24], 4);
			// Read Byte rate
			memcpy(&wav.ByteRate, &buffer[28], 4);
			// Read Block align
			memcpy(&wav.BlockAlign, &buffer[32], 2);
			// Read bits per sample 
			memcpy(&wav.BitsPerSample, &buffer[34], 2);
			// Read Subchunk2ID
			wav.Subchunk2ID[0] = buffer[36]; wav.Subchunk2ID[1] = buffer[37]; wav.Subchunk2ID[2] = buffer[38]; wav.Subchunk2ID[3] = buffer[39];
			//Read Subchunk2 size
			memcpy(&wav.Subchunk2Size, &buffer[40], 4);

			return wav;
		}

		void Wav::ReadData(const char * filename, MusicBuffer& buffer, int filelength, int isHeader)
		{
			// Put a guard before reading a file.
			std::mutex mut;
			std::lock_guard<std::mutex> guard(mut);

			// Read only binary data or else the sound data will be wrong.
			std::ifstream ifs(filename, std::ifstream::binary);

			if (isHeader) 
				ifs.seekg(0);
			else
				ifs.seekg(WAVE_HEADER_SIZE);

			ifs.read((char*)buffer.Get(), filelength); // read to buffer			
		}

		void Wav::PrintHeader(WavHeader wavHeader)
		{
			std::cout << "RIFF header                :" << wavHeader.RIFF[0] << wavHeader.RIFF[1] << wavHeader.RIFF[2] << wavHeader.RIFF[3] << std::endl;
			std::cout << "WAVE header                :" << wavHeader.WAVE[0] << wavHeader.WAVE[1] << wavHeader.WAVE[2] << wavHeader.WAVE[3] << std::endl;
			std::cout << "FMT                        :" << wavHeader.Subchunk1ID[0] << wavHeader.Subchunk1ID[1] << wavHeader.Subchunk1ID[2] << wavHeader.Subchunk1ID[3] << std::endl;
			std::cout << "Data size                  :" << wavHeader.ChunkSize << std::endl;

			// Display the sampling Rate from the header
			std::cout << "Sampling Rate              :" << wavHeader.SampleRate << std::endl;
			std::cout << "Number of bits used        :" << wavHeader.BitsPerSample << std::endl;
			std::cout << "Number of channels         :" << wavHeader.NumChannels << std::endl;
			std::cout << "Number of bytes per second :" << wavHeader.ByteRate << std::endl;
			std::cout << "Data length                :" << wavHeader.Subchunk2Size << std::endl;
			std::cout << "Audio Format               :" << wavHeader.AudioFormat << std::endl;
			// Audio format 1=PCM,6=mulaw,7=alaw, 257=IBM Mu-Law, 258=IBM A-Law, 259=ADPCM

			std::cout << "Block align                :" << wavHeader.BlockAlign << std::endl;
			std::cout << "Data string                :" << wavHeader.Subchunk2ID[0] << wavHeader.Subchunk2ID[1] << wavHeader.Subchunk2ID[2] << wavHeader.Subchunk2ID[3] << std::endl;
		}

		int Wav::GetFileSize(const char * filename)
		{			
			std::ifstream in(filename, std::ifstream::ate | std::ifstream::binary);
			return in.tellg();	
		}
	}
}