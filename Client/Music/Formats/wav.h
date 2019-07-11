#pragma once
#include "../../resource.h"

#include <iostream>
#include <string>
#include <fstream>
#include <cstdint>

#define WAVE_HEADER_SIZE 44

namespace musicStreamer {
	namespace music {	
		struct WavHeader
		{
			Byte RIFF[4];		// ChunkID
			uint32_t ChunkSize;
			Byte WAVE[4];

			// the fmt describes the format of 
			// the sound information in the data
			// sub-chunk
			Byte Subchunk1ID[4];
			uint32_t Subchunk1Size;
			uint16_t AudioFormat;
			uint16_t NumChannels;
			uint32_t SampleRate;
			uint32_t ByteRate;
			uint16_t BlockAlign;
			uint16_t BitsPerSample;

			// data sub-chunck
			Byte Subchunk2ID[4];
			uint32_t Subchunk2Size;
		};

		class Wav
		{			
		public:
			static WavHeader ReadHeader(const BytePtr buffer);
			//static void ReadData(const char* filename, MusicBuffer& buffer, int fileLength);
			// Reads the header data from the file.
			static void ReadHeaderData(const char* filename, MusicBuffer& buffer, int filelength);

			static void PrintHeader(WavHeader wavHeader);
			static int getFileSize(FILE* inFile);
		};


	}
}

