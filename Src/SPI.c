/*
 * SPI.c
 *
 *  Created on: Jun 15, 2019
 *      Author: Matt
 */

#include "SPI.h"


void spiCS(GPIO_PinState PinState)
{
	HAL_GPIO_WritePin(SD_CS_GPIO_Port,SD_CS_Pin,PinState);
}

/**
 * @brief  Sends a command over the SPI bus
 * @param  command: Command index
 * @param  argument: 32-bit argument
 * @retval : R1 response (0x80 =Failed to send)
 */
uint8_t spiSendCommand(uint8_t command, uint32_t argument)
{
	//CMD structure is seen here http://elm-chan.org/docs/mmc/mmc_e.html
	//0b01|CMD (6 bit)|Argument(32 bit)|CRC(7 bit)|1

	//response will either be R1 (1 byte) or R3/R7 (R1 followed by 32bit data)
	uint8_t response = 0x80; //R1 response starts with 0 so we make it start with 1 to tell if we got real resposne

    //split message into bytes
	uint8_t message[] = {command | 0x40, (uint8_t) (argument >> 24), (uint8_t) (argument >> 16),
			(uint8_t) (argument >> 8), (uint8_t) (argument), 0x1};


	//ACMD means CMD55 -> desired command, so check for that and then send cmd 55 before desired command
	if (command & 0x80){
	    command &= 0x7F;
	    message[0] = command;
	    response = spiSendCommand(CMD55,0);
//	    printf("CMD55 Sent\r\n");
	}

    //CRC is needed for commands while in idle state. After that the CRC will go off in SPI mode
	//CRC7 is used [X7+X3+1]
	//hardcoded CRC values
	switch (command) {
	case CMD0:
		message[5] = 0x95;
		break;

	case CMD8:
		message[5] = 0x87;
		break;

	case CMD55:
		message[5] = 0x65;
		break;

	case (ACMD41 & 0x7F): //0x7F strips out 0x80 from ACMD assignment
		message[5] = 0x77;
		break;
	};


    /* Select the card and wait for ready except to stop multiple block read */
	if (command != CMD12 && command != CMD0) {
	    SPI_Deselect();
	    if (!SPI_Select()){
	    	return 0xFF;
	    	//printf("Here?");
	    }
	}

    /* In some cards, the SPI_Select() will fail after a power-on-reset because
    * the card doesn't answer until a CMD0 is received. So, we simply assert the
    * SS and go on. */
    if (command == CMD0) {
    HAL_GPIO_WritePin(SD_CS_GPIO_Port, SD_CS_Pin, GPIO_PIN_RESET);
//    printf("CS Low for CMD0\r\n");
    }


	/* Send the command over the SPI bus */
    if(command == (ACMD41 & 0x7F)){
 //   	printf("ACMD41 to send\r\n");
    	message[0] = 0x69; //change message. The compiler misinterprets ACMD41 (0x80+41) it thinks 41 is hex and not int
    	message[1] = 0x40; //so this corrects the command to be 41 + the high bit per SD protocol -> (0x69)
    	message[2] = 0x00;
    	message[3] = 0x00;
    	message[4] = 0x00;
    	message[5] = 0x77;

/*
    	for(uint8_t i=0;i < 6; i++){
    		for(uint8_t j=0; j < 8; j++){
    			if(((message[i])<<j) & (0x80)){
    				printf("1");
    			}else{
    				printf("0");
    			}
    		}
    	}
    	printf("\r\n");
*/
    }
/*
    if(command == CMD17){
    	//message[0] = 0xFF;
    	//HAL_SPI_Transmit(&hspi1,message[0],1,_FS_TIMEOUT); //send some dummy data to clear
    	//message[0] = 0x51;
    	//message[5] = 0x00;
    	printf("CMD17: ");
    	for(uint8_t i=0;i < 6; i++){
    	    		for(uint8_t j=0; j < 8; j++){
    	    			if(((message[i])<<j) & (0x80)){
    	    				printf("1");
    	    			}else{
    	    				printf("0");
    	    			}
    	    		}
    	    	}
    	    	printf("\r\n");
    }
*/
	if(HAL_SPI_Transmit(&hspi1, message, 6, _FS_TIMEOUT) != HAL_OK){
		return response; //returns bad response if not OK
		//printf("this is bad\r\n");
	}

	if (command == CMD12) {
	    message[0] = 0xFF;
	    if(HAL_SPI_Transmit(&hspi1, message, 1, _FS_TIMEOUT) != HAL_OK){ /* Discard following one byte when CMD12 */
	    	return response;
	    	//printf("CMD12\r\n");
	    }
	}

	/* Receive command response waiting 10 bytes MAX */
	 for (uint8_t i = 0; i < 10; i++) {
	   message[0] = 0xFF;
	   if(HAL_SPI_TransmitReceive(&hspi1, message, message + 1, 1, _FS_TIMEOUT)){
		return response;
		//printf("x\r\n");
	   }

	   if (!(message[1] & 0x80)) { //wait for valid response since first bit is 0 in R1 response)
		 response = message[1];
		 //printf("y\r\n");
		 break;
	   }
   }
/*
	 for(uint8_t i = 0; i < 32; i++){
	 	  //printf((char*)i);
	 	  if((0x80000000 & (response<<i))){
	 		  //tempResult = "1\r\n";
	 		  printf("1");
	 	  }else{
	 		  //tempResult = "0\r\n";
	 		  printf("0");
	 	  }
	 }
	 	 printf("\r\n");
*/
    return response;
}

