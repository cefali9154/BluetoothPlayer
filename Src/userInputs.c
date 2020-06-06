/*
 * userInputs.c
 *
 *  Created on: May 19, 2020
 *      Author: Matt
 */

#include "userInputs.h"
#include "globals.h"
#include "gpio.h"
#include "settings.h"
#include "AudioFile.h"


uint16_t oneshotBTNs;
uint16_t debouncedBTNs;

void btnDebounce(void)
{
	static uint32_t debounceTime;
	static uint16_t lastBTNs;
	uint16_t currBTNs = 0;

	if((debounceTime == 0) || (debounceTime + DEBOUCE_TIME < HAL_GetTick())){

		//currBTNs = 0;
		currBTNs = currBTNs | (GPIOB->IDR & S1_Pin);
		currBTNs = currBTNs | (GPIOB->IDR & S2_Pin);
		currBTNs = currBTNs | (GPIOB->IDR & S3_Pin);
		currBTNs = currBTNs | (GPIOB->IDR & S6_Pin);
		currBTNs = currBTNs | (GPIOB->IDR & S7_Pin);

		debouncedBTNs = currBTNs & lastBTNs;

		lastBTNs = currBTNs;

		debounceTime = HAL_GetTick();
	}
}

void btnOneShot(void)
{
	static uint16_t debouncedBTNs_last;

	oneshotBTNs = debouncedBTNs_last & (~debouncedBTNs); /*oneshot triggers on release*/
	debouncedBTNs_last = debouncedBTNs;
}

void checkButtonPresses(void)
{

	btnDebounce();
	btnOneShot();

	/* check oneshot buttons - should only ever get one at a time */
	if(oneshotBTNs){
		switch(oneshotBTNs){
			case S1_Pin:
				printf("Button 1 Pressed\r\n");
				break;
			case S2_Pin:
				printf("Button 2 Pressed\r\n");
				break;
			case S3_Pin:
				printf("Button 3 Pressed\r\n");
				break;
			case S6_Pin:
				printf("Button 6 Pressed\r\n");
				volumeDown();
				break;
			case S7_Pin:
				printf("Button 7 Pressed\r\n");
				volumeUp();
				break;
		}
	}
}
