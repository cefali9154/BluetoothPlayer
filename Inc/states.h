/*
 * states.h
 *
 *  Created on: May 26, 2020
 *      Author: Matt
 */

#ifndef STATES_H_
#define STATES_H_

#include <stdint.h>
#include "AudioFile.h"
#include "BC127.h"

//extern bc127Device_t bc127Device;
typedef void (*Run_States)(WAVaudioFile *WAVfile, bc127Device_t *bc127Device);
extern Run_States runStates;


void initState(WAVaudioFile *WAVfile, bc127Device_t *bc127Device);
void waitForAutoconnState(WAVaudioFile *WAVfile, bc127Device_t *bc127Device);
void pairingModeState(WAVaudioFile *WAVfile, bc127Device_t *bc127Device);
void openA2DPState(WAVaudioFile *WAVfile, bc127Device_t *bc127Device);
void connectedState(WAVaudioFile *WAVfile, bc127Device_t *bc127Device);
void loadFileState(WAVaudioFile *WAVfile, bc127Device_t *bc127Device);
void idleState(WAVaudioFile *WAVfile, bc127Device_t *bc127Device);
void previousSongState(WAVaudioFile *WAVfile, bc127Device_t *bc127Device);
void nextSongState(WAVaudioFile *WAVfile, bc127Device_t *bc127Device);
void playSongState(WAVaudioFile *WAVfile, bc127Device_t *bc127Device);
void pauseSongState(WAVaudioFile *WAVfile, bc127Device_t *bc127Device);


#endif /* STATES_H_ */
