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

extern bc127Device_t bc127Device;
typedef void (*Run_States)(void);
extern Run_States runStates;


void initState(void);
void waitForAutoconnState(void);
void pairingModeState(void);
void openA2DPState(void);
void connectedState(void);
void loadFileState(void);
void idleState(void);
void previousSongState(void);
void nextSongState(void);
void playSongState(void);
void pauseSongState(void);


#endif /* STATES_H_ */
