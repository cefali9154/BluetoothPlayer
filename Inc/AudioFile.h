/*
 * AudioFile.h
 *
 *  Created on: May 17, 2020
 *      Author: Matt
 */

#ifndef AUDIOFILE_H_
#define AUDIOFILE_H_

#include "globals.h"
#include <stdint.h>
#include "ff.h"

typedef struct{
	char fileName[12];
	uint16_t audioFormat;
	uint16_t numberOfChannels;
	uint32_t sampleRate;
	uint16_t bitsPerSample;
} WAVaudioFile;

enum WAVerror_e{
	NO_ERROR = 0x00,
	OPEN_ERROR = 0x01,
	FORMAT_ERROR = 0x02,
	CHANNEL_ERROR = 0x04,
	RATE_ERROR = 0x08,
	SAMPLE_ERROR = 0x10
};

enum UpdateFlag_e{
	UPDATE_NONE,
	UPDATE_UPPER,
	UPDATE_LOWER
};


extern FIL FatFsFile;
//extern WAVaudioFile WAVfile;

//Prototypes
void initializeAudioFile(FIL *FatFsFile, WAVaudioFile *WAVfile);
void fileError(WAVaudioFile *WAVfile, enum WAVerror_e error);
void fileCheck(WAVaudioFile *WAVfile);
uint8_t streamAudioFile(FIL *FatFsFile, WAVaudioFile *WAVfile);
void startStream(void);
void volumeDown(void);
void volumeUp(void);
FRESULT scan_files(char* path);
void sdInit(void);
void resumeStream(void);
void pauseStream(void);
void stopStream(void);
void findWAVFiles(void);
void nextSong(void);
void previousSong(void);
void selectSong(WAVaudioFile *WAVfile);
void fillAudioBuffer(enum UpdateFlag_e bufferPos, WAVaudioFile *WAVfile);


#endif /* AUDIOFILE_H_ */
