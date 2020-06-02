/* USER CODE BEGIN Header */
/**
 ******************************************************************************
  * @file    user_diskio.c
  * @brief   This file includes a diskio driver skeleton to be completed by the user.
  ******************************************************************************
  * This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * Copyright (c) 2019 STMicroelectronics International N.V. 
  * All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this 
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under 
  *    this license is void and will automatically terminate your rights under 
  *    this license. 
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

    //Portions of code taken from https://github.com/cnoviello/mastering-stm32/blob/master/nucleo-l476RG/Middlewares/FatFs/src/drivers/sd_diskio_spi.c
 /* USER CODE END Header */

#ifdef USE_OBSOLETE_USER_CODE_SECTION_0
/* 
 * Warning: the user section 0 is no more in use (starting from CubeMx version 4.16.0)
 * To be suppressed in the future. 
 * Kept to ensure backward compatibility with previous CubeMx versions when 
 * migrating projects. 
 * User code previously added there should be copied in the new user sections before 
 * the section contents can be deleted.
 */
/* USER CODE BEGIN 0 */
/* USER CODE END 0 */
#endif

/* USER CODE BEGIN DECL */

/* Includes ------------------------------------------------------------------*/
#include "user_diskio.h"
/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/


/* Private variables ---------------------------------------------------------*/
/* Disk status */
static volatile DSTATUS Stat = STA_NOINIT;
static volatile BYTE CardType = 0; /* Card type flags */

/* USER CODE END DECL */

/* Private function prototypes -----------------------------------------------*/
DSTATUS USER_initialize (BYTE pdrv);
DSTATUS USER_status (BYTE pdrv);
DRESULT USER_read (BYTE pdrv, BYTE *buff, DWORD sector, UINT count);
#if _USE_WRITE == 1
  DRESULT USER_write (BYTE pdrv, const BYTE *buff, DWORD sector, UINT count);  
#endif /* _USE_WRITE == 1 */
#if _USE_IOCTL == 1
  DRESULT USER_ioctl (BYTE pdrv, BYTE cmd, void *buff);
#endif /* _USE_IOCTL == 1 */

Diskio_drvTypeDef  USER_Driver =
{
  USER_initialize,
  USER_status,
  USER_read, 
#if  _USE_WRITE
  USER_write,
#endif  /* _USE_WRITE == 1 */  
#if  _USE_IOCTL == 1
  USER_ioctl,
#endif /* _USE_IOCTL == 1 */
};

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Initializes a Drive
  * @param  pdrv: Physical drive number (0..)
  * @retval DSTATUS: Operation status
  */
