/*
 * BC127.c
 *
 *  Created on: 17 May 2020
 *      Author: Matt
 *
 *      Contains information for audio files
 */

#include "AudioFile.h"

static uint8_t dataBuffer[AUDIO_BUFFER_SIZE];
static uint16_t bigEndianData[AUDIO_BUFFER_SIZE];
static enum UpdateFlag_e updateFlag;
static uint8_t volShift;
static enum WAVerror_e WAVerror;
static char fbuf[10];
static uint8_t totalAmountOfFiles;
static char songList[MAX_SONG_COUNT][12]; /*we can cut this down eventually but we need to pad or justify it*/
/*if we cut off the ".wav" after every filename we can bring file name size from 12 bytes to 8 bytes. This is
 * a 33% decrease in size so we can store 33% more songs in the same amount of space. We do need to make something that
 * handles files less than 12 bytes though. e.g. if we have a file named "test.wav" then if we store in in 12 bytes then
 * we get "test.wav". so if we just cut off the last 4 bytes then we still get "test.wav" and then add the ".wav" on after
 * we would be looking for "test.wav.wav". Maybe we could look for a "." and cut off everything after that? and then when
 * we re-add the ".wav" we search for 0x00 in the array and start there and add the .wav after that?
 */
static uint8_t currentSong;
WAVaudioFile WAVfile;
FRESULT res; //fatfs function result code
FATFS *FatFs_Obj;
FIL FatFsFile;

void sdInit(void)
{
	FatFs_Obj = malloc(sizeof (FATFS)); //assume roughly 46 bytes
	res = f_mount(FatFs_Obj,"",1);
	printf("f_mount result is: %i\r\n",res);

	if(res == FR_OK){ //Lists all files in (main?) directory
		strcpy(fbuf,"/");
		//res = scan_files(fbuf);
		findWAVFiles();
	}
}


void initializeAudioFile(FIL *FatFsFile, WAVaudioFile *WAVfile)
{

	TCHAR *name;
	char line[44];
	UINT bytesRead;
	uint8_t res;

	name = WAVfile->fileName;

	res = f_open(FatFsFile, name, FA_READ);
#ifdef DEBUG
	if (res) printf("f_open result is: %i\r\n",res);
#endif
	if (res){
		fileError(WAVfile, OPEN_ERROR);
	}

	f_read(FatFsFile, &line, 44, &bytesRead);

#ifdef DEBUG
	  for (uint8_t i = 0; i < 43; i++){
		  if(line[i]){
			  printf("%c",line[i]);
	      }else{
			  printf(".");
		  }
	  }
	  printf("\r\n");
#endif

	  //Fill the WAVaudioFile type. Shift bytes because most data is little endian
	  WAVfile->audioFormat = (line[21] << 8) | line[20];
	  WAVfile->numberOfChannels = (line[23] << 8) | line[22];
	  WAVfile->sampleRate = (line[27] << 24) | (line[26] << 16) | (line[25] << 8) | line[24];
	  WAVfile->bitsPerSample = (line[35] << 8) | line[34];

#ifdef DEBUG
	  printf("audioFormat: %u\r\n",WAVfile->audioFormat);
	  printf("numberOfChannels: %u\r\n",WAVfile->numberOfChannels);
	  printf("sampleRate: %u\r\n",(uint)WAVfile->sampleRate);
	  printf("bitsPerSample: %u\r\n",WAVfile->bitsPerSample);
#endif

	  //check that the file falls within the programmed parameters
	  fileCheck(WAVfile);

	  //place filePointer back at zero so we read on the block boundary. This increases read speed
	  f_lseek(FatFsFile,0);

	  f_read(FatFsFile, &dataBuffer, AUDIO_BUFFER_SIZE, &bytesRead);
#ifdef DEBUG
	  printf("BytesRead = %i \r\n",bytesRead);
#endif

	  //fill buffer with initial data
	  for(uint16_t i=0;i<AUDIO_BUFFER_SIZE / 2;i++){
		  bigEndianData[i] = ((int16_t) ((dataBuffer[(i * 2) + 1] << 8) | (dataBuffer[(i * 2)])) >> volShift);
	  }
}