/**
 * @brief  Deselect the SD card
 * @retval : None
 */
void SPI_Deselect (void)
{
  uint8_t DUMMY_CLK = 0xFF;

  HAL_GPIO_WritePin(SD_CS_GPIO_Port, SD_CS_Pin, GPIO_PIN_SET);
  HAL_SPI_Transmit(&hspi1, &DUMMY_CLK, 1, _FS_TIMEOUT);
}

int SPI_Select (void)
{
	uint8_t DUMMY_CLK = 0xFF;

  HAL_GPIO_WritePin(SD_CS_GPIO_Port, SD_CS_Pin, GPIO_PIN_RESET);
  if(HAL_SPI_Transmit(&hspi1, &DUMMY_CLK, 1, _FS_TIMEOUT) != HAL_OK)
    return 0;

  if (SPI_WaitReady(500)) return 1;  /* Wait for card ready */

  SPI_Deselect(); //--------------------------------------------------------No idea why this was here. Try uncommenting if it doesnt work

  return 0; /* Timeout */
}

/**
 * @brief  Wait for card ready
 * @param  wt: Timeout in ms
 * @retval : 1:Read, 0:Timeout
 */
int SPI_WaitReady (uint32_t wt)
{
  uint8_t DUMMY_CLK[] = {0xFF, 0x0};
  uint32_t tick = HAL_GetTick();

  do {
    if(HAL_SPI_TransmitReceive(&hspi1, DUMMY_CLK, DUMMY_CLK+1, 1, _FS_TIMEOUT) != HAL_OK){
    	return 0;
    }
    /* Wait for card goes ready or timeout */
  } while ((DUMMY_CLK[1] != 0xFF) && ((HAL_GetTick() - tick) < wt ));

  if (DUMMY_CLK[1] == 0xFF){
	  return 1;
  }else{
	  return 0;
  }
  return 0;
}

int spiReadDatablock(uint8_t *buff,uint32_t btr)
{
	uint8_t token = 0xFF;
	uint32_t tick = HAL_GetTick();
	//printf("we read datablock now\r\n");

	do{
		if(HAL_SPI_TransmitReceive(&hspi1,&token,&token,1,_FS_TIMEOUT) != HAL_OK){
			//printf("read data 1\r\n");
			return 0;
		}
	}while((token == 0xFF) && (HAL_GetTick() - tick < 200L));

	if(token != 0xFE){
		//printf("read data 2\r\n");
		return 0;
	}

	//must keep MOSI line high during transfer
	memset(buff,0xFF,btr);
	//store trailing data to buffer
	//while(HAL_SPI_GetState(&hspi1) != HAL_SPI_STATE_READY){printf("waiting for SPI\r\n");}
	//printf("yyyyy\r\n");
	HAL_SPI_Receive_DMA(&hspi1,buff,btr);
	//HAL_Delay(100);
	//printf("1Res is: %i\r\n",res);
/*
	if(res != HAL_OK){
		//printf("read data 3\r\n");
		printf("Res is: %i\r\n",res);
		return 0;
	}

	for(uint16_t i=0; i < 512; i++){
		temp = buff[i];
		printf("%i %c\r\n",i,temp);
	}
*/
	while(HAL_SPI_GetState(&hspi1) == HAL_SPI_STATE_BUSY_RX){} //wait for end of transfer

	//discard CRC
	token = 0xFF;
	for(uint8_t i = 0; i < 2; i++){
		if(HAL_SPI_Transmit(&hspi1,&token,1,_FS_TIMEOUT) != HAL_OK){
			//printf("read data 4\r\n");
			return 0;
		}
	}

	return 1; //success
}

void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi1)
{
#ifdef DEBUG
	printf("HAL_SPI_ErrorCallback\r\n");
	int res;
	res = HAL_SPI_GetError(hspi1);
	printf("Error res is: %i\r\n",res);
#endif
}

void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi)
{
#ifdef DEBUG
	//printf("RX cplt\r\n");
#endif
}



