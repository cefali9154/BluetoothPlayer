/*
 * userInputs.h
 *
 *  Created on: May 19, 2020
 *      Author: Matt
 */

#ifndef USERINPUTS_H_
#define USERINPUTS_H_

#include <stdint.h>

extern uint16_t oneshotBTNs;
extern uint16_t debouncedBTNs;

/* Prototypes */
void btnOneShot(void);
void btnDebounce(void);
void checkButtonPresses(void);

#endif /* USERINPUTS_H_ */
