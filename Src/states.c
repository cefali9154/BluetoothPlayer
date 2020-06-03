/*
 * states.c
 *
 *  Created on: May 26, 2020
 *      Author: Matt
 */

#include "states.h"
#include "bc127.h"
#include "Settings.h"
#include "stm32l0xx_hal.h"
#include "AudioFile.h"
#include "gpio.h"
#include "userInputs.h"

static uint32_t timeoutCount;
Run_States runStates = initState;


void initState(WAVaudioFile *WAVfile, bc127Device_t *bc127Device)
{
	//printf("init\n");
	if(bc127Device->connectionStatus == READY){
		/*send blank command to clear*/
		bc127SendCommand(bc127Device, BLANK);
		timeoutCount = HAL_GetTick();
		runStates = waitForAutoconnState;
	}
}

/*Do nothing until either we autoconnect or user enters pairing mode */
void waitForAutoconnState(WAVaudioFile *WAVfile, bc127Device_t *bc127Device)
{
	/*Reset if we timeout*/
	if((HAL_GetTick() - timeoutCount) > AUTOCONN_TIMEOUT_MS){
		bc127SendCommand(bc127Device, RESET_BC127);
		timeoutCount = HAL_GetTick();
	}

	if(bc127Device->connectionStatus == CONNECTED){
		runStates = connectedState;
	}

	if((debouncedBTNs & S1_Pin) && (debouncedBTNs & S2_Pin)){
		/*clear bt address just in case*/
		memset(bc127Device->bluetoothAddress, 0, 12);
		bc127SendCommand(bc127Device, PAIR_MODE);
		runStates = pairingModeState;
		printf("pairing\r\n");
		timeoutCount = HAL_GetTick();
	}
}

void pairingModeState(WAVaudioFile *WAVfile, bc127Device_t *bc127Device)
{
	char blankAddress[12] = "000000000000";
	/*wait for INQU_OK or timeout*/
	if((HAL_GetTick() - timeoutCount) > PAIR_MODE_TIMEOUT_MS){
		bc127SendCommand(bc127Device, PAIR_MODE);
		timeoutCount = HAL_GetTick();
	}

	if((bc127Device->notification == INQUIRY_OK) && strncmp(bc127Device->bluetoothAddress, blankAddress,12)){
		runStates = openA2DPState;
		printf("opening\r\n");
		bc127SendCommand(bc127Device, PAIR);
	}
}

void openA2DPState(WAVaudioFile *WAVfile, bc127Device_t *bc127Device)
{
	if(bc127Device->connectionStatus == CONNECTED){
		printf("opened\r\n");
		runStates = connectedState;
	}

	if(bc127Device->notification == PAIR_ERROR){
		runStates = waitForAutoconnState;
	}
}

/*This state doesnt do anything right now*/
void connectedState(WAVaudioFile *WAVfile, bc127Device_t *bc127Device)
{
	printf("connected\r\n");
	runStates = loadFileState;
}

void loadFileState(WAVaudioFile *WAVfile, bc127Device_t *bc127Device)
{
	printf("load\r\n");
	//strcpy(WAVfile.fileName, "BABYEL~1.WAV"); /*This is where we will choose the file*/
	selectSong(WAVfile);
	initializeAudioFile(&FatFsFile,WAVfile);
	runStates = idleState;
}

void idleState(WAVaudioFile *WAVfile, bc127Device_t *bc127Device) /*we can sit in this state for a while*/
{
	if(bc127Device->playStatus == PLAYING){
		runStates = playSongState;
		startStream();
	}
	if(oneshotBTNs & S1_Pin){
		runStates = previousSongState;
	}
	else if(oneshotBTNs & S2_Pin){
		runStates = nextSongState;
	}
	else if(oneshotBTNs & S3_Pin){
		runStates = playSongState;
		bc127SendCommand(bc127Device, PLAY);
		bc127Device->playStatus = PLAYING;
		startStream();
		printf("Playing\r\n");
	}
	if(bc127Device->connectionStatus == NOT_CONNECTED){
		runStates = waitForAutoconnState;
	}
}

void previousSongState(WAVaudioFile *WAVfile, bc127Device_t *bc127Device)
{
	previousSong();
	runStates = loadFileState;
}

void nextSongState(WAVaudioFile *WAVfile, bc127Device_t *bc127Device)
{
	nextSong();
	runStates = loadFileState;
}

void playSongState(WAVaudioFile *WAVfile, bc127Device_t *bc127Device) /*we can sit in this state for a while*/
{
	if(streamAudioFile(&FatFsFile,WAVfile) == 1){
		stopStream();
		runStates = nextSongState; /*if we reach EOF then go to next song*/
	}
	//streamAudioFile(&FatFsFile,WAVfile);

	/*add handling for if BT device is disconnected*/
	if(oneshotBTNs){
		if(oneshotBTNs & S1_Pin){
			runStates = previousSongState;
			//bc127SendCommand(bc127Device, PAUSE); /*TODO dont drop A2DP just change the data over*/
			bc127Device->playStatus = PLAYING; /*Because we are time dependent, I dont want to rewrite this value every
			time we enter the state. Instead we only write it on the state transition*/
			stopStream();
		}
		else if(oneshotBTNs & S2_Pin){
			runStates = nextSongState;
			//bc127SendCommand(bc127Device, PAUSE);
			bc127Device->playStatus = PLAYING;
			stopStream();
		}
		else if(oneshotBTNs & S3_Pin){
			runStates = pauseSongState;
			//bc127SendCommand(bc127Device, PAUSE);
			bc127Device->playStatus = PLAYING;
			pauseStream();
		}
	}
	if(bc127Device->connectionStatus == NOT_CONNECTED){
		runStates = waitForAutoconnState;
	}
}

void pauseSongState(WAVaudioFile *WAVfile, bc127Device_t *bc127Device) /*we can sit in this state for a while*/
{
	if(oneshotBTNs & S1_Pin){
		runStates = previousSongState;
		/*we dont change the status here so the user can skip through a bunch of files easily*/
	}
	else if(oneshotBTNs & S2_Pin){
		runStates = nextSongState;
	}
	else if(oneshotBTNs & S3_Pin){
		runStates = playSongState;
		bc127SendCommand(bc127Device, PLAY);
		bc127Device->playStatus = PLAYING;
		resumeStream();
	}

	if(bc127Device->connectionStatus == NOT_CONNECTED){
		runStates = waitForAutoconnState;
	}
}
