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


void initState(void)
{
	//printf("init\n");
	if(bc127Device.status == READY){
		/*send blank command to clear*/
		bc127SendCommand(BLANK);
		timeoutCount = HAL_GetTick();
		runStates = waitForAutoconnState;
	}
}

/*Do nothing until either we autoconnect or user enters pairing mode */
void waitForAutoconnState(void)
{
	/*Reset if we timeout*/
	if((HAL_GetTick() - timeoutCount) > AUTOCONN_TIMEOUT_MS){
		bc127SendCommand(RESET_BC127);
		timeoutCount = HAL_GetTick();
	}

	if(bc127Device.status == CONNECTED){
		runStates = connectedState;
	}

	if((debouncedBTNs & S1_Pin) && (debouncedBTNs & S2_Pin)){
		/*clear bt address just in case*/
		memset(bc127Device.bluetoothAddress, 0, 12);
		bc127SendCommand(PAIR_MODE);
		runStates = pairingModeState;
		printf("pairing\r\n");
		timeoutCount = HAL_GetTick();
	}
}

void pairingModeState(void)
{
	char blankAddress[12] = "000000000000";
	/*wait for INQU_OK or timeout*/
	if((HAL_GetTick() - timeoutCount) > PAIR_MODE_TIMEOUT_MS){
		bc127SendCommand(PAIR_MODE);
		timeoutCount = HAL_GetTick();
	}

	if((bc127Device.notification == INQUIRY_OK) && strncmp(bc127Device.bluetoothAddress, blankAddress,12)){
		runStates = openA2DPState;
		printf("opening\r\n");
		bc127SendCommand(PAIR);
	}
}

void openA2DPState(void)
{
	if(bc127Device.status == CONNECTED){
		printf("opened\r\n");
		runStates = connectedState;
	}

	if(bc127Device.notification == PAIR_ERROR){
		runStates = waitForAutoconnState;
	}
}

/*This state doesnt do anything right now*/
void connectedState(void)
{
	printf("connected\r\n");
	runStates = loadFileState;
}

void loadFileState(void)
{
	printf("load\r\n");
	//strcpy(WAVfile.fileName, "BABYEL~1.WAV"); /*This is where we will choose the file*/
	selectSong();
	initializeAudioFile(&FatFsFile,&WAVfile);

	runStates = idleState;
}

void idleState(void)
{
	if(oneshotBTNs & S1_Pin){
		runStates = previousSongState;
	}
	else if(oneshotBTNs & S2_Pin){
		runStates = nextSongState;
	}
	else if(oneshotBTNs & S3_Pin){
		runStates = playSongState;
		bc127SendCommand(PLAY);
		startStream();
		printf("Playing\r\n");
	}
}

void previousSongState(void)
{
	previousSong();
}

void nextSongState(void)
{
	nextSong();
}

void playSongState(void)
{
	streamAudioFile(&FatFsFile,&WAVfile);

	if(oneshotBTNs){
		if(oneshotBTNs & S1_Pin){
			runStates = previousSongState;
			bc127SendCommand(PAUSE);
			stopStream();
		}
		else if(oneshotBTNs & S2_Pin){
			runStates = nextSongState;
			bc127SendCommand(PAUSE);
			stopStream();
		}
		else if(oneshotBTNs & S3_Pin){
			runStates = pauseSongState;
			bc127SendCommand(PAUSE);
			pauseStream();
		}
	}
}

void pauseSongState(void)
{
	if(oneshotBTNs & S1_Pin){
		runStates = previousSongState;
	}
	else if(oneshotBTNs & S2_Pin){
		runStates = nextSongState;
	}
	else if(oneshotBTNs & S3_Pin){
		runStates = playSongState;
		bc127SendCommand(PLAY);
		resumeStream();
	}
}