DSTATUS USER_initialize (
	BYTE pdrv           /* Physical drive number to identify the drive */
)
{
  /* USER CODE BEGIN INIT */
    Stat = STA_NOINIT;
    uint8_t attempts;
    uint8_t dummyData[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
    uint8_t rxData[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };


    uint32_t tick;
    uint8_t cmd;

    //printf("In user FATFS\r\n");

    if (pdrv > 0){
    	return STA_NOINIT; /* This implementation supports only drive 0 */
    }

    //This code will attempt to initialize the SD card and put into a generic read write mode.
    //succesful init will result the flag being cleared. (Stat = 0x00?)

    //steps: (from http://elm-chan.org/docs/mmc/mmc_e.html)

    attempts = 0;

    //DI and CS high, apply 74 or more SCLK pulses
    //spiCS(GPIO_PIN_SET);
    if (HAL_SPI_Transmit(&hspi1,dummyData,10,_FS_TIMEOUT) == HAL_OK){
    	//printf("74 SCK pulses\r\n");
    	while (spiSendCommand(CMD0,0) != 0x1 && (attempts++) < 5){
    		SPI_Deselect();
    		HAL_Delay(10);

    	}

    	if (attempts < 5){
    		//printf("attempts < 5\r\n");
    		if(spiSendCommand(CMD8,0x1AA) == 0x1){
    			//look for 32 bit response
    			if(HAL_SPI_TransmitReceive(&hspi1,dummyData,rxData,4,_FS_TIMEOUT) == HAL_OK){
    				//printf("32 bit response\r\n");
    				if(rxData[2] == 0x1 && rxData[3] == 0xAA){
    					//printf("rxData[2] == 0x1 && rxData[3] == 0xAA\r\n");
    					tick = HAL_GetTick();
    					//printf("here1\r\n");
    					SPI_Select();
    					HAL_Delay(100);
    					SPI_Deselect();
    					while((HAL_GetTick() - tick) <= _FS_TIMEOUT && spiSendCommand(ACMD41,0x40000000)){}
    					//printf("here2\r\n");
    					//check CCS bit
    					SPI_Select();
    					HAL_Delay(100);
    					SPI_Deselect();
    					if((HAL_GetTick() - tick) < _FS_TIMEOUT && spiSendCommand(CMD58,0) == 0 ){
    						//printf("_FS_TIMEOUT && spiSendCommand(CMD58,0) == 0 \r\n");
    						if(HAL_SPI_TransmitReceive(&hspi1,dummyData,rxData,4,_FS_TIMEOUT) == HAL_OK){
    							CardType = (rxData[0] & 0x40 ? CT_SD2 | CT_BLOCK : CT_SD2); //if CCS in OCR then set cardtype to block addressing vice byte
    							//printf("CardType = (rxData[0] & 0x40 ? CT_SD2 | CT_BLOCK : CT_SD2\r\n");
    						}
    					}
    					//printf("here3\r\n");
    				}
    			}
    		}else{//not SDV2
    			//printf("Here 4\r\n");
    			if(spiSendCommand(ACMD41,0) <= 1){//SDv1 or MMC?
    				//printf("spiSendCommand(ACMD41,0) <= 1\r\n");
    				CardType = CT_SD1;
    				cmd = ACMD41;
    			}else{
    				//printf("CardType = CT_MMC;\r\n");
    				CardType = CT_MMC;
    				cmd = CMD1;
    			}
    			tick = HAL_GetTick();
    			while((HAL_GetTick() - tick <= _FS_TIMEOUT) && (spiSendCommand(cmd,0)));
    			if(!((HAL_GetTick() - tick) <= _FS_TIMEOUT) || spiSendCommand(CMD16,512) != 0) //set block length to 512
    				CardType = 0;
    		}
    	}
    }

    SPI_Deselect();
    //printf("CardType is: %i\r\n",CardType);
    if(CardType != 0){
    	Stat &= ~STA_NOINIT;
    	//if we have a good card set to high freq
    	hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2; //set to 4MHz

    	HAL_SPI_DeInit(&hspi1);
    	HAL_SPI_Init(&hspi1);

    	//printf("4MHz SPI\r\n");

    	//set blocklen to 512 TODO
    	if(spiSendCommand(CMD16,0x200) == 0){
    		//printf("block size set\r\n");
    	}else{
    		//printf("CMD16 error\r\n");
    	}

    	if(spiSendCommand(CMD13,0) == 0){ //send CMD 13 to get R2 response
    		if(HAL_SPI_TransmitReceive(&hspi1,dummyData,rxData,1,_FS_TIMEOUT) == HAL_OK){
    			//printf("R2 2nd byte is: %i\r\n",rxData[0]);
    		}
    	}

    }else{
    	Stat = STA_NOINIT;
    }
    return Stat;
  /* USER CODE END INIT */
}
 
/**
  * @brief  Gets Disk Status 
  * @param  pdrv: Physical drive number (0..)
  * @retval DSTATUS: Operation status
  */
DSTATUS USER_status (
	BYTE pdrv       /* Physical drive number to identify the drive */
)
{
  /* USER CODE BEGIN STATUS */
	if(pdrv > 0){ //only one drive here
		return STA_NOINIT;
	}
    return Stat;
  /* USER CODE END STATUS */
}

/**
  * @brief  Reads Sector(s) 
  * @param  pdrv: Physical drive number (0..)
  * @param  *buff: Data buffer to store read data
  * @param  sector: Sector address (LBA)
  * @param  count: Number of sectors to read (1..128)
  * @retval DRESULT: Operation result
  */
DRESULT USER_read (
	BYTE pdrv,      /* Physical drive number to identify the drive */
	BYTE *buff,     /* Data buffer to store read data */
	DWORD sector,   /* Sector address in LBA */
	UINT count      /* Number of sectors to read */
)
{
  /* USER CODE BEGIN READ */
	uint8_t res;
	uint8_t res2;
	if(pdrv || !count){
		return RES_PARERR;
		//printf("a\r\n");
	}

	if(Stat & STA_NOINIT){
		return RES_NOTRDY;
		//printf("b\r\n");
	}

	if(!(CardType & CT_BLOCK)){
		sector *= 512;
		//printf("c\r\n");
	}

	if(count == 1){ //single sector
		//printf("d\r\n");
		//printf("Sector is: %i\r\n",(int)sector);

		res = spiSendCommand(CMD17,sector);
		//printf("CMD17 result is: %i\r\n",res);
		//while(HAL_SPI_GetState(&hspi1) != HAL_SPI_STATE_READY){printf("waiting for SPI\r\n");}
		//printf("xxxx\r\n");
		//HAL_Delay(100);
		res2 = spiReadDatablock(buff,512);
		//printf("ReadDatablock result is: %i\r\n",res2);

		if((res == 0) && res2){
		/*
		if((spiSendCommand(CMD17,sector) == 0) && spiReadDatablock(buff,512)){
			count = 0;
			printf("e\r\n");
			}
			*/
			count = 0;
			//printf("e\r\n");
		}
	}else{//multiple sector
		//printf("f\r\n");
		if(spiSendCommand(CMD18,sector) == 0){
			//printf("g\r\n");
			do{
				if(!spiReadDatablock(buff,512)){
					//printf("h\r\n");
					break;
				}
				buff += 512;
			}while(--count);
			//printf("i\r\n");
			spiSendCommand(CMD12,0);
		}

	}

	SPI_Deselect();
	//printf("j\r\n");
	//printf("count is: %i\r\n",count);

    return count ? RES_ERROR : RES_OK;
}

  /* USER CODE END READ */

/**
  * @brief  Writes Sector(s)  
  * @param  pdrv: Physical drive number (0..)
  * @param  *buff: Data to be written
  * @param  sector: Sector address (LBA)
  * @param  count: Number of sectors to write (1..128)
  * @retval DRESULT: Operation result
  */
#if _USE_WRITE == 1
DRESULT USER_write (
	BYTE pdrv,          /* Physical drive nmuber to identify the drive */
	const BYTE *buff,   /* Data to be written */
	DWORD sector,       /* Sector address in LBA */
	UINT count          /* Number of sectors to write */
)
{ 
  /* USER CODE BEGIN WRITE */
  /* USER CODE HERE */
    return RES_OK;
  /* USER CODE END WRITE */
}
#endif /* _USE_WRITE == 1 */

/**
  * @brief  I/O control operation  
  * @param  pdrv: Physical drive number (0..)
  * @param  cmd: Control code
  * @param  *buff: Buffer to send/receive control data
  * @retval DRESULT: Operation result
  */
#if _USE_IOCTL == 1
DRESULT USER_ioctl (
	BYTE pdrv,      /* Physical drive nmuber (0..) */
	BYTE cmd,       /* Control code */
	void *buff      /* Buffer to send/receive control data */
)
{
  /* USER CODE BEGIN IOCTL */
    DRESULT res = RES_ERROR;
    return res;
  /* USER CODE END IOCTL */
}
#endif /* _USE_IOCTL == 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
