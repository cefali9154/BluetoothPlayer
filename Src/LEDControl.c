/*
 * LEDControl.c
 *
 *  Created on: June 24, 2020
 *      Author: Matt
 */

#include "LEDControl.h"
#include "gpio.h"
#include "stm32l0xx_hal.h"
#include "BC127.h"
#include <stdint.h>

void LEDBlinkControl(bc127Device_t *bc127Device){
	static uint32_t lastMsCount1; /*could optimize but not worth it at this point*/
	static uint32_t lastMsCount2;
	/*LED 1 will show connection status*/
	if(bc127Device->connectionStatus == CONNECTED){
		if(HAL_GetTick() < lastMsCount1 + LED_SLOW_HALF_PERIOD_MS){
			HAL_TIM_PWM_Start_IT(&htim2,TIM_CHANNEL_1); //change to DMA maybe?
		}

		if(HAL_GetTick() > lastMsCount1 + LED_SLOW_HALF_PERIOD_MS){
			HAL_TIM_PWM_Stop_IT(&htim2,TIM_CHANNEL_1);
		}

		if(HAL_GetTick() > lastMsCount1 + (LED_SLOW_HALF_PERIOD_MS * 2)){
			lastMsCount1 = HAL_GetTick();
		}
	}

	/*Use LED 2 to show battery for now*/
	if(bc127Device->batteryStatus == CHARGED_FULL){
		HAL_TIM_PWM_Start_IT(&htim2,TIM_CHANNEL_2); //change to DMA maybe? if this causes issues just trigger once instead of every loop
	}
	if(bc127Device->batteryStatus == CHARGING){
		if(HAL_GetTick() < lastMsCount2 + LED_SLOW_HALF_PERIOD_MS){
			HAL_TIM_PWM_Start_IT(&htim2,TIM_CHANNEL_2); //change to DMA maybe?
		}

		if(HAL_GetTick() > lastMsCount2 + LED_SLOW_HALF_PERIOD_MS){
			HAL_TIM_PWM_Stop_IT(&htim2,TIM_CHANNEL_2);
		}

		if(HAL_GetTick() > lastMsCount2 + (LED_SLOW_HALF_PERIOD_MS * 2)){
			lastMsCount2 = HAL_GetTick();
		}
	}

	if(bc127Device->batteryStatus == NOT_CHARGING){
		HAL_TIM_PWM_Stop_IT(&htim2,TIM_CHANNEL_2); //change to DMA maybe?
	}


	/*add check for battery voltage via ADC -- I screwed up and didnt use an ADC pin for VSENSe. Either lift pad
	 * and run a jumper or rely on BC127. TBD
	 */

}