void streamAudioFile(FIL *FatFsFile, WAVaudioFile *WAVfile)
{

	uint16_t i;
	UINT bytesRead;

	  if(updateFlag == UPDATE_LOWER){ //fill lower half of the circular buffer
		  f_read(FatFsFile, &dataBuffer, AUDIO_BUFFER_SIZE, &bytesRead);
		  updateFlag = UPDATE_NONE;

		  if(WAVfile->numberOfChannels == 2){
			  for(i=0;i<(AUDIO_BUFFER_SIZE / 2);i++){
				  bigEndianData[i] = ((int16_t) ((dataBuffer[(i * 2) + 1] << 8) | (dataBuffer[(i * 2)])) >> volShift);
			  }
		  }

		  if((bytesRead < AUDIO_BUFFER_SIZE)){ // this will cut off up to last 510 bytes. That is ok
			  f_close(FatFsFile);
			  HAL_I2S_DMAStop(&hi2s2);
			  HAL_UART_Transmit(&huart2,(uint8_t*)"MUSIC 10 PAUSE\r",15,1000);
#ifdef DEBUG
			  printf("close1\r\n");
#endif
		  }
	  }

	  if(updateFlag == UPDATE_UPPER){//fill upper half of the circular buffer
		  f_read(FatFsFile, &dataBuffer, AUDIO_BUFFER_SIZE, &bytesRead);
		  updateFlag = UPDATE_NONE;

		  if(WAVfile->numberOfChannels == 2){ /*Need to make this for mono still*/
			  for(i=0;i< AUDIO_BUFFER_SIZE / 2;i++){
				  bigEndianData[i + (AUDIO_BUFFER_SIZE / 2)] = ((int16_t) ((dataBuffer[(i * 2) + 1] << 8) | (dataBuffer[(i * 2)])) >> volShift);
			  }
		  }

		  if((bytesRead < AUDIO_BUFFER_SIZE)){ // this will cut off up to last 510 bytes. That is ok
			  f_close(FatFsFile);
			  HAL_I2S_DMAStop(&hi2s2);
			  HAL_UART_Transmit(&huart2,(uint8_t*)"MUSIC 10 PAUSE\r",15,1000);
#ifdef DEBUG
			  printf("close2\r\n");
#endif
		  }

	  }
}

void fileError(WAVaudioFile *WAVfile, enum WAVerror_e error)
{
	//TODO - implement error handler. make this function return something and make functions above return as well
#ifdef DEBUG
	printf("fileError: %u\r\n",error);
#endif

}

void fileCheck(WAVaudioFile *WAVfile)
{

	WAVerror = NO_ERROR;

	if(WAVfile->audioFormat != 1){ //yes this will overwrite the previous one but hopefully there is only one error
		WAVerror |= FORMAT_ERROR;
	}

	if((WAVfile->numberOfChannels != 1) && (WAVfile->numberOfChannels != 2)){
		WAVerror |= CHANNEL_ERROR;
	}

	if((WAVfile->sampleRate != 48000)){
		WAVerror |= RATE_ERROR;
	}

	if((WAVfile->bitsPerSample != 16)){
		WAVerror |= SAMPLE_ERROR;
	}

	if(WAVerror){
		fileError(WAVfile, WAVerror);
	}
}

void volumeUp(void)
{
	volShift = volShift - 1;
	if(volShift > 16){
		volShift = 0;
	}
}

void volumeDown(void)
{
	volShift = volShift + 1;
	if(volShift > 7){
		volShift = 7;
	}
}

void startStream(void)
{
	HAL_I2S_Transmit_DMA(&hi2s2,bigEndianData,AUDIO_BUFFER_SIZE);
}

void stopStream(void)
{
	HAL_I2S_DMAStop(&hi2s2);
}

void pauseStream(void)
{
	HAL_I2S_DMAPause(&hi2s2);
}

void resumeStream(void)
{
	HAL_I2S_DMAResume(&hi2s2);
}

void HAL_I2S_TxHalfCpltCallback(I2S_HandleTypeDef *hi2s)
{
	updateFlag = UPDATE_LOWER;
}

void HAL_I2S_TxCpltCallback(I2S_HandleTypeDef *hi2s)
{
	updateFlag = UPDATE_UPPER;
}

FRESULT scan_files(char* path)
{
	FRESULT res;
	DIR dir;
	UINT i;
	static FILINFO fno;

	res = f_opendir(&dir, path);
	printf("Scan Res is: %i\r\n",res);
	if(res == FR_OK){
		for(;;){
			res = f_readdir(&dir, &fno);
			if(res != FR_OK || fno.fname[0] == 0) break;
			if(fno.fattrib & AM_DIR){
				i = strlen(path);
				sprintf(&path[i], "/%s", fno.fname);
				res = scan_files(path);
				if (res != FR_OK) break;
				path[i] = 0;
			}else{
				printf("%s/%s\r\n", path, fno.fname);
				//totalAmountOfFiles++;
			}
		}
		f_closedir(&dir);
	}
	return res;
}

void findWAVFiles(void)
{
    FRESULT res;     /* Return value */
    DIR dir;         /* Directory object */
    FILINFO fno;    /* File information */

    res = f_findfirst(&dir, &fno, "", "*.wav");  /* Start to search for WAV files */

    while (res == FR_OK && fno.fname[0]) {         /* Repeat while an item is found */
    	memcpy((songList + currentSong),fno.fname,12);
        printf("%s\r\n", fno.fname);                /* Print the object name */
        res = f_findnext(&dir, &fno);               /* Search for next item */
        currentSong++;
        if(currentSong > MAX_SONG_COUNT){ //keeps us from overflowing our list
        	break;
        }
    }
    if(currentSong == 0){ //prevents underflow in case of 0 files
    	currentSong = 1;
    }
    totalAmountOfFiles = currentSong - 1;
    currentSong = 0;
    f_closedir(&dir);
}

void nextSong(void)
{
	currentSong++;
	if(currentSong > MAX_SONG_COUNT){
		currentSong = 0;
	}
}

void previousSong(void)
{
	currentSong--;
	if(currentSong > MAX_SONG_COUNT){
		currentSong = MAX_SONG_COUNT;
	}
}

void selectSong(void)
{
	memcpy(WAVfile.fileName,(songList + currentSong),12);
}
