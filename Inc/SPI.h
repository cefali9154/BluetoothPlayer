/*
 * SPI.h
 *
 *  Created on: Jun 15, 2019
 *      Author: Matt
 *
 *      SPI functions FOR USE WITH SD CARD ONLY live here (SPI is only for SD card in this project)
 */

#ifndef SPI_H_
#define SPI_H_

#include"stm32l0xx_hal.h"
#include "ErrorHandler.h"
#include "gpio.h"
#include <stdint.h>
#include "ffconf.h"
#include "globals.h"

/* SD commands */
#define CMD0  (0)     /* GO_IDLE_STATE */
#define CMD1  (1)     /* SEND_OP_COND (MMC) */
#define ACMD41  (0x80+41) /* SEND_OP_COND (SDC) */
#define CMD8  (8)     /* SEND_IF_COND */
#define CMD9  (9)     /* SEND_CSD */
#define CMD10 (10)    /* SEND_CID */
#define CMD12 (12)    /* STOP_TRANSMISSION */
#define CMD13 (13)    /* STOP_TRANSMISSION */
#define ACMD13  (0x80+13) /* SD_STATUS (SDC) */
#define CMD16 (16)    /* SET_BLOCKLEN */
#define CMD17 (17)    /* READ_SINGLE_BLOCK */
#define CMD18 (18)    /* READ_MULTIPLE_BLOCK */
#define CMD23 (23)    /* SET_BLOCK_COUNT (MMC) */
#define ACMD23  (0x80+23) /* SET_WR_BLK_ERASE_COUNT (SDC) */
#define CMD24 (24)    /* WRITE_BLOCK */
#define CMD25 (25)    /* WRITE_MULTIPLE_BLOCK */
#define CMD32 (32)    /* ERASE_ER_BLK_START */
#define CMD33 (33)    /* ERASE_ER_BLK_END */
#define CMD38 (38)    /* ERASE */
#define CMD55 (55)    /* APP_CMD */
#define CMD58 (58)    /* READ_OCR */

#define CT_MMC    0x01    /* MMC ver 3 */
#define CT_SD1    0x02    /* SD ver 1 */
#define CT_SD2    0x04    /* SD ver 2 */
#define CT_SDC    (CT_SD1|CT_SD2) /* SD */
#define CT_BLOCK 0x08 /* Block addressing */

//Func prototypes
void spiInit(void);
void spiCS(GPIO_PinState PinState);
uint8_t spiSendCommand(uint8_t command, uint32_t argument);
void SPI_Deselect (void);
int SPI_Select (void);
int SPI_WaitReady (uint32_t wt);
int spiReadDatablock(uint8_t *buff,uint32_t btr);

#endif /* SPI_H_ */
