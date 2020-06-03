/*
 * BC127.c
 *
 *  Created on: Jul 28, 2019
 *      Author: Matt
 *
 *      Contains functions for BC127 module
 */

#include "BC127.h"
#include "stm32l0xx_hal.h"
#include "globals.h"
#include <string.h>

static char bc127Data[100];
static uint8_t bcDataFlag;

void bc127SendCommand(bc127Device_t *bc127, enum bc127Command_e command)
{
	switch(command){
		case BLANK:
			HAL_UART_Transmit(&huart2,(uint8_t*)"XX\r",3,1000);
			break;
		case PLAY:
			HAL_UART_Transmit(&huart2,(uint8_t*)"MUSIC 10 PLAY\r",14,1000);
			break;
		case PAUSE:
			HAL_UART_Transmit(&huart2,(uint8_t*)"MUSIC 10 PAUSE\r",15,1000);
			break;
		case PAIR:
			HAL_UART_Transmit(&huart2,(uint8_t*)"OPEN ",5,1000);
			HAL_UART_Transmit(&huart2,(uint8_t*)bc127->bluetoothAddress,12,1000);
			HAL_UART_Transmit(&huart2,(uint8_t*)" A2DP\r",6,1000);
			break;
		case PAIR_MODE:
			HAL_UART_Transmit(&huart2,(uint8_t*)"INQUIRY\r",8,1000);
			break;
		case RESET_BC127:
			HAL_UART_Transmit(&huart2,(uint8_t*)"RESET\r",6,1000);
			break;
	}
}

void bc127Read(bc127Device_t *bc127)
{

	if(bcDataFlag == DATA){;
		/*Parse data from input*/
		if(strncmp(bc127Data,"CHARGE_IN_PROGRESS",18) == 0){
			bc127->notification = CHARGING;
			bc127->batteryStatus = CHARGING;
		}
		else if(strncmp(bc127Data,"CHARGE_COMPLETE",15) == 0){
			bc127->notification = CHARGED_FULL;
			bc127->batteryStatus = CHARGED_FULL;
		}
		else if(strncmp(bc127Data,"CHARGER_DISABLED",16) == 0){
			bc127->notification = NOT_CHARGING;
			bc127->batteryStatus = NOT_CHARGING;
		}
		else if(strncmp(bc127Data,"OPEN_OK",7) == 0){
			bc127->notification = A2DP_OPEN;
			bc127->connectionStatus = CONNECTED;
		}
		else if(strncmp(bc127Data,"CLOSE_OK",8) == 0){
			bc127->notification = A2DP_CLOSED;
			bc127->connectionStatus = NOT_CONNECTED;
		}
		else if(strncmp(bc127Data,"PAIR_ERROR",10) == 0){
			bc127->notification = PAIR_ERROR;
		}
		else if(strncmp(bc127Data,"PAIR_OK",7) == 0){
			bc127->notification = PAIR_OK;
		}
		else if(strncmp(bc127Data,"INQUIRY",7) == 0){
			memcpy(bc127->bluetoothAddress,bc127Data + 8,12);
			printf("string: %s\r\n",bc127->bluetoothAddress);
		}
		else if(strncmp(bc127Data,"INQU_OK",7) == 0){
			bc127->notification = INQUIRY_OK;
		}
		else if(strncmp(bc127Data,"A2DP_STREAM_START",17) == 0){
			bc127->playStatus = PLAYING;
		}
		else if(strncmp(bc127Data,"A2DP_STREAM_SUSPEND",19) == 0){
			bc127->playStatus = NOT_PLAYING;
		}
		else if(strncmp(bc127Data,"Sierra",6) == 0){
			bc127->connectionStatus = READY;
		}
		memset(bc127Data, 0, 100);
		bcDataFlag = NO_DATA;
	}
}

void bc127UartHandler(void)
{
	if(usart2data){
		if(bc127DataBuff[bc127DataBuffCt - 1] == 0x0D){
			for(uint8_t i=0;i <= (bc127DataBuffCt - 1);i++){
#ifdef DEBUG
				printf("%c",bc127DataBuff[i]);
#endif
				bc127Data[i] = bc127DataBuff[i]; //sloppy
				bc127DataBuff[i] = 0;
			}
			bcDataFlag = DATA;
			bc127DataBuffCt = 0;
#ifdef DEBUG
			printf("\n");
#endif
		}
	}
}
